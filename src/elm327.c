/*
 * Minimal ELM327-compatible command interpreter for ISO 15765-4 CAN.
 * It deliberately implements the documented, commonly used command subset;
 * it is not an emulation of undocumented ELM327 behaviour.
 */
#include "elm327.h"

#include <string.h>

#include "can.h"
#include "usbd_cdc_if.h"

#define ELM_RESPONSE_MTU 64
#define ELM_OUTPUT_MTU   224

typedef struct {
    uint8_t echo;
    uint8_t linefeeds;
    uint8_t spaces;
    uint8_t headers;
    uint8_t protocol;
    uint8_t timeout_4ms;
    uint16_t tx_id;
    uint16_t rx_id;
} elm_settings_t;

static elm_settings_t settings;
static volatile uint8_t waiting;
static volatile uint8_t response_ready;
static volatile uint8_t response_len;
static volatile uint8_t expected_length;
static volatile uint8_t next_sequence;
static volatile uint16_t response_id;
static volatile uint32_t response_deadline;
static uint8_t response[ELM_RESPONSE_MTU];
static char output[ELM_OUTPUT_MTU];
static uint16_t output_len;

static void defaults(void)
{
    settings.echo = 1;
    settings.linefeeds = 1;
    settings.spaces = 1;
    settings.headers = 0;
    settings.protocol = 6;       /* ISO 15765-4 CAN, 11 bit ID, 500 kbit/s */
    settings.timeout_4ms = 0x32; /* 200 ms */
    settings.tx_id = 0x7DF;
    settings.rx_id = 0;
}

static void out_reset(void)
{
    output_len = 0;
}

static void out_char(char value)
{
    if (output_len < sizeof(output) - 1) {
        output[output_len++] = value;
    }
}

static void out_text(const char *value)
{
    while (*value != '\0') {
        out_char(*value++);
    }
}

static void out_eol(void)
{
    out_char('\r');
    if (settings.linefeeds) {
        out_char('\n');
    }
}

static void out_prompt(void)
{
    out_char('>');
}

static void out_hex(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    out_char(hex[value >> 4]);
    out_char(hex[value & 0x0F]);
}

static void out_nibble(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    out_char(hex[value & 0x0F]);
}

static void out_flush(void)
{
    uint16_t position = 0;
    while (position < output_len) {
        uint16_t chunk = output_len - position;
        if (chunk > TX_BUF_SIZE) {
            chunk = TX_BUF_SIZE;
        }
        if (CDC_Transmit_FS((uint8_t *)&output[position], chunk) != USBD_OK) {
            break;
        }
        position += chunk;
    }
}

static void answer(const char *value)
{
    out_text(value);
    out_eol();
    out_prompt();
    out_flush();
}

static int8_t hex_value(uint8_t value)
{
    if (value >= '0' && value <= '9')
        return (int8_t)(value - '0');
    if (value >= 'A' && value <= 'F')
        return (int8_t)(value - 'A' + 10);
    if (value >= 'a' && value <= 'f')
        return (int8_t)(value - 'a' + 10);
    return -1;
}

static uint8_t protocol_is_29bit(void)
{
    return settings.protocol == 7 || settings.protocol == 9 ||
           settings.protocol == 10;
}

static void apply_protocol(void)
{
    can_disable();
    switch (settings.protocol) {
    case 8:
    case 9:
        can_set_bitrate(CAN_BITRATE_250K);
        break;
    case 10:
        can_set_bitrate(CAN_BITRATE_250K);
        break;
    case 11:
        can_set_bitrate(CAN_BITRATE_125K);
        break;
    case 12:
        can_set_bitrate(CAN_BITRATE_50K);
        break;
    case 6:
    case 7:
    case 0: /* Auto: use the common 11/500 CAN default. */
    default:
        can_set_bitrate(CAN_BITRATE_500K);
        break;
    }
}

static void start_request(const uint8_t *request, uint8_t request_len)
{
    CAN_TxHeaderTypeDef header = {0};
    uint8_t frame[8] = {0};

    if (request_len == 0 || request_len > 7 || protocol_is_29bit())
    {
        answer("?");
        return;
    }

    apply_protocol();
    can_enable();

    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.StdId = settings.tx_id;
    header.DLC = 8;
    frame[0] = request_len;
    memcpy(&frame[1], request, request_len);
    if (can_tx(&header, frame) != HAL_OK)
    {
        answer("BUFFER FULL");
        return;
    }

    response_len = 0;
    expected_length = 0;
    response_ready = 0;
    waiting = 1;
    response_deadline = HAL_GetTick() + ((uint32_t)settings.timeout_4ms * 4U);
}

static void print_protocol(void)
{
    switch (settings.protocol)
    {
    case 8:
        out_text("ISO 15765-4 CAN (11 bit ID, 250 kbaud)");
        break;
    case 9:
        out_text("ISO 15765-4 CAN (29 bit ID, 250 kbaud)");
        break;
    case 10:
        out_text("SAE J1939 CAN (29 bit ID, 250 kbaud)");
        break;
    case 11:
        out_text("USER1 CAN (11 bit ID, 125 kbaud)");
        break;
    case 12:
        out_text("USER2 CAN (11 bit ID, 50 kbaud)");
        break;
    case 7:
        out_text("ISO 15765-4 CAN (29 bit ID, 500 kbaud)");
        break;
    case 0:
        out_text("AUTO, ISO 15765-4 CAN (11 bit ID, 500 kbaud)");
        break;
    default:
        out_text("ISO 15765-4 CAN (11 bit ID, 500 kbaud)");
        break;
    }
}

static void at_command(const uint8_t *command, uint8_t len)
{
    if (len == 2)
    {
        answer("OK");
        return;
    }
    if (len == 3 && command[2] == 'Z')
    {
        defaults();
        answer("ELM327 v1.5");
        return;
    }
    if (len == 3 && command[2] == 'I')
    {
        answer("ELM327 v1.5");
        return;
    }
    if (len == 4 && command[2] == '@' && command[3] == '1')
    {
        answer("GiUCAN ELM327");
        return;
    }
    if (len == 3 && command[2] == 'D')
    {
        defaults();
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'B' && command[3] == 'I')
    {
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'E' &&
        (command[3] == '0' || command[3] == '1'))
    {
        settings.echo = command[3] == '1';
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'L' &&
        (command[3] == '0' || command[3] == '1'))
    {
        settings.linefeeds = command[3] == '1';
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'S' &&
        (command[3] == '0' || command[3] == '1'))
    {
        settings.spaces = command[3] == '1';
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'H' &&
        (command[3] == '0' || command[3] == '1'))
    {
        settings.headers = command[3] == '1';
        answer("OK");
        return;
    }
    /* Timing and automatic-formatting controls are accepted; CAN framing is
       always handled by this firmware's ISO-TP implementation. */
    if (len == 5 && command[2] == 'A' && command[3] == 'T' &&
        command[4] >= '0' && command[4] <= '2')
    {
        answer("OK");
        return;
    }
    if (len == 6 && command[2] == 'C' && command[3] == 'A' &&
        command[4] == 'F' && (command[5] == '0' || command[5] == '1'))
    {
        answer("OK");
        return;
    }
    if (len == 6 && command[2] == 'C' && command[3] == 'F' &&
        command[4] == 'C' && (command[5] == '0' || command[5] == '1'))
    {
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'P' && command[3] == 'C')
    {
        waiting = 0;
        response_ready = 0;
        can_disable();
        answer("OK");
        return;
    }
    if (len == 4 && command[2] == 'D' && command[3] == 'P')
    {
        print_protocol();
        out_eol();
        out_prompt();
        out_flush();
        return;
    }
    if (len == 5 && command[2] == 'D' && command[3] == 'P' &&
        command[4] == 'N')
    {
        out_char('A');
        out_nibble(settings.protocol);
        out_eol();
        out_prompt();
        out_flush();
        return;
    }
    if (len == 5 && command[2] == 'S' && command[3] == 'P')
    {
        int8_t value = hex_value(command[4]);
        if (value >= 0 && value <= 12)
        {
            settings.protocol = (uint8_t)value;
            apply_protocol();
            answer("OK");
        }
        else
            answer("?");
        return;
    }
    if (len == 6 && command[2] == 'S' && command[3] == 'T')
    {
        int8_t a = hex_value(command[4]);
        int8_t b = hex_value(command[5]);
        if (a >= 0 && b >= 0)
        {
            settings.timeout_4ms = (uint8_t)((a << 4) | b);
            answer("OK");
        }
        else
            answer("?");
        return;
    }
    if (len == 7 && command[2] == 'S' && command[3] == 'H')
    {
        int8_t a = hex_value(command[4]);
        int8_t b = hex_value(command[5]);
        int8_t c = hex_value(command[6]);
        if (a >= 0 && b >= 0 && c >= 0)
        {
            settings.tx_id = (uint16_t)((a << 8) | (b << 4) | c);
            answer("OK");
        }
        else
            answer("?");
        return;
    }
    if (len == 7 && command[2] == 'C' && command[3] == 'R' &&
        command[4] == 'A')
    {
        answer("?");
        return;
    }
    if (len == 8 && command[2] == 'C' && command[3] == 'R' &&
        command[4] == 'A')
    {
        int8_t a = hex_value(command[5]);
        int8_t b = hex_value(command[6]);
        int8_t c = hex_value(command[7]);
        if (a >= 0 && b >= 0 && c >= 0)
        {
            settings.rx_id = (uint16_t)((a << 8) | (b << 4) | c);
            answer("OK");
        }
        else
            answer("?");
        return;
    }
    answer("?");
}

void elm327_init(void)
{
    defaults();
}

void elm327_command(const uint8_t *line, uint8_t len)
{
    uint8_t command[ELM327_LINE_MTU];
    uint8_t command_len = 0;
    uint8_t request[7];
    uint8_t request_len = 0;

    out_reset();
    if (settings.echo) {
        for (uint8_t i = 0; i < len; i++) out_char((char)line[i]);
        out_eol();
    }
    for (uint8_t i = 0; i < len && command_len < sizeof(command); i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            command[command_len++] = (line[i] >= 'a' && line[i] <= 'z') ? (uint8_t)(line[i] - 32) : line[i];
        }
    }
    if (command_len == 0)
    {
        out_prompt();
        out_flush();
        return;
    }
    if (waiting)
    {
        waiting = 0;
        response_ready = 0;
        answer("STOPPED");
        return;
    }
    if (command_len >= 2 && command[0] == 'A' && command[1] == 'T')
    {
        at_command(command, command_len);
        return;
    }
    if ((command_len & 1U) != 0)
    {
        answer("?");
        return;
    }
    for (uint8_t i = 0; i < command_len; i += 2)
    {
        int8_t high = hex_value(command[i]);
        int8_t low = hex_value(command[i + 1]);
        if (high < 0 || low < 0 || request_len == sizeof(request))
        {
            answer("?");
            return;
        }
        request[request_len++] = (uint8_t)((high << 4) | low);
    }
    /* OBD replies are asynchronous; send the optional echo now rather than
       retaining it in the shared output buffer until a CAN reply arrives. */
    out_flush();
    out_reset();
    start_request(request, request_len);
}

void elm327_on_can_frame(const CAN_RxHeaderTypeDef *header, const uint8_t *data)
{
    uint8_t pci, copy_len;
    if (!waiting || header->IDE != CAN_ID_STD ||
        header->RTR != CAN_RTR_DATA || header->DLC == 0)
        return;
    if (settings.rx_id != 0 ? header->StdId != settings.rx_id :
        (header->StdId < 0x7E8 || header->StdId > 0x7EF))
        return;
    pci = data[0] >> 4;
    if (pci == 0) {
        copy_len = data[0] & 0x0F;
        if (copy_len > header->DLC - 1)
            return;
        memcpy(response, &data[1], copy_len);
        response_len = copy_len;
        response_id = header->StdId;
        response_ready = 1;
        waiting = 0;
    } else if (pci == 1 && header->DLC == 8) {
        CAN_TxHeaderTypeDef fc_header = {0};
        uint8_t fc[8] = {0x30, 0, 0, 0, 0, 0, 0, 0};
        expected_length = (uint8_t)(((data[0] & 0x0F) << 8) | data[1]);
        if (expected_length > ELM_RESPONSE_MTU)
        {
            waiting = 0;
            return;
        }
        memcpy(response, &data[2], 6);
        response_len = 6;
        response_id = header->StdId;
        next_sequence = 1;
        fc_header.IDE = CAN_ID_STD;
        fc_header.RTR = CAN_RTR_DATA;
        fc_header.StdId = header->StdId - 8;
        fc_header.DLC = 8;
        can_tx(&fc_header, fc);
    }
    else if (pci == 2 && expected_length != 0 &&
             (data[0] & 0x0F) == next_sequence)
    {
        copy_len = expected_length - response_len;
        if (copy_len > 7) copy_len = 7;
        memcpy(&response[response_len], &data[1], copy_len);
        response_len += copy_len;
        next_sequence = (next_sequence + 1) & 0x0F;
        if (response_len >= expected_length)
        {
            response_ready = 1;
            waiting = 0;
        }
    }
}

void elm327_process(void)
{
    if (waiting && (int32_t)(HAL_GetTick() - response_deadline) >= 0)
    {
        waiting = 0;
        out_reset(); answer("NO DATA");
    }
    if (response_ready)
    {
        response_ready = 0;
        out_reset();
        if (settings.headers)
        {
            out_nibble((uint8_t)(response_id >> 8));
            out_hex((uint8_t)response_id);
            if (settings.spaces)
                out_char(' ');
        }
        for (uint8_t i = 0; i < response_len; i++)
        {
            if (i != 0 && settings.spaces)
                out_char(' ');
            out_hex(response[i]);
        }
        out_eol();
        out_prompt();
        out_flush();
    }
}

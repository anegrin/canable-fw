# GiUCAN

**Demo video**

[![GiUCAN demo](http://img.youtube.com/vi/BP4tkZTBur8/0.jpg)](https://youtu.be/BP4tkZTBur8 "GiUCAN demo")

## DISCLAIMER

GiUCAN is an experimental project developed solely for educational and research purposes. It is strictly prohibited to use this tool on vehicles operating on public roads or in any manner that may pose a risk to public safety, violate laws or regulations, or cause harm to people or property.

The developer of GiUCAN assumes no liability for any damages, malfunctions, legal issues, or other consequences arising from its use. All responsibility lies with the end user, who assumes full civil, criminal, and legal liability for any misuse.

Do not use this project in real-world vehicle control or autonomous driving applications.

## Rationale

I'm contributing to the [BACCAble](https://github.com/gaucho1978/BACCAble) repository, I'm keeping this one strictly target to my needs; the goal is to use it as a learning playground.
Please check the amazing [BACCAble](https://github.com/gaucho1978/BACCAble) project!

## What you need and how to connect

### Premise

Since MY2018 Giulia and Stelvio are equipped with SGW; if you car has SGW you need a bypass.

### Hardware

GiUCAN has been developed and tested for [FYSETC UCAN](https://github.com/FYSETC/UCAN) so you need:

- 2 FYSETC UCAN boards; from now on we'll refer at them as "BHCAN" and "C1CAN"
- 1 wire to connect BHCAN to C1CAN on CK pin
- 1 OBD2 male connector
- 4 wires to connect OBD2 pins for BH and C1 CanCAN Busses to UCAN boards

### USB powered connection schema

![image](images/schema_usb.png)

- connect CK pin of the 2 boards
- connect C1CAN CAN_H to OBD2 pin 6 
- connect C1CAN CAN_L to OBD2 pin 14
- connect BHCAN CAN_H to OBD2 pin 3 
- connect BHCAN CAN_L to OBD2 pin 11

Boards must be powered with 5V, you can get the power from the USB connector below the AC; if your ETM has a free USB HSD connector you can use that one to keep the power connection hidden

### OBD2 powered connection schema

You need an additional DC-DC step down (12v -> 5v) board.

![image](images/schema_obd2.png)

- connect all pins as for USB
- connect DO pin of the C1 board to the RES pin of the BH board
- connect DC-DC step down input to OBD2 mass (pins 4 and 5) and 12v (pin 16)
- connect DC-DC step down output to USB-C male connectors

When the C1 bust stop sending data (car goes in sleep mode) for `STANDBY_DELAY_MS` milliseconds the BHCAN board will be put in STANDBY mode and the the C1CAN board will enter the standby mode; when the C1 bus will send data again boards will reboot and start processing data as usual.

When in low consumption mode on 12v the 2 boards should draw around 7 mA; considering that most Giulia/Stelvio batteries capacity is 80Ah it would take 3 months to drain 20% of the battery's charge.

## Building

Just run [`build.sh`](https://github.com/anegrin/GiUCAN/blob/main/build.sh) and it will create 3 elf firmware files under `dist` folder:

- GiUCAN_SLCAN.elf: it's basically the original firmware, flash this if you wanna use the board to sniff CAN Bus (for example with [SavvyCAN](https://github.com/collin80/SavvyCAN)); there are plenty of examples online on how to use canable-fw with SavvyCAN
- GiUCAN_BHCAN.elf: firmware for BHCAN board
- GiUCAN_C1CAN.elf: firmware for C1CAN board

By default builds are for Diesel and 7 inch dashboard display

Run `build.sh help` to get help for additional binaries building (i.e. BACCAble compatible ones)

## Flashing

Repository **Stable Diesel 7-inch TFT for UCAN boards** releases can be flashed using [GiUploader](https://github.com/anegrin/GiUploader) on Android devices.

You can use [dfu-util](https://github.com/anegrin/GiUCAN/blob/main/Makefile) or [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html); connect boards via USB in boot mode by short cutting BO and 3V3 pins then flash the firmware you like via DFU (Erasing & Programming -> USB from the dropdown on the left).

## Usage

Once flashed and connected power on your car; after few seconds you'll see the SNS led on the left active and it means that the SNS has been disabled.

To view messages on the dashboard just keep the cruise control RES button pressed for a couple of seconds, keep it pressed again to hide; you can use cruise control speed joystick to move across dashboard items (1 by 1 or 10 by 10 using "hard press").

If ETM need to show something it will stay on the dashboard for a little bit more than a second, then GiUCAN will display the item again.

If a DPF regeneration is about to start and it's not a "forced" one GiUCAN will play a sound notification (seat belts alarm) and display the "DPF status" item on the dashboard (even if you haven't enabled messages by long press of RES button).

## USB behavior

- SLCAN firmware setup board's USB as serial CDC so just connect it to your laptop to sniff CAN Bus messages  
- BHCAN firmware setup board's USB as storage CDC (read-only); currently not used, it will just contain a file named `giucan.ver` with the version of the firmware that initialized the file system
- C1CAN firmware setup board's USB as storage CDC (read-write); it will just contain a file named `giucan.ver` with the version of the firmware that initialized the file system and user can store an optional `settings.ini` (see [Settings](#settings) paragraph)

## Customization

By creating a file named `inc/user_config.h` you can customize almost any feature or behavior of GiUCAN; please do check the content of [inc/config.h](https://github.com/anegrin/GiUCAN/blob/main/inc/config.h) as reference

### Features and behaviors

- `#define DISABLE_EXTERNAL_OSCILLATOR`: UCAN has and external 8Mhz oscillator, GiUCAN use it by default; you can disable it and use internal.
- `#define GIUCAN_VERSION "foo"`: default is `dev` or `commit short hash` but you can provide your own like "foo"
- `#define DISPLAY_INFO_CODE 0x09`: dashboard message icon, default is `0x08` (Center USB); values reference [here](https://github.com/anegrin/GiUCAN/blob/main/inc/config.h)
- `#define DEFAULT_VALUES_REFRESH_MS 1000`: how often to refresh values of the visible dashboard item in milliseconds, default is 0.333s
- `#define VALUES_TIMEOUT_MS 15000`: after how many milliseconds GiUCAN should stop refreshing items (as the car is off), default is 30s
- `#define STANDBY_DELAY_MS 30000`: how many more milliseconds GiUCAN should wait before putting boards in sleep/standby mode (C1CAN/BHCAN) when the car is off, default is 60s
- `#define DASHBOARD_MESSAGE_MAX_LENGTH 18`: suggested value if you have 3.5 inches dashboard; **MUST BE MULTIPLE of 3**, values greater than 27 are not recommended; there are some considerations to make: GiUCAN is refreshing values twice per second and sending 3 chars to the dashboard every `DASHBOARD_FRAME_QUEUE_POLLING_INTERVAL_MS` milliseconds so a full message takes `DASHBOARD_FRAME_QUEUE_POLLING_INTERVAL_MS*DASHBOARD_MESSAGE_MAX_LENGTH/3` milliseconds to be rendered
- `#define DASHBOARD_FRAME_QUEUE_POLLING_INTERVAL_MS 50`: messages are sent do dashboard in frames of 3 characters; this controls the pace, default is 29 milliseconds.
- `#define DISABLE_DASHBOARD_FORCED_REFRESH`: GiUCAN will refresh items only when their values change
- `#define DASHBOARD_FORCED_REFRESH_MS 1000`: if GiUCAN can refresh items even if no values have changed this controls how often to do so, in milliseconds. Default is 1.5s
- `#define DISABLE_DPF_REGEN_NOTIFICATION`: completely disable the DPF regeneration notification feature
- `#define DISABLE_DPF_REGEN_SOUND_NOTIFICATION`: disable the sound notification when DPF regeneration starts
- `#define DISABLE_DPF_REGEN_VISUAL_NOTIFICATION`: disable visual notification (dashboard item display) when DPF regeneration starts
- `#define DPF_REGEN_VISUAL_NOTIFICATION_ITEM`: what dashboard item to show as visual notification, default is `DPF_STATUS_ITEM`
- `#define DISABLE_DASHBOARD`: disable the dashboard feature, not items will be displayed
- `#define RES_LONG_PRESS_DURATION_MS 3000`: how long RES button must be kept pressed to display items on dashboard in milliseconds, default is 2s
- `#define DASHBOARD_PAGE_SIZE 5`: how many items to skip when "hard pressing" speed control joystick; default is 10
- `#define DISABLE_SNS_AUTO_OFF`: disable Start and Stop auto off
- `#define SNS_AUTO_OFF_DELAY_MS 30000`: if GiUCAN can disable Start and Stop this controls after how many milliseconds to do so. default is 20s
- `#define TANK_CAPACITY 53.0f`: fuel-tank capacity; default is 52 (liters)
- `#define SERVICE_INTERVAL 10000.0f`: service interval as distance; default is 20000 (km)
- `#define CAR_IS_ON_MIN_RPM 800`: the minimum value for Revolutions Per Minute for considering the engine on, default is 400; it's used by Start and Stop auto off and by dashboard feature to stop sending messages on BH (if you power off the car and the dashboard items feature was on)

### Customize items

You can customize what items are displayed, how they are rendered and how they are extracted

- `DASHBOARD_ITEMS` defines what items to display, in what order and how they are rendered (printf pattern); every item can display at most 2 values, both optional
- `CONVERTERS` defines how to convert a value from float to any other type before rendering
- `EXTRACTION_FUNCTIONS`: defines how to extract a value from current [state](https://github.com/anegrin/GiUCAN/blob/main/inc/model.h) or from CAN message (8 bytes)
- `EXTRACTORS`: defines how extract first and second value for each item, defines if it needs to send a query to the bus and what function to use
- `VALUES_REFRESH_MS`: defines values extraction rate, by default values are refreshed every `DEFAULT_VALUES_REFRESH_MS`

take a look at [inc/dashboard.h](https://github.com/anegrin/GiUCAN/blob/main/inc/dashboard.h) for reference

#### Patterns

GiUCAN do render messages using `snprintf`, so output is always truncated to `DASHBOARD_MESSAGE_MAX_LENGTH`

#### Converters

A converter is defined as 

`item_type, forV0_return_type, forV0_convert_function_code, forV1_return_type, forV1_convert_function_code`

so it defines how first and second value are converted before passing them to `snsprintf`; they do render to

```c
forV0_return_type item_type_V0Converter(float value)  {return forV0_convert_function_code;}
forV1_return_type item_type_V1Converter(float value)  {return forV1_convert_function_code;}
```

so for example

```c
X(GEAR_ITEM, char, ((unsigned char)value), bool, false)
```

renders to 

```c
char GEAR_ITEM_V0Converter(float value)  {return ((unsigned char)value);}

bool GEAR_ITEM_V1Converter(float value)  {return false;}
```

so you might wanna then use it for an item with a pattern like `"Current gear: %c"` (it's expecting a char, not a float) and no second value is expected.

#### Extraction functions

Yuo can extract values from the [state](https://github.com/anegrin/GiUCAN/blob/main/inc/model.h) or from a CAN response depending on how extractors are defined; a function is defined as

`function_name, code`

and it renders to

```c
float function_name(GlobalState *s, uint8_t *r) { return code; }
```

you can the extract a float from `s` or `r` or both; for example

```c
X(extractTempCommon, ((float)(((A(r) * 256) + B(r))) * 0.02f) - 40.0f)
```

renders to

```c
float extractTempCommon(GlobalState *s, uint8_t *r) { return ((float)(((A(r) * 256) + B(r))) * 0.02f) - 40.0f; }
```

(GiUCAN does provide defines for [A, B, C, D for all frame response types and E, F, G for multiline responses](https://github.com/anegrin/GiUCAN/blob/main/inc/dashboard.h) to facilitate the extraction of bytes from `r`)

also for example

```c
X(extractBoardUptime, (((float)s->board.now) / 60000.0f))                      \
```

renders to

```c
float extractBoardUptime(GlobalState *s, uint8_t *r) { return (((float)s->board.now) / 60000.0f); }
```

#### Extractors

Every item that can display values need extractors for first and second optional value; extractors are defined as 

```
item_type,
hasV0,
forV0_needsQuery,
forV0_query_reqId,
forV0_query_reqData,
forV0_extraction_function
hasV1,
forV1_needsQuery,
forV1_query_reqId,
forV1_query_reqData,
forV1_extraction_function
```

and the renders to [CarValueExtractors](https://github.com/anegrin/GiUCAN/blob/main/inc/dashboard.h); they defines if a value needs a query, what are query request id, request data and reply id and what function to use to extract the value from state or response (see [Extraction functions](#extraction-functions))

for example

```c
X(UPTIME_ITEM, true, true, 0x18DA10F1, 0x03221009, extractCarUptime, true, false, 0, 0, extractBoardUptime)
```

renders to:

```c
static CarValueExtractors UPTIME_ITEM_extractors = {
    .hasV0 = true,
    .forV0 = {
        .needsQuery = true,
        .query = {
            .reqId = 0x18DA10F1,
            .reqData = SWAP_ENDIAN32(0x03221009),
            .replyId = REQ_RES_ID_CONVERSION(0x18DA10F1),
        },
        .extract = extractCarUptime,
    },
    .hasV1 = true,
    .forV1 = {
        .needsQuery = false,
        .query = {
            .reqId = 0,
            .reqData = SWAP_ENDIAN32(0),
            .replyId = REQ_RES_ID_CONVERSION(0)
        },
        .extract = extractBoardUptime,
    }};
```

GiUCAN will use extractors to send queries - if needed - the consume the response and extract first and second float values using extraction functions (`noop_extract` is provided in the codebase for convenience).

### user config examples

- Diesel 3.5 inches display [user_config.h](samples/diesel_small_config.md)
- Gasoline 7 inches display [user_config.h](samples/gasoline_large_config.md)

Take a look at [danardi78/Alfaromeo-Giulia-Stelvio-PIDs](https://github.com/danardi78/Alfaromeo-Giulia-Stelvio-PIDs) for all known PIDs

## Settings

Connect the board with C1CAN firmware to your laptop and it will be shown as an external USB storage labeled `GIUCAN`; create a file named `settings.ini` with this content:

```ini
[dpf]
notifyWhenFinished=true
[dashboard]
favorites=5,13,21
[carousel]
delayMs=10000
intervalMs=3500
loops=3
enabled=true
```

dpf:
  - notifyWhenFinished: plays a sound notification and displays status when DPF regeneration finishes, not just when it starts, default is false

dashboard:
  - favorites: 0-based position of items to use as favorites;  pressing RES button when dashboard is visible will jump to the next favorite item; not mandatory, default is empty

carousel:
  - enabled: enables carousel display of favorites when car starts, default is false
  - loops: how many times to iterate through favorites, default is 3
  - delayMs: after how many milliseconds to display the carousel, default is 10000 (i.e. 10s)
  - intervalMs: how many times to wait between items, default is 3000 (i.e. 3s)

#ifndef INC_ELM327_H_
#define INC_ELM327_H_

#include <stdint.h>
#include "stm32f0xx_hal.h"

/* Maximum CR-terminated command accepted from the USB CDC port. */
#define ELM327_LINE_MTU 64

void elm327_init(void);
void elm327_command(const uint8_t *line, uint8_t len);
void elm327_process(void);
void elm327_on_can_frame(const CAN_RxHeaderTypeDef *header, const uint8_t *data);

#endif /* INC_ELM327_H_ */

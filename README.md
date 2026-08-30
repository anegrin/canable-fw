# GiUCAN Firmware

This repository contains sources for GiUCAN, a minimalistic version of [BACCAble](https://github.com/gaucho1978/BACCAble) for Alfa Romeo Giulia and Stelvio; it's interacting with C1 nad BH bus only and providing this subset of functionalities:

- Smart Start and Stop disabling ✔
- DPF regeneration warnings ✔
- Dashboard info display ✔

It's tested and documented using [FYSETC UCAN](https://github.com/FYSETC/UCAN) **ONLY**; as it's derived from [Canable-fw](https://github.com/normaldotcom/canable-fw) it should work with other STM32F0x based boards with minimal changes.

## DISCLAIMER

GiUCAN is an experimental project developed solely for educational and research purposes. It is strictly prohibited to use this tool on vehicles operating on public roads or in any manner that may pose a risk to public safety, violate laws or regulations, or cause harm to people or property.

The developer of GiUCAN assumes no liability for any damages, malfunctions, legal issues, or other consequences arising from its use. All responsibility lies with the end user, who assumes full civil, criminal, and legal liability for any misuse.

Do not use this project in real-world vehicle control or autonomous driving applications.

## Rationale

I'm contributing to the [BACCAble](https://github.com/gaucho1978/BACCAble) repository, I'm keeping this one strictly target to my needs; the goal is to use it as a learning playground.
Please check the amazing [BACCAble](https://github.com/gaucho1978/BACCAble) project!

## Supported Commands in CANable mode

- `O` - Open channel 
- `C` - Close channel 
- `S0` - Set bitrate to 10k
- `S1` - Set bitrate to 20k
- `S2` - Set bitrate to 50k
- `S3` - Set bitrate to 100k
- `S4` - Set bitrate to 125k
- `S5` - Set bitrate to 250k
- `S6` - Set bitrate to 500k
- `S7` - Set bitrate to 750k
- `S8` - Set bitrate to 1M
- `M0` - Set mode to normal mode (default)
- `M1` - Set mode to silent mode
- `A0` - Disable automatic retransmission 
- `A1` - Enable automatic retransmission (default)
- `TIIIIIIIILDD...` - Transmit data frame (Extended ID) [ID, length, data]
- `tIIILDD...` - Transmit data frame (Standard ID) [ID, length, data]
- `RIIIIIIIIL` - Transmit remote frame (Extended ID) [ID, length]
- `rIIIL` - Transmit remote frame (Standard ID) [ID, length]
- `V` - Returns firmware version and remote path as a string

Note: Channel configuration commands must be sent before opening the channel. The channel must be opened before transmitting frames.

This firmware currently does not provide any ACK/NACK feedback for serial commands.

## ELM327 mode

ELM327 support was developed using Codex Luna.

Build with `CFLAGS="-DELM327" make clean all`, or use `build.sh` to create the
ELM327 binary. This USB CDC variant supports the common ELM327 AT settings and
ISO 15765-4 CAN OBD requests using 11-bit CAN at 500 kbit/s by default. It
supports single-frame requests and single- or multi-frame ISO-TP responses.
`ATSPC` selects the ELM user-2 CAN profile (11-bit, 50 kbit/s), and `ATBI`
is accepted as the initialization-bypass command.
It is ELM327-compatible, not a full clone: legacy K-line protocols, 29-bit CAN
requests, and multi-frame requests are not implemented yet.

## Building, Flashing  and Debugging

Please read [Canable-fw README.md](https://github.com/normaldotcom/canable-fw/blob/master/README.md) as `make` will build SLCAN firmware by default.

Plaese read [GiUCAN GitHub Pages](https://anegrin.github.io/GiUCAN/) to learn about:

- Hardware and connections
- Building for SLCAN, C1CAN and BHCAN (and BACCAble compatible binaries too)
- Flashing using [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)
- Flashing **Stable Diesel 7-inch TFT for UCAN boards** repository releases using [GiUploader](https://github.com/anegrin/GiUploader) on Android devices
- Customizing build for small displays, gasoline etc.
- Configure already built firmware (via `settings.ini`)

## License

Creative Commons Attribution-NonCommercial 4.0 International License

Copyright (c) 2025 Alessandro Negrin

This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. 
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc/4.0/.

You are free to:
- Share: Copy and redistribute the material in any medium or format
- Adapt: Remix, transform, and build upon the material

Under the following terms:
- NonCommercial: You may not use the material for commercial purposes.

No additional restrictions: You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.

See LICENSE.md for licenses about other software this one is derived from.

#!/usr/bin/env bash
set -e

rm -rf dist build
mkdir -p dist build

EXTRA_FLAG=""
if [[ -f inc/user_config.h ]]; then
    EXTRA_FLAG="-DINCLUDE_USER_CONFIG_H"
fi

build_giucan() {
    CFLAGS="-DSLCAN $EXTRA_FLAG" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_SLCAN.elf

    CFLAGS="-DELM327 $EXTRA_FLAG" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_ELM327.elf

    CFLAGS="-DC1CAN $EXTRA_FLAG" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_C1CAN.elf

    CFLAGS="-DBHCAN $EXTRA_FLAG" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_BHCAN.elf
}

build_baccable() {
    CFLAGS="-DC2CAN $EXTRA_FLAG \
            -DRESET_CMD_PIN=GPIO_PIN_4 \
            -DCAN_S_PIN=GPIO_PIN_5 \
            -DCAN_S_PIN_PORT=GPIOA \
            -DLED_RX_Pin=GPIO_PIN_1 \
            -DDISABLE_EXTERNAL_OSCILLATOR" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_C2CAN_BACCAble.elf

    CFLAGS="-DC1CAN $EXTRA_FLAG \
            -DRESET_CMD_PIN=GPIO_PIN_4 \
            -DCAN_S_PIN=GPIO_PIN_5 \
            -DCAN_S_PIN_PORT=GPIOA \
            -DDISABLE_EXTERNAL_OSCILLATOR" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_C1CAN_BACCAble.elf

    CFLAGS="-DBHCAN $EXTRA_FLAG \
            -DRESET_CMD_PIN=GPIO_PIN_4 \
            -DCAN_S_PIN=GPIO_PIN_5 \
            -DCAN_S_PIN_PORT=GPIOA \
            -DLED_RX_Pin=GPIO_PIN_1 \
            -DDISABLE_EXTERNAL_OSCILLATOR" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_BHCAN_BACCAble.elf

    CFLAGS="-DSLCAN $EXTRA_FLAG \
            -DRESET_CMD_PIN=GPIO_PIN_4 \
            -DCAN_S_PIN=GPIO_PIN_5 \
            -DCAN_S_PIN_PORT=GPIOA \
            -DLED_RX_Pin=GPIO_PIN_1 \
            -DDISABLE_EXTERNAL_OSCILLATOR" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_SLCAN_BACCAble.elf

    CFLAGS="-DELM327 $EXTRA_FLAG \
            -DRESET_CMD_PIN=GPIO_PIN_4 \
            -DCAN_S_PIN=GPIO_PIN_5 \
            -DCAN_S_PIN_PORT=GPIOA \
            -DLED_RX_Pin=GPIO_PIN_1 \
            -DDISABLE_EXTERNAL_OSCILLATOR" make clean all
    mv build/GiUCAN*.elf dist/GiUCAN_ELM327_BACCAble.elf
}

if [[ $# -eq 0 ]]; then
    set -- giucan
fi

for FLAVOR in "$@"; do
    case "$FLAVOR" in
        giucan)
            build_giucan
            ;;
        baccable)
            build_baccable
            ;;
        help)
            echo
            echo "Usage: $0 (help|giucan|baccable)"
            echo
            echo "help: prints this message"
            echo "giucan: builds firmware binaries for UCAN boards (C1, BH, SLCAN, ELM327)"
            echo "baccable: builds firmware binaries for BACCAble boards (C1, C2, BH, SLCAN, ELM327)"
            echo
            echo "you can build both sets of binaries with: $0 giucan baccable "
            echo
            exit 1
            ;;
        *)
            echo "Unknown flavor: $FLAVOR"
            echo "Valid flavors: giucan, baccable"
            exit 1
            ;;
    esac
done

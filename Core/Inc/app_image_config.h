#ifndef APP_IMAGE_CONFIG_H
#define APP_IMAGE_CONFIG_H

#include "boot_state.h"

/*
 * Build-time image identity.
 *
 * Slot A build:
 *   APP_IMAGE_SLOT defaults to BOOT_SLOT_A and the image must be linked at 0x08040000.
 *
 * Slot B / Update FW build:
 *   define APP_IMAGE_SLOT=BOOT_SLOT_B and use the Slot B linker script at 0x08100000.
 */
#ifndef APP_IMAGE_SLOT
#define APP_IMAGE_SLOT BOOT_SLOT_A
#endif

#if (APP_IMAGE_SLOT == BOOT_SLOT_A)
#define APP_IMAGE_BASE_ADDR       ADDR_FLASH_SLOT_A
#define APP_IMAGE_MAX_SIZE        SIZE_FLASH_SLOT_A
#define APP_IMAGE_NAME            "Slot A / Current FW capable"
#elif (APP_IMAGE_SLOT == BOOT_SLOT_B)
#define APP_IMAGE_BASE_ADDR       ADDR_FLASH_SLOT_B
#define APP_IMAGE_MAX_SIZE        SIZE_FLASH_SLOT_B
#define APP_IMAGE_NAME            "Slot B / Update FW capable"
#else
#error "APP_IMAGE_SLOT must be BOOT_SLOT_A or BOOT_SLOT_B"
#endif

#endif /* APP_IMAGE_CONFIG_H */

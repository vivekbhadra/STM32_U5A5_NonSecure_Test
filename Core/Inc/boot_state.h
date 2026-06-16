#ifndef BOOT_STATE_H
#define BOOT_STATE_H

#include <stdint.h>
#include "flash_layout.h"

/*
 * Boot-state metadata shared by:
 *
 * 1. Application/updater firmware
 *    - writes update-pending information after the DFU image is received
 *      and verified.
 *
 * 2. Bootloader
 *    - reads this metadata on reset and decides whether to boot the active
 *      image or trial-boot the pending DFU image.
 *
 * This header intentionally contains definitions only. Step 1 must not change
 * runtime behaviour.
 */

#define BOOT_STATE_RECORD_MAGIC          0x42535455UL  /* 'BSTU' */
#define BOOT_STATE_RECORD_VERSION        1UL

#define BOOT_STATE_ADDR                  ADDR_BOOT_STATE
#define BOOT_ACTIVE_IMAGE_ADDR           ADDR_FLASH_ACTIVE
#define BOOT_PENDING_IMAGE_ADDR          ADDR_FLASH_DFU
#define BOOT_PENDING_IMAGE_MAX_SIZE      SIZE_FLASH_DFU

typedef enum
{
    BOOT_STATE_NORMAL         = 0x11111111UL,
    BOOT_STATE_UPDATE_PENDING = 0x22222222UL,
    BOOT_STATE_TRIAL_BOOT     = 0x33333333UL,
    BOOT_STATE_CONFIRMED      = 0x44444444UL,
    BOOT_STATE_ROLLBACK       = 0x55555555UL
} BootStateValue_t;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t state;
    uint32_t active_version;

    uint32_t pending_version;
    uint32_t pending_size;
    uint32_t pending_crc;
    uint32_t boot_attempt_count;
} BootStateRecord_t;

_Static_assert((sizeof(BootStateRecord_t) % 16U) == 0U,
               "BootStateRecord_t must remain 16-byte aligned in size");

#endif /* BOOT_STATE_H */
#include "boot_state.h"

const BootStateRecord_t *BootState_GetRecord(void)
{
    return (const BootStateRecord_t *)BOOT_STATE_ADDR;
}

int BootState_IsValidRecord(const BootStateRecord_t *record)
{
    if (record == 0)
    {
        return 0;
    }

    if (record->magic != BOOT_STATE_RECORD_MAGIC)
    {
        return 0;
    }

    if (record->version != BOOT_STATE_RECORD_VERSION)
    {
        return 0;
    }

    if (!BootState_IsPendingImageSizeValid(record->pending_size))
    {
        return 0;
    }

    switch ((BootStateValue_t)record->state)
    {
        case BOOT_STATE_NORMAL:
        case BOOT_STATE_UPDATE_PENDING:
        case BOOT_STATE_TRIAL_BOOT:
        case BOOT_STATE_CONFIRMED:
        case BOOT_STATE_ROLLBACK:
            return 1;

        default:
            return 0;
    }
}

int BootState_IsPendingUpdate(const BootStateRecord_t *record)
{
    return BootState_IsValidRecord(record) &&
           (record->state == BOOT_STATE_UPDATE_PENDING);
}

int BootState_IsTrialBoot(const BootStateRecord_t *record)
{
    return BootState_IsValidRecord(record) &&
           (record->state == BOOT_STATE_TRIAL_BOOT);
}

int BootState_IsConfirmed(const BootStateRecord_t *record)
{
    return BootState_IsValidRecord(record) &&
           (record->state == BOOT_STATE_CONFIRMED);
}

int BootState_IsPendingImageSizeValid(uint32_t image_size)
{
    if (image_size == 0U)
    {
        return 1;
    }

    if (image_size > BOOT_PENDING_IMAGE_MAX_SIZE)
    {
        return 0;
    }

    return 1;
}

BootStateRecord_t BootState_MakeUpdatePendingRecord(uint32_t pending_version,
                                                    uint32_t pending_size,
                                                    uint32_t pending_crc)
{
    BootStateRecord_t record = {
        .magic = BOOT_STATE_RECORD_MAGIC,
        .version = BOOT_STATE_RECORD_VERSION,
        .state = BOOT_STATE_UPDATE_PENDING,
        .active_version = 0U,
        .pending_version = pending_version,
        .pending_size = pending_size,
        .pending_crc = pending_crc,
        .boot_attempt_count = 0U
    };

    return record;
}
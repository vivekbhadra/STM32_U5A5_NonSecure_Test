#ifndef FLASH_LAYOUT_H
#define FLASH_LAYOUT_H

#include <stdint.h>

/* STM32U5 Flash Physical Characteristics */
#define FLASH_PHYSICAL_BASE       0x08000000U
#define FLASH_BANK_SIZE           (1024 * 1024)       /* 1 MB per Bank */
#define FLASH_PAGE_SIZE           8192                /* 8 KB per Page */

/* Physical Partition Base Addresses */
#define ADDR_BOOTLOADER           0x08000000U
#define ADDR_BOOT_STATE           0x08020000U
#define ADDR_FLASH_ACTIVE         0x08040000U
#define ADDR_FLASH_DFU            0x08100000U
#define ADDR_SPARE                0x081C0000U

/* Partition Sizes */
#define SIZE_BOOTLOADER           (128 * 1024)        /* 128 KB */
#define SIZE_BOOT_STATE           (128 * 1024)        /* 128 KB */
#define SIZE_FLASH_ACTIVE         (768 * 1024)        /* 768 KB */
#define SIZE_FLASH_DFU            (768 * 1024)        /* 768 KB */
#define SIZE_SPARE                (256 * 1024)        /* 256 KB */

/* Partition Page Allocations */
#define PAGES_BOOTLOADER          (SIZE_BOOTLOADER / FLASH_PAGE_SIZE)   /* 16 Pages */
#define PAGES_BOOT_STATE          (SIZE_BOOT_STATE / FLASH_PAGE_SIZE)   /* 16 Pages */
#define PAGES_FLASH_ACTIVE        (SIZE_FLASH_ACTIVE / FLASH_PAGE_SIZE) /* 96 Pages */
#define PAGES_FLASH_DFU           (SIZE_FLASH_DFU / FLASH_PAGE_SIZE)    /* 96 Pages */

#endif /* FLASH_LAYOUT_H */
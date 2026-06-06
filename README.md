
# U5A5_NonSecure_Test

A minimal, non-secure C bootloader project built in STM32CubeIDE for the **NUCLEO-U5A5ZJ-Q** board (featuring the STM32U5A5ZJT6Q Cortex-M33 MCU). This bootloader is designed to initialize the system cleanly on startup, execute safety checks, and transition control to a relocated non-secure application payload (such as a Rust Embassy firmware or a relocated C HAL application) residing at `0x08040000` [1].

---

## Flash Memory Map

The 4MB on-chip flash memory is divided as follows [4]:

```
0x08000000  FLASH_BOOTLOADER  256K  <-- This C Bootloader lives here
0x08040000  FLASH (ACTIVE)    768K  <-- Relocated Application lives here (Target Jump Address)
0x08100000  FLASH_BL_DFU      768K
0x081C0000  FLASH_SPARE       256K
```

---

## Bootloader Execution Flow

Upon power-on reset, the processor begins execution at `0x08000000` [4]. The bootloader executes the following sequence:

1. **HAL Initialization:** `HAL_Init()` initializes the flash interface, instruction cache, and SysTick [1].
2. **Clock Configuration:** `SystemClock_Config()` configures the clock tree using the low-power Multi-Speed Internal (MSI) oscillator running at 4 MHz to guarantee safe, independent operation [1].
3. **Application Jump:** `JumpToApplication()` is called with `APP_ADDRESS` set to `0x08040000UL` [1].

---

## The Jump Routine (`JumpToApplication`)

The `JumpToApplication` function performs a clean, hardware-safe handover to the user application, avoiding residual interrupts or clock configurations that could cause immediate faults [1]:

*   **Read Target Vectors:** Reads the initial Main Stack Pointer (MSP) and the Reset Vector (PC) directly from the application's vector table located at `0x08040000` [1].
*   **Disable Interrupts:** Calls `__disable_irq()` to prevent any pending bootloader interrupts from firing during the jump transition [1].
*   **Stop SysTick:** Stops and clears the SysTick timer control, reload, and value registers [1].
*   **Clear NVIC:** Disables and clears pending states for all Nest Vector Interrupt Controller (NVIC) channels across 8 registers [1].
*   **De-initialize Clocks & HAL:** Calls `HAL_RCC_DeInit()` and `HAL_DeInit()` to restore the clock tree and peripheral states to their default reset configuration (MSI clock) [1]. This ensures the guest application (e.g., an Embassy-based runtime) can cleanly configure the Phase-Locked Loop (PLL) and bus dividers from a native reset baseline [1].
*   **Relocate Vector Table:** Sets the Vector Table Offset Register (`SCB->VTOR`) to point to the guest application's offset (`0x08040000`) [1].
*   **Set MSP:** Sets the CPU's Main Stack Pointer to the target value read from the application's vector table [1].
*   **Adjust Flash Latency:** Safely scales back the Flash Access Control Register (`FLASH->ACR`) latency wait-states to zero to align with the de-initialized MSI clock [1].
*   **Barriers & Jump:** Executes Data Synchronization Barrier (`__DSB()`) and Instruction Synchronization Barrier (`__ISB()`), and calls the target application's reset handler [1].

---

## Linker Configuration

The linker script **`STM32U5A5ZJTXQ_FLASH.ld`** maps the bootloader executable to the base of the flash [4]:

```ld
/* Memories definition */
MEMORY
{
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 2496K
  SRAM4 (xrw) : ORIGIN = 0x28000000, LENGTH = 16K
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K  /* Restricts bootloader to its allocated 256KB region */
}
```

*Note: In the default linker script, `FLASH` is configured with `LENGTH = 4096K` [4]. In production, it is recommended to restrict this to `256K` to prevent compilation if the bootloader accidentally overflows into the application slot.*

---

## Relocated Application Requirements

To successfully boot and run an application launched by this bootloader, the guest application **must** implement these immediate operations upon entering `main()`:

1.  **Forced Vector Table Relocation:** The guest application must explicitly set `SCB->VTOR = 0x08040000UL` as its first instruction. This prevents subsequent peripheral or SysTick initialization from triggering interrupts that resolve back to the bootloader's de-allocated vector space, which causes a HardFault.
2.  **Global Interrupt Enable:** Since the bootloader disables global interrupts before jumping [1], the guest application must call `__enable_irq()` during startup. Without this, standard delays (like `HAL_Delay()`) relying on SysTick interrupts will hang indefinitely.
3.  **Clock Stability:** Guest applications running on standalone boards (where the ST-LINK MCO external clock bypass is physically disconnected) should rely on the internal **HSI (High-Speed Internal)** clock or properly initialized on-board crystals rather than bypass modes, to avoid clock timeout hangs on a cold boot.

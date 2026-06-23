/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_ADDRESS 0x08040000UL /* Rust app base = FLASH_ACTIVE */
#define ADDR_BOOT_STATE 0x08020000U

/* embassy-boot state magic constants (16-byte patterns) */
static const uint8_t MAGIC_BOOT[16] = {0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
                                       0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
                                       0xD0, 0xD0, 0xD0, 0xD0};
static const uint8_t MAGIC_SWAP[16] = {0xD1, 0xD1, 0xD1, 0xD1, 0xD1, 0xD1,
                                       0xD1, 0xD1, 0xD1, 0xD1, 0xD1, 0xD1,
                                       0xD1, 0xD1, 0xD1, 0xD1};
static const uint8_t MAGIC_REVERT[16] = {0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2,
                                         0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2,
                                         0xD2, 0xD2, 0xD2, 0xD2};

typedef enum {
  BOOT_STATE_BOOT,
  BOOT_STATE_SWAP,
  BOOT_STATE_REVERT,
  BOOT_STATE_UNKNOWN
} BootState_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void JumpToApplication(uint32_t app_address);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  JumpToApplication(APP_ADDRESS);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void JumpToApplication(uint32_t app_address) {
  uint32_t app_sp = *((volatile uint32_t *)(app_address));
  uint32_t app_pc = *((volatile uint32_t *)(app_address + 4U));
  uint32_t app_reset_addr = app_pc & ~1UL;

  /* Basic image validation before touching the handoff state. */
  if ((app_sp < 0x20000000UL) || (app_sp > 0x20020000UL) ||
      ((app_sp & 0x7UL) != 0UL)) {
    Error_Handler();
  }

  if (((app_pc & 0x1UL) == 0UL) || (app_reset_addr < app_address) ||
      (app_reset_addr >= (app_address + (768UL * 1024UL)))) {
    Error_Handler();
  }

  /*
   * Match the working Rust bootloader handoff:
   * - temporarily mask interrupts during cleanup
   * - stop SysTick
   * - disable and clear pending NVIC interrupts
   * - set VTOR to the application vector table
   * - clear CONTROL, BASEPRI, FAULTMASK
   * - load the application's MSP
   * - re-enable normal interrupts
   * - branch directly to the application reset handler
   *
   * Do not call HAL_RCC_DeInit(), HAL_DeInit(), or change FLASH latency here.
   * The working Rust bootloader does not do those operations during handoff.
   */
  __disable_irq();

  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;

  for (uint32_t i = 0U; i < 8U; i++) {
    NVIC->ICER[i] = 0xFFFFFFFFUL;
    NVIC->ICPR[i] = 0xFFFFFFFFUL;
  }

  SCB->VTOR = app_address;

  __asm volatile("msr control, %[zero]\n"
                 "msr basepri, %[zero]\n"
                 "msr faultmask, %[zero]\n"
                 "msr msp, %[app_sp]\n"
                 "dsb\n"
                 "isb\n"
                 "cpsie i\n"
                 "bx %[app_pc]\n"
                 :
                 : [zero] "r"(0UL), [app_sp] "r"(app_sp), [app_pc] "r"(app_pc)
                 : "memory");

  while (1) {
  }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

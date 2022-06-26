/* USER CODE BEGIN Header */

/*---------------------------------------
- WeAct Studio Official Link
- taobao: weactstudio.taobao.com
- aliexpress: weactstudio.aliexpress.com
- github: github.com/WeActTC
- gitee: gitee.com/WeAct-TC
- blog: www.weact-tc.cn
---------------------------------------*/

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "rng.h"
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED_Blink(uint32_t Hdelay,uint32_t Ldelay)
{
	HAL_GPIO_WritePin(PE3_GPIO_Port,PE3_Pin,GPIO_PIN_SET);
	HAL_Delay(Hdelay - 1);
	HAL_GPIO_WritePin(PE3_GPIO_Port,PE3_Pin,GPIO_PIN_RESET);
	HAL_Delay(Ldelay-1);
}

static uint32_t GetSector(uint32_t Address);
static FLASH_EraseInitTypeDef EraseInitStruct;
#define FLASH_USER_START_ADDR   (FLASH_BASE + (FLASH_SECTOR_SIZE * 4)) /* Start @ of user Flash area in Bank1 */
#define FLASH_USER_END_ADDR     (FLASH_BASE + FLASH_BANK_SIZE - 1)     /* End @ of user Flash area in Bank1 */

uint32_t FirstSector = 0, NbOfSectors = 0;
uint32_t Address = 0, SECTORError = 0;
__IO uint32_t MemoryProgramStatus = 0;
__IO uint64_t data64 = 0;
uint32_t Index = 0;
uint64_t FlashWord[2] =
{
  0x0102030405060708, 0x1112131415161718
};

/**
  * @brief  Get the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    sector = (Address - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    sector = (Address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_SECTOR_SIZE;
  }

  return sector;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RNG_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  uint32_t rng;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* -2- Unlock the Flash to enable the flash control register access *************/
		HAL_FLASH_Unlock();

		/* -3- Erase the user Flash area
			(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

		/* Get the 1st sector to erase */
		FirstSector = GetSector(FLASH_USER_START_ADDR);
		/* Get the number of sector to erase from 1st sector*/
		NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;

		/* Fill EraseInit structure*/
		EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.Banks         = FLASH_BANK_1;
		EraseInitStruct.Sector        = FirstSector;
		EraseInitStruct.NbSectors     = NbOfSectors;
		
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
		{
			/*
				Error occurred while sector erase.
				User can add here some code to deal with this error.
				SECTORError will contain the faulty sector and then to know the code error on this sector,
				user can call function 'HAL_FLASH_GetError()'
			*/
			/* Infinite loop */
			while (1)
			{
				LED_Blink(500,500);
			}
		}

		/* -4- Program the user Flash area word by word
			(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

		Address = FLASH_USER_START_ADDR;

		while (Address < FLASH_USER_END_ADDR)
		{
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, Address, ((uint32_t)FlashWord)) == HAL_OK)
			{
				Address = Address + 16; /* increment for the next Flash word*/
			}
			else
			{
				/* Error occurred while writing data in Flash memory.
					 User can add here some code to deal with this error */
				while (1)
				{
					LED_Blink(500,500);
				}
			}
		}

		/* -5- Lock the Flash to disable the flash control register access (recommended
			 to protect the FLASH memory against possible unwanted operation) *********/
		HAL_FLASH_Lock();

		/* -6- Check if the programmed data is OK
				MemoryProgramStatus = 0: data programmed correctly
				MemoryProgramStatus != 0: number of words not programmed correctly ******/
		Address = FLASH_USER_START_ADDR;
		MemoryProgramStatus = 0x0;

		while (Address < FLASH_USER_END_ADDR)
		{
			for(Index = 0; Index<2; Index++)
			{
				data64 = *(uint64_t*)Address;
				__DSB();
				if(data64 != FlashWord[Index])
				{
					MemoryProgramStatus++;
				}
				Address +=8;
			}
		}

		/* -7- Check if there is an issue to program data*/
		if (MemoryProgramStatus == 0)
		{
			/* No error detected. Switch on LED1*/
			LED_Blink(3000,500);
		}
		else
		{
			/* Error detected. Toggle LED3*/
			while (1)
				{
					LED_Blink(500,500);
				}
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    while(1)
    {
      HAL_RNG_GenerateRandomNumber(&hrng,&rng);
      
      HAL_GPIO_TogglePin(PE3_GPIO_Port,PE3_Pin);
      HAL_Delay(rng%600);
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 112;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

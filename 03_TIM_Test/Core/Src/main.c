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
/**
  * 工程：TIM_Test —— 定时器更新中断 + 按键中断消抖
  * 硬件：TIM2 更新中断里翻转 PA9（把 PA9 当方波输出观察）；KEY 接 PA0（上拉、下降沿触发，按下接 GND）；LED 接 PC13（开漏输出，低电平点亮）
  * 时钟：HSE 8 MHz → PLL ×9 → SYSCLK/HCLK 72 MHz；APB1 /2（36 MHz）、APB2 /1（72 MHz）
  * 定时：TIM2 挂 APB1，定时器时钟 = 36 MHz × 2 = 72 MHz
  *       PSC = 7200-1 → 计数频率 72 MHz / 7200 = 10 kHz
  *       ARR = 1000-1 → 每 1000 个计数产生一次更新事件，即每 100 ms 进一次中断
  *       所以 PA9 每 100 ms 翻转一次，输出 5 Hz 方波（周期 200 ms）
  * 验证：PA9 上能看到 5 Hz 方波；每按一次按键，PC13 上的 LED 翻转一次
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t key_tick=0;	// 按键按下的时刻（SysTick 毫秒数），主循环用它判断 20 ms 消抖窗口
uint8_t key_flag=0;		// 中断置 1 通知主循环。注意：这里是“中断写、主循环读”，严格说应加 volatile（EXTI_Test 里加了），否则开优化后可能一直读到旧值
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);	// 启动 TIM2 并使能更新中断；之后每 100 ms 进一次 HAL_TIM_PeriodElapsedCallback
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/* 主循环：处理按键中断留下的标志，做 20 ms 消抖确认 */
		if(key_flag == 1)
		{
			/* 距按下已过 20 ms 才继续（机械抖动一般 <10 ms）；
			   用“现在 - 当时”的写法，SysTick 回绕时依然算得对 */
			if((HAL_GetTick() - key_tick) >= 20)
			{
				key_flag = 0;		// 清标志，同一次按下只处理一次
				/* 再读一次电平：仍为低才是真按下，不是抖动残留或已经松手 */
				if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
				{
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);	// 翻转 LED（PC13 低电平点亮）
				}
			}
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* TIM2 更新中断回调：每 100 ms 进入一次，这里只翻转 PA9，用示波器或逻辑分析仪就能看到定时器在跑 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)	
{
	if(htim->Instance ==TIM2) 
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);	// 每进一次中断翻转一次 → 高、低各 100 ms → 5 Hz 方波
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* 按键外部中断回调：与 EXTI_Test 同样的思路，只置标志、记时刻，判断留给主循环 */
	if(GPIO_Pin == KEY_Pin)		// KEY_Pin 就是 GPIO_PIN_0
	{
		key_tick = HAL_GetTick();	// 记录按下时刻，作为消抖计时起点
		key_flag = 1;				// 通知主循环“有按键事件”
	}
}
/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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

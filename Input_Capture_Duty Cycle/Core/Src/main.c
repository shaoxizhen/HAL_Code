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
  * 工程：Input_Capture_Duty Cycle —— 输入捕获测脉宽与占空比（DAY17）
  * 硬件：TIM3_CH1 从 PA6 输出 1 kHz PWM，用跳线把 PA6 接到 PA0；TIM2_CH1（PA0）负责输入捕获
  * 时钟：HSE 8 MHz → PLL ×9 → 72 MHz；TIM2、TIM3 挂 APB1（36 MHz），定时器时钟 = 36 MHz × 2 = 72 MHz
  * 计数：两个定时器 PSC 都取 72-1 → 计数频率 = 72 MHz / 72 = 1 MHz，1 个计数 = 1 µs
  *       TIM3：ARR = 1000-1 → PWM 频率 = 1 MHz / 1000 = 1 kHz，CCR = 500 → 占空比 50%
  *       TIM2：ARR = 65535，自由计数，只当“时间尺子”
  * 思路：单通道靠切换极性（上升沿 ↔ 下降沿）轮流捕获，三个状态分别得到脉宽和周期
  *       占空比 = 高电平计数 / 周期计数 × 100；频率 = 1 MHz / 周期计数
  * 说明：回调里保留了当时练习用的“填空”注释，作为思路记录
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
volatile uint8_t capture_state = 0;		// 状态机：0 = 等第一次上升沿，1 = 等下降沿，2 = 等第二次上升沿
volatile uint32_t rise_value1 = 0;		// 第一次上升沿捕获到的 CNT（周期起点）
volatile uint32_t fall_value = 0;		// 下降沿捕获到的 CNT（高电平终点）
volatile uint32_t rise_value2 = 0;		// 第二次上升沿捕获到的 CNT（周期终点）
volatile uint32_t high_counts = 0;		// 高电平期间的 CNT 个数，也就是脉宽（单位 µs）
volatile uint32_t period_counts = 0;	// 一个完整周期对应的 CNT 个数（单位 µs）
volatile float duty = 0.0f;				// 占空比，这里存的是百分数
volatile float fre = 0.0f;				// 频率（Hz）
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	// TIM3_CH1（PA6）输出 1 kHz PWM，作为被测信号源
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);	// TIM2_CH1（PA0）开始输入捕获并允许捕获中断
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
/* TIM2 输入捕获回调：PA0 上每来一个边沿进一次；极性由程序在运行时切换，所以上升沿和下降沿都会进来 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)	//输入捕获中断回调函数
{
    if (htim->Instance == TIM2)		// 确认中断来自 TIM2
    {
        uint32_t now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);// 读 CCR1：边沿到来那一刻的 CNT 值

        switch (capture_state)
        {
        case 0:   // 第一次上升沿
            // 填空1：now 应该存到哪个变量？
						rise_value1 = now;
            // 填空2：接下来要捕获下降沿，用下面这行切换极性
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                          TIM_INPUTCHANNELPOLARITY_FALLING);
            // 填空3：capture_state 应该变成几？
						capture_state = 1;
            break;

        case 1:   // 下降沿
            // 填空4：now 应该存到哪个变量？
						fall_value = now;
            // 填空5：high_counts = ? - ?
						high_counts = fall_value - rise_value1;
            // 填空6：接下来要捕获上升沿，极性切回 RISING
						__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                          TIM_INPUTCHANNELPOLARITY_RISING);
            // 填空7：capture_state 应该变成几？
						capture_state = 2;
            break;

        case 2:   // 第二次上升沿
            // 填空8：now 应该存到哪个变量？
						rise_value2 = now;
            // 填空9：period_counts = ? - ?
						period_counts = rise_value2 - rise_value1;
            // 填空10：duty = high_counts 除以 period_counts，再乘 100
						duty = (float)high_counts / period_counts*100;
            // 填空11：freq = 1000000.0f / period_counts
						fre = 1000000.0f / period_counts;
            // 填空12：capture_state 回到几？
						capture_state = 0;
            break;
        }
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

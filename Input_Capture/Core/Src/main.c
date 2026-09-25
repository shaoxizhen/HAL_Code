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
  * 工程：Input_Capture —— 输入捕获综合验收（DAY18）：PWM 输出 → 输入捕获，闭环自测
  * 硬件：TIM2_CH1 从 PA0 输出 PWM，用跳线把 PA0 接到 PA6；TIM3_CH1（PA6）负责输入捕获
  * 时钟：HSE 8 MHz → PLL ×9 → 72 MHz；TIM2、TIM3 都挂 APB1（36 MHz），总线预分频不为 1，
  *       所以两者定时器时钟 = 36 MHz × 2 = 72 MHz
  * 计数：两个定时器 PSC 都取 72-1 → 计数频率 = 72 MHz / 72 = 1 MHz，也就是 1 个计数 = 1 µs
  *       TIM2：ARR = 1000-1 → PWM 频率 = 1 MHz / 1000 = 1 kHz
  *       TIM3：ARR = 65535，自由计数，只当“时间尺子”用，不计周期
  * 思路：同一个通道靠切换极性来轮流捕获（上升沿 ↔ 下降沿），三个状态走完一轮：
  *       ① 第一个上升沿记起点 → ② 下降沿得到高电平计数 → ③ 第二个上升沿得到周期计数
  *       脉宽 = 第 2 次捕获 - 第 1 次捕获；周期 = 第 3 次捕获 - 第 1 次捕获
  * 验证：主循环依次把 CCR 改成 250 / 500 / 750（占空比 25% / 50% / 75%），
  *       捕获侧的 high_counts 应分别是 250 / 500 / 750，period_counts 恒为 1000，频率恒为 1 kHz
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define IC_COUNT_FREQ_HZ 1000000	// 捕获定时器的计数频率：72 MHz / 72 = 1 MHz，即 1 个计数 = 1 µs
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
volatile uint8_t capture_state = 0;		// 捕获状态机：0 = 等第一个上升沿，1 = 等下降沿，2 = 等第二个上升沿
volatile uint32_t rise_value1 = 0;		// 第一次上升沿捕获到的 CNT（周期起点）
volatile uint32_t fall_value = 0;		// 下降沿捕获到的 CNT（高电平终点）
volatile uint32_t rise_value2 = 0;		// 第二次上升沿捕获到的 CNT（周期终点）
volatile uint32_t high_counts = 0;		// 高电平期间的计数个数（就是脉宽，单位 µs）
volatile uint32_t period_counts = 0;	// 一个完整周期的计数个数（单位 µs）
volatile float duty = 0.0f;				// 占空比 = 高电平计数 / 周期计数
volatile float fre = 0.0f;				// 频率 = 计数频率 / 周期计数
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
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);	// 让 TIM2_CH1（PA0）输出 1 kHz PWM，作为被测信号源
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);	// 启动 TIM3_CH1 输入捕获并使能中断：PA6 上每来一个边沿，硬件把当前 CNT 存进 CCR1
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		TIM2->CCR1 = 250;		// 改 PWM 占空比：250/1000 = 25%
		HAL_Delay(2000);		// 留 2 s 让捕获侧跑几轮，方便在 Keil Watch 里读数
		TIM2->CCR1 = 500;		// 50%
		HAL_Delay(2000);
		TIM2->CCR1 = 750;		// 75%
		HAL_Delay(2000);
		
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
/* TIM3 输入捕获中断回调：PA6 上每来一个边沿就进一次，取出当前 CNT 后按状态机推进 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)	// 多个定时器可能共用这个回调，先确认中断来自 TIM3
	{
		uint32_t now_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);	// 读 CCR1：边沿到来那一刻的 CNT 值
		
		switch(capture_state)	// 按状态机决定这次边沿该记到哪一步
		{
			case 0 : // 状态 0：这次是第一个上升沿，作为整个周期的起点
				rise_value1 = now_value;	// 记下起点计数
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);	// 同一个通道改成等下降沿，下一次中断就落在下降沿上
				capture_state = 1;	// 进入状态 1
				break;
			case 1 : // 状态 1：这次是下降沿，高电平到此结束
				fall_value = now_value;	// 记下高电平终点计数
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);	// 再切回等上升沿
				high_counts = fall_value - rise_value1;	// 两点相减 = 高电平期间的计数个数（1 个计数 = 1 µs，所以这就是脉宽）
				capture_state = 2;	// 进入状态 2
				break;
			case 2 : // 状态 2：这次是第二个上升沿，一个完整周期结束
				rise_value2 = now_value;	// 记下周期终点计数
				period_counts = rise_value2 - rise_value1;	// 两个上升沿相减 = 周期计数
				duty = (float)high_counts / period_counts;	// 占空比 = 高电平计数 / 周期计数
				fre = IC_COUNT_FREQ_HZ / (float)period_counts;	// 频率 = 计数频率 / 周期计数（1 MHz / 1000 = 1 kHz）
				capture_state = 0;	// 回到状态 0，准备下一轮
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

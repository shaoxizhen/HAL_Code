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
  * 工程：UART_RX —— UART 接收与指令解析（DAY21）：电脑发 ON / OFF 控制板载 LED
  * 硬件：USART1，TX = PA9、RX = PA10；115200 8N1、收发都打开；LED 接 PC13（开漏输出，低电平点亮）
  * 时钟：HSE 8 MHz → PLL ×9 → 72 MHz；USART1 挂 APB2，时钟 = PCLK2 = 72 MHz（BRR = 625 = 0x271）
  * 思路：主循环里阻塞接收 1 个字节（Timeout = 100 ms），把字节攒进 buf；
  *       收到 \r 或 \n 就认为一条指令结束，补 '\0' 后和 "ON" / "OFF" 比较，再点灯并回显
  * 三个关键点：
  *   1) 回显只在“一条指令结束时”做一次。若每收一个字节就 printf，阻塞发送会拖住主循环，
  *      后面到达的字节来不及读走就被覆盖，触发 ORE 溢出丢失
  *   2) 这套写法用“收一个字节阻塞等一次”换来简单；收发同时繁忙时要靠中断或 DMA 接收（后面才学）
  *   3) 缓冲区必须留一格给 '\0'，写入前要检查下标，越界会踩坏内存里别的变量
  * 验证：发 ON → LED 亮并回显 ON；发 OFF → LED 灭并回显 OFF；发别的 → 回显 UNKNOWN 且 LED 不动
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"		// strcmp：把攒好的字符串和 "ON" / "OFF" 比较
#include "stdio.h"		// printf：用于回显；重定向还要在 Keil 里勾上 Use MicroLIB
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
uint8_t receive_data;		// 接住“这一次”收到的 1 个字节，再决定是存进缓冲区还是当成结束符
HAL_StatusTypeDef ret; 		// 接收函数的返回值：等于 HAL_OK 才算真收到；HAL_TIMEOUT 表示这一轮没数据
char buf[16];				// 指令缓冲区：最多存 15 个字符，最后一格留给字符串结束符 '\0'
uint8_t buf_count;			// 已经攒了几个字符，同时也就是下一个字符要写入的下标（全局变量上电默认是 0）
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/* 阻塞接收 1 个字节，最多等 100 ms：有数据立刻返回，没数据才等满并返回 HAL_TIMEOUT，
		   所以主循环不会被永久卡死 */
		ret = HAL_UART_Receive(&huart1, &receive_data, 1, 100);
		
		if(ret == HAL_OK)	// 只有确实收到字节才往下处理
		{
			if(receive_data == '\r' || receive_data == '\n')	// 行结束符：一条指令到此结束。\r 和 \n 都要认，不同串口工具发的换行不一样
			{
				buf[buf_count] = '\0';	// 在末尾补结束符，这一串字符才算合法的 C 字符串，strcmp / printf 才知道读到哪停
				if(buf_count > 0)		// 空指令（例如连续两个换行）直接跳过，既不打印也不点灯
				{
					if(strcmp(buf, "ON") == 0)	// strcmp 返回 0 才表示两个字符串完全相同
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);	// PC13 输出低 → LED 点亮
						printf("%s\r\n", buf);	// 回显收到的指令，\r\n 让串口助手换行显示
					}
					else if(strcmp(buf, "OFF") == 0)
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);	// PC13 输出高（开漏相当于高阻）→ LED 熄灭
						printf("%s\r\n", buf);
					}
					else
					{
						printf("UNKNOWN");	// 认不出的指令：只提示，不动 LED
					}
				}
				buf_count = 0;	// 收尾：计数器归零，下一条指令从 buf[0] 重新写
			}
			else
			{
				if(buf_count < sizeof(buf)-1)	// 越界保护：16 格最多写 15 个字符，最后一格要留给 '\0'
				{
					buf[buf_count] = receive_data;	// 普通字符：存到当前空位
					buf_count++;					// 下标后移一格
				}
				/* 下标到顶后，多余的字符直接丢掉 —— 宁可少收字符，也不能写到数组外面去 */
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
/* printf 重定向：C 库每输出一个字符就调用一次 fputc，这里把它接到 USART1 上 */
int fputc(int ch, FILE *f)
{
    uint8_t c = (uint8_t)ch;					// 形参是 int，转成本次要发送的那 1 个字节
    HAL_UART_Transmit(&huart1, &c, 1, 100);		// 阻塞发送这个字节；每个字符单独调用一次，所以回显字符之间会有间隙
    return ch;									// 按 C 库约定返回写出去的字符
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

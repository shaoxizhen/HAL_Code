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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MPU6050_ADDR (0x68<<1)  //宏定义MPU6050的地址+写操作
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t data = 0x00;

char oled_buffer[20];

uint8_t accel_data[6] = {0};	//保存加速度数据，一个轴两个字节
uint8_t gyro_data[6] = {0};	//保存角速度数据，一个轴两个字节

int16_t accel_x = 0;	//保存x轴加速度,16 位有符号整数
int16_t accel_y = 0;	//保存y轴加速度
int16_t accel_z = 0;	//保存z轴加速度

int16_t gyro_x = 0;	//保存x轴角速度,16 位有符号整数
int16_t gyro_y = 0;	//保存y轴角速度
int16_t gyro_z = 0;	//保存z轴角速度

float accel_x_g;	//加速度原始值/16384 -> g(量程+-2g)
float accel_y_g;
float accel_z_g;

float gyro_x_dps;	//角速度原始值/131 -> °/s(陀螺仪量程 = ±250 °/s灵敏度 = 131 LSB/(°/s))
float gyro_y_dps;
float gyro_z_dps;

HAL_StatusTypeDef status;	//返回状态
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
	OLED_Clear();

	status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x6B, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);		//向MPU6050的0x6B上的PWR_MGMT_1电源管理寄存器 1写0x00,唤醒MPU6050
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//读取加速度
		status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, I2C_MEMADD_SIZE_8BIT, accel_data, 6, 100);	//从MPU6050的0x3B上的加速度寄存器读取三轴6字节数据保存到accel_data数组里面
	
		accel_x = (int16_t)((accel_data[0] << 8) | accel_data[1]);  //把 6 个 8 位数据还原成 3 个 16 位有符号原始值。
		accel_y = (int16_t)((accel_data[2] << 8) | accel_data[3]);
		accel_z = (int16_t)((accel_data[4] << 8) | accel_data[5]);
	
		accel_x_g = accel_x / 16384.0f;
		accel_y_g = accel_y / 16384.0f;
		accel_z_g = accel_z / 16384.0f;
		
		//读取角速度
		status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x43, I2C_MEMADD_SIZE_8BIT, gyro_data, 6, 100);		//从MPU6050的0x43上的角速度寄存器读取三轴6字节数据保存到gyro_data数组里面
		
		if(status == HAL_OK)
		{
			gyro_x = (int16_t)((gyro_data[0] << 8) | gyro_data[1]);	//把 6 个 8 位数据还原成 3 个 16 位有符号原始值。
			gyro_y = (int16_t)((gyro_data[2] << 8) | gyro_data[3]);
			gyro_z = (int16_t)((gyro_data[4] << 8) | gyro_data[5]);
		
			gyro_x_dps = gyro_x / 131.0f;
			gyro_y_dps = gyro_y / 131.0f;
			gyro_z_dps = gyro_z / 131.0f;
			
			//OLED显示角速度
			sprintf(oled_buffer, "GX: %.2f", gyro_x_dps);
			OLED_ShowString(0, 0, (uint8_t *)oled_buffer, 16);
			sprintf(oled_buffer, "GY: %.2f", gyro_y_dps);
			OLED_ShowString(0, 2, (uint8_t *)oled_buffer, 16);
			sprintf(oled_buffer, "GZ: %.2f", gyro_z_dps);
			OLED_ShowString(0, 4, (uint8_t *)oled_buffer, 16);
		}
		else
		{
			OLED_ShowString(0, 0, (uint8_t *)"I2C ERROR", 16);
		}
		HAL_Delay(50);   // 20Hz 刷新
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

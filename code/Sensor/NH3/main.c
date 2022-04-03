/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "NH3.h"
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
uint8_t flag=0;
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim2);
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
////���ջص����ѽ��յ������ݱ���
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	UNUSED(huart);
//	RxLine++;
//	NH3Buffer[RxLine-1]=RecieveBuffer[0];
////�ж�֡ͷ�Ƿ���ȷ
//	if (RxLine==1)
//	{
//		if(RecieveBuffer[0]==0xFF)
//		{
//			RxLine=1;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==2)
//	{
//		if(RecieveBuffer[0]==0x01)
//		{
//			RxLine=2;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==3)
//	{
//		if(RecieveBuffer[0]==0x07)
//		{
//			RxLine=3;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==4)
//	{
//		if(RecieveBuffer[0]==0x02)  //�ֱ��ʹ̶�Ϊ0x02����0.01
//		{
//			RxLine=4;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==5)
//	{
//		gaozijie=RecieveBuffer[0];
//	}
//	else if(RxLine==6)
//	{
//		dizijie=RecieveBuffer[0];
//		//16����ת��10����,�ֱ���0.01
//		NH3Dec=gaozijie*256+dizijie*1;
//		NH3Dec=NH3Dec*0.01;
//	}
//	else if(RxLine==7)
//	{
//		if(RecieveBuffer[0]==0x00) 
//		{
//			RxLine=7;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==8)
//	{
//		if(RecieveBuffer[0]==0x00)  
//		{
//			RxLine=8;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==9)
//	{
//		if(RecieveBuffer[0]==0x00)  
//		{
//			RxLine=9;
//		}
//		else
//		{
//			RxLine=0;
//		}	
//	}
//	else if(RxLine==10&flag==0)
//	{
////		HAL_UART_Transmit_IT(&huart1, (uint8_t *)NH3Buffer,10);
//		printf("%.2f ppm",NH3Dec);
//		flag=1;
//		RxLine=0;	
//	}
//	else 
//	{
//		flag=1;
//		RxLine=0;	
//	}
//	
//	RecieveBuffer[0]=0;
//	HAL_UART_Receive_IT(&huart1, (uint8_t *)RecieveBuffer, 1);
//}
//��ʱ���жϣ�ÿ1�����һ��
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&htim2))
    {
      NH3_Tx();
			flag=0;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
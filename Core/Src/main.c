/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "./ssd1306/ssd1306.h"
#include "stdio.h"
#include <string.h>
#include <stdio.h>
#include "./ssd1306/ssd1306_tests.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

uint32_t runtime = 6000;
uint32_t sensitivity = 50;

//uint32_t timecount=0;
uint32_t stage = 0;
uint32_t last_stage = 0;
uint32_t start = 0;
uint32_t end = 0;

int settingMode = 0;

int i;
uint32_t ADC_Value[100];
uint32_t ad1, ad2;
uint32_t real_adc1, real_adc2;
uint32_t keep_adc1, keep_adc2;
uint32_t PWM1 = 0, PWM2 = 0;
uint32_t OUTPUT_1_State = 0, OUTPUT_2_State = 0;
uint32_t sysinfo_State = 0, button_State = 0;
uint32_t dir_flag = 0;

int button_flag[5] = { 0 };

char buff[64];

int i2c_working = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void motor_control(int dir, int pwm) {
	if (dir >= 1) {
		OUTPUT_1_State = 1;
		OUTPUT_2_State = 0;
		HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 1);
		HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
		PWM1 = pwm;
		PWM2 = 0;
		user_pwm_setvalue_1(pwm);
		user_pwm_setvalue_2(0);

	} else if (dir <= -1) {
		OUTPUT_1_State = 0;
		OUTPUT_2_State = 1;
		HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
		HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 1);
		PWM1 = 0;
		PWM2 = pwm;
		user_pwm_setvalue_1(0);
		user_pwm_setvalue_2(pwm);
	} else if (dir == 0) {
		OUTPUT_1_State = 0;
		OUTPUT_2_State = 0;
		PWM1 = 0;
		PWM2 = 0;
		HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
		HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
		user_pwm_setvalue_1(0);
		user_pwm_setvalue_2(0);
	}
}
void motor_point(int pwm) {
	HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 1);
	PWM1 = 0;
	PWM2 = pwm;
	HAL_Delay(10);
	HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 1);
	HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
	PWM1 = pwm;
	PWM2 = 0;
	HAL_Delay(10);
}

void clean_button_flag(void) {
	//Usart2DmaPrintf("clena\n");
	for (i = 0; i <= 5; i++) { //clean flag
		button_flag[i] = 0;
		//HAL_Delay(1);
	}
}
int read_ADC() {
	for (i = 0, ad1 = 0, ad2 = 0; i < 10;) {
		ad1 += ADC_Value[i++];
		ad2 += ADC_Value[i++];
		//HAL_Delay(1);
	}

	real_adc1 = ad1 / 5;
	real_adc2 = ad2 / 5;
	sensitivity=real_adc2;
	if (real_adc1 <= real_adc2) {
		return 1;
	} else {
		return 0;
	}
}
//檢知附載
int detection_load(int time, int th) {
	if (time == 0)
		keep_adc1 = real_adc1;
	else {
		if (abs(keep_adc1 - real_adc1) > th) {
			keep_adc1 = real_adc1;
			return 1;
		} else {
			keep_adc2 = real_adc1;
			return 0;
		}
	}
}

void Display(int mode) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(2, 0);
	int line_count=1;
	if (mode == 0) {

		snprintf(buff, sizeof(buff), "%s,%s", __DATE__, __TIME__);
		ssd1306_WriteString(buff, Font_6x8, White);

		ssd1306_SetCursor(2, 8 * line_count++);
		snprintf(buff, sizeof(buff), "steta:%d,%d", sysinfo_State, stage);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "[B0]:%d,KA1:%d", real_adc1, keep_adc1);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "[A7]:%d,KA2:%d", real_adc2, keep_adc2);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "PWM1:%d, GPIO1:%d", PWM1, OUTPUT_1_State);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "PWM2:%d, GPIO2:%d", PWM2, OUTPUT_2_State);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "button[%d]:%d,%d,%d,%d", button_State,
				button_flag[1], button_flag[2], button_flag[3], button_flag[4]);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		//snprintf(buff, sizeof(buff), "time:%d,%d", end, start);
		//ssd1306_SetCursor(2, 8*7);
		//ssd1306_WriteString(buff, Font_6x8, White);
		if (end > start) {
			snprintf(buff, sizeof(buff), "time:%d", end - start);
			ssd1306_SetCursor(2, 8 * 7);
			ssd1306_WriteString(buff, Font_6x8, White);
		}
	}else if(mode ==1){//setting mode
		snprintf(buff, sizeof(buff), "Setting mode.");
		ssd1306_WriteString(buff, Font_6x8, White);

		ssd1306_SetCursor(2, 8 * line_count++);
		snprintf(buff, sizeof(buff), "button[%d]:%d,%d,%d,%d", button_State,
				button_flag[1], button_flag[2], button_flag[3], button_flag[4]);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "[B0]:%d,KA1:%d", real_adc1, keep_adc1);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

	}
	ssd1306_UpdateScreen();

}
void u32tostr(unsigned long dat, char *str) {
	char temp[20];
	unsigned char i = 0, j = 0;
	i = 0;
	while (dat) {
		temp[i] = dat % 10 + 0x30;
		i++;
		dat /= 10;
	}
	j = i;
	for (i = 0; i < j; i++) {
		str[i] = temp[j - i - 1];
	}
	if (!i) {
		str[i++] = '0';
	}
	str[i] = 0;
}
int read_GPIO(int th) {
	int state = 0;
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
		button_flag[1]++;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
		button_flag[2]++;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET) {
		button_flag[3]++;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET) {
		button_flag[4]++;
	}

	if (button_flag[1] >= th)
		state = 1;
	if (button_flag[2] >= th)
		state = state + 2;
	if (button_flag[3] >= th)
		state = state + 4;
	if (button_flag[4] >= th)
		state = state + 8;

	return state;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	/*
	 switch (GPIO_Pin) {
	 case GPIO_PIN_12: // GPIO_PIN_13 is the Blue Button
	 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); //PC13 Led
	 button_flag[1]++;
	 break;
	 case GPIO_PIN_13:
	 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); //PC13 Led
	 button_flag[2]++;
	 break;
	 case GPIO_PIN_14:
	 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); //PC13 Led
	 button_flag[3]++;
	 break;
	 case GPIO_PIN_15:
	 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); //PC13 Led
	 button_flag[4]++;
	 break;
	 }
	 */
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
	//ssd1306_Reset();
	ssd1306_Init();
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_Value, 100);
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	motor_control(0, 0);
	//ssd1306_TestLine();
	//ssd1306_TestAll();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		//系統運行時間
		start = HAL_GetTick();
		if (start < end) {	//時間未結束
			if (dir_flag == 1) {
				if (start < end - 4000) {
					stage = 1;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(1, 1000);
				} else if (start < end - 2000) {
					stage = 2;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(1, 4000);
				} else if (start < end) {
					stage = 3;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(1, 1000);
				}
			} else if (dir_flag == 4) {
				if (start < end - 4000) {
					stage = 1;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(-1, 1000);
				} else if (start < end - 2000) {
					stage = 2;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(-1, 4000);
				} else if (start < end) {
					stage = 3;
					if (stage != last_stage) {
						last_stage = stage;
						detection_load(0, sensitivity);
					}
					if (detection_load(1, sensitivity))
						sysinfo_State = 1;
					motor_control(-1, 1000);
				}
			}
			if (sysinfo_State == 1)
				end = start;	//壓力觸發
		} else {	//時間結束
			stage = 0;
			sysinfo_State = 0;
			detection_load(0, 50);
			motor_control(0, 0);
		}

		//壓力觸動時輸出1
		//sysinfo_State = read_ADC();
		read_ADC();

		//讀取案就狀態
		button_State = read_GPIO(1);
		if (button_State == 1) {
			end = start + runtime;
			dir_flag = 1;
		} else if (button_State == 4) {
			end = start + runtime;
			dir_flag = 4;
		} else if (button_State == 2) {
			end = start;
			motor_control(0, 0);
		} else if (button_State == 5) {
			motor_point(4000);
			settingMode = 1;
		}else if(button_State==8){
			settingMode=0;
		}

		//LCD畫面
		if (i2c_working == 1)
			Display(settingMode);
		if (settingMode == 1) {

		}

		//系統LED PC13
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

		//清除button flag
		clean_button_flag();
	}

  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

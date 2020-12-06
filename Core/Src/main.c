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

#include "./FLASH_PAGE/FLASH_PAGE.h"
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
int pressTimer = 0;
int nowPosition = 0;
int stayPositionUp = 0;
int stayPositionDown = 20;
int slowValue = 5;
int sensitivityValue = 50;

int i;
uint32_t ADC_Value[100];
int maxLoadCount = 0;
uint32_t ad1, ad2;
uint32_t real_adc1, real_adc2;
uint32_t keep_adc1, keep_adc2;
uint32_t maxLoad = 0;
uint32_t PWM1 = 0, PWM2 = 0;
uint32_t OUTPUT_1_State = 0, OUTPUT_2_State = 0;
uint32_t sysinfo_State = 0, button_State = 0;
uint32_t dir_flag = 0;
uint32_t setDir_flag = 0;

int button_flag[5] = { 0 };

char buff[64];

int i2c_working = 1;

uint32_t *data = (uint32_t*) "Hello World";
uint32_t Rx_Data[4];
char string[30];
char tempString[30];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void motor_stop(int dir) {
	if (dir >= 1) {
		OUTPUT_1_State = 1;
		OUTPUT_2_State = 1;
		PWM1 = 0;
		PWM2 = 0;
		HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 1);
		HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 1);
		user_pwm_setvalue_1(0);
		user_pwm_setvalue_2(0);

	} else if (dir <= -1) {
		OUTPUT_1_State = 0;
		OUTPUT_2_State = 0;
		PWM1 = 4000;
		PWM2 = 4000;
		HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
		HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
		user_pwm_setvalue_1(4000);
		user_pwm_setvalue_2(4000);
	}
	HAL_Delay(100);
	motor_control(0,0);
}
void motor_control(int dir, int pwm) {
	if (setDir_flag >= 1) {
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
	} else {
		if (dir <= -1) {
			OUTPUT_1_State = 1;
			OUTPUT_2_State = 0;
			HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 1);
			HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
			PWM1 = pwm;
			PWM2 = 0;
			user_pwm_setvalue_1(pwm);
			user_pwm_setvalue_2(0);

		} else if (dir >= 1) {
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
}

void WriteSet(int dataInt1, int dataInt2, int dataInt3, int dataInt4,
		int dataInt5, int dataInt6) {
	//sprintf(*tempString,"%s%s%s%s%s",dataInt1,dataInt2,dataInt3,dataInt4,dataInt5);
	sprintf(tempString, "%4d%4d%4d%4d%4d%4d", dataInt1, dataInt2, dataInt3,
			dataInt4, dataInt5, dataInt6);
	Flash_Write_Data(0x0801FBF8, tempString);
}
void KeepSet() {
	WriteSet(nowPosition, stayPositionUp, stayPositionDown, sensitivity,
			slowValue, setDir_flag);
}
void ReadSet() {
	Flash_Read_Data(0x0801FBF8, Rx_Data);
	Convert_To_Str(Rx_Data, string);
	int xx[6] = { 0 };
	int xflag = 0;
	for (int x = 0; x < 23; x = x + 4) {
		char temp[5];
		temp[0] = string[x];
		temp[1] = string[x + 1];
		temp[2] = string[x + 2];
		temp[3] = string[x + 3];
		temp[4] = '\0';

		xx[xflag++] = atoi(temp);
		HAL_Delay(1);
	}
	nowPosition = xx[0];
	stayPositionUp = xx[1];
	stayPositionDown = xx[2];
	sensitivity = xx[3];
	slowValue = xx[4];
	setDir_flag = xx[5];
	//Convert_To_Str(Rx_Data, string);
}

void motor_point(int time) {
	HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 1);
	PWM1 = 0;
	PWM2 = 4000;
	user_pwm_setvalue_1(PWM1);
	user_pwm_setvalue_2(PWM2);
	HAL_Delay(time);
	HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 1);
	HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
	PWM1 = 4000;
	PWM2 = 0;
	user_pwm_setvalue_1(PWM1);
	user_pwm_setvalue_2(PWM2);
	HAL_Delay(time);
	HAL_GPIO_WritePin(OUTPUT_M1_GPIO_Port, OUTPUT_M1_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_M2_GPIO_Port, OUTPUT_M2_Pin, 0);
	PWM1 = 0;
	PWM2 = 0;
	user_pwm_setvalue_1(PWM1);
	user_pwm_setvalue_2(PWM2);
}
void timerControl() {
	if (start < end) {	//時間未結束
		if (dir_flag == 1) {
			if (start < end - 4000) {
				stage = 1;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(1, 1000);
			} else if (start < end - 2000) {
				stage = 2;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(1, 4000);
			} else if (start < end) {
				stage = 3;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(1, 1000);
			}
		} else if (dir_flag == 4) {
			if (start < end - 4000) {
				stage = 1;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(-1, 1000);
			} else if (start < end - 2000) {
				stage = 2;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(-1, 4000);
			} else if (start < end) {
				stage = 3;
				if (stage != last_stage) {
					last_stage = stage;
					detection_load(0, sensitivity);
				}
				if (detection_load(1, sensitivity)) {
					sysinfo_State = 1;
				}
				motor_control(-1, 1000);
			}
		}
		if (sysinfo_State == 1)
			end = start;	//壓力觸發
	} else {	//運行時間結束
		stage = 0;
		sysinfo_State = 0;
		detection_load(0, 50);
		motor_control(0, 0);
	}
}
void stepControl() {
	switch (dir_flag) {
	case 1:
		if (nowPosition > stayPositionUp) {
			motor_control(1, 1500);
			HAL_Delay(slowValue * 100);
		}
		while (nowPosition > stayPositionUp) {
			motor_control(1, 4000);
			HAL_Delay(20);
			//read_ADC();
			detection_load(0, sensitivity);
			nowPosition--;
			for (int timer = 0; timer < 5; timer++) {
				HAL_Delay(1);
				Display(settingMode);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET
						|| detection_load(1, sensitivity)) {
					dir_flag = 0;
					break;
				}
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET
					|| detection_load(1, sensitivity)) {
				dir_flag = 0;
				break;
			}
		}
		motor_stop(1);
		dir_flag=0;
		break;
	case 4:
		if (nowPosition < stayPositionDown) {
			motor_control(-1, 1500);
			HAL_Delay(slowValue * 100);
		}
		while (nowPosition < stayPositionDown) {
			motor_control(-1, 4000);
			HAL_Delay(20);
			//read_ADC();
			detection_load(0, sensitivity);
			nowPosition++;
			for (int timer = 0; timer < 5; timer++) {
				HAL_Delay(1);
				Display(settingMode);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET
						|| detection_load(1, sensitivity)) {
					dir_flag = 0;
					break;
				}
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET
					|| detection_load(1, sensitivity)) {
				dir_flag = 0;
				break;
			}
		}
		motor_stop(-1);
		dir_flag=0;
		break;
	default:
		//motor_control(0, 0);
		break;
	}
}
void buzzerTimes(int times) {
	for (int x = 0; x < times; x++) {
		HAL_GPIO_WritePin(GPIOA, OUTPUT_BUZZER_Pin, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, OUTPUT_BUZZER_Pin, 0);
		HAL_Delay(100);
	}

}
void clean_button_flag(void) {
	//Usart2DmaPrintf("clena\n");
	for (i = 0; i <= 5; i++) { //clean flag
		button_flag[i] = 0;
		//HAL_Delay(1);
	}
}
int read_ADC() {
	int times = 50; //採樣次數
	for (i = 0, ad1 = 0, ad2 = 0; i < times;) {
		ad1 += ADC_Value[i++];
		ad2 += ADC_Value[i++];
		//HAL_Delay(1);
	}

	real_adc1 = ad1 / (times / 2);
	real_adc2 = ad2 / (times / 2);
	//sensitivity = real_adc2;
	if (real_adc1 <= real_adc2) {
		return 1;
	} else {
		return 0;
	}
}
//檢知附載
//int time =0 ;為第一次設置
int detection_load(int times, uint32_t th) {
	read_ADC();
	int setCount = 3;
	if (times == 0) {
		//keep_adc1 = real_adc1;
		maxLoad = 0;
		maxLoadCount = 0;
	} else {
		maxLoadCount++;
		if (maxLoadCount < setCount) {
			if (real_adc1 > maxLoad) {
				maxLoad = real_adc1 + th;
			}
		} else {
			if (real_adc1 > (maxLoad)) {
				//maxLoad = real_adc1;
				return 1;
			}
		}
	}
	return 0;
}

void Display(int mode) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(2, 0);
	int line_count = 1;
	if (mode == 0) {
		//snprintf(buff, sizeof(buff), "%s,", string);
		//ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "%s,%s", __DATE__, __TIME__);
		ssd1306_WriteString(buff, Font_6x8, White);

		ssd1306_SetCursor(2, 8 * line_count++);
		snprintf(buff, sizeof(buff), "state:%d,%d nowP:%d", sysinfo_State,
				stage, nowPosition);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "[B0]:%d,Sent:%d", real_adc1, sensitivity);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "mLoad:%d count:%d", maxLoad,
				maxLoadCount);
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
	} else if (mode == 1) {		//setting mode
		snprintf(buff, sizeof(buff), "Setting mode:%d", settingMode);
		ssd1306_WriteString(buff, Font_6x8, White);

		ssd1306_SetCursor(2, 8 * line_count++);
		snprintf(buff, sizeof(buff), "button[%d]:%d,%d,%d,%d", button_State,
				button_flag[1], button_flag[2], button_flag[3], button_flag[4]);
		ssd1306_WriteString(buff, Font_6x8, White);
		/*
		 snprintf(buff, sizeof(buff), "[B0]:%d,KA1:%d", real_adc1, keep_adc1);
		 ssd1306_SetCursor(2, 8 * line_count++);
		 ssd1306_WriteString(buff, Font_6x8, White);
		 */
		snprintf(buff, sizeof(buff), "now:%d,up:%d,down:%d", nowPosition,
				stayPositionUp, stayPositionDown);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "LongPress:%d", pressTimer);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "DIR:%d,nowP:%d", setDir_flag,
				nowPosition);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "PWM1:%d, GPIO1:%d", PWM1, OUTPUT_1_State);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		snprintf(buff, sizeof(buff), "PWM2:%d, GPIO2:%d", PWM2, OUTPUT_2_State);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

		if (end > start) {
			snprintf(buff, sizeof(buff), "time:%d", end - start);
			ssd1306_SetCursor(2, 8 * 7);
			ssd1306_WriteString(buff, Font_6x8, White);
		}

	} else if (mode == 2) {		//setting mode
		snprintf(buff, sizeof(buff), "s_Value:%d", sensitivityValue);
		ssd1306_SetCursor(2, 8 * line_count++);
		ssd1306_WriteString(buff, Font_6x8, White);

	} else if (mode == 3) {		//setting mode
		snprintf(buff, sizeof(buff), "slowValue:%dms", slowValue * 100);
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
		//Flash_Write_Data(0x0801FBF8 , "A");
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
		button_flag[2]++;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET) {
		button_flag[3]++;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET) {
		button_flag[4]++;
		//Flash_Write_Data(0x0801FBF8 , "D");
	}

	if (button_flag[1] >= th)
		state = 1;
	if (button_flag[2] >= th)
		state = state + 2;
	if (button_flag[3] >= th)
		state = state + 4;
	if (button_flag[4] >= th)
		state = state + 8;
	if (state == 0)
		clean_button_flag();
	return state;
}
int check_buttom() {
	//讀取案就狀態

	button_State = read_GPIO(1);
	int exitSetTime = 8000;

	if (settingMode == 0) {
		if (button_State == 1) {
			//正轉
			end = start + runtime;
			dir_flag = 1;
		} else if (button_State == 4) {
			//反轉
			end = start + runtime;
			dir_flag = 4;
		} else if (button_State == 2) {
			//停止
			end = start;
			motor_control(0, 0);
		} else if (button_State == 5) {
			//尚未定義 強制停止
			end = start;
			motor_control(0, 0);
		} else if (button_State == 8) {
			//進入設定模式
			motor_control(0, 0);
			end = start + exitSetTime;
			//motor_point(100);
			settingMode = 1;
			buzzerTimes(3);
		}
	} else {

		if (button_State > 0) {
			end = start + exitSetTime;
		}
		if (button_State == 1) {//shrot press is move ,long press into set adc-power.
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
				pressTimer++;
				Display(settingMode);
			}
			//pressTimer++;
			if (pressTimer > 5) {
				pressTimer = 0;
				settingMode = 2;
				while (settingMode == 2) {
					Display(settingMode);
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
						if (sensitivityValue <= 1000) {
							sensitivityValue += 20;
						} else {
							sensitivityValue = 1000;
						}
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
						settingMode = 1;
						sensitivity = sensitivityValue;
						buzzerTimes(3);
						KeepSet();
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET) {
						if (sensitivityValue >= 20) {
							sensitivityValue -= 20;
						} else {
							sensitivityValue = 0;
						}
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET) {
						settingMode = 1;
					}
					HAL_Delay(250);
				}
			} else {
				motor_control(1, 4000);
				HAL_Delay(500);
				motor_control(0, 0);
				if (nowPosition >= 1000) {
					nowPosition = 1000;
				} else {
					nowPosition++;
				}

			}

		} else if (button_State == 4) {	//shrot press is move ,long press into set slow-val.
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET) {
				pressTimer++;
				Display(settingMode);
			}
			//pressTimer++;
			if (pressTimer > 5) {
				pressTimer = 0;
				settingMode = 3;
				while (settingMode == 3) {
					Display(settingMode);
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
						if (slowValue <= 100) {
							slowValue += 1;
						} else {
							slowValue = 100;
						}
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
						settingMode = 1;
						buzzerTimes(3);
						KeepSet();
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET) {
						if (slowValue >= 1) {
							slowValue -= 1;
						} else {
							slowValue = 0;
						}
					}
					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET) {
						settingMode = 1;
					}
					HAL_Delay(250);
				}
			} else {
				motor_control(-1, 4000);
				HAL_Delay(500);
				motor_control(0, 0);
				if (nowPosition < 1) {
					nowPosition = 0;
				} else {
					nowPosition--;
				}
			}
		} else if (button_State == 2) {		//set favorite-point

		} else if (button_State == 5) {		//change motor dir
			ReadSet();
			buzzerTimes(3);
			settingMode = 1;
			if (setDir_flag == 1) {
				setDir_flag = 0;
			} else {
				setDir_flag = 1;
			}
			KeepSet();
			HAL_Delay(500);

		} else if (button_State == 3) {		//set up-point
			settingMode = 1;
			stayPositionUp = nowPosition;
			buzzerTimes(3);
			KeepSet();
		} else if (button_State == 6) {		//set donw-point
			settingMode = 1;
			stayPositionDown = nowPosition;
			buzzerTimes(3);
			KeepSet();

		} else if (button_State == 8) {
			settingMode = 1;
		} else if (button_State == 0) {
			pressTimer = 0;
		}
	}
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

int main(void) {

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

	//nowp,UP,Down,sensitivity,SlowTime
	//Flash_Write_Data(0x0801FBF8 , "0000,0000,0000,0000,0000");
	//Flash_Read_Data(0x0801FBF8, Rx_Data);
	/*
	 1.nowPosition
	 2.stayPositionUp
	 3.stayPositionDown
	 4.sensitivity
	 5.slowValue
	 6.dir_flag

	 */
	//WriteSet(5,1,20,100,5,1);
	ReadSet();
	buzzerTimes(1);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//系統運行時間
		start = HAL_GetTick();
		if (settingMode == 0) {	//運行模式
			//timerControl();
			stepControl();
		} else {	//設定模式
			if (start < end) {	//時間未結束

			} else {
				settingMode = 0;
			}
		}
		//壓力觸動時輸出1
		//sysinfo_State = read_ADC();
		read_ADC();

		check_buttom();

		//LCD畫面
		if (i2c_working == 1)
			Display(settingMode);

		//系統LED PC13
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

		//清除button flag
		clean_button_flag();
	}

	/* USER CODE END 3 */

}

/** System Clock Configuration
 */
void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

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
void _Error_Handler(char *file, int line) {
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

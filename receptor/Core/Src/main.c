/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define T_PUNTO 200 //Tiempo máximo del punto
#define T_RAYA 400 //Tiempo máximo de la raya
#define T_PR 200 //Tiempo máximo apagado entre códigos
#define T_CARACTER 400 //Tiempo máximo entre caracteres
#define T_PALABRA 750 //Tiempo máximo entre palabras
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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char car[8]="";
uint32_t luz=0;
enum estado {X0,X1,X2,X3,X4,X5,X6};
enum estado estado_act, estado_sig;

char cact[5] = "", csig[5] = "****"; //Secuencias de . -
volatile char pact[10] = ""; //Secuencias de caracteres
char psig[10] = "*********"; //Secuencias de caracteres

uint32_t pos_csig = 0, pos_psig = 0;
uint8_t cleido = 0, pleida = 0;

volatile char frase_recv[64] = "";
volatile char frase_inter[64] = "";
volatile char frase_recv_m[256] = "";

volatile uint8_t mensaje_completo = 0;


void eliminarAsteriscos(char* orig){
	int len = strlen(orig);

	for(int i=0; i<len; i++){
		if(orig[i] == '*')
			orig[i] = ' ';
	}
}

char morse2asci(char* car){
	char asci;

	//Decoder
	if(car[0] == '.'){
		if(car[1] == '.'){
			if(car[2] == '.'){
				if(car[3] == '.'){
					asci = 'H'; //....
				}
				else if(car[3] == '-'){
					asci = 'V'; //...-
				}
				else asci = 'S'; // ...
			}
			else if(car[2] == '-'){
				if(car[3] == '.'){
					asci = 'F'; //..-.
				}
				else if(car[3] == '-'){
					asci = '*'; //..-- no existe
				}
				else asci = 'U'; // ..-
			}
			else asci = 'I'; // ..
		}
		else if(car[1] == '-'){
			if(car[2] == '.'){
				if(car[3] == '.'){
					asci = 'L'; //.-..
				}
				else if(car[3] == '-'){
					asci = '*'; // .-.- no existe
				}
				else asci = 'R'; // .-.
			}
			else if(car[2] == '-'){
				if(car[3] == '.'){
					asci = 'P'; //.--.
				}
				else if(car[3] == '-'){
					asci = 'J'; //.---
				}
				else asci = 'W'; // .--
			}
			else asci = 'A'; //.-
		}
		else asci = 'E'; // .
	}
	else if(car[0] == '-'){
		if(car[1] == '.'){
			if(car[2] == '.'){
				if(car[3] == '.'){
					asci = 'B'; //-...
				}
				else if(car[3] == '-'){
					asci = 'X'; //-..-
				}
				else asci = 'D'; // -..
			}
			else if(car[2] == '-'){
				if(car[3] == '.'){
					asci = 'C'; //-.-.
				}
				else if(car[3] == '-'){
					asci = 'Y'; //-.--
				}
				else asci = 'K'; // -.-
			}
			else asci = 'N'; //-.
		}
		else if(car[1] == '-'){
			if(car[2] == '.'){
				if(car[3] == '.'){
					asci = 'Z'; //--..
				}
				else if(car[3] == '-'){
					asci = 'Q'; //--.-
				}
				else asci = 'G'; // --.
			}
			else if(car[2] == '-'){
				if(car[3] == '.'){
					asci = '*'; //---. no existe
				}
				else if(car[3] == '-'){
					asci = '*'; //---- no existe
				}
				else asci = 'O'; // ---
			}
			else asci = 'M'; // --
		}
		else asci = 'T'; // -
	}
	return asci;
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
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();
  uint32_t tact = 0, tim2=0, tim3=0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  tact = HAL_GetTick();
	  uint8_t y = 0;
	 ssd1306_Fill(Black);
	 #ifdef SSD1306_INCLUDE_FONT_16x26
	 ssd1306_SetCursor(2, y);
	 ssd1306_WriteString(pact, Font_16x26, White);
	 #endif
	 ssd1306_UpdateScreen();



	  	  HAL_ADC_Start(&hadc1);
	  	  if(HAL_ADC_PollForConversion(&hadc1, 1000)==HAL_OK){
	  		  luz=HAL_ADC_GetValue(&hadc1);
	  	  }
	  	  HAL_ADC_Stop(&hadc1);

	  	  if(estado_act == X0){
	  		  if(luz < 2000) {
	  			  estado_sig = X1;
	  			  tim2 = HAL_GetTick();
	  		  }
	  		  else estado_sig = X0;
	  	  }
	  	  else if(estado_act == X1){
	  		  if(tact - tim2 > T_PUNTO && tact - tim2 < T_RAYA && luz > 2000){ //Lee raya
	  			  estado_sig = X2;
	  			  tim2 = 0;
	  			  tim3 = HAL_GetTick();
	  			  csig[pos_csig] = '-';
	  		  }
	  		  else if(tact - tim2 < T_PUNTO && luz > 2000){ //Lee punto
	  			  estado_sig = X3;
	  			  tim2 = 0;
	  			  tim3 = HAL_GetTick();
	  			  csig[pos_csig] = '.';
	  		  }
	  		  else if(tact - tim2 > T_RAYA){ //Error tiempo encendido
	  			  estado_sig = X4;
	  			  tim2 = 0;
	  			  cleido = 1; //Lee el caracter y palabra actuales
	  			  pleida = 1;
	  		  }
	  		  else estado_sig=X1;
	  	  }
	  	  else if(estado_act == X2){
	  		  if(tact - tim3 > T_PALABRA){ //Error tiempo apagado
	  			  estado_sig = X0;
	  			  tim2 = 0;
	  			  tim3 = 0;
	  			  cleido = 1;
	  			  pleida = 1;
	  		  }
	  		  else if(luz < 2000 && tact - tim3 < T_PR){ //Siguiente punto/raya del caracter
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  pos_csig++; //Siguiente punto/raya del caracter
	  		  }
	  		  else if(luz < 2000 && tact - tim3 > T_PR && tact - tim3 < T_CARACTER){ //Siguiente caracter
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  cleido = 1; //Caracter leido --> Tratamiento del caracter
	  		  }
	  		  else if(luz < 2000 && tact - tim3 > T_CARACTER && tact - tim3 < T_PALABRA){ //Siguiente palabra
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  cleido = 1;
	  			  pleida = 1; //Palabra leída --> Tratamiento de la palabra
	  		  }
	  		  else estado_sig = X2;
	  	  }
	  	  else if(estado_act == X3){
	  		  if(tact - tim3 > T_PALABRA){ //Error tiempo apagado
	  			  estado_sig = X0;
	  			  tim2 = 0;
	  			  tim3 = 0;
	  			  cleido = 1;
	  			  pleida = 1;
	  		  }
	  		  else if(luz < 2000 && tact - tim3 < T_PR){ //Siguiente punto/raya del caracter
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  pos_csig++; //Siguiente punto/raya del caracter
	  		  }
	  		  else if(luz < 2000 && tact - tim3 > T_PR && tact - tim3 < T_CARACTER){ //Siguiente caracter
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  cleido = 1; //Caracter leido --> Tratamiento del caracter
	  		  }
	  		  else if(luz < 2000 && tact - tim3 > T_CARACTER && tact - tim3 < T_PALABRA){ //Siguiente palabra
	  			  estado_sig = X1;
	  			  tim3 = 0;
	  			  tim2 = HAL_GetTick();
	  			  cleido = 1;
	  			  pleida = 1; //Palabra leída --> Tratamiento de la palabra
	  		  }
	  		  else estado_sig = X3;
	  	  	  }
	  	  else if(estado_act == X4){
	  		  if(luz > 2000) {
	  			  estado_sig = X0;
	  			  pos_csig = 0; //Reinicia a 0 las posiciones
	  			  pos_psig = 0;
	  		  }
	  		  else estado_sig = X4;
	  	  }

	  	  estado_act = estado_sig;

	  	  if(cleido){
	  		  strcpy(cact, csig);
	  		  strcpy(csig, "****");
	  		  psig[pos_psig] = morse2asci(cact);
	  		  pos_psig++; //Siguiente caracter de la palabra
	  		  cleido = 0;
	  		  pos_csig = 0; //Empieza un nuevo caracter
	  	  }
	  	  if(pleida){
	  		  strcpy(pact, psig);
	  		  strcpy(psig, "*********");
	  		  pleida = 0;
	  		  pos_psig = 0; //Empieza una nueva palabra

	  		  eliminarAsteriscos(pact);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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

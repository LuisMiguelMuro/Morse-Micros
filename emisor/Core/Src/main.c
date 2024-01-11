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
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char car[8]="";
uint32_t luz=0;
enum estado {X0,X1,X2,X3,X4,X5,X6};
//enum estado estado_act, estado_sig;

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

void fraseMorse(char* frase_recv, char* frase_recv_m) {
    for (int i = 0; i < strlen(frase_recv); i++) {
        char letra = frase_recv[i];
        char letra_m[10];
        asci2morse(letra, letra_m);

        if (('A' <= letra && letra <= 'Z') || ('a' <= letra && letra <= 'z')) {
            strcat(frase_recv_m, letra_m);
            if (('A' <= frase_recv[i + 1] && frase_recv[i + 1] <= 'Z') || ('a' <= frase_recv[i + 1] && frase_recv[i + 1] <= 'z'))
                strcat(frase_recv_m, "&");
        }
        else if (letra == ' ')
            strcat(frase_recv_m, "%");

        if (i == strlen(frase_recv)-1)
            strcat(frase_recv_m, "#");
    }
}

void asci2morse(char src, char* dest) {
    switch (src) {
    case 'A':
    case 'a':
        strcpy(dest, "./-");
        break;
    case 'B':
    case 'b':
        strcpy(dest, "-/././.");
        break;
    case 'C':
    case 'c':
        strcpy(dest, "-/./-/.");
        break;
    case 'D':
    case 'd':
        strcpy(dest, "-/./.");
        break;
    case 'E':
    case 'e':
        strcpy(dest, ".");
        break;
    case 'F':
    case 'f':
        strcpy(dest, "././-/.");
        break;
    case 'G':
    case 'g':
        strcpy(dest, "-/-/.");
        break;
    case 'H':
    case 'h':
        strcpy(dest, "./././.");
        break;
    case 'I':
    case 'i':
        strcpy(dest, "./.");
        break;
    case 'J':
    case 'j':
        strcpy(dest, "./-/-/-");
        break;
    case 'K':
    case 'k':
        strcpy(dest, "-/./-");
        break;
    case 'L':
    case 'l':
        strcpy(dest, "./-/./.");
        break;
    case 'M':
    case 'm':
        strcpy(dest, "-/-");
        break;
    case 'N':
    case 'n':
        strcpy(dest, "-/.");
        break;
    case 'O':
    case 'o':
        strcpy(dest, "-/-/-");
        break;
    case 'P':
    case 'p':
        strcpy(dest, "./-/-/.");
        break;
    case 'Q':
    case 'q':
        strcpy(dest, "-/-/./-");
        break;
    case 'R':
    case 'r':
        strcpy(dest, "./-/.");
        break;
    case 'S':
    case 's':
        strcpy(dest, "././.");
        break;
    case 'T':
    case 't':
        strcpy(dest, "-");
        break;
    case 'U':
    case 'u':
        strcpy(dest, "././-");
        break;
    case 'V':
    case 'v':
        strcpy(dest, "./././-");
        break;
    case 'W':
    case 'w':
        strcpy(dest, "./-/-");
        break;
    case 'X':
    case 'x':
        strcpy(dest, "-/././-");
        break;
    case 'Y':
    case 'y':
        strcpy(dest, "-/./-/-");
        break;
    case 'Z':
    case 'z':
        strcpy(dest, "-/-/./.");
        break;
    default:
        strcpy(dest, "");
        break;
    }
}

void transmitirLedMorse(char* cadena, uint8_t* start){
	static uint8_t index = 0;
	static enum estado estado_act = X0, estado_sig = X0;

	uint32_t tact = HAL_GetTick();
	volatile static uint32_t tim1 = 0;

	if(*start == 1){
		if(estado_act == X0){
			if(cadena[index] == '.'){
				estado_sig = X1;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);
				tim1 = HAL_GetTick();
			}
			else if(cadena[index] == '-'){
				estado_sig = X2;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);
				tim1 = HAL_GetTick();
			}
			else if(cadena[index] == '/'){
				estado_sig = X3;
				tim1 = HAL_GetTick();
			}
			else if(cadena[index] == '&'){
				estado_sig = X4;
				tim1 = HAL_GetTick();
			}
			else if(cadena[index] == '%'){
				estado_sig = X5;
				tim1 = HAL_GetTick();
			}
			else if(cadena[index] == '#'){
				estado_sig = X6;
				tim1 = HAL_GetTick();
			}
		}
		else if(estado_act == X1){
			if(tact - tim1 > T_PUNTO -50){
				estado_sig = X0;
				tim1 = 0;
				index++;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
			}
		}
		else if(estado_act == X2){
			if(tact - tim1 > T_RAYA -50){
				estado_sig = X0;
				tim1 = 0;
				index++;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
			}
		}
		else if(estado_act == X3){
			if(tact - tim1 > T_PR -50){
				estado_sig = X0;
				tim1 = 0;
				index++;
			}
		}
		else if(estado_act == X4){
			if(tact - tim1 > T_CARACTER -50){
				estado_sig = X0;
				tim1 = 0;
				index++;
			}
		}
		else if(estado_act == X5){
			if(tact - tim1 > T_PALABRA -100){
				estado_sig = X0;
				tim1 = 0;
				index++;
			}
		}
		else if(estado_act == X6){
			if(tact - tim1 > T_PALABRA){
				estado_sig = X0;
				tim1 = 0;
				index = 0; //Devuelve el índice al principio
				*start = 0; //Deja de transmitir
				strcpy(cadena, ""); //Limpiar cadena
			}
		}
		estado_act = estado_sig;
	}
	else return;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (frase_recv[strlen(frase_recv) - 1] == '\n') {
		mensaje_completo = 1;
	}
	if(mensaje_completo == 0)
		HAL_UART_Receive_IT(huart, (uint8_t *)&frase_recv[strlen(frase_recv)], 1);
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  uint32_t tact = 0, tim2 =0, tim3 = 0;
  char mdest[5];
  HAL_StatusTypeDef status_recv;
  uint32_t t_start_recv;
  uint8_t receiving = 0;
  char cadena[] = "-/-/./-&././-&.%-&./-&./-/./.%.&././.&-&./-&././.#";
  char start_led = 1;

  HAL_UART_Receive_IT(&huart1, (uint8_t *)frase_recv, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(mensaje_completo){


	  			 fraseMorse(frase_recv, frase_recv_m);


	  			 mensaje_completo = 0;
	  			 start_led = 1;
	  			 HAL_UART_Receive_IT(&huart1, (uint8_t *)frase_recv, 1);
	  			 memset(frase_recv, 0, sizeof(frase_recv));

  }
	  transmitirLedMorse(frase_recv_m, &start_led);
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
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

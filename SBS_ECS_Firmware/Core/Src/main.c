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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ecs_can.h"
#include "ecs_math.h"
#include <stdio.h>
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
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE BEGIN PV */
float g_Last_Angle = 0.0f; // 계산된 최신 방위각 기억용
uint8_t uart_rx_byte;      // 1바이트씩 받을 변수
uint8_t uart_rx_buf[10];   // 패킷을 모아둘 버퍼
uint8_t uart_rx_idx = 0;
/* USER CODE END PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
 if ( ch == '\n' )
	 HAL_UART_Transmit(&huart2, (uint8_t*)&"\r", 1, HAL_MAX_DELAY);
 HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
 return ch;
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
  MX_USART2_UART_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
    ECS_CAN_Filter_Init();

    // 💡 메인 루프 진입 전에 UART 인터럽트 수신을 최초 1회 시작해 줍니다.
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);

    // [추가] UART 동작 확인용 테스트 메시지
    printf("ECS System Started...\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)"UART OK\r\n", 9, 100);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // 1. 탐색기 데이터 처리 (의존성 분리)
        // 데이터가 들어올 때만 '좌표'와 '계산된 각도'를 최신화함
        if (g_NewTarget_Flag == 1) {
            g_NewTarget_Flag = 0;

            // 최신 좌표와 각도 업데이트 (저장만 하고 발사대로 명령은 안 쏨)
            g_Last_Angle = ECS_Math_CalAngle(g_Target_X_mm, g_Target_Y_mm);
        }

        // 2. UI 주기적 업데이트 (독립적 동작)
        // 탐색기 데이터 수신 여부와 상관없이 1000ms(1초)마다 UI에 현재 상태 전송
        static uint32_t last_ui_tick = 0;
        if (HAL_GetTick() - last_ui_tick >= 1000) {
            last_ui_tick = HAL_GetTick();

            char uartBuf[100];
            // 현재 저장된 X, Y, 각도, 그리고 발사대로부터 받은 최신 상태를 무조건 전송
            sprintf(uartBuf, "%.1f,%.1f,%.1f,%d\n",
                    g_Target_X_mm, g_Target_Y_mm, g_Last_Angle, g_Launcher_Status);

            HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), 10);
        }

        /* 💡 [참고] 발사대 제어 명령(ALIGN, FIRE 등)은 여기서 처리하지 않습니다.
           앞서 수정했던 'HAL_UART_RxCpltCallback' 인터럽트 함수가 UI 버튼 신호를
           받는 즉시 독립적으로 발사대에 CAN 메시지를 쏘게 됩니다.
        */
        // ⭐ [테스트 코드] 2초마다 강제로 발사대에 '정렬' 명령을 보냄
//            static uint32_t test_tick = 0;
//            if (HAL_GetTick() - test_tick >= 2000) {
//                test_tick = HAL_GetTick();
//
//                // 45도 각도로 정렬 명령 강제 전송
//                ECS_CAN_SendToLauncher(45.0f, LTL_CMD_ALIGN);
//                printf("[TEST] Manually sending ALIGN to Launcher...\n");
//            }

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
    	// ** 테스트용[진단 로그] 어떤 바이트든 들어오면 무조건 출력해봅니다.
		//printf("RX: 0x%02X, Idx: %d\n", uart_rx_byte, uart_rx_idx);

        // 1바이트 수신 데이터를 버퍼에 저장
        uart_rx_buf[uart_rx_idx] = uart_rx_byte;

        // [패킷 종료 확인] ETX(0x03)가 들어오면 해석 시작
        //if (uart_rx_byte == 0x03) {
            // [유효성 검사] 시작이 STX(0x02)인지 확인
        if(1) {
            //if (uart_rx_buf[0] == 0x02) {
        	if(1) {
                uint8_t cmd = uart_rx_buf[uart_rx_idx++]; // UI가 보낸 명령어 코드

                switch (cmd) {
                    case 0x00: // LauncherAlign (UI: 0x00)
                        ECS_CAN_SendToLauncher(g_Last_Angle, LTL_CMD_ALIGN);
                        printf("[UART] Launcher ALIGN Command\n");
                        break;

                    case 0x01: // LauncherFire (UI: 0x01)
                        ECS_CAN_SendToLauncher(g_Last_Angle, LTL_CMD_FIRE);
                        printf("[UART] Launcher FIRE Command\n");
                        break;

                    case 0x02: // SeekerStart (UI: 0x02)
                        ECS_CAN_SendToSeeker(DET_CMD_STANDBY);
                        printf("[UART] Seeker START Command\n");
                        break;

                    case 0x03: // SeekerStop (UI: 0x03)
                        ECS_CAN_SendToSeeker(DET_CMD_RESET);
                        printf("[UART] Seeker STOP Command\n");
                        break;

                    case 0x04: // EmergencyStop (UI: 0x04)
                        ECS_CAN_SendToLauncher(g_Last_Angle, LTL_CMD_EMERGENCY);
                        ECS_CAN_SendToSeeker(DET_CMD_RESET);
                        printf("[UART] EMERGENCY STOP Command\n");
                        break;

                    default:
                        printf("[UART] Unknown Command: 0x%02X\n", cmd);
                        break;
                }
            }
            // 패킷 처리 완료 후 인덱스 초기화
            uart_rx_idx = 0;
        }

        // 버퍼 오버플로우 방지
        if (uart_rx_idx >= 10) uart_rx_idx = 0;

        // 다음 바이트 수신 대기 (중요!)
        HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
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

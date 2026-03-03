/*
 * ecs_can.c
 *
 *  Created on: Feb 26, 2026
 *      Author: SeungMin
 */
#include "ecs_can.h"
#include "stm32f1xx_hal.h"

extern CAN_HandleTypeDef hcan;

// SDD 규격에 맞춘 float형 전역 변수
volatile float g_Target_X_mm = 0.0f;
volatile float g_Target_Y_mm = 0.0f;
volatile uint8_t g_NewTarget_Flag = 0;
volatile uint8_t g_Launcher_Status = 0;

void ECS_CAN_Filter_Init(void) {
    CAN_FilterTypeDef sFilterConfig;

    // ---------------------------------------------------------
    // 1. 탐색기(Seeker) 수신 필터 (FilterBank 0)
    // ---------------------------------------------------------
    uint32_t det_filter_id = 0x00000200;
    uint32_t strict_mask = 0x1FFFFFFF; // 29비트 전체 엄격 검사

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = (uint16_t)((det_filter_id << 3) >> 16);
    sFilterConfig.FilterIdLow  = (uint16_t)((det_filter_id << 3) | CAN_ID_EXT);
    sFilterConfig.FilterMaskIdHigh = (uint16_t)((strict_mask << 3) >> 16);
    sFilterConfig.FilterMaskIdLow  = (uint16_t)((strict_mask << 3) | CAN_ID_EXT);
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;

    HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);

    // ---------------------------------------------------------
    // 2. 발사대(Launcher) 수신 필터 추가 (FilterBank 1)
    // ---------------------------------------------------------
    uint32_t ltl_filter_id = 0x00000400; // 발사대 상태 수신 ID: 0x400

    sFilterConfig.FilterBank = 1; // 💡 다른 뱅크를 사용해야 기존 필터가 안 지워집니다.
    sFilterConfig.FilterIdHigh = (uint16_t)((ltl_filter_id << 3) >> 16);
    sFilterConfig.FilterIdLow  = (uint16_t)((ltl_filter_id << 3) | CAN_ID_EXT);
    sFilterConfig.FilterMaskIdHigh = (uint16_t)((strict_mask << 3) >> 16);
    sFilterConfig.FilterMaskIdLow  = (uint16_t)((strict_mask << 3) | CAN_ID_EXT);

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
        Error_Handler();
    }

    // CAN 시작 및 인터럽트 활성화
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

// ecs_can.c 내 수신 처리 예시
void ECS_CAN_ParseRxMessage(uint32_t rxId, uint8_t* rxData) {
    if (rxId == 0x00000200) { // DET
        // 💡 8바이트 데이터를 float 2개로 해석
        float* pData = (float*)rxData;
        g_Target_X_mm = pData[0]; // target_x
        g_Target_Y_mm = pData[1]; // target_y
        g_NewTarget_Flag = 1;
    }
    if (rxId == 0x00000400) { // LANCHER

    	//
           uint8_t temp_state_Launcher = rxData[0];
    }


}

void ECS_CAN_SendToLauncher(float angle, LtlCommand_e cmd) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    LauncherPayload_t txPayload;

    TxHeader.ExtId = CAN_ID_LTL_TX;    // 💡 Extended ID 사용
    TxHeader.IDE = CAN_ID_EXT;         // 💡 확장 ID 플래그
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    txPayload.controlData.targetAngle_deg = angle;
    txPayload.controlData.commandMode = (uint8_t)cmd;

    for(int i = 5; i < 8; i++) txPayload.buffer[i] = 0;

    HAL_CAN_AddTxMessage(&hcan, &TxHeader, txPayload.buffer, &TxMailbox);
}

void ECS_CAN_SendToSeeker(DetCommand_e cmd) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    uint8_t txData[8] = {0,};

    // 💡 탐색기 팀의 필터(0x100 + Strict Mask)를 통과하기 위한 설정
    TxHeader.ExtId = 0x100;           // CAN_ID_DET_TX (SDD 규격)
    TxHeader.IDE   = CAN_ID_EXT;      // 확장 ID 필수
    TxHeader.RTR   = CAN_RTR_DATA;
    TxHeader.DLC   = 8;               // 탐색기 팀과 맞춤 (8바이트)
    TxHeader.TransmitGlobalTime = DISABLE;

    // 💡 탐색기 팀은 RxData[0]을 g_SystemMode로 사용함
    txData[0] = (uint8_t)cmd;

    // 메시지 전송
    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, txData, &TxMailbox) != HAL_OK) {
        // 송신 실패 시 디버깅 (필요 시 LED 제어 등)
        Error_Handler();
    }
}

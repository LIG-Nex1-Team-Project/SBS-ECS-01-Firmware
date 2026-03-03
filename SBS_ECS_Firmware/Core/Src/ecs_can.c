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
    CAN_FilterTypeDef canFilterConfig;

    canFilterConfig.FilterBank = 0;
    canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

    // 💡 0x0000으로 설정하면 아이디 검사를 하지 않고 모든 CAN 메시지를 수신합니다!
    canFilterConfig.FilterIdHigh = 0x0000;
    canFilterConfig.FilterIdLow = 0x0000;
    canFilterConfig.FilterMaskIdHigh = 0x0000;
    canFilterConfig.FilterMaskIdLow = 0x0000;

    canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilterConfig.FilterActivation = ENABLE;
    canFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &canFilterConfig) != HAL_OK) {
        Error_Handler();
    }
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void ECS_CAN_ParseRxMessage(uint32_t rxId, uint8_t* rxData) {
    if (rxId == CAN_ID_DET_RX) {
        // 탐색기 좌표 수신 로직 (기존과 동일, float 캐스팅 확인)
        TargetPayload_t payload;
        for(int i = 0; i < 8; i++) payload.buffer[i] = rxData[i];
        g_Target_X_mm = payload.targetPos.x_mm;
        g_Target_Y_mm = payload.targetPos.y_mm;
        g_NewTarget_Flag = 1;
    }
    else if (rxId == CAN_ID_LTL_RX) {
        // 발사대 상태 수신 로직 추가
        g_Launcher_Status = rxData[0]; // 1번째 바이트에 상태 저장
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

// 💡 탐색기 제어 명령 송신 함수 추가
void ECS_CAN_SendToSeeker(DetCommand_e cmd) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    uint8_t txData[8] = {0,}; // 8바이트 0으로 초기화 (패딩)

    TxHeader.ExtId = CAN_ID_DET_TX;    // 0x00000100
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8; // 고정 8바이트
    TxHeader.TransmitGlobalTime = DISABLE;

    txData[0] = (uint8_t)cmd; // 1번째 바이트에 명령 코드 삽입

    HAL_CAN_AddTxMessage(&hcan, &TxHeader, txData, &TxMailbox);
}

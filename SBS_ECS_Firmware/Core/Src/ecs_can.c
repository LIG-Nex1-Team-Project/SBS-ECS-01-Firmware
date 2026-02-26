/*
 * ecs_can.c
 *
 *  Created on: Feb 26, 2026
 *      Author: SeungMin
 */
#include "ecs_can.h"

// 공유 메모리 실제 할당
volatile int32_t g_Target_X_mm = 0;
volatile int32_t g_Target_Y_mm = 0;
volatile uint8_t g_NewTarget_Flag = 0;

void ECS_CAN_Filter_Init(void) {
    // STM32 HAL 드라이버 기준 CAN 필터 세팅 로직
    // (이 부분은 사용하시는 보드의 CAN 설정에 따라 코드가 달라집니다.
    //  CubeMX에서 자동 생성된 hcan 인스턴스를 활용하여 필터를 설정하고 인터럽트를 켭니다.)
}

// CAN Rx 인터럽트 콜백이나 FIFO 대기 함수에서 호출할 파싱 함수
void ECS_CAN_ParseRxMessage(uint32_t rxId, uint8_t* rxData) {

    // 수신된 ID가 탐색기 데이터(0x0200)인지 확인
    if (rxId == CAN_ID_DET_RX) {
        TargetPayload_t payload;

        // 1. 버퍼에 8바이트 데이터 적재
        for(int i = 0; i < 8; i++) {
            payload.buffer[i] = rxData[i];
        }

        // 2. Race Condition 방지를 위해 한 번에 데이터 갱신
        // (필요 시 이 구역 앞뒤로 __disable_irq(); 와 __enable_irq(); 를 추가하여 보호)
        g_Target_X_mm = payload.targetPos.x_mm;
        g_Target_Y_mm = payload.targetPos.y_mm;
        g_NewTarget_Flag = 1; // 새로운 데이터가 왔음을 메인 루프에 알림
    }
}


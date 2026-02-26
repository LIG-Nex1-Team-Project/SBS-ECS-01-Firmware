/*
 * ecs_can.h
 *
 *  Created on: Feb 26, 2026
 *      Author: SeungMin
 */

#ifndef INC_ECS_CAN_H_
#define INC_ECS_CAN_H_

#include <stdint.h>

// 탐색기 -> ECS 표적 제원 수신 ID (SDD 요구사항)
#define CAN_ID_DET_RX  0x0200

// 8 Byte 통신 페이로드를 X, Y 좌표로 분해하는 공용체
typedef union {
    uint8_t buffer[8];
    struct {
        // 오리지널 좌표 변환 방식이 적용된 직교 좌표 (mm 단위)
        int32_t x_mm;
        int32_t y_mm;
    } targetPos;
} TargetPayload_t;

// 외부에서 접근할 수 있는 공유 메모리 선언 (extern)
extern volatile int32_t g_Target_X_mm;
extern volatile int32_t g_Target_Y_mm;
extern volatile uint8_t g_NewTarget_Flag;

// 함수 프로토타입
void ECS_CAN_Filter_Init(void);  // CAN 필터 초기화
void ECS_CAN_ParseRxMessage(uint32_t rxId, uint8_t* rxData); // 수신 데이터 파싱

#endif /* INC_ECS_CAN_H_ */

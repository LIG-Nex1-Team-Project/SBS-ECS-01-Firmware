/*
 * ecs_can.h
 *
 *  Created on: Feb 26, 2026
 *      Author: SeungMin
 */

#ifndef INC_ECS_CAN_H_
#define INC_ECS_CAN_H_

#include <stdint.h>

// [SDD 참조] Extended CAN ID (29-bit)
#define CAN_ID_DET_TX  0x00000100  // ECS -> 탐색기 제어 명령 송신
#define CAN_ID_DET_RX  0x00000200  // 탐색기 -> ECS 표적 제원 수신
#define CAN_ID_LTL_TX  0x00000300  // ECS -> 발사대 제어 명령 송신
#define CAN_ID_LTL_RX  0x00000400  // 발사대 -> ECS 상태 수신

// 탐색기 제어 명령 열거형
typedef enum {
    DET_CMD_OP     = 0x01, // 운용 모드
    DET_CMD_STANDBY = 0x02, // 대기 모드 (탐색 시작)
    DET_CMD_RESET  = 0x03  // 리셋 (탐색 중지)
} DetCommand_e;

// 발사대 제어 명령 열거형
typedef enum {
    LTL_CMD_ALIGN     = 0x00, // 정렬 명령
    LTL_CMD_FIRE      = 0x01, // 발사 명령
    LTL_CMD_EMERGENCY = 0x02  // 긴급 중지
} LtlCommand_e;

// 발사대 상태 열거형
typedef enum {
    LTL_STATUS_ALIGN_DONE = 0x01, // 정렬 완료
    LTL_STATUS_FIRE_DONE  = 0x02, // 발사 완료
    LTL_STATUS_ERROR      = 0x03  // 고장 상태
} LtlStatus_e;

// 수신용(DET) 페이로드 (Target_X_Coord, Target_Y_Coord)
typedef union {
    uint8_t buffer[8];
    struct {
        float x_mm; // 4 Byte
        float y_mm; // 4 Byte
    } targetPos;
} TargetPayload_t;

// 송신용(LTL) 페이로드 (Target Degree, Fire Command)
typedef union {
    uint8_t buffer[8];
    struct {
        float targetAngle_deg;   // 1~4 byte
        uint8_t commandMode;     // 5 byte
        uint8_t reserved[3];     // 6~8 byte (0x00 패딩)
    } controlData;
} LauncherPayload_t;

// 외부 공유 메모리
extern volatile float g_Target_X_mm;
extern volatile float g_Target_Y_mm;
extern volatile uint8_t g_NewTarget_Flag;
extern volatile uint8_t g_Launcher_Status; // 발사대 상태 변수 추가

// 함수 프로토타입
void ECS_CAN_Filter_Init(void);
void ECS_CAN_ParseRxMessage(uint32_t rxId, uint8_t* rxData);
void ECS_CAN_SendToLauncher(float angle, LtlCommand_e cmd);
void ECS_CAN_SendToSeeker(DetCommand_e cmd); // 탐색기 제어 함수 추가

#endif /* INC_ECS_CAN_H_ */

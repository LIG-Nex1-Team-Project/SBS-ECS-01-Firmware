/*
 * ecs_math.c
 *
 *  Created on: Feb 28, 2026
 *      Author: SeungMin
 */


#include "ecs_math.h"
#include <math.h>

float ECS_Math_CalAngle(float targetX_mm, float targetY_mm) {
    // 1. 발사대 기준의 상대 좌표로 변환
    // 탐색기가 (0,0)일 때 발사대는 (250, 0)에 있음.
    // 따라서 발사대에서 본 타겟의 상대 좌표는 아래와 같음.
    float relativeX = targetX_mm - 250.0f;
    float relativeY = targetY_mm;

    // 2. 아크탄젠트 계산 (표준 atan2f(y, x))
    // relativeY는 전방 거리, relativeX는 좌우 편차
    float angleRad = atan2f(relativeY, relativeX);
    float angleDeg = angleRad * (180.0f / 3.141592f);

    // 3. 서보 매핑
    // atan2f(500, -250)은 약 116.5도를 반환합니다.
    // 질문자님의 시스템(오른쪽 0도, 정면 90도)은 수학적 표준 좌표계와 일치합니다!
    // 따라서 별도의 90도 덧셈/뺄셈 없이 이 각도 자체가 서보 각도가 됩니다.
    float finalServoAngle = angleDeg;

    // 4. 안전 범위 제한 (0~180도)
    if (finalServoAngle < 0.0f) finalServoAngle = 0.0f;
    if (finalServoAngle > 180.0f) finalServoAngle = 180.0f;

    return finalServoAngle;
}

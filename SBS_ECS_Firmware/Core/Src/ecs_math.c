/*
 * ecs_math.c
 *
 *  Created on: Feb 28, 2026
 *      Author: SeungMin
 */


#include "ecs_math.h"
#include <math.h>

float ECS_Math_CalAngle(float targetX_mm, float targetY_mm) {
    // 1. 발사대 기준 상대 좌표(Delta) 계산
    float deltaX = targetX_mm - LAUNCHER_OFFSET_X;
    float deltaY = targetY_mm - LAUNCHER_OFFSET_Y;

    // 2. 아크탄젠트(atan2f) 함수로 라디안 산출 및 Degree 변환 (float 전용 함수 사용)
    float angleRad = atan2f(deltaY, deltaX);
    float angleDeg = angleRad * (180.0f / MATH_PI);

    // 3. 음수 각도 보정 (서보모터 구동 범위 0~360도)
    if (angleDeg < 0.0f) {
        angleDeg += 360.0f;
    }

    return angleDeg;
}

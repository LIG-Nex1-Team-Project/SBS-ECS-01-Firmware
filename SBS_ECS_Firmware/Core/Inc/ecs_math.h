/*
 * ecs_math.h
 *
 *  Created on: Feb 28, 2026
 *      Author: SeungMin
 */

#ifndef INC_ECS_MATH_H_
#define INC_ECS_MATH_H_

// 발사대의 물리적 위치 오프셋 (탐색기 원점 기준, mm 단위)
// 예시: 발사대가 탐색기 우측으로 500mm 떨어져 있다고 가정
#define LAUNCHER_OFFSET_X  500.0f
#define LAUNCHER_OFFSET_Y  0.0f

#define MATH_PI  3.14159265358979323846f

// 오리지널 좌표 변환 방식을 적용한 방위각 산출 함수
float ECS_Math_CalAngle(float targetX_mm, float targetY_mm);

#endif /* INC_ECS_MATH_H_ */

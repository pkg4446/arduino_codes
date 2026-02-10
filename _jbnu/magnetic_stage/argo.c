#include <stdio.h>
#include <math.h>

// 각 축의 현재 위치
float current_position_x = 0.0;
float current_position_y = 0.0;
float current_position_z = 0.0;

// 목표 위치
float target_position_x = 10.0;
float target_position_y = 100.0;
float target_position_z = 1000.0;

// 각 축의 최소 이동 거리
float min_step_x = 0.1;
float min_step_y = 0.1;
float min_step_z = 0.1;

// 이동 속도
float speed = 1.0; // 일정한 속도

// 각 축의 이동 거리 계산
float distance_x, distance_y, distance_z;

int main() {
    // 각 축의 이동 거리 계산
    distance_x = target_position_x - current_position_x;
    distance_y = target_position_y - current_position_y;
    distance_z = target_position_z - current_position_z;

    // 각 축의 이동 방향 계산
    float direction_x = (distance_x >= 0) ? 1 : -1;
    float direction_y = (distance_y >= 0) ? 1 : -1;
    float direction_z = (distance_z >= 0) ? 1 : -1;

    // 각 축의 이동 속도 계산
    float speed_x = speed * direction_x;
    float speed_y = speed * direction_y;
    float speed_z = speed * direction_z;

    // 목표 위치에 도달할 때까지 반복
    while (fabs(distance_x) >= min_step_x ||
           fabs(distance_y) >= min_step_y ||
           fabs(distance_z) >= min_step_z) {
        // 현재 위치 업데이트
        if (fabs(distance_x) >= min_step_x)
            current_position_x += speed_x;
        if (fabs(distance_y) >= min_step_y)
            current_position_y += speed_y;
        if (fabs(distance_z) >= min_step_z)
            current_position_z += speed_z;

        // 각 축의 이동 거리 재계산
        distance_x = target_position_x - current_position_x;
        distance_y = target_position_y - current_position_y;
        distance_z = target_position_z - current_position_z;

        // 이동한 위치 출력
        printf("Current Position: (%.2f, %.2f, %.2f)\n", current_position_x, current_position_y, current_position_z);
    }

    printf("Target Position Reached!\n");

    return 0;
}

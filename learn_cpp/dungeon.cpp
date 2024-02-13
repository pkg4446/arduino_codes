#include "dungeon.h"

void mapClass::init(void) {
    enter_y = 1;
    exit_y  = 1;
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
        for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
            maze[index_y][index_x] = load;
        }
    }
}
void mapClass::view(void) {
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
        for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
            Serial.print(maze[index_y][index_x]);
            if(index_x != MAP_X-1)Serial.print(",");
        }
        Serial.println();
    }
}

bool mapClass::pathfinder(void) {
    // 초기화
  memset(visited, false, sizeof(visited));

  Point queue[MAP_Y * MAP_X]; // 큐로 사용할 배열
  int front = 0; // 큐의 앞
  int rear = 0; // 큐의 뒤

  // 시작점을 큐에 추가
  queue[rear++] = {enter_y, 0};
  visited[enter_y][0] = true;

  while (front < rear) {
    Point current = queue[front++];
    int row = current.row;
    int col = current.col;

    // 목적지에 도착한 경우
    if (row == exit_y && col == MAP_X-1) return true;

    // 상하좌우로 이동하며 경로 찾기
    if (row - 1 >= 0 && !visited[row - 1][col] && maze[row - 1][col] != wall) { // 위로 이동
      queue[rear++] = {row - 1, col};
      visited[row - 1][col] = true;
    }
    if (row + 1 < MAP_Y && !visited[row + 1][col] && maze[row + 1][col] != wall) { // 아래로 이동
      queue[rear++] = {row + 1, col};
      visited[row + 1][col] = true;
    }
    if (col - 1 >= 0 && !visited[row][col - 1] && maze[row][col - 1] != wall) { // 왼쪽으로 이동
      queue[rear++] = {row, col - 1};
      visited[row][col - 1] = true;
    }
    if (col + 1 < MAP_X && !visited[row][col + 1] && maze[row][col + 1] != wall) { // 오른쪽으로 이동
      queue[rear++] = {row, col + 1};
      visited[row][col + 1] = true;
    }
  }
  // 목적지에 도달하지 못한 경우
  return false;
}
void mapClass::rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types) {
    if(types == wall && axis_y == enter_y &&(axis_x == 0 || axis_x == MAP_X-1)) return;
    maze[axis_x][axis_y] = types;
}
uint8_t mapClass::get(uint8_t axis_x, uint8_t axis_y) {
    return maze[axis_x][axis_y];
}

mapClass playmap;
#include "dungeon.h"

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

void mapClass::init(void) {
  enter_y = (MAP_Y-1)/2;
  exit_y  = (MAP_Y-1)/2;
  memset(maze, load, sizeof(maze));
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
void mapClass::rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types) {
  if(types == wall && axis_y == enter_y &&(axis_x == 0 || axis_x == MAP_X-1)) return;
  uint8_t previuos_type = maze[axis_y][axis_x];
  maze[axis_y][axis_x]  = types;
  if(!pathfinder()){
    maze[axis_y][axis_x] = previuos_type;
    Serial.println(get_progmem(path_err));
  }
}
uint8_t mapClass::get(uint8_t axis_x, uint8_t axis_y) {
  return maze[axis_y][axis_x];
}
uint8_t mapClass::get_enter(void) {
  return enter_y;
}
uint8_t mapClass::get_exit(void) {
  return exit_y;
}

/**************************************************************************************/
uint8_t moveClass::init(uint8_t enter) {
  memset(visited, false, sizeof(visited));
  act_point = random(MAP_Y*MAP_X,MAP_Y*MAP_X*2);
  back_move = false;
  gps_x = 0;
  gps_y = enter;
  return act_point;
}

void moveClass::moving(uint8_t (*maze)[MAP_X]) {
  if(act_point == 0) return;
  else act_point-=1;
  if(back_move){
    visited[gps_y][gps_x] = true;
	  if(gps_x>target_x && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }else if(gps_x<target_x && maze[gps_y][gps_x+1]!=wall){
      gps_x+=1;
    }else if(gps_y>target_y && maze[gps_y-1][gps_x]!=wall){
      gps_y-=1;
    }else if(gps_y<target_y && maze[gps_y+1][gps_x]!=wall){
      gps_y+=1;
    }else if(gps_x==target_x && gps_y==target_y){
      back_move = false;
    }
  }else{
    visited[gps_y][gps_x] = true;
    //move direction 0=x-1,1=x+1,2=y-1,3=y+1.
    uint8_t move_direction = random(4);

    if(gps_x == MAP_X-1){
      Serial.println("Incounter!");
    }

    else if(move_direction == 0 && gps_x>0 && !visited[gps_y][gps_x-1] && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }else if(move_direction == 0 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && maze[gps_y][gps_x+1]!=wall){
      gps_x+=1;
    }else if(move_direction == 0 && gps_y>0 && !visited[gps_y-1][gps_x] && maze[gps_y-1][gps_x]!=wall){
      gps_y-=1;
    }else if(move_direction == 0 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && maze[gps_y+1][gps_x]!=wall){
      gps_y+=1;
    }

    else if(move_direction == 1 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && maze[gps_y][gps_x+1]!=wall){
      gps_x+=1;
    }else if(move_direction == 1 && gps_x>0 && !visited[gps_y][gps_x-1] && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }else if(move_direction == 1 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && maze[gps_y+1][gps_x]!=wall){
      gps_y+=1;
    }else if(move_direction == 1 && gps_y>0 && !visited[gps_y-1][gps_x] && maze[gps_y-1][gps_x]!=wall){
      gps_y-=1;
    }

    else if(move_direction == 2 && gps_y>0 && !visited[gps_y-1][gps_x] && maze[gps_y-1][gps_x]!=wall){
      gps_y-=1;
    }else if(move_direction == 2 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && maze[gps_y+1][gps_x]!=wall){
      gps_y+=1;
    }else if(move_direction == 2 && gps_x>0 && !visited[gps_y][gps_x-1] && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }else if(move_direction == 2 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && maze[gps_y][gps_x+1]!=wall){
      gps_x+=1;
    }
    
    else if(move_direction == 3 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && maze[gps_y+1][gps_x]!=wall){
      gps_y+=1;
    }else if(move_direction == 3 && gps_y>0 && !visited[gps_y-1][gps_x] && maze[gps_y-1][gps_x]!=wall){
      gps_y-=1;
    }else if(move_direction == 3 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && maze[gps_y][gps_x+1]!=wall){
      gps_x+=1;
    }else if(move_direction == 3 && gps_x>0 && !visited[gps_y][gps_x-1] && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }

    else{
      Serial.println("stuck");
      if(gps_x>0 && visited[gps_y][gps_x-1]){
        for(uint8_t index_x=gps_x; index_x>=0; index_x--){
          uint8_t not_wall_num = 0;
          for(uint8_t index_y=0; index_y<MAP_Y; index_y++){
            if(maze[index_y][index_x]!=wall) not_wall_num+=1;
          }
          if(not_wall_num>2){
            target_x = index_x;
            break;
          }
        }
      }else{
        for(uint8_t index_x=gps_x; index_x<MAP_X-1; index_x++){
          uint8_t not_wall_num = 0;
          for(uint8_t index_y=0; index_y<MAP_Y; index_y++){
            if(maze[index_y][index_x]!=wall) not_wall_num+=1;
          }
          if(not_wall_num>2){
            target_x = index_x;
            break;
          }
        }
      }
      for(uint8_t index_y=0; index_y<MAP_Y; index_y++){
        if(maze[index_y][target_x+1]!=wall && !visited[index_y][target_x+1]) target_y=index_y;
      }
      back_move = true;
      memset(visited, false, sizeof(visited));
    }
  }
}

bool moveClass::event(void) {
  return true;
}

/**************************************************************************************/
mapClass playmap;
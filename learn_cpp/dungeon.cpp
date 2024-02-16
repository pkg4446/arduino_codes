#include "dungeon.h"

void mapClass::init(void) {
  enter_y = (MAP_Y-1)/2;
  exit_y  = (MAP_Y-1)/2;
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
  maze[axis_y][axis_x] = types;
}
uint8_t mapClass::get(uint8_t axis_x, uint8_t axis_y) {
  return maze[axis_y][axis_x];
}
uint8_t mapClass::get_enter(void) {
  return enter_y;
}
/**************************************************************************************/
void moveClass::init(uint8_t enter) {
  memset(visited, false, sizeof(visited));
  act_point = random(8,48);
  back_move = false;
  gps_x = 0;
  gps_y = enter;
}

void moveClass::moving(uint8_t (*maze)[MAP_X]) {
  if(back_move){
	  if(gps_x>target_x && maze[gps_y][gps_x-1]!=wall){
      gps_x-=1;
    }else{
      if(gps_x==target_x && gps_y==target_y){
        back_move = false;
      }else if(gps_x==target_x){
        if(gps_y>target_y)  gps_y-=1;
        else                gps_y+=1;
      }else if(gps_y == 1 && maze[0][gps_x]!=wall){
        gps_y++;
      }else if(gps_y == 1 && maze[2][gps_x]!=wall){
        gps_y--;
      }else if(gps_y==0 && maze[1][gps_x]!=wall){
        gps_y++;
      }else if(gps_y==2 && maze[1][gps_x]!=wall){
        gps_y--;
      }
    }
  }else{
    //move direction 0=x-1,1=x+1,2=y-1,3=y+1.
    uint8_t move_direction = random(4);
    /*
    if(move_direction == 0 && gps_x>0 && !visited[gps_y][gps_x-1] && !visited[gps_y][gps_x-1]!=wall){
      visited[gps_y][gps_x-1] = true;
      gps_x-=1;
    }else if(move_direction == 1 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && !visited[gps_y][gps_x+1]!=wall){
      visited[gps_y][gps_x+1] = true;
      gps_x+=1;
    }else if(move_direction == 2 && gps_y>0 && !visited[gps_y-1][gps_x] && !visited[gps_y-1][gps_x]!=wall){
      visited[gps_y-1][gps_x] = true;
      gps_y-=1;
    }else if(move_direction == 3 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && !visited[gps_y+1][gps_x]!=wall){
      visited[gps_y+1][gps_x] = true;
      gps_y+=1;
    }
    */
    if(move_direction == 0 && gps_x>0 && !visited[gps_y][gps_x-1] && !visited[gps_y][gps_x-1]!=wall){
      visited[gps_y][gps_x-1] = true;
      gps_x-=1;
    }else if(move_direction == 0 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && !visited[gps_y][gps_x+1]!=wall){
      visited[gps_y][gps_x+1] = true;
      gps_x+=1;
    }else if(move_direction == 0 && gps_y>0 && !visited[gps_y-1][gps_x] && !visited[gps_y-1][gps_x]!=wall){
      visited[gps_y-1][gps_x] = true;
      gps_y-=1;
    }else if(move_direction == 0 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && !visited[gps_y+1][gps_x]!=wall){
      visited[gps_y+1][gps_x] = true;
      gps_y+=1;
    }

    else if(move_direction == 1 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && !visited[gps_y][gps_x+1]!=wall){
      visited[gps_y][gps_x+1] = true;
      gps_x+=1;
    }else if(move_direction == 1 && gps_x>0 && !visited[gps_y][gps_x-1] && !visited[gps_y][gps_x-1]!=wall){
      visited[gps_y][gps_x-1] = true;
      gps_x-=1;
    }else if(move_direction == 1 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && !visited[gps_y+1][gps_x]!=wall){
      visited[gps_y+1][gps_x] = true;
      gps_y+=1;
    }else if(move_direction == 1 && gps_y>0 && !visited[gps_y-1][gps_x] && !visited[gps_y-1][gps_x]!=wall){
      visited[gps_y-1][gps_x] = true;
      gps_y-=1;
    }

    else if(move_direction == 2 && gps_y>0 && !visited[gps_y-1][gps_x] && !visited[gps_y-1][gps_x]!=wall){
      visited[gps_y-1][gps_x] = true;
      gps_y-=1;
    }else if(move_direction == 2 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && !visited[gps_y+1][gps_x]!=wall){
      visited[gps_y+1][gps_x] = true;
      gps_y+=1;
    }else if(move_direction == 2 && gps_x>0 && !visited[gps_y][gps_x-1] && !visited[gps_y][gps_x-1]!=wall){
      visited[gps_y][gps_x-1] = true;
      gps_x-=1;
    }else if(move_direction == 2 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && !visited[gps_y][gps_x+1]!=wall){
      visited[gps_y][gps_x+1] = true;
      gps_x+=1;
    }
    
    else if(move_direction == 3 && gps_y<MAP_Y-1 && !visited[gps_y+1][gps_x] && !visited[gps_y+1][gps_x]!=wall){
      visited[gps_y+1][gps_x] = true;
      gps_y+=1;
    }else if(move_direction == 3 && gps_y>0 && !visited[gps_y-1][gps_x] && !visited[gps_y-1][gps_x]!=wall){
      visited[gps_y-1][gps_x] = true;
      gps_y-=1;
    }else if(move_direction == 3 && gps_x<MAP_X-1 && !visited[gps_y][gps_x+1] && !visited[gps_y][gps_x+1]!=wall){
      visited[gps_y][gps_x+1] = true;
      gps_x+=1;
    }else if(move_direction == 3 && gps_x>0 && !visited[gps_y][gps_x-1] && !visited[gps_y][gps_x-1]!=wall){
      visited[gps_y][gps_x-1] = true;
      gps_x-=1;
    }

    else if(gps_x == MAP_X-1){
      //goal
    }else if(gps_x == 0){
      //run out
    }else{
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
      for(uint8_t index_y=0; index_y<MAP_Y; index_y++){
        if(maze[index_y][target_x+1]!=wall && !visited[index_y][target_x+1]) target_y=index_y;
      }
      back_move = true;
    }
  }
}

bool moveClass::event(void) {
  return true;
}

/**************************************************************************************/
mapClass playmap;
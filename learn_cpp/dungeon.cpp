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

void mapClass::parse_map(String  text){
    uint8_t text_end = 11;
    text_end += text.length()/3;
    Serial.print(text);
    for(uint8_t index=text.length(); index<text_end; index++){
        Serial.print(" ");
    }
}

void mapClass::save_csv(void) {
  String csv_file = "";
  for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
    String csv_x = "";
    for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
      make_csv(&csv_x,String(maze[index_y][index_x]));
    }
    make_csv_text(&csv_file,csv_x);
  }
  String csv_door = "";
  make_csv(&csv_door,String(enter_y));
  make_csv(&csv_door,String(exit_y));
  make_csv_text(&csv_file,csv_door);
  file_write(path_config()+file_map(), csv_file);
}

void mapClass::init(void) {
  enter_y = (MAP_Y-1)/2;
  exit_y  = (MAP_Y-1)/2;
  memset(maze, road, sizeof(maze));
  if(exisits_check(path_config()+file_map())){
    String csv_file_str = file_read(path_config()+file_map());
    char *csv_file      = const_cast<char*>(csv_file_str.c_str());
    char *csv_text[MAP_Y+1];
    csv_text[0] = strtok(csv_file, "\n");
    for(uint8_t index=1; index<=MAP_Y; index++){
      csv_text[index] = strtok(0x00, "\n");
    }
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
      maze[index_y][0] = atoi(strtok(csv_text[index_y], ","));
      for (uint8_t index_x = 1; index_x < MAP_X; index_x++){
        maze[index_y][index_x] = atoi(strtok(0x00, ","));
      }
    }
    enter_y = atoi(strtok(csv_text[MAP_Y], ","));
    exit_y  = atoi(strtok(0x00, ","));
  }
}
void mapClass::view(uint8_t axis_x, uint8_t axis_y) {
  paging();
  for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
    for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
      String  text="";
      if(index_x==axis_x && index_y==axis_y)    text = get_progmem(word_now);
      if(index_x==MAP_X-1 && index_y==exit_y)   text += get_progmem(word_core);
      else if(index_x==0  && index_y==enter_y)  text += get_progmem(word_enter);
      else if(maze[index_y][index_x] == wall)   text += get_progmem(word_obstruct);
      else if(maze[index_y][index_x] == road)   text += get_progmem(word_waylay);
      else if(maze[index_y][index_x] == trap)   text += get_progmem(word_trap);
      else if(maze[index_y][index_x] == prison) text += get_progmem(word_prison);
      else if(maze[index_y][index_x] == spa)    text += get_progmem(word_spa);
      else if(maze[index_y][index_x] == inn)    text += get_progmem(word_inn);
      else if(maze[index_y][index_x] == farm)   text += get_progmem(word_farm);
      else if(maze[index_y][index_x] == cage)   text += get_progmem(word_cage);
      parse_map(text);
    }
    Serial.println();
  }
}
void mapClass::pos_move(uint8_t *axis_x, uint8_t *axis_y, uint16_t direction) {
  if(direction == COMMAND_MOVE8      && *axis_y>0) *axis_y -= 1;
  else if(direction == COMMAND_MOVE4 && *axis_x>0) *axis_x -= 1;
  else if(direction == COMMAND_MOVE6 && *axis_x<MAP_X) *axis_x += 1;
  else if(direction == COMMAND_MOVE2 && *axis_y<MAP_Y) *axis_y += 1;
  view(*axis_x,*axis_y);
}
void mapClass::rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types) {
  if(maze[axis_y][axis_x] == types || (types == wall && axis_y == enter_y &&(axis_x == 0 || axis_x == MAP_X-1))) return;
  uint8_t previuos_type = maze[axis_y][axis_x];
  maze[axis_y][axis_x]  = types;
  if(pathfinder()){
    save_csv();
  }else{
    maze[axis_y][axis_x] = previuos_type;
    Serial.println(get_progmem(path_err));
  }
}
uint8_t mapClass::get(uint8_t axis_x, uint8_t axis_y) {
  return maze[axis_y][axis_x];
}
void mapClass::put_enter(uint8_t axis_y) {
  uint8_t previuos_y = enter_y;
  enter_y = axis_y;
  if(pathfinder()){
    save_csv();
  }else{
    enter_y = previuos_y;
  }
}
void mapClass::put_exit(uint8_t axis_y) {
  uint8_t previuos_y = exit_y;
  exit_y  = axis_y;
  if(pathfinder()){
    save_csv();
  }else{
    exit_y = previuos_y;
  }
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

uint8_t moveClass::moving(uint8_t (*maze)[MAP_X], uint8_t exit_y) {
  if(act_point == 0) return COMMAND_CANCLE;
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

    if(gps_x == MAP_X-1 && gps_y == exit_y){ return COMMAND_MAIN;
    }else if(move_direction == 0 && gps_x>0 && !visited[gps_y][gps_x-1] && maze[gps_y][gps_x-1]!=wall){
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
      //stuck
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
  return  maze[gps_y][gps_x];
}

/**************************************************************************************/
mapClass playmap;
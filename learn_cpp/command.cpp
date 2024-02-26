#include "command.h"

void command_dos(char *command_buf, String *path, bool *exit){
  String temp_text = "";
  for(uint8_t index_check=3; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x00) break;
    temp_text += command_buf[index_check];
  }
  if(command_buf[0]=='h' && command_buf[1]=='e' && command_buf[2]=='l' && command_buf[3]=='p'){
    display_help_cmd();
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]==0x20){
    if(exisits_check(*path+path_slash()+temp_text)){
      *path += path_slash()+temp_text;
    }
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]=='/'){
    *path = "";
  }else if(command_buf[0]=='l' && command_buf[1]=='s'){
    if(command_buf[2] != 0x00) dir_list(*path+path_slash()+temp_text,true,true);
    else if(*path=="")  dir_list("/",true,true);
    else dir_list(*path,true,true);
  }else if(command_buf[0]=='m' && command_buf[1]=='d'){
    dir_make(*path+path_slash()+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='d'){
    dir_remove(*path+path_slash()+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='f'){
    file_remove(*path+path_slash()+temp_text);
  }else if(command_buf[0]=='o' && command_buf[1]=='p'){
    Serial.println(file_read(*path+path_slash()+temp_text));
  }else if(command_buf[0]=='r' && command_buf[1]=='f'){
    file_remove(*path+path_slash()+temp_text);
  }else if(command_buf[0]=='e' && command_buf[1]=='x' && command_buf[2]=='i' && command_buf[3]=='t'){
    *exit = false;
    display_boot();
  }
};
/*************************************************************/
void cmd_chs_victim(uint16_t *scene_number, uint8_t model_max_num){
  display_victim(scene_number);
  for(uint8_t index=1; index<=model_max_num; index++){
    space_option(true,index,get_model_name(path_captive()+path_slash()+dir_index(path_captive(),true,index)));
    if(index%3 == 0)Serial.println();
  }
}
bool path_victim_chk(String path){
  char *path_check = const_cast<char*>(path.c_str());
  strtok(path_check, "/");
  String check_path = strtok(0x00, "/");
  return check_path == check_captive();
}
void cmd_dng_manage(uint16_t *scene_number, String *path){
  if(path_victim_chk(*path)){
    display_management(scene_number, get_model_name(*path));
  }else{
    uint8_t model_max_num = dir_list(path_captive(),true,false);
    if(model_max_num == 0){
      display_no_victim();
    }else if(model_max_num == 1){
      *path = path_captive() + path_slash() + dir_index(path_captive(),true,model_max_num);
      display_management(scene_number, get_model_name(*path));
    }else cmd_chs_victim(scene_number, model_max_num);
  }
};
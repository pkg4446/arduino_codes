#include <Arduino.h>

void err_msg(HardwareSerial *uart, char *msg);
void command_help(HardwareSerial *uart);
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass);
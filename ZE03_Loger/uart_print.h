#include <Arduino.h>

void serial_command_help(HardwareSerial *uart);
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass);
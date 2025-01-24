#include "uart_print.h"

void serial_err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void serial_command_help(HardwareSerial *uart) {
  uart->println("************* help *************");
  uart->println("reboot   * system reboot");
  uart->println("sensor   * sensor read");
  uart->println("ssid     * ex)ssid your ssid");
  uart->println("pass     * ex)pass your password");
  uart->println("wifi     * WIFI connet");
  uart->println("   scan  * WIFI scan");
  uart->println("   stop  * WIFI disconnet");
  uart->println("help     * this text");
  uart->println("************* help *************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
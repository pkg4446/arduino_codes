#include "uart_print.h"

void nextion_print(HardwareSerial *uart, String cmd) {
  uart->print(cmd);
  uart->write(0xFF);
  uart->write(0xFF);
  uart->write(0xFF);
}

void nextion_display(String IDs, uint16_t values, HardwareSerial *uart) {
  String cmd;
  char buf[8] = {0};
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  nextion_print(uart,cmd);
}

void serial_err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void serial_command_help(HardwareSerial *uart) {
  uart->println("************* help *************");
  uart->println("reboot  * system reboot");
  uart->println("manual  * relay onoff");
  uart->println("page    * moniter page change");
  uart->println("send    * command send to moniter");
  uart->println("run     * ");
  uart->println("stp     * stop");
  uart->println("memo    * ");
  uart->println("minute  * runtime set");
  uart->println("seconde * runtime set");
  uart->println("ssid    * ex)ssid your ssid");
  uart->println("pass    * ex)pass your password");
  uart->println("wifi    * WIFI connet");
  uart->println("   scan * WIFI scan");
  uart->println("   stop * WIFI disconnet");
  uart->println("help    * this text");
  uart->println("************* help *************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
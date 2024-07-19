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
  uart->println("send    * command send to moniter");
  uart->println("wing    * house open&close");
  uart->println("set     * system setup");
  uart->println("   temp * water a contole");
  uart->println("   circul_i,h circul_o");
  uart->println("     ena   * enable set");
  uart->println("     run   * run time");
  uart->println("     stp   * puase time");
  uart->println("config  * read system setup");
  uart->println("temp    * temperature show");
  uart->println("help    * this text");
  uart->println("************* help *************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
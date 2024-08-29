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
  uart->println("time    * show system time");
  uart->println("timeset * change system time");
  uart->println("manual  * manual num : relay on-off");
  uart->println("reboot  * system reboot");
  uart->println("send    * command send to moniter");
  uart->println("wing    * on, off");
  uart->println("set     * system setup");
  uart->println("   water   * water a contole");
  uart->println("   liquid, line_a,b, wat_h, liq_h");
  uart->println("   lamp_a,b,c,d, circul_i,o, temp");
  uart->println("     ena   * enable set");
  uart->println("     run   * run time");
  uart->println("     stp   * puase time");
  uart->println("debug   * on, off: systme message");
  uart->println("config  * read system setup");
  uart->println("ssid    * ex)ssid your ssid");
  uart->println("pass    * ex)pass your password");
  uart->println("wifi    * WIFI connet");
  uart->println("   scan * WIFI scan");
  uart->println("   stop * WIFI disconnet");
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
#include "uart_print.h"

void serial_err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void serial_command_help(HardwareSerial *uart) {
  uart->println("************* help *************");
  uart->println("reboot  * system reboot");
  uart->println("temp    * temperature show");
  uart->println("run     * homeothermy mode");
  uart->println("   on   *");
  uart->println("   off  *");
  uart->println("set     * temperature goal setup. ex) set 25");
  uart->println("config  * read temperature setup");
  uart->println("gap     * set upload interval");
  uart->println("test    *");
  uart->println("   mode * MOSFET manual mode");
  uart->println("   on   * test on");
  uart->println("   off  * test off");
  uart->println("   data * data to server");
  uart->println("ssid    * ex)ssid your ssid");
  uart->println("pass    * ex)pass your password");
  uart->println("wifi    * WIFI connet");
  uart->println("   scan * WIFI scan");
  uart->println("   stop * WIFI disconnet");
  uart->println("firm    * firmware update");
  uart->println("********************************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
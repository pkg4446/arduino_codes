#include "uart_print.h"

void serial_err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void serial_command_help(HardwareSerial *uart) {
  uart->println("************* LED *************");
  uart->println("1. WIFI connetion");
  uart->println("2. Manual mode");
  uart->println("3. Homeothermy mode");
  uart->println("4. Heater work");
  uart->println("************* help *************");
  uart->println("reboot  * system reboot");
  uart->println("temp    * temperature show");
  uart->println("run     * homeothermy mode");
  uart->println("   on   *");
  uart->println("   off  *");
  uart->println("set     * temperature goal setup, 0~4,5 is all . ex) set 0 25");
  uart->println("config  * read temperature setup");
  uart->println("gap     * set upload interval");
  uart->println("test    *");
  uart->println("   mode * MOSFET manual mode");
  uart->println("   on   * test on  0~4,5 is all");
  uart->println("   off  * test off 0~4,5 is all");
  uart->println("   data * data to server");
  uart->println("time    * show system time");
  uart->println("timeset * change system time, WIFI require");
  uart->println("ssid    * ex)ssid your ssid");
  uart->println("pass    * ex)pass your password");
  uart->println("wifi    * WIFI connet");
  uart->println("   scan * WIFI scan");
  uart->println("   stop * WIFI disconnet");
  uart->println("********************************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
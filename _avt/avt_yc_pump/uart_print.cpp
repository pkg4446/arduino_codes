#include "uart_print.h"

void serial_err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void serial_command_help(HardwareSerial *uart) {
  uart->println("************* LED *************");
  uart->println("1. WIFI connetion");
  uart->println("2. Valve A");
  uart->println("3. Valve B");
  uart->println("************* help *************");
  uart->println("reboot  * system reboot");
  uart->println("gap     * set upload interval");
  uart->println("time    * show system time");
  uart->println("sensor  * sht, water level show");
  uart->println("Valve   *");
  uart->println("      a * ex) Valve a 09 13");
  uart->println("      b * ex) Valve b 18 21");
  uart->println("heat    * ex) heat 20 08");
  uart->println("test    *");
  uart->println("   mode * MOSFET manual mode");
  uart->println("   on   * test on  0~4");
  uart->println("   off  * test off 0~4,5 is all");
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
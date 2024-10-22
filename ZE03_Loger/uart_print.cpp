#include "uart_print.h"

void serial_command_help(HardwareSerial *uart) {
  uart->println("************* help *************");
  uart->println("reboot  * system reboot");
  uart->println("show    * ZE03 data show");
  uart->println("post    * ZE03 data post to server");
  uart->println("set     * set config");
  uart->println("   interval * post interval");
  uart->println("   farm     * farm code");
  uart->println("   sens     * sensor code");  
  uart->println("config  * read setup");
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
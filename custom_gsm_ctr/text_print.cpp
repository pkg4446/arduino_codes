#include "text_print.h"

void err_msg(HardwareSerial *uart, char *msg){
  uart->print("wrong cmd: ");
  uart->println(msg);
}
void command_help(HardwareSerial *uart) {
  uart->println("************* help *************");
  uart->println("time    * show system time");
  uart->println("timeset * change system time");
  uart->println("temp    * temperature show");
  uart->println("set     * systme setup");
  uart->println("   water_a * water a contole");
  uart->println("   water_b,h lamp_a,b,c, circul, temp");
  uart->println("     ena   * enable set");
  uart->println("     run   * run time");
  uart->println("     stp   * puase time");
  uart->println("ssid    * ex)ssid your ssid");
  uart->println("pass    * ex)pass your password");
  uart->println("wifi    * WIFI connet");
  uart->println("   scan * WIFI scan");
  uart->println("   stop * WIFI disconnet");
  uart->println("reboot  * system reboot");
  uart->println("help    * this text");
  uart->println("************* help *************");
}
void serial_wifi_config(HardwareSerial *uart, char *ssid, char *pass){
  uart->println("********* wifi config *********");
  uart->print("your ssid: "); uart->println(ssid);
  uart->print("your pass: "); uart->println(pass);
  uart->println("********* wifi config *********");
}
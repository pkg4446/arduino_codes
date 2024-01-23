#define ENABLE_GxEPD2_GFX 0
//E-paper에서 3-컬러모듈을 사용하기위한 헤더파일
#include <GxEPD2_3C.h>
//글자를 화면에 출력하기 위한 폰트
#include <Fonts/FreeMonoBold9pt7b.h>

#include "nockanda2.h" // 2.9"  b/w/r

//GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT> display(GxEPD2_290c(/*CS=5*/ 2, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW029Z10 128x296, UC8151 (IL0373)
//GxEPD2_3C<GxEPD2_290_Z13c, GxEPD2_290_Z13c::HEIGHT> display(GxEPD2_290_Z13c(/*CS=5*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH029Z13 128x296, UC8151D
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(/*CS=5*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEM029C90 128x296, SSD1680

//검정색과 컬러부분을 구조체로 만듬!(편하게 쓸려고 만듬)
struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};

void setup()
{
  //아래 코드는 라이브러리에서 사용하라는대로 사용한다!
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display_init();
  //글자 화면에 출력하기!
  //helloWorld();
  display.setFullWindow();
  drawBitmaps3c128x296();
  display.hibernate();
  display.powerOff();
}
void loop() {};

const char HelloWorld[] = "hello world!";

void display_init()
{
  display.setRotation(3);//화면방향이 가로방향이다!
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_RED);
  }
  while (display.nextPage()); 
  do
  {
    display.fillScreen(GxEPD_BLACK);
  }
  while (display.nextPage()); 
  do
  {
    display.fillScreen(GxEPD_WHITE);
  }
  while (display.nextPage());
}

void helloWorld()
{
  //화면방향
  display.setRotation(3);//화면방향이 가로방향이다!
  display.setFont(&FreeMonoBold9pt7b);
  
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  
  display.setFullWindow();
  display.firstPage();

  do
  {
    //화면클리어-검정색글자 출력-노란색글자출력
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y-10);
    display.println(HelloWorld);
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y+10);
    display.println(HelloWorld);
  }
  while (display.nextPage());  
}

void drawBitmaps3c128x296()
{
  bitmap_pair bitmap_pairs = {Bitmap3c128x296_2_black, Bitmap3c128x296_2_red};

  //아래부분은 E-paper에 이미지를 드로잉하는 과정으로 우리가 손댈부분이 전혀없음!
  if ((display.epd2.WIDTH == 128) && (display.epd2.HEIGHT == 296) && display.epd2.hasColor)
  {
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs.black, 128, 296, GxEPD_BLACK);
      display.drawInvertedBitmap(0, 0, bitmap_pairs.red, 128, 296, GxEPD_RED);
      /*
      if (bitmap_pairs.red == WS_Bitmap3c128x296_red)
      {
        display.drawInvertedBitmap(0, 0, bitmap_pairs.red, 128, 296, GxEPD_RED);
      }
      else display.drawBitmap(0, 0, bitmap_pairs.red, 128, 296, GxEPD_RED);
      */
    }
    while (display.nextPage());
  }
}
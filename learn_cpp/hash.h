#include <Arduino.h>

#define TEXT_LENGTH 4

const PROGMEM char hash_hex1[] = "rRseEfaqQtTdwWcz";
const PROGMEM char hash_hex2[] = "xvkoiOjpuPhynbml";

class hashClass : public Print
{
  public:
    uint32_t  hash_value;
    void      init(void);
    String    result(void);
    virtual   size_t write(uint8_t);
    using     Print::write;
  private:
    String    hash_text(bool hex_num);
    String    printHash(uint32_t* hash);

    uint16_t  byteCount;
};
extern hashClass hash;
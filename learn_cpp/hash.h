#include <Arduino.h>

#ifndef Sha1_h
#define Sha1_h

#define HASH_LENGTH   16
#define BLOCK_LENGTH  64

const PROGMEM uint8_t sha1InitState[] = {
  0x01,0x23,0x45,0x67, // H0
  0x89,0xab,0xcd,0xef, // H1
  0xfe,0xdc,0xba,0x98, // H2
  0x76,0x54,0x32,0x10, // H3
  0xf0,0xe1,0xd2,0xc3  // H4
};
const PROGMEM char hash_hex1[] = "rRseEfaqQtTdwWcz";
const PROGMEM char hash_hex2[] = "xvkoiOjpuPhynbml";

class Sha1Class : public Print
{
  public:
    union _buffer {
      uint8_t b[BLOCK_LENGTH];
      uint32_t w[BLOCK_LENGTH/4];
    };
    union _state {
      uint8_t b[HASH_LENGTH];
      uint32_t w[HASH_LENGTH/4];
    };
    void      init(void);
    String    result(void);
    virtual   size_t write(uint8_t);
    using     Print::write;
  private:
    void pad();
    void addUncounted(uint8_t data);
    void hashBlock();
	
    uint32_t  rol32(uint32_t number, uint8_t bits);
    String    hash_text(bool hex_num);
    String    printHash(uint8_t* hash);
    _buffer   buffer;/**< hold the buffer for the hashing process */
    uint8_t   bufferOffset;/**< indicates the position on the buffer */
    _state    state;/**< identical structure with buffer */
    uint32_t  byteCount;/**< Byte counter in order to initialize the hash process for a block */
};
extern Sha1Class Sha1;

#endif
#include "hash.h"

#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

void Sha1Class::init(void) {
  memcpy_P(state.b,sha1InitState,HASH_LENGTH);
  byteCount = 0;
  bufferOffset = 0;
}

String Sha1Class::result(void) {
  // Pad to complete the last block
  pad();
  // Swap byte order back
  for (int i=0; i<HASH_LENGTH/4; i++) {
    uint32_t a,b;
    a=state.w[i];
    b=a<<24;
    b|=(a<<8) & 0x00ff0000;
    b|=(a>>8) & 0x0000ff00;
    b|=a>>24;
    state.w[i]=b;
  }
  // Return pointer to hash (20 characters)
  return printHash(state.b);
}

size_t Sha1Class::write(uint8_t data) {
  ++byteCount;
  addUncounted(data);
  return 1;
}

void Sha1Class::hashBlock() {
  uint8_t i;
  uint32_t a,b,c,d,e,t;

  a=state.w[0];
  b=state.w[1];
  c=state.w[2];
  d=state.w[3];
  e=state.w[4];
  for (i=0; i<80; i++) {
    if (i>=16) {
      t = buffer.w[(i+13)&15] ^ buffer.w[(i+8)&15] ^ buffer.w[(i+2)&15] ^ buffer.w[i&15];
      buffer.w[i&15] = rol32(t,1);
    }
    if (i<20) {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    } else if (i<40) {
      t = (b ^ c ^ d) + SHA1_K20;
    } else if (i<60) {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    } else {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t+=rol32(a,5) + e + buffer.w[i&15];
    e=d;
    d=c;
    c=rol32(b,30);
    b=a;
    a=t;
  }
  state.w[0] += a;
  state.w[1] += b;
  state.w[2] += c;
  state.w[3] += d;
  state.w[4] += e;
}

void Sha1Class::pad() {
  // Implement SHA-1 padding (fips180-2 §5.1.1)
  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  while (bufferOffset != 56) addUncounted(0x00);
  // Append length in the last 8 bytes
  addUncounted(0); // We're only using 32 bit lengths
  addUncounted(0); // But SHA-1 supports 64 bit lengths
  addUncounted(0); // So zero pad the top bits
  addUncounted(byteCount >> 29); // Shifting to multiply by 8
  addUncounted(byteCount >> 21); // as SHA-1 supports bitstreams as well as
  addUncounted(byteCount >> 13); // byte.
  addUncounted(byteCount >> 5);
  addUncounted(byteCount << 3);
}

void Sha1Class::addUncounted(uint8_t data) {
  buffer.b[bufferOffset ^ 3] = data;
  bufferOffset++;
  if (bufferOffset == BLOCK_LENGTH) {
    hashBlock();
    bufferOffset = 0;
  }
}

uint32_t Sha1Class::rol32(uint32_t number, uint8_t bits) {
  return ((number << bits) | (number >> (32-bits)));
}

String Sha1Class::hash_text(bool hex_num){
  String response = "";
  if(hex_num){
    for(uint16_t index=0; index<strlen_P(hash_hex1); index++){
        response += char(pgm_read_byte_near(hash_hex1+index));
    }
  }else{
    for(uint16_t index=0; index<strlen_P(hash_hex2); index++){
        response += char(pgm_read_byte_near(hash_hex2+index));
    }
  }
  return response;
}

String Sha1Class::printHash(uint8_t* hash) {
  String response = "";
  String hash_1 = hash_text(true);
  String hash_2 = hash_text(false);
  for (uint8_t index=0; index<HASH_LENGTH; index++) {
    response += hash_1[hash[index]>>4];
    response += hash_2[hash[index]&0xf];
  }
  return response;
}

Sha1Class Sha1;
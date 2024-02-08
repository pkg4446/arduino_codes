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
    uint32_t a, b, c, d, e, temp;
    uint32_t w[BLOCK_LENGTH];

    // Message schedule
    for (int i = 0; i < 16; ++i) {
        w[i] = buffer.w[i];
    }
    for (int i = 16; i < BLOCK_LENGTH; ++i) {
        w[i] = rol16(w[i - 4] ^ w[i - 8] ^ w[i - 12] ^ w[i - 16], 1);
    }

    // Initialize hash value for this chunk
    a = state.w[0];
    b = state.w[1];
    c = state.w[2];
    d = state.w[3];
    e = state.w[4];

    // Main loop
    for (int i = 0; i < BLOCK_LENGTH; ++i) {
        if (i < BLOCK_LENGTH/4) {
            temp = (b & c) | ((~b) & d);
            temp += rol16(a, 5) + e + w[i] + SHA1_K0;
        } else if (i < BLOCK_LENGTH/2) {
            temp = b ^ c ^ d;
            temp += rol16(a, 5) + e + w[i] + SHA1_K20;
        } else if (i < BLOCK_LENGTH/4*3) {
            temp = (b & c) | (b & d) | (c & d);
            temp += rol16(a, 5) + e + w[i] + SHA1_K40;
        } else {
            temp = b ^ c ^ d;
            temp += rol16(a, 5) + e + w[i] + SHA1_K60;
        }

        e = d;
        d = c;
        c = rol16(b, 14);
        b = a;
        a = temp;
    }

    // Add this chunk's hash to result so far
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
  while (bufferOffset != BLOCK_LENGTH-8) addUncounted(0x00);
  addUncounted(0); // So zero pad the top bits
  
  addUncounted(byteCount >> 13);
  addUncounted(byteCount >> 5);
  addUncounted(byteCount << 3);
}

void Sha1Class::addUncounted(uint8_t data) {
  buffer.b[bufferOffset] = data;
  bufferOffset++;
  if (bufferOffset == BLOCK_LENGTH) {
    hashBlock();
    bufferOffset = 0;
  }
}

uint32_t Sha1Class::rol16(uint16_t number, uint8_t bits) {
  return ((number << bits) | (number >> (16-bits)));
}

String Sha1Class::hash_text(bool hex_num) {
    String response = "";
    const char* hash_ptr = hex_num ? hash_hex1 : hash_hex2;
    uint16_t hash_len = strlen_P(hash_ptr);
    for (uint16_t index = 0; index < hash_len; index++) {
        response += char(pgm_read_byte_near(hash_ptr + index));
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
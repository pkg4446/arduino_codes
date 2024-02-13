#include "hash.h"

void hashClass::init(void) {
  hash_value = 0;
  byteCount  = 0;
}

String hashClass::result(void) {
  return printHash(&hash_value);
}

size_t hashClass::write(uint8_t data) {
  hash_value ^= data;
  hash_value *= 0x5bd1e995;
  hash_value ^= hash_value >> 24;
  return true;
}

String hashClass::hash_text(bool hex_num) {
    String response = "";
    const char* hash_ptr = hex_num ? hash_hex1 : hash_hex2;
    uint16_t hash_len = strlen_P(hash_ptr);
    for (uint16_t index = 0; index < hash_len; index++) {
        response += char(pgm_read_byte_near(hash_ptr + index));
    }
    return response;
}

String hashClass::printHash(uint32_t* hash) {
  String response = "";
  String hash_1 = hash_text(true);
  String hash_2 = hash_text(false);
  for (uint8_t index=0; index<4; index++) {
    response += hash_1[((*hash >> (8*index)) & 0xFF) % 16];
    response += hash_2[((*hash >> (8*index)) & 0xFF) % 16];
  }
  return response;
}

hashClass hash;
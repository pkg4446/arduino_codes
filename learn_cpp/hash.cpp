#include "hash.h"

char hashClass::hash_text(bool hex_num, uint8_t index) {
  const char* hash_ptr = hex_num ? hash_hex1 : hash_hex2;
  return pgm_read_byte_near(hash_ptr + index);
}

String hashClass::printHash(uint32_t* hash) {
  String response = "";
  for (uint8_t index = 0; index < TEXT_LENGTH; index++) {
    response += hash_text(true,((*hash >> ((32/TEXT_LENGTH)*index)) & 0xFF) % 16);
    response += hash_text(false,((*hash >> ((32/TEXT_LENGTH)*index)) & 0xFF) % 16);
  }
  return response;
}

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

hashClass hash;
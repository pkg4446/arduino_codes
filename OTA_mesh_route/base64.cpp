#include "base64.h"

#ifdef BASE64_ENCODER
String base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
    String ret;
    int index_i = 0;
    int index_j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
      char_array_3[index_i++] = *(bytes_to_encode++);
      if (index_i == 3) {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                          ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                          ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (index_i = 0; index_i < 4; index_i++) {
          ret += base64_chars[char_array_4[index_i]];
        }
        index_i = 0;
      }
    }

    if (index_i) {
        for (index_j = index_i; index_j < 3; index_j++) {
            char_array_3[index_j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (index_j = 0; index_j < index_i + 1; index_j++) {
            ret += base64_chars[char_array_4[index_j]];
        }

        while ((index_i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

String base64_encode(const String& str, size_t in_len) {
    return base64_encode(reinterpret_cast<const unsigned char*>(str.c_str()), in_len);
}
#endif

#ifdef BASE64_DECODER
bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

String base64_decode(const String& encoded_string) {
    size_t in_len = encoded_string.length();
    size_t index_i = 0;
    size_t index_j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    String ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
      char_array_4[index_i++] = encoded_string[in_];
      in_++;
      if (index_i == 4) {
        for (index_i = 0; index_i < 4; index_i++) {
          char_array_4[index_i] = static_cast<unsigned char>(strchr(base64_chars, char_array_4[index_i]) - base64_chars);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (index_i = 0; index_i < 3; index_i++) {
          char ch = char_array_3[index_i];
          ret += ch;
        }
        index_i = 0;
      }
    }

    if (index_i) {
        for (index_j = index_i; index_j < 4; index_j++) {
            char_array_4[index_j] = 0;
        }
        for (index_j = 0; index_j < 4; index_j++) {
            char_array_4[index_j] = static_cast<unsigned char>(strchr(base64_chars, char_array_4[index_j]) - base64_chars);
        }
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        for (index_j = 0; index_j < index_i - 1; index_j++) {
          char ch = char_array_3[index_i];
          ret += ch;
        }
    }

    return ret;
}
#endif
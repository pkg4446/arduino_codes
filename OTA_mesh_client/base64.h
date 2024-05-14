#pragma once
#include <arduino.h>

//#define   BASE64_ENCODER
#define   BASE64_DECODER

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

String base64_encode(const String& str, size_t in_len);
String base64_decode(const String& encoded_string);
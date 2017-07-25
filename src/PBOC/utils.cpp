#include "utils.h"
byte addLen(byte len, byte* value, byte * buf, bool addzero){
  buf[0] = len;
  memcpy(buf + 1, value, len);
  if(addzero){
    buf[len + 1] = 0;
    return len + 2;
  }
  return len + 1;
}
byte concatBytes(byte l1, byte l2, byte* s1, byte* s2, byte* s3){
  if(s2 != s3){
    memmove(s3, s1, l1);
    memcpy(s3 + l1, s2, l2);
    return l1 + l2;
  }else{
    Serial.print(F("Overlap in concat!"));
    return 0;
  }
}
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
int printUntil(byte* data, byte end){
    for(int i = 0;i < 256;i++){
      byte h1 = data[i] >> 4;
      byte h2 = data[i] & 0xF;
      if(h1 == end) return i * 2;
      Serial.print(h1, HEX); 
      if(h2 == end) return i * 2 + 1;
      Serial.print(h2, HEX);
    }
}
int printHalfBytes(byte* data, int begin, byte len)
{
    for(int i = 0;i < len;i++){
      byte h = 0;
      if(begin & 1) h = data[begin >> 1] & 0xF;
      else  h = data[begin >> 1] >> 4;
      Serial.print(h, HEX);
      begin++;
    }
    return begin;
}


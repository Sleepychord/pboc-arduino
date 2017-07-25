#ifndef utils_h
#define utils_h
#include <Arduino.h>

byte addLen(byte len, byte* value, byte * buf, bool addzero = true);
byte concatBytes(byte l1, byte l2, byte* s1, byte* s2, byte* s3);
void dump_byte_array(byte *buffer, byte bufferSize);
int printUntil(byte* data, byte end = 0x00);
int printHalfBytes(byte* data, int begin, byte len);
class BytePool{
public:
    byte total;
    static const byte MAXPOOL = 5;
    static const byte BUFLEN = 128;
    byte* pool[MAXPOOL];
    bool used[MAXPOOL];
    byte defaultPool[BUFLEN * MAXPOOL];
    BytePool():total(0){
        memset(used, 0, sizeof(used));
        total = MAXPOOL;
        for(byte i = 0;i < total;i++)
            pool[i] = defaultPool + i * BUFLEN;
    }
    byte* getBytes(){
        for(int i = 0;i < total;i++)
            if(!used[i]){
                used[i] = true;
                return pool[i];
            }
        Serial.println("No buffer~~~");
        return NULL;
    }
    byte release(byte * buf){
        /*
            0 : normal return
            1 : used error
            2 : do not managed by pool
        */
        for(int i = 0;i < total;i++)
            if(pool[i] == buf){
                if(!used[i]) return 1;
                used[i] = false;
                return 0;
            }
        return 2;
    }
};

struct tlv{
    byte * p;
    tlv(byte* _p):p(_p){}
    tlv():p(NULL){}
    void set(byte* _p){p = _p;}
    byte len(){ return p?*p:0; }
    byte* data(){ return p + 1; }
};
#endif

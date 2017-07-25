#include "PBOC.h"

char PBOC::PSE[] ="1PAY.SYS.DDF01";
char PBOC::PPSE[] ="2PAY.SYS.DDF01";

PBOC::Respond PBOC::select(byte len, byte* data){
        Respond ret;
        byte* buf1 = pool.getBytes(), *buf2 = pool.getBytes();
        len = addLen(len, data, buf1);
        static const byte select_command[] = {0x00, 0xA4, 0x04, 0x00};
        len = concatBytes(4, len, select_command, buf1, buf2);
        ret.len = pool.BUFLEN;
        ret.status = base.TCL_Transceive(&tag, buf2, len, buf1, &ret.len);
        if(ret.status == MFRC522::STATUS_OK){
            ret.sw2 = buf1[--ret.len];
            ret.sw1 = buf1[--ret.len];
            ret.info = buf1;
        }else{
            ret.info = NULL;
            ret.sw1 = ret.sw2 = ret.len = 0;
            pool.release(buf1);
        }
        pool.release(buf2);
        return ret;
    }
bool PBOC::PSE_FCI::init(byte len, byte* data){
            byte* x = data;
            if(*x == 0x6f){
                fci = tlv(++x);
                x += 1;
                if(*x == 0x84){
                    df = tlv(++x);
                    x += df.len() + 1;
                }else goto ParseError;
                if(*x == 0xa5){
                    fcip = tlv(++x);
                    x += 1;
                    if(*x == 0x88){
                        sfi = tlv(++x);
                        x += sfi.len() + 1;
                    }else goto ParseError;
                    if(x < data + len && *x == 0x5f && *(x + 1) == 0x2d){
                        x += 2;
                        lp = tlv(x);
                        x += lp.len() + 1;
                    }
                    if(x < data + len && *x == 0x9f && *(x + 1) == 0x11){
                        x += 2;
                        icti = tlv(x);
                        x += icti.len() + 1;
                    }
                }else goto ParseError;
            } else goto ParseError;
        if(x == data + len) return true; else {Serial.print('Parse finished at ');Serial.print(x - data);}
        ParseError:
            Serial.print('ParseError!');
            return false;
    }

PBOC::Respond PBOC::readRecord(tlv sfi, byte p1){
    byte buf1[] = {0x00, 0xb2, p1, 0x04, 0x00};
    byte* buf2 = pool.getBytes();
    buf1[3] += (*sfi.data()) << 3;
    Respond ret;
    ret.len = pool.BUFLEN;
    // dump_byte_array(buf1, 5);
    ret.status = base.TCL_Transceive(&tag, buf1, 5, buf2, &ret.len);
    if(ret.status == MFRC522::STATUS_OK){
            ret.sw2 = buf2[--ret.len];
            ret.sw1 = buf2[--ret.len];
            ret.info = buf2;
        }else{
            ret.info = NULL;
            ret.sw1 = ret.sw2 = ret.len = 0;
    }
    return ret;
}
bool PBOC::ADFS::init(byte len, byte* data){
    tot = 0;
    byte* x = data;
    if(*x == 0x70 && *(x + 1) + 2 == len){
        x += 2;

        while(x < data + len){
            if(*x != 0x61) break;
            if(tot == MAXADF) break;
            l[tot++].init(x);
            x += *(x + 1) + 2;
        }
    }
    if(x == data + len) return true;
    Serial.print(F("ParseError!"));
    return false;
}

void PBOC::ADF::init(byte* x){
    if(*x == 0x61){
        x += 2;
        if(*x == 0x4f){
            aid.set(++x);
            x += aid.len() + 1;
        }
        if(*x == 0x50){
            label.set(++x);
            x += label.len() + 1;
        }
        if(*x == 0x87){
            api.set(++x);
            x += api.len() + 1;
        }
        if(*x == 0x9f && *(x + 1) == 0x12)
        {
            x += 2;
            apn.set(x);
            x += apn.len() + 1;
        }    
    }
}
PBOC::Respond PBOC::gpo(byte len, byte* data)
{
    static const byte gpo_command[] = {0x80, 0xA8, 0x00, 0x00};
    byte * buf1 = pool.getBytes(), *buf2 = pool.getBytes();
    len = addLen(len, data, buf1, false);
    byte tmpchar = 0x83;
    len = concatBytes(1, len, &tmpchar, buf1, buf2);
    len = addLen(len, buf2, buf1);
    len = concatBytes(4, len, gpo_command, buf1, buf2);
    Respond ret;
    ret.len = pool.BUFLEN;
    ret.status = base.TCL_Transceive(&tag, buf2, len, buf1, &ret.len);
    if(ret.status == MFRC522::STATUS_OK){
        ret.sw2 = buf1[--ret.len];
        ret.sw1 = buf1[--ret.len];
        ret.info = buf1;
    }else{
        ret.info = NULL;
        ret.sw1 = ret.sw2 = ret.len = 0;
        pool.release(buf1);
    }
    pool.release(buf2);
    return ret;
}
PBOC::Respond PBOC::getData(byte p1, byte p2){
    byte gpo_command[] = {0x80, 0xCA, p1, p2, 0x00};
    byte * buf1 = pool.getBytes();
    Respond ret;
    ret.len = pool.BUFLEN;
    ret.status = base.TCL_Transceive(&tag, gpo_command, 5, buf1, &ret.len);
    if(ret.status == MFRC522::STATUS_OK){
        ret.sw2 = buf1[--ret.len];
        ret.sw1 = buf1[--ret.len];
        ret.info = buf1;
    }else{
        ret.info = NULL;
        ret.sw1 = ret.sw2 = ret.len = 0;
        pool.release(buf1);
    }
    return ret;
}


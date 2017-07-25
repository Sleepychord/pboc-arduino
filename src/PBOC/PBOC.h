#ifndef pboc_h
#define pboc_h
#include <Arduino.h>
#include "../MFRC522Extended.h"
#include "utils.h"

class PBOC{
private:
    PBOC(){}
public:
    static char PSE[];
    static char PPSE[];

    MFRC522Extended base;
    BytePool pool;
    MFRC522Extended::TagInfo tag;

    PBOC(uint8_t ss, uint8_t rst):base(ss, rst), pool(){}
    void init(){
        base.PCD_Init();
        tag.blockNumber = false;
        tag.ats.tc1.supportsCID = false;
        tag.ats.tc1.supportsNAD = false;
    }
    bool active(){
        return base.PICC_IsNewCardPresent() && base.PICC_ReadCardSerial();
    }
    void printUID(){
        Serial.print(F("Card UID:"));
        dump_byte_array(base.uid.uidByte, base.uid.size);
        Serial.println();
    }
    void halt(){
        base.PICC_HaltA();
        base.PCD_StopCrypto1();
    }
    typedef struct {
        MFRC522::StatusCode status;
		byte sw1;
        byte sw2;
        byte* info;
        byte len;
	} Respond;

    Respond select(byte len, byte* data);
    struct PSE_FCI{
        tlv fci, df;// Dedicated File Name
        tlv fcip;// FCI propritetary Template
        tlv sfi, lp, icti;//Short file identifier, language perference, issuer code table index
        bool init(byte len, byte* data);
    }; // File Control Information Template
    Respond readRecord(tlv sfi, byte p1);
    struct ADF{// Application Definition File
        tlv aid; // Application Identifier
        tlv label; //Application Label
        tlv api; // Application Priority Indicator
        tlv apn; // Application Priority Name;
        ADF(){}
        void init(byte* data);
    };
    struct ADFS{
        enum consts : byte { MAXADF = 3};
        ADF l[MAXADF];
        byte tot;
        bool init(byte len, byte* data);
    };
    Respond gpo(byte len, byte* data);
    Respond getData(byte p1, byte p2);
};

#endif
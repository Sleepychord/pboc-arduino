/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 * 
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <PBOC/PBOC.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

PBOC pboc(SS_PIN, RST_PIN);   // Create MFRC522 instance.
PBOC::ADFS adfs;
PBOC::PSE_FCI fci;

/**
 * Initialize.
 */
void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    pboc.init();    
}
  
/**
 * Main loop.
 */
 void loop(){
    if(!pboc.active()) return;
    pboc.printUID();
    pboc.tag.blockNumber = false;
    pboc.tag.ats.tc1.supportsCID = false;
    pboc.tag.ats.tc1.supportsNAD = false;
//    Serial.print(pboc.base.PCD_ReadRegister(pboc.base.TxModeReg), HEX);
    PBOC::Respond res = pboc.select(strlen(PBOC::PSE), PBOC::PSE);
    while(res.status != MFRC522::STATUS_OK){
      Serial.print(F("Select Failed!"));
      Serial.println(MFRC522::GetStatusCodeName(res.status));
      return ;
    }
      if(!fci.init(res.len, res.info)) return;
    //read record
    byte * buf1 = res.info;
    res = pboc.readRecord(fci.sfi, 0x01);
    pboc.pool.release(buf1);//release buf storing respond of select
//    Serial.print(F("ADFS:"));
//    dump_byte_array(res.info, res.len);
    if (!adfs.init(res.len, res.info)) return ;
    Serial.println();
    Serial.print(F("AID:"));
    dump_byte_array((byte *)(adfs.l[0].aid.data()), (byte)(adfs.l[0].aid.len()));
    Serial.println();
    Serial.print(F("LABEL:"));
    Serial.write(adfs.l[0].apn.data(), adfs.l[0].apn.len());
    Serial.println();
    buf1 = res.info;
    res = pboc.select(adfs.l[0].aid.len(), adfs.l[0].aid.data());
    while(res.status != MFRC522::STATUS_OK){
      Serial.print(F("Select APP Failed!"));
      Serial.println(MFRC522::GetStatusCodeName(res.status));
      pboc.pool.release(buf1);
      return;
    }
      pboc.pool.release(buf1);
//    Serial.print(F("Received:"));
//    dump_byte_array(res.info, res.len);
    pboc.pool.release(res.info);
    Serial.println();

    
//    byte gpo_data[] = {0x60, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
//      0x56,0x00,0x00,0x00,0x00,0x00,0x01,0x56,0x13,0x09,0x25,0x00,0x00,0x00,0x00,0x00};
//    res = pboc.gpo(sizeof(gpo_data), gpo_data);

    byte meta_sfi_addr[] = {0x1, 0x1};
    tlv meta_sfi(meta_sfi_addr);
    res = pboc.readRecord(meta_sfi, 0x1);
//    printStatus(res);
    Serial.print(F("银行卡号:"));
    int t = printUntil(res.info + 4, 0xD) + 1;
    Serial.println();
    Serial.print(F("有效日期至:20"));
    t = printHalfBytes(res.info + 4, t, 2);
    Serial.print(F("年"));
    t = printHalfBytes(res.info + 4, t, 2);
    Serial.println(F("月"));
    pboc.pool.release(res.info);

//    res = pboc.readRecord(meta_sfi, 0x2); //可能有姓名等，不同银行差别较大 6A83说明无此记录
//    printStatus(res);
//    dump_byte_array(res.info, res.len);
//    pboc.pool.release(res.info);

    
    res = pboc.getData(0x9F, 0x79);
//    printStatus(res);
    Serial.print(F("金额:"));
      t = printHalfBytes(res.info + 3, 0, 10);
      Serial.print(".");
      t = printHalfBytes(res.info + 3, t, 2);
      Serial.println(F("元 "));
    pboc.pool.release(res.info);

    res = pboc.getData(0x9F, 0x4F);
//    dump_byte_array(res.info, res.len);//按标准应该是9A 03 9F 21 03 9F 02 06 9F 03 06 9F 1A 02 5F 2A 02 9F 4E 14 9C 01 9F 36 02 
//  "日期 时间 授权金额 其他金额 终端国家代码 货币类型 商户名称 交易类型 应用交易计数器"
    Serial.println(F("日期 时间 授权金额 商户名称"));//更准确应该按照上边读出来的
    pboc.pool.release(res.info);

    byte rec_sfi_addr[] = {0x1, 0xB};
    for(byte i = 1;i <= 10;i++){
      tlv rec_sfi(rec_sfi_addr);
      res = pboc.readRecord(rec_sfi, i);
//      printStatus(res);
      if(res.sw1 != 0x90 || res.sw2 != 0x00){
        Serial.println("没有其他交易记录！");
        pboc.pool.release(res.info);
        break;
      }
//      dump_byte_array(res.info, res.len);
      int t = 0;
      Serial.print("20");
      t = printHalfBytes(res.info, t, 2);
      Serial.print(F("年"));
      t = printHalfBytes(res.info, t, 2);
      Serial.print(F("月"));
      t = printHalfBytes(res.info, t, 2);
      Serial.print("日 ");
      t = printHalfBytes(res.info, t, 2);
      Serial.print(":");
      t = printHalfBytes(res.info, t, 2);
      Serial.print(":");
      t = printHalfBytes(res.info, t, 2);
      Serial.print(" ");

      if(res.info[t/2 + 6 * 2 + 2 * 2 + 20] <= 0x1)  Serial.print("-");
      else if(res.info[t/2 + 6 * 2 + 2 * 2 + 20] == 0x21) Serial.print("+");
      t = printHalfBytes(res.info, t, 10);
      Serial.print(".");
      t = printHalfBytes(res.info, t, 2);
      Serial.print(F("元 "));

      t += (6 + 2 + 2) * 2;

      Serial.write(res.info + t / 2, 20);
      Serial.println();
      pboc.pool.release(res.info);
    }
    pboc.halt();
    Serial.println();
    Serial.println(F("End a loop"));
 }
 void printStatus(PBOC::Respond& res){
    Serial.print("Respond Status:");
    Serial.print(MFRC522::GetStatusCodeName(res.status));
    Serial.print(res.sw1 < 0x10 ? " 0" : " ");
    Serial.print(res.sw1, HEX);
    Serial.print(res.sw2 < 0x10 ? " 0" : " ");
    Serial.print(res.sw2, HEX);
    Serial.println();
 }




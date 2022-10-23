//   Protokollaufbau 
//   Byte 1    : Syncronbyte z.b. 170 / AA
//   Byte 2    : Zieladresse  1=LC; 2=WiFi;
//   Byte 3    : Antwort erwünscht ? 0 = Nein, 255 = Ja
//   Byte 4    : Befehl
//   Byte 5-8  : Wert als longInt
//   Byte 9    : CRC8

// Zum Ladecontroller
#define LADEN_STOP                    0  //  Unterbricht das Laden sofort
#define SOLAR_LADEN                   1  // *Schaltet um auf Solarladen. Leistung wird über 3 mitgeteilt. Abhängigkeit: 3, 4, 5 werden berücksichtigt
#define DAUER_LADEN                   2  //  Schaltet um auf Dauerladen und lädt mit der festgelegten Leistung. Dieser Wert ist negativ (wird aus dem Netz gezogen). 3, 4, 5 werden ignoriert

#define SOLAR_LEISTUNG                3  // *Leistung mit der geladen werden kann (Leistung wird fortlaufend vom Stromzähler gemeldet). Dieser Wert ist negativ                                 (- = Einspeisen ins Netz)
#define POWER_MARGE                   4  // *Wird von der echten verfügbaren Leistung abgezogen und gibt somit den Wert der ins Netz gehen muss. Dieser Wert ist positiv.                       (+ = Bezug aus dem Netz)
#define POWER_NET_CUTOFF              5  // *Ab dieser Abnahmeleistung muss das Laden unterbrochen werden (achtung: es kann zu Schwingungen kommen). Dieser Wert ist positiv                    (+ = Bezug aus dem Netz)


// Zum LC und zum WiFi (bidirektional)
#define ONE_PH_CHRG                   10 // 0x0A // Vorgabe an den LC nur 1Ph zu laden. Sobald dieser Befehl kommt, wird zuerst das Laden unterbrochen. Wenn bereits konfiguriert, wird der Befehl ignoriert
#define TWO_PH_CHRG                   11 // 0x0B // Vorgabe an den LC nur 2Ph zu laden. Sobald dieser Befehl kommt, wird zuerst das Laden unterbrochen. Wenn bereits konfiguriert, wird der Befehl ignoriert
#define TRE_PH_CHRG                   12 // 0x0C // Vorgabe an den LC nun 3Ph zu laden. Sobald dieser Befehl kommt, wird zuerst das Laden unterbrochen. Wenn bereits konfiguriert, wird der Befehl ignoriert

#define STANDBY_LADEN                 13 // 0x0D //  Wird nicht voll allen Fahrzeugen unterstützt! PWM 100%


// Zum WiFi-Modul
#define LADEN_BEENDET                 20
#define LADEN_GESTARTET               21
#define LADE_LEISTUNG                 22
#define LADE_UNTERBRECHUNG            23
#define LADE_TRENNUNG                 24
#define LADE_VERBINDUNG               25

#define TASTER1_PUSHED                30  
#define TASTER2_PUSHED                31

#define BOARD_TEMPERATUR              40

#define CP_WERT                       50

#define BOOT_START                    120

#define SYNC                          170

typedef struct SerialComData {        // 9 Byte
    byte Sync                 = 170;  // 1
    byte Zieladresse          = 0;    // 1
    byte Antwort              = 0;    // 1
    byte Befehl               = 2;    // 1
    long Wert                 = 3;    // 4
    byte CRC8                 = 1;    // 1
    
} SerialComData;

SerialComData SerialData;

unsigned char checksum(unsigned char *data, uint8_t *crc) {
    unsigned char sum=0;
    while(data != crc) {
      sum+=*data;
      data++;
      }
    return sum;
}
    
byte DataValid                = 0;

void SendData(){
          SerialData.Sync = SYNC;
          byte *ptr;
          
          SerialData.CRC8 = checksum((unsigned char *)&SerialData, &SerialData.CRC8);   

          ptr = (byte *)&SerialData;
          Serial.write(ptr, 9);
}

void serial_flush_buffer()
{
  while (Serial.read() >= 0)
   ; // do nothing
}

bool LadenBeendet() {

  return false;
}


void ReceiveData(){
  DataValid = 0;
  if (Serial.available()){
    //digitalWrite(13, HIGH); 
          delay(10);
          byte buf[20];
          //int len = Serial.readBytes(buf, 10);
          buf[0] = Serial.read();
          buf[1] = Serial.read();
          buf[2] = Serial.read();
          buf[3] = Serial.read();
          buf[4] = Serial.read();
          buf[5] = Serial.read();
          buf[6] = Serial.read();
          buf[7] = Serial.read();
          buf[8] = Serial.read();
          
          if (buf[0] == SYNC) {
                
            //DataValid = 1;
            SerialData.Zieladresse          = buf[1];
            SerialData.Antwort              = buf[2];
            SerialData.Befehl               = buf[3];   
            long val = 0;
            val += buf[7] << 24;
            val += buf[6] << 16;
            val += buf[5] << 8;
            val += buf[4];         
            SerialData.Wert                 = val;
            SerialData.CRC8                 = buf[8];

            if (SerialData.CRC8 == checksum((unsigned char *)&SerialData, &SerialData.CRC8) ){
             DataValid = 1; 
            }else{
             DataValid = 0; 
            }
            //Serial.println(val);
                
          } else { serial_flush_buffer();}

 }          
              
           //  digitalWrite(13, LOW);  
  }

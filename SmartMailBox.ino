/* 
 * ไลบรารี่ TridentTD_LineNotify version 2.1
 * ใช้สำหรับ ส่ง แจ้งเตือนไปยัง LINE สำหรับ ESP8266 และ ESP32
 * สามารถส่งได้ทั้ง ข้อความ , สติกเกอร์ และรูปภาพ(ด้วย url)
 * -----------------------------------------------------
 * ให้ save เป็น file ต่างหากก่อนถึงจะส่ง Line Notify ภาษาไทยได้
 */

#include <EEPROM.h>
#include <TridentTD_LineNotify.h>

#define SSID        "WIFI_SSID"
#define PASSWORD    "WIFI_PASSWORD"
#define LINE_TOKEN  "LINE_TOKEN"

void ICACHE_RAM_ATTR MailInt();
void ICACHE_RAM_ATTR DoorOpenInt();

unsigned int mail_counter;
bool flag_send_notification;
bool flag_send_door_open;

// -------------------- READ MAIL COUNTER ----------------------
void read_mail_counter()
{
  unsigned char data1, data2;

  data1 = EEPROM.read(0);
  data2 = EEPROM.read(1);

  mail_counter = (data1 << 8) | data2;
  Serial.println("Mail counter: " + String(mail_counter));
}

// -------------------- WRITE MAIL COUNTER -----------------------
void write_mail_counter()
{
  EEPROM.write(0, (unsigned char)(mail_counter >> 8));
  EEPROM.write(1, (unsigned char)(mail_counter & 0x0F));

  if (EEPROM.commit()) 
  {
    Serial.println("EEPROM successfully committed");
  } 
  else 
  {
    Serial.println("ERROR! EEPROM commit failed");
  }
}

// -------------------- GET MAIL INTERRUPT  ----------------------
unsigned long ms_buf;
void MailInt()
{
  unsigned long time_ms = millis();
  unsigned long ms_dif = time_ms - ms_buf;

  ms_buf = time_ms;


  if ( ms_dif >= 3000 && digitalRead(D2) ) // 3 seconds
  { 
    mail_counter++;
    write_mail_counter();
    Serial.println("Get mail: " + String(mail_counter));

    flag_send_notification = 1;
  }
}

// -------------------- DOOR OPEN INTERRUPT  ----------------------
void DoorOpenInt()
{
  if (mail_counter != 0)
  {
    mail_counter = 0;
    write_mail_counter();
    flag_send_door_open = 1;
  }

  Serial.println("Door Open");
}

// -------------------- INITIALIZATION  ----------------------
void setup() 
{
  Serial.begin(115200); 
  Serial.println();
  Serial.println(LINE.getVersion());

  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D4, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(D1), MailInt, FALLING);
  attachInterrupt(digitalPinToInterrupt(D2), DoorOpenInt, FALLING);

  EEPROM.begin(512);
  read_mail_counter();
  
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);

  // ส่งข้อความเมื่อเชื่อมต่อ WiFi สำเร็จแล้ว
  LINE.notify("Smart Mailbox เชื่อมต่อ WiFi แล้ว");
}

// -------------------- LOOP  ----------------------
void loop() 
{
  // Send notification -> getting mail
  if (flag_send_notification)
  {
    flag_send_notification = 0;
    LINE.notify("มีจดหมายเข้า " + String(mail_counter) + " ฉบับ");
  }

  // Send nitification -> door open
  if (flag_send_door_open)
  {
    flag_send_door_open = 0;
    LINE.notify("ตู้รับจดหมายถูกเปิด ไม่มีจดหมายค้างในตู้แล้ว");
  }
  
  delay(1);
}

#include <FastBot.h> 
#include <Wire.h>

#define TRANS_ID 8
#define WIFI_SSID "viva"
#define WIFI_PASS "12344321"
#define BOT_TOKEN "6581455945:AAEQHJXD-zuBxOz6R_E-dLZAptht0_IZSQA"
#define CHAT_ID "2059227823"
// переменная, в которую запишем ID сообщения   
// с меню для дальнейшего редактирования 
int32_t menuID = 0; 
byte depth = 0; 
bool state_water = false;
FastBot bot(BOT_TOKEN);

void connectWiFi() { 
  delay(2000); 
  WiFi.begin(WIFI_SSID, WIFI_PASS); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");
    if (millis() > 15000) ESP.restart(); 
  } 
  Serial.println("Connected");
}  

#define DHTPIN D0
#define auto_water_pin D3
#define auto_felt_pin D4
#define drnk D7
#define LED_PIN_f D5
#define LED_PIN_h D6
#define turb_sens A0

#define LED_NUM_f 9
#define LED_NUM_h 8
const unsigned long INTERVAL_send = 60*1000*5;
unsigned long  waitTime_send = 0;
#define felt_time 400
#define water_time 4800
#define dob_wat 1000

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip_f(LED_NUM_f, LED_PIN_f, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_h(LED_NUM_h, LED_PIN_h, NEO_GRB + NEO_KHZ800);

#include "DHT.h"  
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

bool auto_felt = false;
bool auto_water = true;
bool led_ham = false;
bool led_aqua = false;
float air_temp = 0;
float air_humid = 0;
int humid_1 = 0;
int humid_2 = 0;
int turb = 0;
int water_l = 0;

FB_Time t;

FB_Time felt_date;

String times_hams[3] = {"-","-","-"}; 
String times_fish[3] = {"-","-","-"}; 

void setup() {
  Serial.begin(115200);
  Wire.begin(); 

  pinMode(DHTPIN,INPUT);
  pinMode(auto_water_pin,OUTPUT);
  pinMode(auto_felt_pin,OUTPUT);
  pinMode(drnk,OUTPUT);
  pinMode(LED_PIN_f,OUTPUT);
  pinMode(LED_PIN_h,OUTPUT);
  pinMode(turb_sens,INPUT);
  dht.begin();

  connectWiFi();   
  bot.setChatID(CHAT_ID); 
  bot.attach(newMsg); 
  bot.setTextMode(FB_MARKDOWN);
  strip_f.begin();
  strip_h.begin();
  strip_f.setBrightness(30);
  strip_h.setBrightness(30);
  strip_f.show();  // Очистка первой ленты
  strip_h.show();  // Очистка второй ленты
}

void requestData() {
  Wire.requestFrom(TRANS_ID, 16);  // Запрашиваем данные

  String receivedData = "";
  while (Wire.available()) {  
    char c = Wire.read();
    receivedData += c;  
  }
  sscanf(receivedData.c_str(), "%d %d %d", &humid_1, &humid_2, &water_l);
}

void sending(String str, String ID){ 
  bot.deleteMessage(menuID); 
  bot.sendMessage(str,ID); 
  String menu = F("cage \t garden \n aqua \t info "); 
  bot.inlineMenu("*Выберете команду*", menu); 
  depth = 0;
  menuID = bot.lastBotMsg(); 
} 

void transmit(String text) {
  Wire.beginTransmission(TRANS_ID);
  Wire.write(text.c_str());          
  Wire.endTransmission();             
}

void newMsg(FB_msg& msg) {
    if (msg.text == "/start") {  
      requestData();
      String ham = "*Влажность почвы 1 :*"; 
      ham+=("%d",map(humid_1,0,700,0,100)); 
      ham+="*%*"; 
      bot.sendMessage(ham,msg.chatID); 
      ham = "*Влажность почвы 2 :*"; 
      ham+=("%d",map(humid_2,0,700,0,100)); 
      ham+="*%*"; 
      bot.sendMessage(ham,msg.chatID); 
      String t = "*Температура воздуха:*"; 
      t+=("%d",22); 
      t+="*°С*";  
      bot.sendMessage(t,msg.chatID); 
      String h = "*Влажность воздуха:*"; 
      h+=("%d",53); 
      h+="*%*"; 
      bot.sendMessage(h,msg.chatID);
      bot.sendMessage("Вода в аквариуме чистая",msg.chatID);
      if(water_l < 300){
        bot.sendMessage("Необходимо долить воду",msg.chatID);
      }else{
        bot.sendMessage("Уровень воды нормальный",msg.chatID);
      }
      if(humid_1<400){
        bot.sendMessage("Необходимо полить растение 1",msg.chatID);
      }else{
        bot.sendMessage("Растение 1 полито",msg.chatID);
      }
      if(humid_2<400){
        bot.sendMessage("Необходимо полить растение 2",msg.chatID);
      }else{
        bot.sendMessage("Растение 2 полито",msg.chatID);
      }
      if(water_l < 300){
        digitalWrite(drnk,1);
        delay(dob_wat);
        digitalWrite(drnk,0);
        delay(200);
        bot.sendMessage("Вода долита",msg.chatID);
      }
      if((humid_1<400) or (humid_2<400)){
        transmit("plants");
        delay(3000); 
        digitalWrite(auto_water_pin,1);
        delay(water_time);
        digitalWrite(auto_water_pin,0);
        delay(200);
        digitalWrite(auto_felt_pin,1);
        delay(felt_time);
        digitalWrite(auto_felt_pin,0);
        delay(200);
        bot.sendMessage("Растение 1 полито",msg.chatID);
        delay(3000);
        delay(5000);
        digitalWrite(auto_water_pin,1);
        delay(water_time);
        digitalWrite(auto_water_pin,0);
        delay(200);
        digitalWrite(auto_felt_pin,1);
        delay(felt_time);
        digitalWrite(auto_felt_pin,0);
        delay(200);
        bot.sendMessage("Растение 2 полито",msg.chatID);
      }
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
      depth = 0;
    }
// ЦВЕТОЧКИ ------------------------------------------------------
    if (msg.data == "garden") { 
      String menu = F("auto water \n fertilizer \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1;
    } 
    if (msg.data == "auto water") { 
      String menu = F("auto on \n auto off \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 3;
    } 
    if (msg.data == "auto on" and depth == 3) { 
      auto_water = true;
      sending("Автополив включен",msg.chatID); 
    } 
    if (msg.data == "auto off" and depth == 3) { 
      auto_water = false;
      sending("Автополив выключен",msg.chatID); 
    } 
    if (msg.data == "fertilizer") { 
      String menu = F("auto on \t auto off \n interval \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 4;
    } 
    if (msg.data == "auto on" and depth == 4) { 
      auto_felt = true;
      //felting();
      FB_Time felt_day = bot.getTime(3); 
      sending("Удобрение включено",msg.chatID); 
    } 
    if (msg.data == "auto off" and depth == 4) { 
      auto_felt = false;
      sending("Удобрение выключено",msg.chatID); 
    } 
    if (msg.data == "interval" and depth == 4) { 
    } 
// ХОМЯК ---------------------------------------------------------------   
    if (msg.data == "cage") { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    } 
    if (msg.data == "led" and depth == 1) { 
      String menu = F("ON \n OFF"); 
      bot.editMenu(menuID, menu); 
      depth = 2; 
    } 
    if (msg.data == "ON" and depth == 2) { 
      led_ham = true;
      sending("Освещение клетки включено",msg.chatID);
    } 
    if (msg.data == "OFF" and depth == 2) { 
      led_ham = false;
      sending("Освещение клетки выключено",msg.chatID);
    } 
    if (msg.data == "feed now" and depth == 1) {  
      transmit("feed hamster");
      sending("Хомячок покормлен",msg.chatID);
    }
    if (msg.data == "time" and depth == 1) {
      String menu = F("list \n add time \n del time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 7; 
    }
    if ( msg.data == "add time" and depth == 7){ 
      if(times_hams[0]=="-" or times_hams[1]=="-" or times_hams[2]=="-" ){
        bot.sendMessage("Введите время",msg.chatID); 
        bot.attach(newTime_cage); 
      }else{
        bot.sendMessage("Нет места , удалите что-то",msg.chatID); 
      }
    } 
    if ( msg.data == "del time" and depth == 7){ 
      if(times_hams[0] != "-" or times_hams[1] != "-" or times_hams[2] != "-" ){
        bot.sendMessage("Выберете что удалить",msg.chatID); 
        String t = "1: "; 
        t += times_hams[0]; 
        bot.sendMessage(t,msg.chatID); 
        t = "2: "; 
        t += times_hams[1]; 
        bot.sendMessage(t,msg.chatID); 
        t = "3: "; 
        t += times_hams[2]; 
        bot.sendMessage(t,msg.chatID); 
        bot.attach(delTime_cage); 
      }else{
        bot.sendMessage("Нечего удалять",msg.chatID); 
      } 
    } 
    if(msg.data == "list" and depth == 7){
      bot.sendMessage("Время кормления хомяка",msg.chatID);
      bot.deleteMessage(menuID); 
      String t = "1: "; 
      t += times_hams[0]; 
      bot.sendMessage(t,msg.chatID); 
      t = "2: "; 
      t += times_hams[1]; 
      bot.sendMessage(t,msg.chatID); 
      t = "3: "; 
      t += times_hams[2];  
      sending(t,msg.chatID); 
    }

    
// АКВАС ---------------------------------------------------------------   
    if (msg.data == "aqua") { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 2;
    } 
    if (msg.data == "led" and depth == 2) { 
      String menu = F("ON \n OFF"); 
      bot.editMenu(menuID, menu); 
      depth = 5; 
    } 
    if (msg.data == "ON" and depth == 5) { 
      led_aqua = true;
      sending("Освещение аквариума включено",msg.chatID);
    } 
    if (msg.data == "OFF" and depth == 5) { 
      led_aqua = false;
      sending("Освещение аквариума выключено",msg.chatID);
    } 
    if (msg.data == "feed now" and depth == 2) {  
      String menu = F("1 \n 2"); 
      bot.editMenu(menuID, menu); 
      depth = 8; 
    }
    if(msg.data == "1" and depth == 8){
      transmit("feed fish 1");
      sending("Рыбка 1 покормлена",msg.chatID);
    }
    if(msg.data == "2" and depth == 8){
      transmit("feed fish 2");
      sending("Рыбка 2 покормлена",msg.chatID);
    }
    if (msg.data == "time" and depth == 2) {
      String menu = F("list \n add time \n del time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 8; 
    }
    if ( msg.data == "add time" and depth == 8){ 
      if(times_fish[0]=="-" or times_fish[1]=="-" or times_fish[2]=="-" ){
        bot.sendMessage("Введите время",msg.chatID); 
        bot.attach(newTime_fish); 
      }else{
        bot.sendMessage("Нет места , удалите что-то",msg.chatID); 
      }
    } 
    if ( msg.data == "del time" and depth == 8){ 
      if(times_fish[0] != "-" or times_fish[1] != "-" or times_fish[2] != "-" ){
        bot.sendMessage("Выберете что удалить",msg.chatID); 
        String t = "1: "; 
        t += times_fish[0]; 
        bot.sendMessage(t,msg.chatID); 
        t = "2: "; 
        t += times_fish[1]; 
        bot.sendMessage(t,msg.chatID); 
        t = "3: "; 
        t += times_fish[2]; 
        bot.sendMessage(t,msg.chatID); 
        bot.attach(delTime_cage); 
      }else{
        bot.sendMessage("Нечего удалять",msg.chatID); 
      } 
    } 
    if(msg.data == "list" and depth == 8){
      bot.sendMessage("Время кормления рыб",msg.chatID);
      bot.deleteMessage(menuID); 
      String t = "1: "; 
      t += times_fish[0]; 
      bot.sendMessage(t,msg.chatID); 
      t = "2: "; 
      t += times_fish[1]; 
      bot.sendMessage(t,msg.chatID); 
      t = "3: "; 
      t += times_fish[2];  
      sending(t,msg.chatID); 
    }

// ИНФОРМАЦИЯ ---------------------------------------------------------------   
    if (msg.data == "info") { 
      String menu = F("air info \t soil info \n water info \t Back \n test"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    }
    if (msg.data == "test") {
      bot.sendMessage("Тест устройства начат",msg.chatID); 
      for (int i = 0; i < LED_NUM_f; i += 2) {  // Каждый второй светодиод
        strip_f.setPixelColor(i, strip_f.Color(255, 255, 255));
      }
      strip_f.show(); // Показываем изменения
      bot.sendMessage("Свет включен",msg.chatID); 
      transmit("hamster");
      delay(5000);
      bot.sendMessage("Хомяк покормлен",msg.chatID); 
      transmit("testing"); 
      bot.sendMessage("Рыбки покормлены",msg.chatID); 
      sending("Тест устройства закончен",msg.chatID); 
    } 
    if (msg.data == "soil info") { 
      String ham = "*Влажность почвы 1 :*"; 
      ham+=("%d",map(humid_1,0,700,0,100)); 
      ham+="*%*"; 
      bot.sendMessage(ham,msg.chatID); 
      ham = "*Влажность почвы 2 :*"; 
      ham+=("%d",map(humid_2,0,700,0,100)); 
      ham+="*%*"; 
      sending(ham,msg.chatID); 
    }
    if (msg.data == "air info") { 
      String t = "*Температура воздуха:*"; 
      t+=("%d",22); 
      t+="*°С*";  
      bot.sendMessage(t,msg.chatID); 
      String h = "*Влажность воздуха:*"; 
      h+=("%d",53); 
      h+="*%*"; 
      sending(h,msg.chatID);
    }
    if (msg.data == "water info") { 
      if(state_water == 0){
        sending("Вода чистая",msg.chatID); 
      }else{
        sending("Вода мутная",msg.chatID); 
      }
    }
// НАЗАД
    if (msg.data == "Back" && (depth == 1 || depth == 2)) { 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.editMenu(menuID, menu); 
      depth = 0; 
    } 
    if ((msg.data == "Back" && depth == 4)||(msg.data == "Back" && depth == 3)) { 
      String menu = F("auto water \n fertilizer \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1;
    } 
    if (msg.data == "Back" && depth == 7) { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    }
    if (msg.data == "Back" && depth == 8) { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 2;
    }
    
}

void newTime_cage(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if (times_hams[0] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[0] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_hams[1] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[1] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_hams[2] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[2] = msg.text;   
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
     
  } 
} 

void newTime_fish(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if (times_fish[0] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[0] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_fish[1] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[1] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_fish[2] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[2] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
     
  } 
} 

void delTime_cage(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if(msg.text == "1"){ 
      bot.deleteMessage(menuID); 
      times_hams[0] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "2"){ 
      bot.deleteMessage(menuID); 
      times_hams[1] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "3"){ 
      bot.deleteMessage(menuID); 
      times_hams[2] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info ");   
      bot.inlineMenu("*Выберете команду*", menu);
      menuID = bot.lastBotMsg(); 
    } 
  } 
} 

void delTime_fish(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if(msg.text == "1"){ 
      bot.deleteMessage(menuID); 
      times_fish[0] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "2"){ 
      bot.deleteMessage(menuID); 
      times_fish[1] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "3"){ 
      bot.deleteMessage(menuID); 
      times_fish[2] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info ");   
      bot.inlineMenu("*Выберете команду*", menu);
      menuID = bot.lastBotMsg(); 
    } 
  } 
} 

void loop() {
  bot.tick();
  t = bot.getTime(3); 
} 

#include <StepperMotor.h>
#include <Wire.h>

#define TRANS_ID 8

String com;

#define water_sens A2
#define soil1_pin A0
#define soil2_pin A1

void receiveEvent(int howMany) {
  com = "";
  while (Wire.available()) { 
    char c = Wire.read();  // Читаем байт
    com += c;  // Добавляем к строке
  }
  Serial.println(com); // Выводим строку
}

StepperMotor motor1(8, 7, 9);// движение цветов
StepperMotor motor2(11, 10, 12);// движение рыб

void setup() {
  pinMode(water_sens,INPUT);
  pinMode(soil1_pin,INPUT);
  pinMode(soil2_pin,INPUT);

  Serial.begin(115200);

  motor1.begin();
  motor2.begin();
  motor1.setSpeed(800, 400);
  motor2.setSpeed(800, 400);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  digitalWrite(2,HIGH);
  digitalWrite(6,HIGH);
  Wire.begin(TRANS_ID);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}
#define water_time 1000*8
#define foster_time 1000*2

void requestEvent() {
  int s1 = analogRead(A0);
  int s2 = analogRead(A1);
  int l = analogRead(A2);

  // Отправляем данные в формате "A0 A1 A2\n"
  String data = String(s1) + " " + String(s2) + " " + String(l) + "\n";
  Wire.write(data.c_str());  // Отправляем строку через I2C
}

void feed(int STEP, int DIR, int EN, int cnt){
  digitalWrite(EN, LOW); 
  for(int j = 0; j< cnt; j++){
    digitalWrite(DIR, HIGH); 
    for (int i = 0; i < 125; i++){   
      digitalWrite(STEP, HIGH);
      delayMicroseconds(1700);  
      digitalWrite(STEP, LOW);
      delayMicroseconds(1700); 
    }
    delay(10);
    digitalWrite(DIR, LOW); 
    for (int i = 0; i < 100; i++){   
      digitalWrite(STEP, HIGH);
      delayMicroseconds(1700);  
      digitalWrite(STEP, LOW);
      delayMicroseconds(1700); 
    }
  }
  digitalWrite(EN, HIGH); 
}

void hams(){
  feed(3,4,2,6);
}
void fishes(){
  motor2.rotate(1950); // Двигаемся на 200 шагов с 500 шагами в секунду
  delay(300);
  feed(5,4,6,3);
  delay(300);
  motor2.rotate(-1950); // Двигаемся обратно на 200 шагов
  delay(500);
  motor2.rotate(800); // Двигаемся на 200 шагов с 500 шагами в секунду
  delay(300);
  feed(5,4,6,3);
  delay(300);
  motor2.rotate(-800); // Двигаемся обратно на 200 шагов
  delay(500);
}

void test(){
  fishes();
}

void plants(){ 
  motor1.rotate(900); // Двигаемся на 200 шагов с 500 шагами в секунду
  delay(7000);
  motor1.rotate(-900); // Двигаемся обратно на 200 шагов
  delay(1000);
  motor1.rotate(1900); // Двигаемся на 200 шагов с 500 шагами в секунду
  delay(7000);
  motor1.rotate(-1900); // Двигаемся обратно на 200 шагов
  delay(1000);
}

void loop() {
  if(com != ""){
    if(com == "plant_test"){
      plants();
    }else if(com == "testing"){
      test();
    }else if(com == "hamster"){
      hams();
    }
    com = "";
  }
}

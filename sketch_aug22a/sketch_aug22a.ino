#include <DHT.h>

#define RELAY_PIN_CURRENT 8
#define RELAY_PIN_TEM 4

#define DHTPIN 7 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

bool CURRENTOn = true; // 릴레이 1 - 전원 선택용,,초기 상태는 ON (가정용전류)
bool DHTOn = true;     // 릴레이 2 - 센서 전원 차단용

int tem = 0;
int hum = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);   
  dht.begin();

  pinMode(RELAY_PIN_CURRENT, OUTPUT);
  digitalWrite(RELAY_PIN_CURRENT, HIGH); // 초기 상태는 ON (가정용전류)

  pinMode(RELAY_PIN_TEM, OUTPUT);
  digitalWrite(RELAY_PIN_TEM, HIGH); // 기본: 센서 ON

}

void dht11() {    //함수 dht11선언
  tem = dht.readTemperature();    //변수 t에 온도 값을 저장
  hum = dht.readHumidity();   //변수 h에 습도 값을 저장

   // NaN (Not a Number) 체크 – 센서 읽기 실패할 경우
  if (isnan(tem) || isnan(hum)) {
    Serial.println("센서에서 데이터를 읽을 수 없습니다.");
    return;
  }

  Serial.print("Temperature : ");   //문자열 출력
  Serial.print(tem);    //변수 t출력
  Serial.println("C");    //문자열 출력
  Serial.print("Humidity : ");    //문자열 출력
  Serial.print(hum);    //변수 h출력
  Serial.println("%");    //문자열 출력

  Serial.println(); // 한 줄 띄우기
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '1') {
      CURRENTOn = true;
      Serial.println("가정용전류로 전환");
    } else if (cmd == '0') {
      CURRENTOn = false;
      Serial.println("태양광전류로 전환");
    }
   
   // 센서 작동 제어
    if (cmd == '2') {
      DHTOn = true;
      Serial.println("온습도 기계 ON");
    } else if (cmd == '3') {
      DHTOn = false;
      Serial.println("온습도 기계 OFF");
    }
  }

   // 릴레이 제어
  digitalWrite(RELAY_PIN_CURRENT, CURRENTOn ? HIGH : LOW);
  digitalWrite(RELAY_PIN_TEM, DHTOn ? HIGH : LOW);

   // 센서 측정 (센서 ON일 때만)
  if (DHTOn) {
    dht11();
  } else {
    Serial.println("센서 OFF 상태");
  }

  
  delay(2000);
}
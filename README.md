# iot_raspberry_pi
#### putty에서 라즈베리파이 종료할 때
```
sudo shutdown -h now
```


## iot smarthome 팀 프로젝트
#### 8/11 
1. 더미데이터 만들기
- <img src='./더미데이터만들기.png'>
2. 도커 mysql 컨테이너 실행 및 테이블 생성
- cmd에서 도커 - 컨테이너 생성
```shell
docker run --name mysql-container2 \
  -e MYSQL_ROOT_PASSWORD=12345 \
  -p 3307:3306 \
  -d mysql:8.0
```
- cmd에서 도커 - 컨테이너 - mysql 실행
    ```shell
    C:\Users\Admin>docker start mysql-container2
    C:\Users\Admin>docker exec -it mysql-container2 mysql -uroot -p
    Enter password:

    mysql> SHOW DATABASES;
    +--------------------+
    | Database           |
    +--------------------+
    | information_schema |
    | mydb               |
    | mysql              |
    | performance_schema |
    | sys                |
    +--------------------+
    mysql> use mydb

    mysql> SHOW TABLES;
    +----------------+
    | Tables_in_mydb |
    +----------------+
    | power_log      |
    | users          |
    +----------------+
    2 rows in set (0.01 sec)
    ```
- csv파일을 테이블에 load
    - <img src='./더미데이터삽입1.png'>
    - <img src='./더미데이터삽입2.png'>
    - <img src='./더미데이터삽입3.png' >

3. 라즈베리파이에서 api호출 파이썬 코드 작성

|fechone()|fetchall()|
|:--:|:--:|
|결과에서 한 행(row)만 가져와요<br>리턴 타입: 보통 딕셔너리 {} 또는 튜플 (값1, 값2, ...)<br>(예) { "total": 130541.84 }| 결과에서 모든 행(row들)을 리스트로 가져와요<br>리턴 타입: 딕셔너리들의 리스트 [ {}, {}, ... ] 또는 튜플들의 리스트<br> (예) [{"temp":22.8,"temp2":24.1}, {"temp":23,"temp2":25}]|

```python
from flask import Flask, jsonify, request
import mysql.connector

app = Flask(__name__)

def get_db_connection():
    return mysql.connector.connect(
        host="192.168.0.2",
        user="root",
        password="12345",
        database="mydb",
        port=3307
    )
#금일 소비 전력량
@app.route("/power-log")
def read_power_log():
    conn = get_db_connection()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT * FROM power_log ORDER BY timestamp DESC LIMIT 10")
    result = cursor.fetchall()
    cursor.close()
    conn.close()
    return jsonify(result)

# 가전제품별 전력 소비량 조회
@app.route("/device-power")
def device_power():
    device_id = request.args.get('deviceId')
    start_day = request.args.get('startDay')
    end_day = request.args.get('endDay')

    query =""
    param=[]

    if device_id is None:
        query = "select DATE(timestamp) AS date, sum(power) as total from power_log where DATE(timestamp) between %s and %s  group by DATE(timestamp) order by DATE(timestamp)"
        param =[start_day, end_day]
    else :
        query = "select DATE(timestamp) ,sum(power) as total from power_log WHERE  DATE(timestamp) between %s and %s  AND id = %s  group by DATE(timestamp) order by DATE(timestamp) "
        param = [start_day, end_day, device_id]

    conn=get_db_connection()
    cursor = conn.cursor(dictionary = True)
    cursor.execute(query, param)

    result = cursor.fetchall()
    cursor.close()
    conn.close()
    return jsonify({"data": result})

# 금일 집안의 온습도
@app.route("/temphumid")
def read_temphumind():
    conn = get_db_connection()
    cursor = conn.cursor(dictionary = True)
    cursor.execute("SELECT * FROM users ")
    result = cursor.fetchall()
    cursor.close()
    conn.close()
    return jsonify({"data" : result })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000, debug=True)

```


4. 윈도우192.168.0.2 도커 실행, 라즈베리파이192.168.0.4 REST API 실행 ->웹사이트에서 192.168.0.4:8000/power-log 로 데이터 json형태 출력 확인함
- <img src='./json확인.png'  width=500 >


#### 8/12 
1. 유니티 시작하기
- 유니티 허브 - 유니티 프로젝트 생성
- 프로젝트가 있는 폴더에 다운로드 받은 Assets, Packages, ProjectSettings 복사붙여 덮어쓰기
- Assets > Scenes >  MainScene.unity 클릭해서 씬을 연다
- File > Build Settings  > Scenes In Build" 아래가 비어 있거나, 원하는 씬이 없으면: 아래쪽 "Add Open Scenes" 버튼 클릭
- 그 씬 앞에 체크박스가 ✔️ 체크되어 있어야 실행 및 빌드 대상이 됩니다.
- Play 버튼 눌러서 씬이 정상 작동하는지 확인

2. 유니티의 db 데이터 가져오기 - REST API , tick (유니티)
- http 연결 허용  : Edit > Project Settings > Player > Configuration > Allow downloads over HTTP 허용
- Canvas > HomePanel > GridPanel> House > Temp(text 컴포넌트) ,Temp2 (text 컴포넌트)
- House - add conponent - Script - HouseManager 
- House -  HouseManager - House Temp에는 Temp를 드래그,  House Temp2에는 Temp2를 드래그
- <img src ='./컴포넌트연결.png'  width=500>

3. 유니티의 db 데이터 가져오기 - REST API , tick (C#스크립트)
- 하단의 Project/Console 중    Project탭 > Assets > Scripts > c#스크립트 생성
- **JsonUtility는 JSON 키와 C# 클래스 필드명이 정확히 일치해야 값을 제대로 파싱합니다.**
```csharp
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;

public class HomeManager : MonoBehaviour
{
    [Header("Text")]
    public GameObject homeTemp;
    public GameObject homeTemp2;

    public float updateInterval = 60f; // 몇 초마다 업데이트할지 (60초 예시)
    private float timer = 0f;

    private void Start()
    {
        StartCoroutine(GetTempData()); // 시작하자마자 1번 요청
    }

    private void Update()
    {
        timer += Time.deltaTime;  // 매 프레임 경과 시간 누적

        if (timer >= updateInterval)
        {
            timer = 0f;  // 타이머 초기화
            StartCoroutine(GetTempData());
        }
    }

    IEnumerator GetTempData()
    {
        UnityWebRequest request = UnityWebRequest.Get("http://192.168.0.4:8000/temphumid");
        yield return request.SendWebRequest();

        if (request.result != UnityWebRequest.Result.Success)
        {
            Debug.LogError("API 요청 실패: " + request.error);
        }
        else
        {
            string json = request.downloadHandler.text;
            Debug.Log("받은 JSON: " + json);
            TempDataList tempDataList = JsonUtility.FromJson<TempDataList>(json);
            Debug.Log("파싱된 데이터 개수: " + tempDataList.data.Length);
            if (tempDataList.data.Length > 0)
            {
                TempData lastData = tempDataList.data[tempDataList.data.Length - 1];
                Debug.Log($"마지막 데이터 - temp: {lastData.temp}, temp2: {lastData.humid}");
                if (homeTemp && homeTemp2)
                {
                    homeTemp.GetComponent<Text>().text = lastData.temp.ToString("F1");
                    homeTemp2.GetComponent<Text>().text = lastData.humid.ToString("F1");

                }
            }
        }
    }
    // 데이터 1개
    [System.Serializable]   
    public class TempData
    {
        public float temp;
        public float humid;
        public int idx;
    }

    //데이터가 리스트 형식으로 옴
    [System.Serializable]
    public class TempDataList
    {
        public TempData[] data;
    }

}
```
- <img src='./온습도,금일전력량호출완료.png' width=500>
- <img src='./가전제품별전력소비량.png' width=500>

4. 가전제품별 소비전력량 차트(유니티)

|UI|과정|이미지|
|:--:|:--:|:--:|
|ChartPanel > Furnitures > AllButton|DeviceBtn 컴포넌트 추가 > DeviceId (0 전체, 1 ~ 10) , chart (chartpanel)<br><br>onclick() 추가 > 현재버튼을 드래그|<img src='./버튼유니티설정.png' >|
|ChartPanel > Chart > ChartPanel panel 생성 |DeviceChart컴포넌트 추가 > Chartpanel, Bar Prefab 설정 |<img src='./chartpanel유니티설정.png'>|
|ChartPanel > Chart >Bar Prefab image 생성|Rec Transform, background 설정|<img src='./barchart유니티설정.png'>|
|ChartPanel > Chart >Bar Prefab > Text ui생성 |예시 글자 입력 및 위치 설정|<img src='./bar차트날짜글자설정.png'>|

5. 가전제품별 소비전력량 차트(c#)
- DeviceBtn.cs
    ```csharp
    using System.Collections;
    using System.Collections.Generic;
    using UnityEngine;
    using UnityEngine.UI;

    public class Devicebtn : MonoBehaviour
    {
        public string deviceId;
        public DeviceChart chart;

        public void OnClick()
        {
            chart.ShowChart(deviceId);
        }
    }
    ```
- DeviceChart.cs
```csharp
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using System;

public class DeviceChart : MonoBehaviour
{
    public GameObject chartPanel;  //차트가 그려질 공간
    public GameObject barPrefab; // Bar 차트
    public float maxHeight = 200f;           // 그래프 최대 높이
    private List<GameObject> bars = new();   // 기존 바 제거용


    [System.Serializable]
    public class DevicePowerData
    {
        public float total;
        public string date;
    }
    
    [System.Serializable]
    public class PowerDataList
    {
        public DevicePowerData[] data;
    }

    public void ShowChart(string deviceId)
    {
        chartPanel.SetActive(true);
        StartCoroutine(FetchDeviceData(deviceId));
    }

    IEnumerator FetchDeviceData(string deviceId)
    {
        string  startDay = "2025-08-11";
        string  endDay = "2025-08-12";
        
        string url = $"http://192.168.0.4:8000/device-power?deviceId={deviceId}&startDay={startDay}&endDay={endDay}";
        UnityWebRequest request = UnityWebRequest.Get(url);
        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.Success)
        {
            PowerDataList dataList = JsonUtility.FromJson<PowerDataList>(request.downloadHandler.text);
            Debug.Log("파싱된 데이터 개수: " + dataList.data.Length);
            DevicePowerData lastData = dataList.data[0];
            Debug.Log("첫번째 데이터 : " + lastData.date + lastData.total);
            //차트 그리기
            DrawChart(new List<DevicePowerData>(dataList.data));
        }
        else
        {
            Debug.LogError("데이터 로딩 실패: " + request.error);
        }
    }


    public void DrawChart(List<DevicePowerData> dataList)
    {
        
        // 1. 기존 바 제거
        foreach (var bar in bars)
        {
            Destroy(bar);
        }
        bars.Clear();

        if (dataList.Count == 0) return;

        // 2. 최대값 구하기
        float maxValue = 0f;
        foreach (var data in dataList)
        {
            if (data.total > maxValue) maxValue = data.total;
        }
        Debug.Log("-------------" + maxValue);
        // 3. 차트 폭 기준
        float barSpacing = 100f;
        float startX = -((dataList.Count - 1) * barSpacing) / 2f;
        Debug.Log("-------------" + startX);

        //4. 차트 요소 만들기
        for (int i = 0; i < dataList.Count; i++)
        {
            var data = dataList[i];
             
            GameObject newBar = Instantiate(barPrefab, chartPanel.transform);
            newBar.SetActive(true);

            RectTransform rt = newBar.GetComponent<RectTransform>();
            float height = (data.total / maxValue) * maxHeight;

            rt.sizeDelta = new Vector2(40f, height);
            rt.anchoredPosition = new Vector2(startX + i * barSpacing, height / 2);

            Text label = newBar.GetComponentInChildren<Text>();
            if (label != null)
            {
                DateTime dt = DateTime.Parse(data.date);
                label.text = dt.ToString("MM/dd");
            }

            bars.Add(newBar);
        }
    }
}

```



|데이터종류|필터조건|유니티 Hierarchy|c# 프로젝트명|
|:--:|:--:|:--:|:--:|
|온습도|오늘날짜, 인덱스가장마지막|House > Temp, Temp2|HouseManager|
|금일전력소비량|오늘날짜|TodayConsumption > Tconsumption|TconsumptionManager|
|가전제품별 전력소비량|request param 시작일,종료일,가전제품id|<img src='./chart폴더구조.png'>|DeviceBtn, DeviceChart|

#### 8/13 
- 회의안건
1. 라즈베리파이의 maria DB에 테이블형성 및 더미 데이터 =>192.168.0.4 하나의 ip로 실행가능 
2. 센서데이터, 카메라 모듈 시간대 저장할 때, 영국기준으로 할 것?
3. 전력량 일정값 이상일 때만 보내는 걸로? 아니면 라즈베리파이에서 db저장할 때 필터?

- 해야할 것
1. 라즈베리파이의 maria DB에 테이블형성 및 더미 데이터 =>192.168.0.4 하나의 ip로 실행가능 
2. 차트 시각화
- 유니티 - 버튼에 컴포넌트/ONCLICK,  차트에 컴포넌트
- c# - image로 차트그리기(*) , 날짜받아오기(달력에서 선택한 날짜가 반영되어야함. 현재는 그냥 값 지정한 상태로 테스트함.)
3. ai분석


#### 8/21
1. 유니티 시작하기
- 유니티 허브 - 유니티 프로젝트 생성
- 프로젝트가 있는 폴더에 다운로드 받은 Assets, Packages, ProjectSettings 복사붙여 덮어쓰기
- Assets > Scenes >  MainScene.unity 클릭해서 씬을 연다
- File > Build Settings  > Scenes In Build" 아래가 비어 있거나, 원하는 씬이 없으면: 아래쪽 "Add Open Scenes" 버튼 클릭
- 그 씬 앞에 체크박스가 ✔️ 체크되어 있어야 실행 및 빌드 대상이 됩니다.
- Play 버튼 눌러서 씬이 정상 작동하는지 확인

2. 폴더구조, 코드 분석

#### 8/22
1. 아두이노 IDE 준비 
- 아두이노 ide 설치
- 도구>보드> 아두이노 우노
- 도구>포트>COM4

2. 온습도 센서 확인
- 하드웨어 연결하기
- 코드작성 , 라이브러리 설치
- 컴파일
- 시리얼모니터 tool> serial monitor

3. 릴레이모듈1 - 태양광전력 OR 가정용전력
- 하드웨어 연결하기

```
아두이노 5v - 릴레이모듈 vcc 
아두이노 gnd - 릴레이모듈 gnd , 온습도센서gnd

아두이노 8번 핀 - 릴레이모듈 sig
아두이노 7번핀 - 온습도 센서 sig

릴레이모듈 com  - 온습도센서 vcc
릴레이모듈 no - 아두이노 5v
릴레이모듈nc - 라즈베리파이 5v

아두이노 5v가 가정용역할
라즈베리파이5v가 태양광전류 역할
```

- 코드작성 , 컴파일
```cpp
#include <DHT.h>

#define RELAY_PIN_CURRENT 8

#define DHTPIN 7 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

bool CURRENTOn = true; // 초기 상태는 ON (가정용전류)

int tem = 0;
int hum = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);   
  dht.begin();

  pinMode(RELAY_PIN_CURRENT, OUTPUT);
  digitalWrite(RELAY_PIN_CURRENT, HIGH); // 초기 상태는 ON (가정용전류)
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
      dht.begin(); // 태양광으로 바뀔 때 센서 재초기화 시도
    }
  }

  if (CURRENTOn) {
    digitalWrite(RELAY_PIN_CURRENT, HIGH);
  } else {
    digitalWrite(RELAY_PIN_CURRENT, LOW);
  }

  dht11();
  delay(2000);
}
```
- 시리얼 모니터 , Message에 1 , 0 입력해서 확인하기
    - <img src='./images/0태양광.png'>
    - <img src='./images/1가정용.png'>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Wire.h>

// 濕度感測器
#define DHT22_PIN_SUR 1    // 環境 DHT22 的接腳
#define DHT22_PIN_CLO_1 17  // 衣物 DHT22 的接腳
#define DHT22_PIN_CLO_2 13 // 衣物 DHT22 的接腳
DHT dht22_sur(DHT22_PIN_SUR, DHT22);
DHT dht22_clo_1(DHT22_PIN_CLO_1, DHT22);
DHT dht22_clo_2(DHT22_PIN_CLO_2, DHT22);

// 重量感測器
HX711 HX711_Clo1;
HX711 HX711_Clo2;
const int CLO_1_DT_PIN = 7;         // HX711 的接腳
const int CLO_1_SCK_PIN = 15;        // HX711 的接腳
const int CLO_2_DT_PIN = 11;         // HX711 的接腳
const int CLO_2_SCK_PIN = 10;        // HX711 的接腳
const int scale_factor_5kg = 428;   // 比例參數，從校正程式中取得
const int scale_factor_10kg = 221;  // 比例參數，從校正程式中取得
const int DELAY_TIME = 5000;        // 延遲時間，用以控制多久讀取一次數值

// Wi-Fi
const char* ssid = "Xiaomi_383B";     // WiFi SSID     Xiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5GXiaomi_383B_5Gㄒ
const char* password = "29009565";  // WiFi 密碼
WiFiMulti wifiMulti;

// 伺服器傳輸
String server = "http://140.115.51.163:40005/api/sensordata";  // 伺服器位址
String uuidAPI = "http://140.115.51.163:40005/api/uuid";       // API 位址
String uuid = "";
StaticJsonDocument<1200> json_doc1 = StaticJsonDocument<1200>();
StaticJsonDocument<1200> json_doc2 = StaticJsonDocument<1200>();
StaticJsonDocument<6000> json_transmit_doc = StaticJsonDocument<6000>();
DeserializationError json_error;
char json_output1[1200] = {0};
char json_output2[1200] = {0};
char json_transmit_output[6000] = {0};
char doc_output[6000] = {0};
HTTPClient http;
int httpResponseCode;

// 變數們

int counter = 0;  // 用以判斷是否經過一分鐘
int steady_point_1 = 0;
int steady_point_2 = 0;

double humidity_sur = -1;
double humidity_clo_1 = -1;
double humidity_clo_2 = -1;

double tempC_sur = -1;
double tempC_clo_1 = -1;
double tempC_clo_2 = -1;

double prev_weight_clo_1 = 0;
double prev_weight_clo_2 = 0;

double weight_clo_1 = -1;
double weight_clo_2 = -1;

bool isSurroundingError = false;
bool isClothes1Error = false;
bool isClothes2Error = false;

int soil_1 = 8;
int soil_2 = 19;

void setup() {
    // 土壤
    pinMode(soil_1,INPUT);
    pinMode(soil_2,INPUT);

    // 設定 WiFi 模式為 Station
    WiFi.mode(WIFI_STA);

    // 把原本的連線斷開
    WiFi.disconnect();
    delay(100);

    wifiMulti.addAP(ssid, password);

    while (true) {
        if (wifiMulti.run() == WL_CONNECTED) {
            delay(2000);
            break;
        } else {
            delay(1000);
        }
    }


    http.begin(uuidAPI);
    httpResponseCode = 100;
    while (httpResponseCode != 200) {
        delay(1000);
        httpResponseCode = http.GET();
        if (httpResponseCode == 200) {
            uuid = http.getString();
            uuid = uuid.substring(9);
            uuid = uuid.substring(0, uuid.length() - 2);
        }
    }
    http.end();

    http.begin(server);
    http.addHeader("Content-Type", "application/json");

    // 初始化環境 DHT22
    dht22_sur.begin();

    // 初始化衣物 DHT22
    dht22_clo_1.begin();
    dht22_clo_2.begin();


    // 初始化 HX711
    HX711_Clo1.begin(CLO_1_DT_PIN, CLO_1_SCK_PIN);
    HX711_Clo2.begin(CLO_2_DT_PIN, CLO_2_SCK_PIN);

    // 設定比例參數
    HX711_Clo1.set_scale(scale_factor_5kg);
    HX711_Clo2.set_scale(scale_factor_10kg);

    // 歸零
    HX711_Clo1.tare();

    HX711_Clo2.tare();

    delay(1000);
}

void loop() {

    // 讀取濕度
    humidity_sur = dht22_sur.readHumidity();
    humidity_clo_1 = dht22_clo_1.readHumidity();
    humidity_clo_2 = dht22_clo_2.readHumidity();

    // 讀取溫度(攝氏)
    tempC_sur = dht22_sur.readTemperature();
    tempC_clo_1 = dht22_clo_1.readTemperature();
    tempC_clo_2 = dht22_clo_2.readTemperature();

    isSurroundingError = false;
    isClothes1Error = false;
    isClothes2Error = false;

    // 檢查是否有數值錯誤
    if (isnan(humidity_sur) || isnan(tempC_sur)) {
      isSurroundingError = true;
      humidity_sur = -1;
      tempC_sur = -1;
      Serial.println("Err sur");
    }
    if (isnan(humidity_clo_1) || isnan(tempC_clo_1)) {
        isClothes1Error = true;
        humidity_clo_1 = -1;
        tempC_clo_1 = -1;
      Serial.println("Err 1");
    }
    if (isnan(humidity_clo_2) || isnan(tempC_clo_2)) {
        isClothes2Error = true;
        humidity_clo_2 = -1;
        tempC_clo_2 = -1;
      Serial.println("Err 2");
    }
    
    // 讀取電子稱數
    // 重量為 10 次測量之平均
    weight_clo_1 = HX711_Clo1.get_units(10);
    weight_clo_2 = HX711_Clo2.get_units(10);
    if (weight_clo_1 < 0) {
        weight_clo_1 = -1;
    }

    if (weight_clo_2 < 0) {
        weight_clo_2 = -1;
    }


    prev_weight_clo_1 = weight_clo_1;
    prev_weight_clo_2 = weight_clo_2;

    // 如果六十秒了
    if (counter < (120000 / DELAY_TIME)) {
        counter++;
        delay(1000);
        return;
    } else {
        counter = 0;
    }

    clearContent(json_output1, 1200);

    // 土壤
    int soil_1_value = analogRead(soil_1);
    int soil_2_value = analogRead(soil_2);
    
    json_doc1 = StaticJsonDocument<1200>();
    json_transmit_doc = StaticJsonDocument<6000>();

    char buffer[1200] = {0};
    clearContent(buffer, 1200);

    // 產生 JSON，記得設定精度到小數點後兩位
    json_doc1["uuid"] = uuid;

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc1["humidity_sur"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc1["temperature_sur"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", humidity_clo_1);
    json_doc1["humidity_clothes"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", tempC_clo_1);
    json_doc1["temperature_clothes"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", weight_clo_1);
    json_doc1["weight"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%d", soil_1_value);
    json_doc1["soil"] = buffer;
    clearContent(buffer, 1200);

    ArduinoJson::serializeJson(json_doc1, json_output1);

    json_doc2["uuid"] = uuid;

    clearContent(json_output2, 1200);

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc2["humidity_sur"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc2["temperature_sur"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", humidity_clo_2);
    json_doc2["humidity_clothes"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", tempC_clo_2);
    json_doc2["temperature_clothes"] = buffer;
    clearContent(buffer, 1200);

    sprintf(buffer, "%.2lf", weight_clo_2);
    json_doc2["weight"] = buffer;
    clearContent(buffer, 1200);


    sprintf(buffer, "%d", soil_2_value);
    json_doc2["soil"] = buffer;
    clearContent(buffer, 1200);

    ArduinoJson::serializeJson(json_doc2, json_output2);

    clearContent(doc_output, 6000);

    StaticJsonDocument<12000> doc;
    JsonArray sensor_data = doc.to<JsonArray>();
    sensor_data.add(json_output1);
    sensor_data.add(json_output2);
    ArduinoJson::serializeJson(doc, doc_output);

    clearContent(json_transmit_output, 6000);

    json_transmit_doc["sensor_data"] = doc_output;
    ArduinoJson::serializeJson(json_transmit_doc, json_transmit_output);

    // 檢查網路連線
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(50);
        }
    }

    // 發送 POST 請求
    httpResponseCode = http.POST(json_transmit_output);

    if (httpResponseCode != 200) {
          httpResponseCode = http.POST(json_transmit_output);
    } 
    delay(DELAY_TIME);
    
}

void clearContent(char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}
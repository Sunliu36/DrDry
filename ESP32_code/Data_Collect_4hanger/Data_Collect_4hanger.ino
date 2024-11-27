// 正式 code

#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Wire.h>

// LCD 顯示器
#define SDA 1
#define SCL 2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 濕度感測器
#define DHT22_PIN_SUR 10    // 環境 DHT22 的接腳
#define DHT22_PIN_CLO_1 11  // 衣物 DHT22 的接腳
#define DHT22_PIN_CLO_2 21  // 衣物 DHT22 的接腳
#define DHT22_PIN_CLO_3 20  // 衣物 DHT22 的接腳
#define DHT22_PIN_CLO_4 19  // 衣物 DHT22 的接腳
DHT dht22_sur(DHT22_PIN_SUR, DHT22);
DHT dht22_clo_1(DHT22_PIN_CLO_1, DHT22);
DHT dht22_clo_2(DHT22_PIN_CLO_2, DHT22);
DHT dht22_clo_3(DHT22_PIN_CLO_3, DHT22);
DHT dht22_clo_4(DHT22_PIN_CLO_4, DHT22);

// 重量感測器
HX711 HX711_Clo1;
HX711 HX711_Clo2;
HX711 HX711_Clo3;
HX711 HX711_Clo4;
const int CLO_1_DT_PIN = 4;         // HX711 的接腳
const int CLO_1_SCK_PIN = 5;        // HX711 的接腳
const int CLO_2_DT_PIN = 6;         // HX711 的接腳
const int CLO_2_SCK_PIN = 7;        // HX711 的接腳
const int CLO_3_DT_PIN = 15;        // HX711 的接腳
const int CLO_3_SCK_PIN = 16;       // HX711 的接腳
const int CLO_4_DT_PIN = 17;        // HX711 的接腳
const int CLO_4_SCK_PIN = 18;       // HX711 的接腳
const int scale_factor_5kg = 428;   // 比例參數，從校正程式中取得
const int scale_factor_10kg = 221;  // 比例參數，從校正程式中取得
const int DELAY_TIME = 5000;        // 延遲時間，用以控制多久讀取一次數值

// Wi-Fi
const char* ssid = "TangPhone";     // WiFi SSID
const char* password = "cs933600";  // WiFi 密碼
WiFiMulti wifiMulti;

// 伺服器傳輸
String server = "http://140.115.51.163:40005/api/sensordata";  // 伺服器位址
String uuidAPI = "http://140.115.51.163:40005/api/uuid";       // API 位址
String uuid = "";
StaticJsonDocument<600> json_doc1 = StaticJsonDocument<600>();
StaticJsonDocument<600> json_doc2 = StaticJsonDocument<600>();
StaticJsonDocument<600> json_doc3 = StaticJsonDocument<600>();
StaticJsonDocument<600> json_doc4 = StaticJsonDocument<600>();
StaticJsonDocument<1600> json_transmit_doc = StaticJsonDocument<1600>();
DeserializationError json_error;
char json_output1[600] = {0};
char json_output2[600] = {0};
char json_output3[600] = {0};
char json_output4[600] = {0};
char json_transmit_output[1600] = {0};
char doc_output[1600] = {0};
HTTPClient http;
int httpResponseCode;

// 變數們

int counter = 0;  // 用以判斷是否經過一分鐘
int steady_point_1 = 0;
int steady_point_2 = 0;
int steady_point_3 = 0;
int steady_point_4 = 0;

double humidity_sur = -1;
double humidity_clo_1 = -1;
double humidity_clo_2 = -1;
double humidity_clo_3 = -1;
double humidity_clo_4 = -1;

double tempC_sur = -1;
double tempC_clo_1 = -1;
double tempC_clo_2 = -1;
double tempC_clo_3 = -1;
double tempC_clo_4 = -1;

double prev_weight_clo_1 = 0;
double prev_weight_clo_2 = 0;
double prev_weight_clo_3 = 0;
double prev_weight_clo_4 = 0;

double weight_clo_1 = -1;
double weight_clo_2 = -1;
double weight_clo_3 = -1;
double weight_clo_4 = -1;

bool isSurroundingError = false;
bool isClothes1Error = false;
bool isClothes2Error = false;
bool isClothes3Error = false;
bool isClothes4Error = false;

void setup() {
    // 開啟序列埠通訊
    Serial.begin(19200);

    // 初始化 LCD 顯示器
    Wire.begin(SDA, SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Hello!");

    delay(1000);

    // 設定 WiFi 模式為 Station
    WiFi.mode(WIFI_STA);

    // 把原本的連線斷開
    WiFi.disconnect();
    delay(100);

    // LCD 顯示 "Connecting to WiFi"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to");
    lcd.setCursor(0, 1);
    lcd.print("WiFi...");

    wifiMulti.addAP("cilab", "cilabwifi");
    wifiMulti.addAP(ssid, password);

    while (true) {
        if (wifiMulti.run() == WL_CONNECTED) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Connected to");
            lcd.setCursor(0, 1);
            lcd.print(WiFi.SSID());
            delay(2000);
            break;
        } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Retrying to");
            lcd.setCursor(0, 1);
            lcd.print("connect...");
            delay(1000);
        }
    }

    // lcd 顯示 "Start getting uuid"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start getting");
    lcd.setCursor(0, 1);
    lcd.print("uuid");

    http.begin(uuidAPI);
    httpResponseCode = 100;
    while (httpResponseCode != 200) {
        delay(1000);
        httpResponseCode = http.GET();
        Serial.println(httpResponseCode);
        if (httpResponseCode == 200) {
            uuid = http.getString();
            uuid = uuid.substring(9);
            uuid = uuid.substring(0, uuid.length() - 2);
        }
    }
    http.end();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start connecting");
    lcd.setCursor(0, 1);
    lcd.print("to server");

    http.begin(server);
    http.addHeader("Content-Type", "application/json");

    // lcd 顯示 "Start initing dht22"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start initing");
    lcd.setCursor(0, 1);
    lcd.print("dht22");

    // 初始化環境 DHT22
    dht22_sur.begin();

    // 初始化衣物 DHT22
    dht22_clo_1.begin();
    dht22_clo_2.begin();
    dht22_clo_3.begin();
    dht22_clo_4.begin();

    // lcd 顯示 "Start initing hx711"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start initing");
    lcd.setCursor(0, 1);
    lcd.print("hx711");

    // 初始化 HX711
    HX711_Clo1.begin(CLO_1_DT_PIN, CLO_1_SCK_PIN);
    HX711_Clo2.begin(CLO_2_DT_PIN, CLO_2_SCK_PIN);
    HX711_Clo3.begin(CLO_3_DT_PIN, CLO_3_SCK_PIN);
    HX711_Clo4.begin(CLO_4_DT_PIN, CLO_4_SCK_PIN);

    // 設定比例參數
    HX711_Clo1.set_scale(scale_factor_5kg);
    HX711_Clo2.set_scale(scale_factor_10kg);
    HX711_Clo3.set_scale(scale_factor_5kg);
    HX711_Clo4.set_scale(scale_factor_10kg);

    // lcd 顯示 "Start tare1"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start tare1");

    // 歸零
    HX711_Clo1.tare();

    // lcd 顯示 "Start tare2"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start tare2");

    HX711_Clo2.tare();

    // lcd 顯示 "Start tare3"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start tare3");

    HX711_Clo3.tare();

    // lcd 顯示 "Start tare4"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start tare4");

    HX711_Clo4.tare();

    // lcd 顯示 "All done, ready to go!"
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("All done,");
    lcd.setCursor(0, 1);
    lcd.print("ready to go!");

    delay(1000);
}

void loop() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading data...");

    // 讀取濕度
    humidity_sur = dht22_sur.readHumidity();
    humidity_clo_1 = dht22_clo_1.readHumidity();
    humidity_clo_2 = dht22_clo_2.readHumidity();
    humidity_clo_3 = dht22_clo_3.readHumidity();
    humidity_clo_4 = dht22_clo_4.readHumidity();

    // 讀取溫度(攝氏)
    tempC_sur = dht22_sur.readTemperature();
    tempC_clo_1 = dht22_clo_1.readTemperature();
    tempC_clo_2 = dht22_clo_2.readTemperature();
    tempC_clo_3 = dht22_clo_3.readTemperature();
    tempC_clo_4 = dht22_clo_4.readTemperature();

    Serial.println(tempC_clo_1);
    Serial.println(tempC_clo_2);
    Serial.println(tempC_clo_3);
    Serial.println(tempC_clo_4);

    isSurroundingError = false;
    isClothes1Error = false;
    isClothes2Error = false;
    isClothes3Error = false;
    isClothes4Error = false;

    // 檢查是否有數值錯誤
    if (isnan(humidity_sur) || isnan(tempC_sur)) {
        isSurroundingError = true;
        humidity_sur = -1;
        tempC_sur = -1;
    }
    if (isnan(humidity_clo_1) || isnan(tempC_clo_1)) {
        isClothes1Error = true;
        humidity_clo_1 = -1;
        tempC_clo_1 = -1;
    }
    if (isnan(humidity_clo_2) || isnan(tempC_clo_2)) {
        isClothes2Error = true;
        humidity_clo_2 = -1;
        tempC_clo_2 = -1;
    }
    if (isnan(humidity_clo_3) || isnan(tempC_clo_3)) {
        isClothes3Error = true;
        humidity_clo_3 = -1;
        tempC_clo_3 = -1;
    }
    if (isnan(humidity_clo_4) || isnan(tempC_clo_4)) {
        isClothes4Error = true;
        humidity_clo_4 = -1;
        tempC_clo_4 = -1;
    }

    String error_message = "";
    if (isSurroundingError) {
        error_message += "Sur";
    }

    if (isClothes1Error || isClothes2Error || isClothes3Error || isClothes4Error) {
        error_message += "Clo_";
    }

    if (isClothes1Error) {
        error_message += "1";
    }

    if (isClothes2Error) {
        error_message += "2";
    }

    if (isClothes3Error) {
        error_message += "3";
    }

    if (isClothes4Error) {
        error_message += "4";
    }

    if (isSurroundingError || isClothes1Error || isClothes2Error || isClothes3Error || isClothes4Error) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DHT22 Error");
        lcd.setCursor(0, 1);
        lcd.print(error_message);
    }

    // 讀取電子稱數值
    // 重量為 10 次測量之平均
    weight_clo_1 = HX711_Clo1.get_units(10);
    weight_clo_2 = HX711_Clo2.get_units(10);
    weight_clo_3 = HX711_Clo3.get_units(10);
    weight_clo_4 = HX711_Clo4.get_units(10);

    if (weight_clo_1 < 0) {
        weight_clo_1 = -1;
    }

    if (weight_clo_2 < 0) {
        weight_clo_2 = -1;
    }

    if (weight_clo_3 < 0) {
        weight_clo_3 = -1;
    }

    if (weight_clo_4 < 0) {
        weight_clo_4 = -1;
    }

    // 未放置衣服
    if (weight_clo_1 < 80) {
        humidity_clo_1 = -1;
        tempC_clo_1 = -1;
        weight_clo_1 = -1;
    }
    if (weight_clo_2 < 80) {
        humidity_clo_2 = -1;
        tempC_clo_2 = -1;
        weight_clo_2 = -1;
    }
    if (weight_clo_3 < 80) {
        humidity_clo_3 = -1;
        tempC_clo_3 = -1;
        weight_clo_3 = -1;
    }
    if (weight_clo_4 < 80) {
        humidity_clo_4 = -1;
        tempC_clo_4 = -1;
        weight_clo_4 = -1;
    }

    // 重量穩定（變化率小於 5%），且不為 -1
    if (abs(weight_clo_1 - prev_weight_clo_1) / prev_weight_clo_1 < 0.05 && weight_clo_1 != -1) {
        steady_point_1++;
    } else {
        steady_point_1 = 0;
    }

    if (abs(weight_clo_2 - prev_weight_clo_2) / prev_weight_clo_2 < 0.05 && weight_clo_2 != -1) {
        steady_point_2++;
    } else {
        steady_point_2 = 0;
    }

    if (abs(weight_clo_3 - prev_weight_clo_3) / prev_weight_clo_3 < 0.05 && weight_clo_3 != -1) {
        steady_point_3++;
    } else {
        steady_point_3 = 0;
    }

    if (abs(weight_clo_4 - prev_weight_clo_4) / prev_weight_clo_4 < 0.05 && weight_clo_4 != -1) {
        steady_point_4++;
    } else {
        steady_point_4 = 0;
    }

    prev_weight_clo_1 = weight_clo_1;
    prev_weight_clo_2 = weight_clo_2;
    prev_weight_clo_3 = weight_clo_3;
    prev_weight_clo_4 = weight_clo_4;

    // 如果六十秒了
    if (counter < (60000 / DELAY_TIME)) {
        counter++;
        delay(1000);
        return;
    } else {
        counter = 0;
    }

    // 有沒有連續穩定五次
    if (steady_point_1 < 5) {
        humidity_clo_1 = -1;
        tempC_clo_1 = -1;
        weight_clo_1 = -1;
    }

    if (steady_point_2 < 5) {
        humidity_clo_2 = -1;
        tempC_clo_2 = -1;
        weight_clo_2 = -1;
    }

    if (steady_point_3 < 5) {
        humidity_clo_3 = -1;
        tempC_clo_3 = -1;
        weight_clo_3 = -1;
    }

    if (steady_point_4 < 5) {
        humidity_clo_4 = -1;
        tempC_clo_4 = -1;
        weight_clo_4 = -1;
    }

    clearContent(json_output1, 600);

    json_doc1 = StaticJsonDocument<600>();
    json_transmit_doc = StaticJsonDocument<1600>();

    char buffer[600] = {0};
    clearContent(buffer, 600);

    // 產生 JSON，記得設定精度到小數點後兩位
    json_doc1["uuid"] = uuid;

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc1["humidity_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc1["temperature_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", humidity_clo_1);
    json_doc1["humidity_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_clo_1);
    json_doc1["temperature_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", weight_clo_1);
    json_doc1["weight"] = buffer;
    clearContent(buffer, 600);

    ArduinoJson::serializeJson(json_doc1, json_output1);

    json_doc2["uuid"] = uuid;

    clearContent(json_output2, 600);

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc2["humidity_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc2["temperature_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", humidity_clo_2);
    json_doc2["humidity_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_clo_2);
    json_doc2["temperature_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", weight_clo_2);
    json_doc2["weight"] = buffer;
    clearContent(buffer, 600);

    ArduinoJson::serializeJson(json_doc2, json_output2);

    json_doc3["uuid"] = uuid;

    clearContent(json_output3, 600);

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc3["humidity_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc3["temperature_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", humidity_clo_3);
    json_doc3["humidity_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_clo_3);
    json_doc3["temperature_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", weight_clo_3);
    json_doc3["weight"] = buffer;
    clearContent(buffer, 600);

    ArduinoJson::serializeJson(json_doc3, json_output3);

    json_doc4["uuid"] = uuid;

    clearContent(json_output4, 600);

    sprintf(buffer, "%.2lf", humidity_sur);
    json_doc4["humidity_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_sur);
    json_doc4["temperature_sur"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", humidity_clo_4);
    json_doc4["humidity_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", tempC_clo_4);
    json_doc4["temperature_clothes"] = buffer;
    clearContent(buffer, 600);

    sprintf(buffer, "%.2lf", weight_clo_4);
    json_doc4["weight"] = buffer;
    clearContent(buffer, 600);

    ArduinoJson::serializeJson(json_doc4, json_output4);

    clearContent(doc_output, 1600);

    StaticJsonDocument<1600> doc;
    JsonArray sensor_data = doc.to<JsonArray>();
    sensor_data.add(json_output1);
    sensor_data.add(json_output2);
    sensor_data.add(json_output3);
    sensor_data.add(json_output4);
    ArduinoJson::serializeJson(doc, doc_output);

    clearContent(json_transmit_output, 1600);

    json_transmit_doc["sensor_data"] = doc_output;
    ArduinoJson::serializeJson(json_transmit_doc, json_transmit_output);

    Serial.println(json_transmit_output);

    // 檢查網路連線
    if (WiFi.status() != WL_CONNECTED) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Error");
        lcd.setCursor(0, 1);
        lcd.print("Reconnecting...");

        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(50);
        }
    }

    // 發送 POST 請求
    httpResponseCode = http.POST(json_transmit_output);

    if (httpResponseCode != 200) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("POST Error");
        lcd.setCursor(0, 1);
        lcd.print("Error code:" + String(httpResponseCode));
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("POST Success");
    }

    delay(DELAY_TIME);
    lcd.clear();
}

void clearContent(char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}
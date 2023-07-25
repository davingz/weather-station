  #if defined(ESP32)
  #include <WiFi.h>
  #elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #endif
  #include <Firebase_ESP_Client.h>
  #include <addons/RTDBHelper.h>
  #include <addons/TokenHelper.h>
  #define WIFI_SSID "Mobile Comm L_2.4G"
  #define WIFI_PASSWORD "mobcommsukses"
  #define DATABASE_URL "iot-colabs-default-rtdb.asia-southeast1.firebasedatabase.app" 
  #include <NTPClient.h>
  #include <WiFiUdp.h>

  const long utcOffsetInSeconds = 25200;
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);  

  //Week Days
  String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  //Month names
  String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

  FirebaseData fbdo;  

  FirebaseAuth auth;

  FirebaseConfig config;


  unsigned long dataMillis = 0;

  String localAddress = "Prototype";
  String currentDate;

  //for dht11
  #include "DHT.h"
  #define DHTPIN D4
  #define DHTTYPE DHT11 
  DHT dht(DHTPIN, DHTTYPE);

  //for rainsensor
  #define rainAnalog A0

  //for bh1750
  #include <BH1750.h> 

  //for bmp280
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_BMP280.h>
  #define BMP_SCK  (13)
  #define BMP_MISO (12)
  #define BMP_MOSI (11)
  #define BMP_CS   (10) 

  //display
  #include <LiquidCrystal_I2C.h>

  LiquidCrystal_I2C lcd(0x27, 16, 2);
  BH1750 lightMeter;
  Adafruit_BMP280 bmp;

  String suhu, temp, rain, lux, prss, hari, jam;

  byte Cok[8] =
    {
    0b01000,
    0b10100,
    0b01000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
    };

void setup() {
    Serial.begin(9600);
    Wire.begin();
    dht.begin();
    lightMeter.begin();
    lcd.begin();
    lcd.backlight();

    lcd.createChar(0,Cok);

    pinMode(rainAnalog,INPUT);

    pinMode(DHTPIN, INPUT);

    bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                 Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                 Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                 Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                 Adafruit_BMP280::STANDBY_MS_500); 

  // conect to wifi and firebase               
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        lcd.setCursor(0,1);
        lcd.print("Connecting to Wi-Fi");
        delay(300);
    }
    Serial.println();
    lcd.setCursor(6,2);
    lcd.print("Berhasil");
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    timeClient.begin();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.database_url = DATABASE_URL;

    config.signer.test_mode = true;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

}

void loop() {
    //Dht11
    byte Kelembaban = dht.readHumidity();
    byte Suhu = dht.readTemperature();

    //rainsensor  
    unsigned sensor;
    sensor = ( 100.097 - ( (analogRead(rainAnalog)/1023.00) * 100.00 ) );
    byte Hujan = sensor;

    //cahaya BH170
    byte Cahaya = lightMeter.readLightLevel();

    //BMP280
    float Pressure;
    float pressure = (bmp.readPressure()); Pressure = pressure;

    //mengubah to string data
        suhu = String(Suhu);
        temp = String(Kelembaban);
        rain = String(Hujan);
        lux = String(Cahaya);
        prss = String(Pressure);

    // waktu
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();

    String waktu = timeClient.getFormattedTime();


    String weekDay = weekDays[timeClient.getDay()];

    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int monthDay = ptm->tm_mday;

    int currentMonth = ptm->tm_mon+1;

    String currentMonthName = months[currentMonth-1];

    int currentYear = ptm->tm_year+1900;

    currentDate = String(weekDay) + "," + String(monthDay) + "-" + String(currentMonthName) + "-" + String(currentYear) + " " + String(waktu);
    jam = String(waktu);
    hari = String(weekDay) + "," + String(monthDay) + "-" + String(currentMonthName) + "-" + String(currentYear);

  connect();
    // //  Firebasegood
    //        if (millis() - dataMillis > 1000)
    // {
    //     dataMillis = millis();

    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Hujan"), rain +" %") ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Suhu"), suhu +" °C") ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Humadity"), temp +" %") ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Tingkat Cahaya"), lux +" %") ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Tekanan"), prss +" nPa") ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Tanggal"), hari) ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set int... %s\n", Firebase.RTDB.pushString(&fbdo, F("/Prototype/Waktu"), jam) ? "ok" : fbdo.errorReason().c_str());

    //     }

    // Display
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hujan:");
  lcd.setCursor(6,0);
  lcd.print(Hujan);
  lcd.setCursor(0,1);
  lcd.print("Cahaya:");
  lcd.setCursor(7,1);
  lcd.print(Cahaya);
  lcd.setCursor(0,2);
  lcd.print("Tekanan:");
  lcd.setCursor(8,2);
  lcd.print(Pressure);
  delay(1000);
  lcd.clear();
 
  lcd.setCursor(0,0);
  lcd.print("Suhu:");
  lcd.setCursor(5,0);
  lcd.print(Suhu);
  lcd.setCursor(7,0);
  lcd.write(0);
  lcd.setCursor(0,1);
  lcd.print("Kelembaban:");
  lcd.setCursor(11,1);
  lcd.print(Kelembaban);
  lcd.setCursor(13,1);
  lcd.print("%");
  delay(1000);
  
}

void connect(){

  FirebaseJson json;

json.add("Temperature", suhu);
json.add("Humadity", temp);
json.add("Hujan", rain);
json.add("Tingkat Cahaya", lux);
json.add("Tekanan Udara", prss);
json.add("Tanggal", hari);
json.add("Waktu", jam);

if (Firebase.RTDB.pushJSON(&fbdo, "/Prototype/test", &json)) {

  Serial.println(fbdo.dataPath());

  Serial.println(fbdo.pushName());

  Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());

} else {
  Serial.println(fbdo.errorReason());
}

 
 delay(3000);
}
//kurang pada bagian button
//kurang pada bagian alarm jam
//kurang pada bagian aktivasi jika jam terpenuhi
//kurang pada bagian parsing state
//kurang pada bagian parsing time
//kurang pada bagian send time
//kurang pada bagian send data sensor
//kurang pada bagian send data state
//kurang reset/delet item

//set name c;<name>;<ui>
//set wifi s;<ssid>;<password>
//reset r

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NewPing.h>
#include "RTClib.h"
#include <HTTPClient.h>
#include "SerialCmd.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
//Servo myServo;     // Objek servo

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Pin konfigurasi baru
const int buttonNext = 34;    // Pin untuk tombol Next
const int buttonSelect = 33;  // Pin untuk tombol Select
const int buttonUp = 25;      // Pin untuk tombol Up
const int buttonDown = 32;    // Pin untuk tombol Down
const int buttonServo = 35;   // Pin untuk tombol kontrol servo
const int servoPin = 27;      // Pin untuk menghubungkan servo

const int wifiLed = 26;

// Pin untuk sensor ultrasonik
const int trigPin = 23;       // Pin trigger
const int echoPin = 19;       // Pin echo


//code
String uid = "rOCBwJVW9QUzKGylMj7RSYz7fif1";
String new_id = "user_1";
String url = "https://pakanternak.pythonanywhere.com/";
String name_ = "makanan_1";
int aktifasi = 0;

String ssid = "";      // Nama WiFi
String password = "";  // Kata sandi WiFi

Preferences preferences;  // Untuk menyimpan dan memuat data
RTC_DS1307 rtc;           // Objek RTC
OneWire oneWire(13);      // Pin sensor DS18B20
DallasTemperature sensors(&oneWire);

NewPing sonar(trigPin, echoPin, 200); // Pin trig, echo, jarak maksimum (200 cm)

int currentMenu = 4;
int totalMenus = 5;  // Total menu menjadi 5 (Set Waktu 1, 2, 3, Data Sensor, Pengaturan)

int hourSet1 = 0, minuteSet1 = 0;
int hourSet2 = 0, minuteSet2 = 0;
int hourSet3 = 0, minuteSet3 = 0;

bool setHourMode = false;
bool setMinuteMode = false;
int selectedSetTime = 0;  // Untuk mengetahui set waktu mana yang sedang diatur

// Timer variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 500;  // 200ms debounce time
unsigned long displayStartTime = 0;
const unsigned long displayDuration = 3000;  // 3 seconds display for sensor or set time result

unsigned long lastTimeUpdate = 0; // Waktu terakhir tampilan diperbarui
const unsigned long updateInterval = 1000; // Interval pembaruan setiap 1 detik

// Variabel untuk menunjukkan data sensor
bool displayTemperature = true; // Flag untuk menentukan apakah akan menampilkan suhu atau jarak

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  
  pinMode(buttonNext, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonServo, INPUT_PULLUP);

  pinMode(wifiLed, OUTPUT);
  pinMode(servoPin, OUTPUT);

  

  //myServo.attach(servoPin);
  //myServo.write(0);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Cek apakah RTC berjalan dan set waktu jika tidak
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set waktu berdasarkan waktu kompilasi
  } else {
    Serial.println("RTC is running. No need to adjust.");
  }

  // Debug: Print current time to ensure it's working
  DateTime now = rtc.now();
  Serial.print("Current RTC Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  preferences.begin("time_data", false);
  loadTimeData();  // Load waktu yang disimpan
  
  if(ssid == ""){
    Serial.print("asdas");
  }
  else{
    Serial.println("ds");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  
  
  //Serial.println(aktifasi);
  if(aktifasi==1){
    get_time();
  }
 
  
  

  showMenu(currentMenu);  // Tampilkan menu pertama
  sensors.begin();

  attachCommand('s', setWifi);
  attachCommand('c', name_user);
}


void loop() {
  serialEvent();
  unsigned long currentTime = millis();

  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(wifiLed,HIGH);
  }
  else{
    
  }

  // Debounce button handling
  if (digitalRead(buttonNext) == LOW && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;  
    if (setHourMode || setMinuteMode) {
      return;
    } else {
      currentMenu++;
      if (currentMenu >= totalMenus) {
        currentMenu = 0;
      }
      showMenu(currentMenu);
    }
  }

  if (digitalRead(buttonSelect) == LOW && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;
    if (currentMenu < 3) {
      if (!setHourMode && !setMinuteMode) {
        setHourMode = true;
        selectedSetTime = currentMenu + 1;
        showSetTimeMenu();
      } else if (setHourMode) {
        setHourMode = false;
        setMinuteMode = true;
        showSetTimeMenu();
      } else if (setMinuteMode) {
        setMinuteMode = false;
        displayStartTime = currentTime;
        saveTimeData();  // Simpan waktu yang disetel
        showSetTimeResult();
      }
    } else if (currentMenu == 3) {
      displayStartTime = currentTime;
      showSensorData(); // Tampilkan data sensor
    } else if (currentMenu == 4) {
      displayStartTime = currentTime;
      showCurrentTime();  // Tampilkan waktu sekarang
    }
  }

  if (digitalRead(buttonUp) == LOW && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;
    increaseTime();
  }

  if (digitalRead(buttonDown) == LOW && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;
    decreaseTime();
  }

  if (digitalRead(buttonServo) == LOW && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;
    controlServo(1);
  }

  if (digitalRead(buttonServo) == HIGH && (currentTime - lastButtonPress) > debounceDelay) {
    lastButtonPress = currentTime;
    controlServo(0);
  }
  

  // Tambahan untuk memperbarui tampilan data sensor secara real-time
  if (currentMenu == 3) {
    if (currentTime - lastTimeUpdate > updateInterval) {
      lastTimeUpdate = currentTime;
      showSensorData();
    }
  }

  // Mengupdate tampilan waktu sekarang secara real-time
  if (currentMenu == 4) {
    if (currentTime - lastTimeUpdate > updateInterval) {
      lastTimeUpdate = currentTime;
      showCurrentTime(); // Perbarui tampilan waktu
    }
  }

  // Mengembalikan ke menu setelah tampilan sensor atau waktu
  if ((currentTime - displayStartTime) > displayDuration && displayStartTime != 0) {
    displayStartTime = 0;  // Reset displayStartTime
    showMenu(currentMenu);  // Tampilkan menu berdasarkan currentMenu
  }
}

void showMenu(int menu) {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch(menu) {
    case 0:
      lcd.print("1. Set Waktu 1");
      lcd.setCursor(0, 1);
      displayTime(hourSet1, minuteSet1);
      break;
    case 1:
      lcd.print("2. Set Waktu 2");
      lcd.setCursor(0, 1);
      displayTime(hourSet2, minuteSet2);
      break;
    case 2:
      lcd.print("3. Set Waktu 3");
      lcd.setCursor(0, 1);
      displayTime(hourSet3, minuteSet3);
      break;
    case 3:
      lcd.print("4. Data Sensor");
      lcd.setCursor(0, 1);
      lcd.print("Next: Tampilkan");
      break;
    case 4:
      lcd.print("5. Waktu Sekarang");
      lcd.setCursor(0, 1);
      lcd.print("Next: Tampilkan");
      break;
  }
}

void displayTime(int hour, int minute) {
  if (hour < 10) lcd.print("0");
  lcd.print(hour);
  lcd.print(":");
  if (minute < 10) lcd.print("0");
  lcd.print(minute);
}

void showSetTimeMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Waktu ");
  lcd.print(selectedSetTime);
  lcd.setCursor(0, 1);
  if (setHourMode) {
    lcd.print("Jam: ");
    lcd.print(getCurrentHour());
  } else if (setMinuteMode) {
    lcd.print("Menit: ");
    lcd.print(getCurrentMinute());
  }
}

void showSetTimeResult() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waktu ");
  lcd.print(selectedSetTime);
  lcd.print(" disetel:");
  
  lcd.setCursor(0, 1);
  int hour = getCurrentHour();
  int minute = getCurrentMinute();
  displayTime(hour, minute);
}

int getCurrentHour() {
  switch (selectedSetTime) {
    case 1: return hourSet1;
    case 2: return hourSet2;
    case 3: return hourSet3;
  }
  return 0;
}

int getCurrentMinute() {
  switch (selectedSetTime) {
    case 1: return minuteSet1;
    case 2: return minuteSet2;
    case 3: return minuteSet3;
  }
  return 0;
}

void setCurrentTime(int hour, int minute) {
  switch (selectedSetTime) {
    case 1: hourSet1 = hour; minuteSet1 = minute; break;
    case 2: hourSet2 = hour; minuteSet2 = minute; break;
    case 3: hourSet3 = hour; minuteSet3 = minute; break;
  }
}

void increaseTime() {
  int hour = getCurrentHour();
  int minute = getCurrentMinute();
  
  if (setHourMode) {
    hour++;
    if (hour >= 24) hour = 0;
  } else if (setMinuteMode) {
    minute++;
    if (minute >= 60) minute = 0;
  }
  
  setCurrentTime(hour, minute);
  showSetTimeMenu();
}

void decreaseTime() {
  int hour = getCurrentHour();
  int minute = getCurrentMinute();
  
  if (setHourMode) {
    hour--;
    if (hour < 0) hour = 23;
  } else if (setMinuteMode) {
    minute--;
    if (minute < 0) minute = 59;
  }
  
  setCurrentTime(hour, minute);
  showSetTimeMenu();
}

void controlServo(int state) { // Specify the type of 'state'
  digitalWrite(servoPin, state);
}


void showSensorData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data Sensor:");
  
  // Tampilkan suhu dari sensor
  sensors.requestTemperatures(); // Meminta suhu dari sensor
  float temperature = sensors.getTempCByIndex(0);
  
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print(" C");
  
  // Mengukur jarak menggunakan sensor ultrasonik
  delay(100); // Tunggu sebentar sebelum pengukuran
  unsigned int distance = sonar.ping_cm(); // Mengukur jarak dalam cm
  
  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print(distance);
  lcd.print(" cm");
  
  // Tampilkan suhu dan jarak secara bergantian
  if (displayTemperature) {
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print(" C");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("D:");
    lcd.print(distance);
    lcd.print(" cm");
  }
  
  displayTemperature = !displayTemperature; // Berganti tampilan
}

void showCurrentTime() {
  DateTime now = rtc.now(); // Mendapatkan waktu saat ini dari RTC
  DateTime future (now + TimeSpan(7,12,30,6));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waktu Sekarang:");
  
  // Debug print untuk memeriksa waktu dari RTC
  Serial.print("Jam: ");
  Serial.print(future.hour(),DEC);
  Serial.print(" Menit: ");
  Serial.println(future.minute(),DEC);

  lcd.setCursor(0, 1);
  displayTime(future.hour(), future.minute()); // Menampilkan jam dan menit
}

void saveTimeData() {
  preferences.putInt("hourSet1", hourSet1);
  preferences.putInt("minuteSet1", minuteSet1);
  preferences.putInt("hourSet2", hourSet2);
  preferences.putInt("minuteSet2", minuteSet2);
  preferences.putInt("hourSet3", hourSet3);
  preferences.putInt("minuteSet3", minuteSet3);
}

void loadTimeData() {
  hourSet1 = preferences.getInt("hourSet1", 0);
  minuteSet1 = preferences.getInt("minuteSet1", 0);
  hourSet2 = preferences.getInt("hourSet2", 0);
  minuteSet2 = preferences.getInt("minuteSet2", 0);
  hourSet3 = preferences.getInt("hourSet3", 0);
  minuteSet3 = preferences.getInt("minuteSet3", 0);
  aktifasi = preferences.getInt("aktifasi", 0);
  ssid = preferences.getString("ssid",ssid);
  password = preferences.getString("password",password);
  name_= preferences.getString("name_", name_);
  new_id =preferences.getString("new_id", new_id);
}


void set_id(){
  HTTPClient http;
  String data_ = url+ "update_id/" + uid + '/' + name_ ;
  Serial.print(data_);
  http.begin(data_);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"new_id\":\"" + String(new_id) + "\"}";
  int httpResponseCode = http.POST(jsonPayload);
 if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("Respons dari server: " + response);
  } else {
    Serial.println("Error pada HTTP request: " + String(httpResponseCode));
  }
  http.end();
}

void send_data(){
  
}

void send_state(){
  
}

void get_time(){
  HTTPClient http;
  String data_ = url+ "data/" + uid + '/' + name_ ;
  Serial.print(data_);
  http.begin(data_);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.GET();

    // Cek respons dari server
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response Code: " + String(httpResponseCode));
      Serial.println("Response Body: " + payload);
    } else {
      Serial.println("Error on HTTP request");
    }
}

void get_state(){
  HTTPClient http;
  String data_ = url+ "data/" + uid + '/' + name_ ;
  Serial.print(data_);
  http.begin(data_);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.GET();

    // Cek respons dari server
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response Code: " + String(httpResponseCode));
      Serial.println("Response Body: " + payload);
    } else {
      Serial.println("Error on HTTP request");
    }
}

void setWifi(){
  Serial.println(cmd[1]);
  Serial.println(cmd[2]);

  WiFi.begin(cmd[1], cmd[2]);
  preferences.putString("ssid", cmd[1]);
  preferences.putString("password", cmd[2]);

  

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.status());
}

void name_user(){
  Serial.print("akifasii:");
  Serial.println(aktifasi);
  name_ = cmd[1];
  new_id = cmd[2];
   preferences.putString("name_", name_);
  preferences.putString("new_id", new_id);
  if(aktifasi == 0){
    Serial.println("kesini");
    set_id();
    aktifasi = 1;
    preferences.putInt("aktifasi", aktifasi);
    
  }
}

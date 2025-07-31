/* Important information
Board library: esp32 by Espressif Systems: 3.0.0 installed

USB CDC on boot: Enable
USB DFU on boot: Disable
Partition Scheme：RainMalker
PSRAM：Enable
*/

#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include <Audio.h>
#include <Preferences.h>
#include <vector>
#include <HTTPClient.h>
// WiFiServer httpServer(80);
// WiFiServer wsServer(80);
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>  // 引入用于 HTTPS 的库
#include <EEPROM.h>
#include <Ticker.h>
#define byte uint8_t
#include <WiFiManager.h>  // 需要安装 WiFiManager 库
#include <ESPmDNS.h>      //
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

//https://github.com/olikraus/u8g2 字体library
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
//中文字 https://www.kidsgo.net/u8g2/index.php
#include <u8g2_font.h>  //2451 字体
#include <time.h>
#include <sntp.h>

Audio audio;
Preferences preferences;

const char* playlistURL = "https://raw.githubusercontent.com/skylai82/WemosS2PicoRadio/main/CudeRadioPlaylist.m3u8";
std::vector<String> songTitles;  // 存储歌曲标题
std::vector<String> songURLs;    // 存储歌曲 URL
int currentSongIndex = 0;
String currentSongTitle = "";
int volume = 10;

#define I2S_DOUT 33
#define I2S_BCLK 35
#define I2S_LRC 37
#define SD_MODE 16
#define NEXT_BUTTON_PIN 3
#define PREV_BUTTON_PIN 5
#define VOLUME_UP_PIN 7
#define VOLUME_DOWN_PIN 9
#define I2C_SDA 11
#define I2C_SCL 12
//Wemos PICO pin18 for Display Reset,
//Wemos Mini Booster pin 18 for MAX audio booster
// Table 8. Gain Selection
// GAIN_SLOT I2S/LJ                            GAIN (dB)
// Connect to GND through 100kΩ ±5% resistor    15
// Connect to GND                               12
// Unconnected                                   9
// Connect to VDD                                6
// Connect to VDD through 100kΩ ±5% resistor     3

#define OLED_RST 18
#define LED_status 15

#define U8X8_PIN_NONE -1
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, I2C_SCL, I2C_SDA);

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 28800;  // 马来西亚时间偏移为+8小时
const int daylightOffset_sec = 0;  // 马来西亚没有夏令时

//////////////////////////////////显示效果Channel 时间 Volume///////////////////////////////
unsigned long previousMillis = 0;
const long interval = 30 * 1000;  // xx秒

// unsigned long previousNosoundMillis = 0;
// const long intervalNosound = 1000;  // 1秒

// #include <ArduinoOTA.h>

// void local_wifi_ota() {
//   // 初始化 OTA
//   ArduinoOTA.setPassword("820105");  // 添加密码
//   ArduinoOTA.onStart([]() {
//     String type;
//     if (ArduinoOTA.getCommand() == U_FLASH) {  //U_FLASH
//       type = "sketch";
//     } else {  // U_SPIFFS
//       type = "filesystem";
//     }
//     // NOTE: 如果使用 SPIFFS，可能需要擦除文件系统
//     Serial.println("Start updating " + type);
//     u8g2.clearBuffer();
//     u8g2.setCursor(0, 14);
//     u8g2.println("ArduinoOTA.onStart");
//     u8g2.setCursor(0, 30);
//     u8g2.print("Start updating " + type);
//     u8g2.sendBuffer();
//   });

//   ArduinoOTA.onEnd([]() {
//     Serial.println("\nUpdate complete");
//     u8g2.clearBuffer();
//     u8g2.setCursor(0, 14);
//     u8g2.println("ArduinoOTA.onEnd");
//     u8g2.setCursor(0, 30);
//     u8g2.print("Upload.Complete");
//     u8g2.sendBuffer();
//   });

//   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//     Serial.printf("Progress: %u%%\r", (progress * 100) / total);
//     u8g2.clearBuffer();
//     u8g2.setCursor(0, 14);
//     u8g2.println("ArduinoOTA.onProgress");
//     u8g2.setCursor(0, 30);
//     u8g2.printf("Progress: %u%%\r", (progress * 100) / total);
//     u8g2.sendBuffer();
//   });

//   ArduinoOTA.onError([](ota_error_t error) {
//     Serial.printf("Error[%u]: ", error);
//     if (error == OTA_AUTH_ERROR) {
//       Serial.println("Auth Failed");
//     } else if (error == OTA_BEGIN_ERROR) {
//       Serial.println("Begin Failed");
//     } else if (error == OTA_CONNECT_ERROR) {
//       Serial.println("Connect Failed");
//     } else if (error == OTA_RECEIVE_ERROR) {
//       Serial.println("Receive Failed");
//     } else if (error == OTA_END_ERROR) {
//       Serial.println("End Failed");
//     }
//   });

//   ArduinoOTA.begin();
//   // Serial.println("OTA ready");
//   // u8g2.clearBuffer();
//   // u8g2.setCursor(0, 14);
//   // u8g2.println("ArduinoOTA.begin");
//   // u8g2.setCursor(0, 30);
//   // u8g2.print("OTA ready");
//   // u8g2.sendBuffer();

//   Serial.println("local_wifi_ota");
//   // u8g2.clearBuffer();
//   // u8g2.setCursor(0, 14);
//   // u8g2.println("Local WIFI OTA");  // 显示广告信息
//   // u8g2.setCursor(0, 30);
//   // Serial.print("IP Address: ");
//   // Serial.println(WiFi.localIP().toString());
//   // u8g2.print(WiFi.localIP().toString());  // 打印 IP 地址
//   // u8g2.sendBuffer();
// }


void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println(F("No time available (yet)"));
    return;
  }
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);
  u8g2.setCursor(65, 30);
  u8g2.println(timeStr);  //中文
  Serial.println(timeStr);
}

void timeavailable(struct timeval* t) {
  Serial.println(F("Got time adjustment from NTP!"));
  printLocalTime();
}

//update Channel & Vol to display
void showCHVL() {
  Serial.println("Now playing: " + songTitles[currentSongIndex]);
  Serial.println(songURLs[currentSongIndex].c_str());
  Serial.println("Volume: " + String(volume));  // Print Volume Level
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println(currentSongTitle);  //中文
  u8g2.setCursor(0, 30);
  u8g2.println("Vol:");  //中文
  u8g2.println(volume);  //中文
  printLocalTime();
  u8g2.sendBuffer();
}

void update_started() {
  Serial.println("Start update");
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("Release/放开");  //中文
  u8g2.setCursor(0, 30);
  u8g2.println("Vol + & -");  //中文
  u8g2.sendBuffer();

  audio.connecttospeech("Start update new firmware", "en");  // Google TTS
  //audio.connecttospeech("欢迎使用方块网络收音机", "cmn-CN");  // Google TTS
  while (audio.isRunning()) {
    audio.loop();
  }

  for (uint8_t t = 5; t > 0; t--) {
    Serial.printf("[Countdown update] WAIT %d...\n", t);
    Serial.flush();
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("Countdown");  //中文
    u8g2.setCursor(0, 30);
    u8g2.printf("Update:%d...\n", t);  //中文
    u8g2.sendBuffer();
    delay(500);
  }
}

void update_progress(int cur, int total) {
  int percentage = (cur * 100) / total;
  Serial.println(F("Updating"));
  Serial.printf(" %d%%...\n", percentage);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("Updating");  //中文
  u8g2.setCursor(0, 30);
  u8g2.printf(" %d%%...\n", percentage);  //中文
  u8g2.sendBuffer();
}

void update_error(int err) {
  Serial.printf("CALLBACK: HTTP update fatal error code %d\n", err);
}

void update_finished() {
  Serial.println("CALLBACK: HTTP update process finished");
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("New Firmware ");  //中文
  u8g2.setCursor(0, 30);
  u8g2.println("Update Done");  //中文
  u8g2.sendBuffer();

  audio.connecttospeech("New firmware update completed", "en");  // Google TTS
  //audio.connecttospeech("欢迎使用方块网络收音机", "cmn-CN");  // Google TTS
  while (audio.isRunning()) {
    audio.loop();
  }

  audio.connecttospeech("Restart", "en");  // Google TTS
  //audio.connecttospeech("欢迎使用方块网络收音机", "cmn-CN");  // Google TTS
  while (audio.isRunning()) {
    audio.loop();
  }
  audio.setVolume(0);
  digitalWrite(SD_MODE, LOW);
  delay(500);
}

void performFirmwareUpdate() {


  // 设置回调
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);

  // 使用 WiFiClientSecure 进行 HTTPS 连接
  WiFiClientSecure client;
  client.setInsecure();  // 跳过证书验证

  // 开始 OTA 更新
  t_httpUpdate_return ret = httpUpdate.update(client, "https://raw.githubusercontent.com/skylai82/WemosS2PicoRadio/refs/heads/main/CudeRadioPlaylist.ino.bin");

  // 处理更新结果
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

//保存5个WiFi ID 和 密码
const int maxNetworks = 5;
void saveWiFiCredentials(const char* ssid, const char* password) {
  preferences.begin("wifi-config", false);

  bool foundDuplicate = false;
  int emptySlot = -1;

  for (int i = 0; i < maxNetworks; i++) {
    String ssidKey = "ssid" + String(i);
    String passwordKey = "password" + String(i);
    String storedSSID = preferences.getString(ssidKey.c_str(), "");
    String storedPassword = preferences.getString(passwordKey.c_str(), "");

    // 检查是否存在完全相同的SSID和密码
    if (storedSSID == ssid && storedPassword == password) {
      foundDuplicate = true;
      break;  // 已经存在完全相同的SSID和密码，不需要保存
    }

    // 找到第一个空的插槽
    if (storedSSID == "" && emptySlot == -1) {
      emptySlot = i;
    }
  }

  if (!foundDuplicate) {
    // 如果找到空槽位，则保存新的WiFi信息
    if (emptySlot != -1) {
      String ssidKey = "ssid" + String(emptySlot);
      String passwordKey = "password" + String(emptySlot);
      preferences.putString(ssidKey.c_str(), ssid);
      preferences.putString(passwordKey.c_str(), password);
      Serial.println("WiFi credentials saved in slot " + String(emptySlot));
    } else {
      // 所有插槽都已满，覆盖第一个插槽
      preferences.putString("ssid0", ssid);
      preferences.putString("password0", password);
      Serial.println("No empty slot available. Overwriting slot 0.");
    }

    // 显示保存成功的信息
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("New WIFI Added");  // 中文
    u8g2.setCursor(0, 30);
    u8g2.println("& Restart");  // 中文
    u8g2.sendBuffer();
    delay(2000);
    ESP.restart();
  } else {
    Serial.println("WiFi credentials already exist. Skipping save.");
  }

  preferences.end();
}

// 加载保存的 WiFi 凭据并添加到 WiFiMulti 中
void loadWiFiCredentials() {
  preferences.begin("wifi-config", true);
  Serial.println("Loading saved WiFi networks:");

  for (int i = 0; i < maxNetworks; i++) {
    String ssidKey = "ssid" + String(i);
    String passwordKey = "password" + String(i);
    String savedSSID = preferences.getString(ssidKey.c_str(), "");
    String savedPassword = preferences.getString(passwordKey.c_str(), "");

    if (savedSSID.length() > 0 && savedPassword.length() > 0) {
      wifiMulti.addAP(savedSSID.c_str(), savedPassword.c_str());  //这样已经加入保存的WIFI SSID 和 Password吗？
      Serial.println("Added WiFi Network " + String(i) + ": " + savedSSID);
    } else {
      Serial.println("Slot " + String(i) + " is empty or invalid.");
    }
  }

  preferences.end();
}

// 连接到 WiFi
void connectToWiFi() {
  Serial.println("Trying to connect to WiFi...");

  // WiFiMulti.run() 会自动选择最优的 WiFi 进行连接
  int maxRetries = 3;
  for (int retryCountWifi = 0; retryCountWifi < maxRetries; retryCountWifi++) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("Connecting..");
    u8g2.setCursor(0, 30);
    u8g2.println("Saved WIFI list");
    u8g2.sendBuffer();


    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.println("Connected to WiFi!");
      Serial.print("Current WiFi SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

      u8g2.clearBuffer();
      u8g2.setCursor(0, 14);
      u8g2.println("WiFi Connected");
      u8g2.setCursor(0, 30);
      u8g2.println(WiFi.SSID());
      u8g2.sendBuffer();
      break;
    }

    Serial.print(".");
    delay(1000);  // 每次重试间隔1秒
  }

  Serial.println("\nNo WiFi could be connected.");
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("No WiFi Connected");
  u8g2.setCursor(0, 30);
  u8g2.println("Starting AP...");
  u8g2.sendBuffer();

  // 如果所有保存的网络都无法连接，可以启动 WiFi Manager 进入 AP 模式
  // wifiManager();
}


void wifiManager() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(300);  // 设置AP模式下的超时时间为300秒(5分钟)
  wifiManager.setConnectTimeout(0);         // 设置连接超时时间为30秒

  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Connect WIFI:");
  u8g2.setCursor(0, 30);
  u8g2.println("VeryCubeWiFi");
  u8g2.sendBuffer();
  delay(3000);  // 减少延迟以加快响应
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Add New WiFi");
  u8g2.setCursor(0, 30);
  u8g2.println("key in password");
  u8g2.sendBuffer();

  delay(1000);
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);  // 填充温度和湿度对象
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degrees C");
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println("% rH");

  if (temp.temperature > 10) {
    // 清空显示缓冲区
    u8g2.clearBuffer();

    // 显示湿度，格式为 H:60.00%
    u8g2.setCursor(0, 14);
    u8g2.print("H:");
    u8g2.setCursor(15, 14);
    u8g2.print(humidity.relative_humidity, 1);  // 直接使用humidity.relative_humidity，保留两位小数显示湿度
    u8g2.print("%");                            // 添加百分号

    // 显示温度，格式为 T:29.3°C
    u8g2.setCursor(60, 14);
    u8g2.print("T:");
    u8g2.setCursor(75, 14);
    u8g2.print(temp.temperature, 1);  // 直接使用temp.temperature，保留一位小数
    u8g2.print("'C");                 // 添加摄氏度符号
    //u8g2.print("\xB0C");     // 使用 \xB0 显示摄氏度符号 °C

    // 显示信息
    u8g2.setCursor(0, 30);
    u8g2.print("Humi & Temp");  // 显示广告信息
    u8g2.sendBuffer();          // 发送缓冲区到显示器
  }

  if (!wifiManager.autoConnect("VeryCubeWiFi")) {
    Serial.println("Restart...");
    ESP.restart();  // 如果连接失败，则重新启动设备
  }

  // 确保WiFiManager连接成功后才获取SSID和密码
  String newSSID = WiFi.SSID();
  String newPassword = WiFi.psk();

  if (newSSID != "" && newPassword != "") {
    Serial.println("Saving new WiFi credentials:");
    Serial.println("SSID: " + newSSID);
    Serial.println("Password: " + newPassword);
    saveWiFiCredentials(newSSID.c_str(), newPassword.c_str());
  } else {
    Serial.println("No new credentials to save.");
  }
}

//Function to restore factory settings(reset saved WiFi credentials)
void resetWifi() {
  Serial.println("Restoring factory settings...");
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("WiFi Reset &");
  u8g2.setCursor(0, 30);
  u8g2.println("Restarting...");
  u8g2.sendBuffer();

  preferences.begin("wifi-config", false);
  for (int i = 0; i < maxNetworks; i++) {
    String ssidKey = "ssid" + String(i);
    String passwordKey = "password" + String(i);
    preferences.remove(ssidKey.c_str());
    preferences.remove(passwordKey.c_str());
  }
  preferences.end();
  Serial.println("All WiFi credentials cleared.");

  // 断开当前WiFi连接并清除配置
  WiFi.disconnect(true, true);  // 断开连接并删除 WiFi 设置
  delay(1000);                  // 等待1秒钟确保断开连接

  // 初始化 WiFiManager 并启动配置门户
  WiFiManager wifiManager;
  wifiManager.resetSettings();  // 这个方法可以清除掉 WiFiManager 存储的所有设置
  Serial.println("WiFiManager settings cleared.");

  ESP.restart();
}


void keyButton() {
  if (digitalRead(PREV_BUTTON_PIN) == LOW) {
    unsigned long currentMillis = 0;
    previousMillis = currentMillis;
    audio.stopSong();
    Serial.println("Previous button pressed");
    Serial.println("Now playing: " + currentSongTitle);  // Print the new song title
    currentSongIndex = (currentSongIndex - 1 + songURLs.size()) % songURLs.size();
    currentSongTitle = songTitles[currentSongIndex];
    savePreferences();  // Save station, title, and volume
    showCHVL();
    delay(500);
    if (digitalRead(PREV_BUTTON_PIN) == LOW) {
      Serial.println("Previous button pressed");
      Serial.println("Now playing: " + currentSongTitle);  // Print the new song title
      currentSongIndex = (currentSongIndex - 1 + songURLs.size()) % songURLs.size();
      currentSongTitle = songTitles[currentSongIndex];
      savePreferences();  // Save station, title, and volume
      showCHVL();
      delay(500);
    } else {
      savePreferences();  // Save station, title, and volume
      audio.connecttohost(songURLs[currentSongIndex].c_str());
      // for (int i = 0; i < 3; i++) {
      //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
      //   Serial.println("Buffering。。");
      // }
    }
  }

  if (digitalRead(NEXT_BUTTON_PIN) == LOW) {
    unsigned long currentMillis = 0;
    previousMillis = currentMillis;
    audio.stopSong();
    Serial.println("Next button pressed");
    Serial.println("Now playing: " + currentSongTitle);  // Print the new song title
    currentSongIndex = (currentSongIndex + 1) % songURLs.size();
    currentSongTitle = songTitles[currentSongIndex];
    savePreferences();  // Save station, title, and volume
    showCHVL();
    delay(500);
    if (digitalRead(NEXT_BUTTON_PIN) == LOW) {
      Serial.println("Next button pressed");
      Serial.println("Now playing: " + currentSongTitle);  // Print the new song title
      currentSongIndex = (currentSongIndex + 1) % songURLs.size();
      currentSongTitle = songTitles[currentSongIndex];
      savePreferences();  // Save station, title, and volume
      showCHVL();
      delay(500);
    } else {
      savePreferences();  // Save station, title, and volume
      audio.connecttohost(songURLs[currentSongIndex].c_str());
      // for (int i = 0; i < 3; i++) {
      //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
      //   Serial.println("Buffering。。");
      // }
    }
  }



  if (digitalRead(VOLUME_UP_PIN) == LOW && digitalRead(VOLUME_DOWN_PIN) == LOW) {  //怎样修改代码，100ms里面按VOLUME_UP、DOWN_PIN 就执行？
    audio.setVolume(0);
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("Audio Mute");  // 显示广告信息
    u8g2.setCursor(0, 30);
    u8g2.println("Press Vol Unmute");  // 显示中文广告信息
    u8g2.sendBuffer();
    delay(500);
    while (digitalRead(VOLUME_UP_PIN) != LOW || digitalRead(VOLUME_DOWN_PIN) != LOW) {
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);  // 填充温度和湿度对象
      Serial.print("Temperature: ");
      Serial.print(temp.temperature);
      Serial.println(" degrees C");
      Serial.print("Humidity: ");
      Serial.print(humidity.relative_humidity);
      Serial.println("% rH");

      if (isnan(humidity.relative_humidity) || isnan(temp.temperature) || temp.temperature < 1 || temp.temperature > 85) {  //AHT30
        Serial.println(F("Failed to read from AHT sensor!"));
        u8g2.clearBuffer();
        u8g2.setCursor(0, 14);
        u8g2.println("Audio Mute");  // 显示广告信息
        u8g2.setCursor(0, 30);
        u8g2.println("Press Vol Unmute");  // 显示中文广告信息
        u8g2.sendBuffer();
      } else {
        // 清空显示缓冲区
        u8g2.clearBuffer();

        // 显示湿度，格式为 H:60.00%
        u8g2.setCursor(0, 14);
        u8g2.print("H:");
        u8g2.setCursor(15, 14);
        u8g2.print(humidity.relative_humidity, 1);  // 直接使用humidity.relative_humidity，保留两位小数显示湿度
        u8g2.print("%");                            // 添加百分号

        // 显示温度，格式为 T:29.3°C
        u8g2.setCursor(60, 14);
        u8g2.print("T:");
        u8g2.setCursor(75, 14);
        u8g2.print(temp.temperature, 1);  // 直接使用temp.temperature，保留一位小数
        u8g2.print("'C");                 // 添加摄氏度符号
                                          //u8g2.print("\xB0C");     // 使用 \xB0 显示摄氏度符号 °C

        // 显示信息
        u8g2.setCursor(0, 30);
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP().toString());
        u8g2.print(WiFi.localIP().toString());  // 打印 IP 地址
        u8g2.sendBuffer();

        unsigned long currentMillis = 0;
        previousMillis = currentMillis;
      }
      if (digitalRead(VOLUME_UP_PIN) == LOW || digitalRead(VOLUME_DOWN_PIN) == LOW) {
        preferences.begin("RadioSettings", false);      // false means read/write mode
        volume = preferences.getInt("lastVolume", 10);  // Load last volume
        preferences.end();
        audio.setVolume(volume);
        audio.connecttohost(songURLs[currentSongIndex].c_str());
        // for (int i = 0; i < 3; i++) {
        //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
        //   Serial.println("Buffering。。");
        // }
        return;
      }
      delay(10);
    }
  }

  if (digitalRead(VOLUME_UP_PIN) == LOW) {
    changeVolume(1);
  }

  if (digitalRead(VOLUME_DOWN_PIN) == LOW) {
    changeVolume(-1);
  }
}

//Volume Key Function
unsigned long lastVolumeChangeTime = 0;
const int VOLUME_CHANGE_DELAY = 180;

void changeVolume(int amount) {
  if (millis() - lastVolumeChangeTime >= VOLUME_CHANGE_DELAY) {
    volume = constrain(volume + amount, 0, 21);
    audio.setVolume(volume);
    lastVolumeChangeTime = millis();

    // 保存音量设置
    savePreferences();  // Save station, title, and volume
    Serial.print("currentVolume:");
    Serial.println(volume);
    showCHVL();
  }
}

void loadPlaylist() {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("Loading...");
  u8g2.setCursor(0, 30);
  u8g2.print("Playlist");
  u8g2.sendBuffer();

  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected. Retrying...");
      delay(1000);
      continue;
    }

    Serial.print("Device IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("WiFi Signal Strength: ");
    Serial.println(WiFi.RSSI());

    //WiFiClientSecure secureClient;
    //secureClient.setInsecure();  // 测试时跳过证书验证
    HTTPClient http;
    //http.begin(secureClient, playlistURL);
    http.begin(playlistURL);

    int httpCode = http.GET();
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      parsePlaylist(payload);
      http.end();
      break;
    } else {
      Serial.print("HTTP error code: ");
      Serial.println(httpCode);
      http.end();
    }

    delay(500);
  }
}

void parsePlaylist(String payload) {
  int pos = 0;
  songTitles.clear();
  songURLs.clear();

  while ((pos = payload.indexOf("#EXTINF", pos)) != -1) {
    int titlePos = payload.indexOf(",", pos);
    int nextLinePos = payload.indexOf("\n", titlePos);

    if (titlePos != -1 && nextLinePos != -1) {
      String songTitle = payload.substring(titlePos + 1, nextLinePos);
      songTitle.trim();
      songTitles.push_back(songTitle);
      //Serial.println("Song Title: " + songTitle);
    }

    pos = nextLinePos + 1;
    int urlEndPos = payload.indexOf("\n", pos);
    if (urlEndPos == -1) urlEndPos = payload.length();

    String songURL = payload.substring(pos, urlEndPos);
    songURL.trim();
    songURLs.push_back(songURL);
    //Serial.println("Song URL: " + songURL);
    pos = urlEndPos;
    //yield();  // 喂狗，防止看门狗定时器重启
    //delay(10);
  }
}


void savePreferences() {
  preferences.begin("RadioSettings", false);
  preferences.putInt("lastStation", currentSongIndex);               // Save station index
  preferences.putString("lastSongTitle", currentSongTitle);          // Save song title
  preferences.putInt("lastVolume", volume);                          // Save volume
  preferences.putString("lastSongURL", songURLs[currentSongIndex]);  // Save stream URL
  preferences.end();
}

void scanNetworks() {
  int numNetworks = WiFi.scanNetworks();
  Serial.println("Available networks:");
  if (numNetworks == 0) {
    Serial.println("No networks found");
  } else {
    for (int i = 0; i < numNetworks; i++) {
      Serial.print("SSID: ");
      Serial.print(WiFi.SSID(i));
      Serial.print(", Signal Strength: ");
      Serial.println(WiFi.RSSI(i));
    }
  }
}

bool firstRun = true;  // 标记是否为第一次运行

///////////////////////////////////////////////////////////Setup//////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(100);

  // // 设置最大 CPU 频率为 240MHz
  // setCpuFrequencyMhz(240);  // Arduino 封装函数
  // delay(100);

  Serial.print("Current CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  aht.begin();
  //while(!Serial); //等到得到串口才下行

  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VOLUME_UP_PIN, INPUT_PULLUP);
  pinMode(VOLUME_DOWN_PIN, INPUT_PULLUP);

  //播放LED
  pinMode(LED_status, OUTPUT);
  digitalWrite(LED_status, LOW);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(100);
  digitalWrite(OLED_RST, HIGH);
  delay(100);
  digitalWrite(OLED_RST, LOW);
  delay(100);

  pinMode(SD_MODE, OUTPUT);
  digitalWrite(SD_MODE, HIGH);  //SD_MODE = High(Left channel), LOW(Shutdown)

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_unifont_myfonts);
  u8g2.clearBuffer();
  u8g2.setCursor(23, 15);
  u8g2.print("My Name iS");  //中文
  u8g2.setCursor(28, 30);
  u8g2.println("Very Cube");  //中文
  u8g2.sendBuffer();
  delay(2000);

  // 设置设备名称
  WiFi.setHostname("Very Cube");
  // 启动mDNS服务，直接省略if语句
  MDNS.begin("Very Cube");

  if (digitalRead(PREV_BUTTON_PIN) == LOW && digitalRead(NEXT_BUTTON_PIN) == LOW && digitalRead(VOLUME_UP_PIN) == LOW && digitalRead(VOLUME_DOWN_PIN) == LOW) {
    resetWifi();
  }

  // force wifi manager
  if (digitalRead(PREV_BUTTON_PIN) == LOW && digitalRead(NEXT_BUTTON_PIN) == LOW) {
    //WiFi.disconnect(true, true);  // 断开连接并删除 WiFi 设置
    wifiManager();
  }

  // 加载保存的 WiFi 凭据并添加到 WiFiMulti 中
  loadWiFiCredentials();
  wifiMulti.addAP("CUBE", "CUBE12345678");
  wifiMulti.addAP("CUBE", "12345678loadWiFiCredentials");
  wifiMulti.addAP("JanetWifi", "0164640031");
  //wifiMulti.addAP("NASA", "0134528247");

  // 尝试连接 WiFi
  connectToWiFi();

  // 通过 wifiMulti 尝试连接已保存的WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.println("\nWiFi Connected.");
    Serial.print("Current WiFi SSID: ");  // 打印连接的WiFi SSID
    Serial.println(WiFi.SSID());          // 打印连接的WiFi SSID
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("WiFi Connected");
    u8g2.setCursor(0, 30);
    u8g2.println(WiFi.SSID());
    u8g2.sendBuffer();
    delay(1000);
  } else {
    wifiManager();
  }

  // 添加内存检查
  Serial.println("Free heap: " + String(ESP.getFreeHeap()));

  sntp_set_time_sync_notification_cb(timeavailable);
  sntp_servermode_dhcp(1);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  u8g2.sendBuffer();
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("(Press and Hold)");
  u8g2.setCursor(0, 30);
  u8g2.println("Vol + & - Update");
  u8g2.sendBuffer();
  delay(1000);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);

  // Manual OTA update
  if (digitalRead(VOLUME_UP_PIN) == LOW && digitalRead(VOLUME_DOWN_PIN) == LOW) {
    performFirmwareUpdate();
    // 在这里进行OTA更新
  }

  Serial.printf("Free heap before load: %u\n", ESP.getFreeHeap());
  loadPlaylist();  // Load the playlist from URL
  Serial.printf("Free heap after load: %u\n", ESP.getFreeHeap());
  u8g2.sendBuffer();
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.println("Playlist Loaded");
  u8g2.setCursor(0, 30);
  u8g2.println("Done");
  u8g2.sendBuffer();
  preferences.begin("RadioSettings", false);                      // false means read/write mode
  currentSongIndex = preferences.getInt("lastStation", 0);        // Load last station index
  currentSongTitle = preferences.getString("lastSongTitle", "");  // Load last song title
  volume = preferences.getInt("lastVolume", 10);                  // Load last volume
  String lastSongURL = preferences.getString("lastSongURL", "");  // Load last stream URL
  preferences.end();

  // volume = max(volume - 4, 7);                                            // 确保音量不会低于 0
  audio.setVolume(volume);                                                // 设置调整后的音量
  savePreferences();                                                      // 保存当前设置
  audio.connecttospeech("Welcome Using Very Cube Internet Radio", "en");  // Google TTS
  //audio.connecttospeech("欢迎使用方块网络收音机", "cmn-CN");  // Google TTS
  while (audio.isRunning()) {
    audio.loop();
    if (digitalRead(PREV_BUTTON_PIN) == LOW || digitalRead(NEXT_BUTTON_PIN) == LOW || digitalRead(VOLUME_UP_PIN) == LOW || digitalRead(VOLUME_DOWN_PIN) == LOW) {
      break;
    }
  }

  firstRun = false;  // 标记为已执行过第一次

  currentSongTitle = songTitles[currentSongIndex];
  Serial.println(volume);
  Serial.println(currentSongTitle);
  Serial.println(songURLs[currentSongIndex].c_str());
  showCHVL();


  // 检查是否有保存的流 URL 并连接
  if (lastSongURL.length() > 0) {
    audio.connecttohost(lastSongURL.c_str());  // Connect to last saved stream URL
  } else {
    Serial.println("No saved stream URL found.");
    audio.connecttospeech("没有最后一个电台记忆", "cmn-CN");  // Google TTS
  }

  // 调用 audio.loop() 来处理音频播放缓冲
  // for (int i = 0; i < 5; i++) {
  //   //keyButton();
  //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
  //   Serial.println("Start up Loop x5");
  // }
  // local_wifi_ota();
}

//////////////////////////////////////////////////////////Loop//////////////////////////////////////////////
void loop() {
  // ArduinoOTA.handle();  // 检测 OTA 更新请求

  keyButton();
  audio.loop();
  digitalWrite(LED_status, LOW);

  if (digitalRead(NEXT_BUTTON_PIN) == LOW || digitalRead(PREV_BUTTON_PIN) == LOW) {
    //为了跳过长安Volume
  } else {
    if (!audio.isRunning()) {
      preferences.begin("RadioSettings", false);                      // false means read/write mode
      currentSongIndex = preferences.getInt("lastStation", 0);        // Load last station index
      currentSongTitle = preferences.getString("lastSongTitle", "");  // Load last song title
      volume = preferences.getInt("lastVolume", 10);                  // Load last volume
      preferences.end();
      currentSongTitle = songTitles[currentSongIndex];
      audio.connecttohost(songURLs[currentSongIndex].c_str());
      audio.loop();
      // for (int i = 0; i < 3; i++) {
      //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
      //   Serial.println("Buffering。。");
      // }
    }
    digitalWrite(LED_status, HIGH);
  }

  unsigned long currentMillis = millis();
  //Serial.print("interval:");
  //Serial.println(currentMillis - previousMillis);
  if (currentMillis - previousMillis >= interval) {
    // 每隔30秒更新时间
    showCHVL();
    previousMillis = currentMillis;
  }

  if (Serial.available()) {
    Serial.println(F("Receive Serial info"));
    String r = Serial.readString();  // 读取Serial数据到r
    // 查找 "http" 子字符串
    if (strstr(r.c_str(), "http")) {
      Serial.println(F("Change Radio"));
      r.trim();  // 去除空格
      Serial.println(r.c_str());
      audio.connecttohost(r.c_str());
      // for (int i = 0; i < 3; i++) {
      //   audio.loop();  // 多次调用 audio.loop() 来确保缓冲区填满
      //   Serial.println("Buffering...");
      // }
    } else if (strstr(r.c_str(), "Mute") || strstr(r.c_str(), "mute") || strstr(r.c_str(), "MUTE")) {
      // 如果收到 "Mute", "mute", 或 "MUTE"，就执行静音
      audio.setVolume(0);
      Serial.println(F("Muted"));
      u8g2.clearBuffer();
      u8g2.setCursor(0, 14);
      u8g2.println("Audio Mute");
      u8g2.setCursor(0, 30);
      u8g2.print("Press Vol +/- unmute");
      u8g2.sendBuffer();
    } else if (strstr(r.c_str(), "--")) {
      Serial.println(F("Channel Down"));
      currentSongIndex = (currentSongIndex - 1 + songURLs.size()) % songURLs.size();
      currentSongTitle = songTitles[currentSongIndex];
      savePreferences();  // Save station, title, and volume
      showCHVL();
      audio.connecttohost(songURLs[currentSongIndex].c_str());
    } else if (strstr(r.c_str(), "++")) {
      Serial.println(F("Channel Up"));
      currentSongIndex = (currentSongIndex + 1) % songURLs.size();
      currentSongTitle = songTitles[currentSongIndex];
      savePreferences();  // Save station, title, and volume
      showCHVL();
      audio.connecttohost(songURLs[currentSongIndex].c_str());
    } else if (strstr(r.c_str(), "-")) {
      Serial.println(F("Volume Down"));
      changeVolume(-1);
    } else if (strstr(r.c_str(), "+")) {
      Serial.println(F("Volume Up"));
      changeVolume(1);
    } else if (strstr(r.c_str(), "0")) {
      Serial.println(F("Gain=0"));
      digitalWrite(OLED_RST, LOW);
    } else if (strstr(r.c_str(), "1")) {
      Serial.println(F("Gain=1"));
      digitalWrite(OLED_RST, HIGH);
    } else if (strstr(r.c_str(), "update")) {
      audio.stopSong();
      Serial.println(F("update"));
      performFirmwareUpdate();
    } else if (strstr(r.c_str(), "*")) {
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);  // 填充温度和湿度对象
      Serial.print("Temperature: ");
      Serial.print(temp.temperature);
      Serial.println(" degrees C");
      Serial.print("Humidity: ");
      Serial.print(humidity.relative_humidity);
      Serial.println("% rH");

      while (temp.temperature > 10) {
        audio.loop();
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);  // 填充温度和湿度对象
        Serial.print("Temperature: ");
        Serial.print(temp.temperature);
        Serial.println(" degrees C");
        Serial.print("Humidity: ");
        Serial.print(humidity.relative_humidity);
        Serial.println("% rH");
        // 清空显示缓冲区
        u8g2.clearBuffer();

        // 显示湿度，格式为 H:60.00%
        u8g2.setCursor(0, 14);
        u8g2.print("H:");
        u8g2.setCursor(15, 14);
        u8g2.print(humidity.relative_humidity, 1);  // 直接使用humidity.relative_humidity，保留两位小数显示湿度
        u8g2.print("%");                            // 添加百分号

        // 显示温度，格式为 T:29.3°C
        u8g2.setCursor(60, 14);
        u8g2.print("T:");
        u8g2.setCursor(75, 14);
        u8g2.print(temp.temperature, 1);  // 直接使用temp.temperature，保留一位小数
        u8g2.print("'C");                 // 添加摄氏度符号
        //u8g2.print("\xB0C");     // 使用 \xB0 显示摄氏度符号 °C

        // 显示信息
        u8g2.setCursor(0, 30);
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP().toString());
        u8g2.print(WiFi.localIP().toString());  // 打印 IP 地址
        u8g2.sendBuffer();                      // 发送缓冲区到显示器
        if (Serial.available() || digitalRead(VOLUME_UP_PIN) == LOW || digitalRead(VOLUME_DOWN_PIN) == LOW) {
          return;
        }
      }
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("WiFi Disconnect");
    u8g2.setCursor(0, 30);
    u8g2.print("Reconnecting..");
    u8g2.sendBuffer();
    //audio.stopSong();
    WiFi.disconnect();    // 断开当前连接
    WiFi.mode(WIFI_OFF);  // 关闭 WiFi 模块
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    scanNetworks();  // 显示可用网络
    delay(100);

    // 加载保存的 WiFi 凭据并添加到 WiFiMulti 中
    loadWiFiCredentials();
    // 尝试连接 WiFi
    connectToWiFi();

    // 尝试连接 WiFi
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Attempting to connect...");

      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);  // 填充温度和湿度对象
      Serial.print("Temperature: ");
      Serial.print(temp.temperature);
      Serial.println(" degrees C");
      Serial.print("Humidity: ");
      Serial.print(humidity.relative_humidity);
      Serial.println("% rH");

      if (temp.temperature > 10) {
        // 清空显示缓冲区
        u8g2.clearBuffer();

        // 显示湿度，格式为 H:60.00%
        u8g2.setCursor(0, 14);
        u8g2.print("H:");
        u8g2.setCursor(15, 14);
        u8g2.print(humidity.relative_humidity, 1);  // 直接使用humidity.relative_humidity，保留两位小数显示湿度
        u8g2.print("%");                            // 添加百分号

        // 显示温度，格式为 T:29.3°C
        u8g2.setCursor(60, 14);
        u8g2.print("T:");
        u8g2.setCursor(75, 14);
        u8g2.print(temp.temperature, 1);  // 直接使用temp.temperature，保留一位小数
        u8g2.print("'C");                 // 添加摄氏度符号
        //u8g2.print("\xB0C");     // 使用 \xB0 显示摄氏度符号 °C

        // 显示信息
        u8g2.setCursor(0, 30);
        u8g2.print("Reconnecting..");  // 显示广告信息
        u8g2.sendBuffer();             // 发送缓冲区到显示器
      }

      delay(100);
      scanNetworks();  // 显示可用网络
      delay(100);

      // 加载保存的 WiFi 凭据并添加到 WiFiMulti 中
      loadWiFiCredentials();
      // 尝试连接 WiFi
      connectToWiFi();


      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected");
        showCHVL();
        // for (int i = 0; i < 5; i++) {
        //   audio.loop();  // 确保缓冲区填满
        //   Serial.println("Start up Loop x5");
        // }
        return;  // 成功连接后退出
      }
      delay(1000);  // 等待 1 秒再重试
    }

    // 如果所有尝试都失败，显示错误信息
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.println("Connection Fail");
    u8g2.setCursor(0, 30);
    u8g2.print("Pls try again.");
    u8g2.sendBuffer();
  }
}

// 用来记录重复空白 title 的次数
int retryNullTitle = 0;
void audio_showstreamtitle(const char* title) {
  if (!firstRun) {
    Serial.print("streamtitle ");
    Serial.println(title);

    // // 如果 title 是空的或只有短横线（带或不带空格），增加计数并跳过显示
    // if (title == NULL || strlen(title) == 0 || strcmp(title, "-") == 0 || strcmp(title, " - ") == 0 || strcmp(title, "  -  ") == 0) {
    //   Serial.print("retryNullTitle ");
    //   retryNullTitle++;
    //   return;  // 跳过，不更新显示
    // } else {
    //   // 如果 title 不是空的或有效，重置计数
    //   retryNullTitle = 0;
    // }

    // // 当计数超过 10 次空标题时，重启设备
    // if (retryNullTitle > 10) {
    //   u8g2.clearBuffer();
    //   u8g2.setCursor(0, 14);
    //   u8g2.println("Loading play list");
    //   u8g2.setCursor(0, 30);
    //   u8g2.println("error, restarting");
    //   u8g2.sendBuffer();
    //   delay(1000);
    //   ESP.restart();
    // }

    // 查找短横线的指针
    char* hyphenPtr = strchr(title, '-');

    // 如果 title 是仅有短横线，跳过显示
    if (hyphenPtr != NULL && strlen(title) == 3 && title[0] == ' ' && title[1] == '-' && title[2] == ' ') {
      Serial.println("无效的标题，跳过显示");
      return;  // 直接跳过，不更新显示
    }

    // 如果找到了短横线，检查是否有有效的歌曲名和艺术家
    if (hyphenPtr != NULL) {
      size_t artistLength = hyphenPtr - title;

      // 分割歌曲名和艺术家名
      char artist[artistLength + 1];
      strncpy(artist, title, artistLength);
      artist[artistLength] = '\0';  // 终止字符串

      char* songTitle = hyphenPtr + 2;  // 假设短横线后有空格并跳过它

      // 如果 artist 或 songTitle 是空的或只包含空格，跳过
      if (strlen(artist) == 0 || strlen(songTitle) == 0 || strcmp(artist, " ") == 0 || strcmp(songTitle, " ") == 0 || strcmp(artist, "  ") == 0 || strcmp(songTitle, "  ") == 0) {
        Serial.println("无效的歌曲名或艺术家，跳过显示");
        return;  // 直接跳过，不更新显示
      }

      // 显示在OLED屏幕上
      u8g2.clearBuffer();
      u8g2.setCursor(0, 14);
      u8g2.println(artist);  // 第一行显示歌手名
      u8g2.setCursor(0, 30);
      u8g2.println(songTitle);  // 第二行显示歌曲名
      u8g2.sendBuffer();
    }
    // 如果包含广告内容
    else if (strstr(title, "adContext=") != NULL) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 14);
      u8g2.println("Advertisement");
      u8g2.setCursor(0, 30);
      u8g2.print("Iklan/有广告");
      u8g2.sendBuffer();
    }
  }
}


// void audio_showstation(const char *info){
//     Serial.print("station     ");Serial.println(info);
// }

// void audio_bitrate(const char *info){
//     Serial.print("bitrate     ");Serial.println(info);
// }
// void audio_commercial(const char *info){  //duration in sec
//     Serial.print("commercial  ");Serial.println(info);
// }
// void audio_icyurl(const char *info){  //homepage
//     Serial.print("icyurl      ");Serial.println(info);
// }
// void audio_lasthost(const char *info){  //stream URL played
//     Serial.print("lasthost    ");Serial.println(info);
// }

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

const char* ssid = "Alex123";
const char* password = "Aa123456";

ESP8266WebServer server(80);

int ledPin = D1;
int ledBrightness = 128;

bool isLoggedIn = false;
String username = "";

const IPAddress serverIP(192, 168, 0, 106);  // MySQL 伺服器 IP 位址
const int serverPort = 3306;                 // MySQL 伺服器埠號
char dbUser[] = "root";        // MySQL 使用者名稱
char dbPassword[] = "Aa123456";    // MySQL 使用者密碼
const char* dbName = "user";        // 資料庫名稱

WiFiClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor *cursor;

void handleRoot() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(303);
    return;
  }
  String html = "<html><head><meta charset='UTF-8'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; }";
  html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; background-color: white; border-radius: 5px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }";
  html += ".header { text-align: center; margin-bottom: 20px; }";
  html += ".button-container { display: flex; flex-direction: column; align-items: center; justify-content: center; margin-top: 20px; }";
  html += ".button { background-color: #3498db; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s; margin-bottom: 10px; }";
  html += ".button:hover { background-color: #2980b9; }";
  html += ".logout-button { background-color: #e74c3c; }";
  html += ".logout-button:hover { background-color: #c0392b; }";
  html += "@media (max-width: 600px) { .button { width: 80%; } }";
  html += "</style>";
  html += "</head><body>";

  html += "<div class='container'>";
  html += "<div class='header'><h1>LED 控制</h1></div>";
  html += "<div class='button-container'>";
  html += "<form method='POST' action='/on'>";
  html += "<button class='button' type='submit' name='button' value='on'>打開 LED</button>";
  html += "</form>";
  html += "<form method='POST' action='/off'>";
  html += "<button class='button' type='submit' name='button' value='off'>關閉 LED</button>";
  html += "</form>";
  html += "<form method='POST' action='/brighter'>";
  html += "<button class='button' type='submit' name='button' value='brighter'>亮一點</button>";
  html += "</form>";
  html += "<form method='POST' action='/dimmer'>";
  html += "<button class='button' type='submit' name='button' value='dimmer'>暗一點</button>";
  html += "</form>";
  html += "<form method='POST' action='/logout'>";
  html += "<button class='button logout-button' type='submit'>登出</button>";
  html += "</form>";
  html += "</div>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleLogin() {
  if (server.hasArg("username") && server.hasArg("password")) {
    String enteredUsername = server.arg("username");
    String enteredPassword = server.arg("password");

    // 在此進行使用者名稱和密碼的驗證，可以與預定義的憑證進行比對
    // 假設憑證驗證通過
    isLoggedIn = true;
    username = enteredUsername;
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    // 顯示登入頁面
    String html = "<html><head><meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; }";
    html += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: white; border-radius: 5px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }";
    html += ".header { text-align: center; margin-bottom: 20px; }";
    html += ".form-container { text-align: center; }";
    html += ".input-field { margin-bottom: 15px; }";
    html += ".input-field input { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 5px; }";
    html += ".login-button { background-color: #3498db; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s; }";
    html += ".login-button:hover { background-color: #2980b9; }";
    html += ".register-link { display: block; margin-top: 10px; }";
    html += "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<div class='header'><h1>登入</h1></div>";
    html += "<div class='form-container'>";
    html += "<form method='POST' action='/login'>";
    html += "<div class='input-field'><input type='text' name='username' placeholder='使用者名稱'></div>";
    html += "<div class='input-field'><input type='password' name='password' placeholder='密碼'></div>";
    html += "<button class='login-button' type='submit'>登入</button>";
    html += "</form>";
    html += "<a class='register-link' href='/register'>註冊</a>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  }
}


void handleLogout() {
  isLoggedIn = false;
  username = "";
  server.sendHeader("Location", "/login");
  server.send(303);
}

void handleButton() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(303);
    return;
  }

  if (server.hasArg("button")) {
    String buttonValue = server.arg("button");

    // 在此處處理按鈕點擊事件
    if (buttonValue == "on") {
      analogWrite(ledPin, ledBrightness);
    } else if (buttonValue == "off") {
      analogWrite(ledPin, 0);
    } else if (buttonValue == "brighter") {
      ledBrightness = min(ledBrightness + 50, 255);
      analogWrite(ledPin, ledBrightness);
    } else if (buttonValue == "dimmer") {
      ledBrightness = max(ledBrightness - 50, 0);
      analogWrite(ledPin, ledBrightness);
    }

    // 將點擊時間記錄到資料庫
    if (conn.connect(serverIP, serverPort, dbUser, dbPassword)) {
      Serial.println("已連接到 MySQL 伺服器");

      // 在連接成功後選擇資料庫
      if (conn.connected()) {
        char query[64];
        snprintf(query, sizeof(query), "USE %s", dbName);
        cursor->execute(query);
      }

      // 插入點擊事件到資料庫
      char query[256];
      snprintf(query, sizeof(query), "INSERT INTO button_clicks (button_value, click_time) VALUES ('%s', NOW())",
               buttonValue.c_str());
      cursor->execute(query);

      conn.close();
    } else {
      Serial.println("無法連接到 MySQL 伺服器");
    }
  }

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleRegister() {
  if (server.hasArg("username") && server.hasArg("password")) {
    String enteredUsername = server.arg("username");
    String enteredPassword = server.arg("password");

    if (conn.connect(serverIP, serverPort, dbUser, dbPassword)) {
      Serial.println("已連接到 MySQL 伺服器");

      // 在連接成功後選擇資料庫
      if (conn.connected()) {
        char query[64];
        snprintf(query, sizeof(query), "USE %s", dbName);
        cursor->execute(query);
      }

      // 執行插入查詢以將使用者名稱和密碼插入資料庫
      char query[128];
      snprintf(query, sizeof(query), "INSERT INTO users (username, password) VALUES ('%s', '%s')",
               enteredUsername.c_str(), enteredPassword.c_str());
      cursor->execute(query);

      String html = "<html><head><meta charset='UTF-8'>";
      html += "<style>";
      html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; }";
      html += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: white; border-radius: 5px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }";
      html += ".header { text-align: center; margin-bottom: 20px; }";
      html += ".form-container { text-align: center; }";
      html += ".input-field { margin-bottom: 15px; }";
      html += ".input-field input { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 5px; }";
      html += ".register-button { background-color: #3498db; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s; }";
      html += ".register-button:hover { background-color: #2980b9; }";
      html += ".login-link { display: block; margin-top: 10px; }";
      html += "</style>";
      html += "</head><body>";

      html += "<div class='container'>";
      html += "<div class='header'><h1>註冊成功</h1></div>";
      html += "<div class='form-container'>";
      html += "<p>感謝您註冊！現在您可以使用您的帳號登入。</p>";
      html += "<a class='login-link' href='/login'>登入頁面</a>";
      html += "</div>";
      html += "</div>";
      html += "</body></html>";
      server.send(200, "text/html", html);
    } else {
      Serial.println("無法連接到 MySQL 伺服器");
      server.send(500, "text/plain", "Internal Server Error");
    }
  } else {
    // 顯示註冊頁面
    String html = "<html><head><meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; }";
    html += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: white; border-radius: 5px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }";
    html += ".header { text-align: center; margin-bottom: 20px; }";
    html += ".form-container { text-align: center; }";
    html += ".input-field { margin-bottom: 15px; }";
    html += ".input-field input { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 5px; }";
    html += ".register-button { background-color: #3498db; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s; }";
    html += ".register-button:hover { background-color: #2980b9; }";
    html += ".login-link { display: block; margin-top: 10px; }";
    html += "</style>";
    html += "</head><body>";

    html += "<div class='container'>";
    html += "<div class='header'><h1>註冊</h1></div>";
    html += "<div class='form-container'>";
    html += "<form method='POST' action='/register'>";
    html += "<div class='input-field'><input type='text' name='username' placeholder='使用者名稱'></div>";
    html += "<div class='input-field'><input type='password' name='password' placeholder='密碼'></div>";
    html += "<button class='register-button' type='submit'>註冊</button>";
    html += "</form>";
    html += "<a class='login-link' href='/login'>返回登入頁面</a>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, ledBrightness);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("連接到 WiFi...");
  }

  Serial.println("WiFi 連接成功");
  Serial.println("IP 地址: ");
  Serial.println(WiFi.localIP());

  if (conn.connect(serverIP, serverPort, dbUser, dbPassword)) {
    Serial.println("已連接到 MySQL 伺服器");

    // 在連接成功後選擇資料庫
    if (conn.connected()) {
      cursor = new MySQL_Cursor(&conn);
      char query[64];
      snprintf(query, sizeof(query), "USE %s", dbName);
      cursor->execute(query);
    }
  } else {
    Serial.println("無法連接到 MySQL 伺服器");
  }

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/logout", handleLogout);
  server.on("/on", handleButton);
  server.on("/off", handleButton);
  server.on("/brighter", handleButton);
  server.on("/dimmer", handleButton);
  server.on("/register", handleRegister);

  server.begin();
  Serial.println("伺服器已啟動");
}

void loop() {
  server.handleClient();
}

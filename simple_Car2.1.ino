#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#include <Blinker.h>  
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define IN_1  D2 //右轮
#define IN_2  D5 //右轮
#define IN_3  D6 //左轮
#define IN_4  D8 //左轮
#define EEPROM_START_ADDRESS 2448 // 存储起始位置  Blinker占用 0-2447

char ssid[32]; // 网络名称
char pswd[64]; // 网络密码
char auth[16]; // Blinker 密钥

const IPAddress LocalIp(192, 168, 4, 1);
const char *ssid_config = "ESP8266_Setup"; //网络配置名称
DNSServer dnsServer;
boolean is_setting_mode; // 设置模式
String ssid_list;
ESP8266WebServer WebServer(80);


int Speed=1023,Time0=100;

BlinkerButton Button1("btn-F");
BlinkerButton Button2("btn-B");
BlinkerButton Button3("btn-L");
BlinkerButton Button4("btn-R");
BlinkerButton Button5("btn-S");
BlinkerSlider Slider1("ran-1");
BlinkerSlider Slider2("ran-2");
// 按下按键即会执行该函数
void button1_callback(const String & state) {
    if(state=="press"){
    Forward();}
    else if(state=="pressup"){
    Stop();}
    else if(state=="tap"){
    Forward();
    delay(Time0);
    Stop();}
}
void button2_callback(const String & state) {
    if(state=="press"){
    Back();}
    else if(state=="pressup"){
    Stop();}
    else if(state=="tap"){
    Back();
    delay(Time0);
    Stop();}
}
void button3_callback(const String & state) {
    if(state=="press"){
    Left();}
    else if(state=="pressup"){
    Stop();}
    else if(state=="tap"){
    Left();
    delay(Time0);
    Stop();}
}
void button4_callback(const String & state) {
    if(state=="press"){
    Right();}
    else if(state=="pressup"){
    Stop();}
    else if(state=="tap"){
    Right();
    delay(Time0);
    Stop();}
}
void button5_callback(const String & state) {
    if(state=="press"){
    Stop();}
    else if(state=="pressup"){
    Stop();}
}
void slider1_callback(int32_t value)
{
    Speed=map(value,0,100,0,1023);
    Serial.println(Speed);
}
void slider2_callback(int32_t value)
{
    Time0=map(value,0,100,0,500);
    Serial.println(Time0);
}

// 如果未绑定的组件被触发，则会执行其中内容
void dataRead(const String & data){BLINKER_LOG("Blinker readString: ", data);}

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
     if (restoreConfig())
  {
    is_setting_mode = false;
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);
    Button1.attach(button1_callback);
    Button2.attach(button2_callback);
    Button3.attach(button3_callback);
    Button4.attach(button4_callback);
    Button5.attach(button5_callback);
    Slider1.attach(slider1_callback);
    Slider2.attach(slider2_callback);
    int i=0;
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(1000);
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));   
      i+=1;
      if(i==15){
       Serial.print("长时间未成功连接网络，将重置！");
       Reset();
        break;
      }
    }
    Serial.println("WiFi连接成功！");
    digitalWrite(LED_BUILTIN, HIGH);   
    pinMode(IN_1, OUTPUT);
    pinMode(IN_2, OUTPUT);
    pinMode(IN_3, OUTPUT);
    pinMode(IN_4, OUTPUT);
    pinMode(D3, INPUT);//Flash按钮，设为输入模式
  }
  else 
  {is_setting_mode = true;
   setupMode();
   digitalWrite(LED_BUILTIN, LOW);  
  }
}
          
void Forward()
{ 
  analogWrite(IN_1,Speed);
  analogWrite(IN_2,LOW);
  analogWrite(IN_3,Speed);
  analogWrite(IN_4,LOW);
}

void Back()
{ 
  analogWrite(IN_1,LOW);
  analogWrite(IN_2,Speed);
  analogWrite(IN_3,LOW);
  analogWrite(IN_4,Speed);
}
 
void Left()
{ 
  analogWrite(IN_1,Speed);
  analogWrite(IN_2,LOW);
  analogWrite(IN_3,LOW);
  analogWrite(IN_4,Speed);
}

void Right()
{
  analogWrite(IN_1,LOW);
  analogWrite(IN_2,Speed);
  analogWrite(IN_3,Speed);
  analogWrite(IN_4,LOW);
}
void Stop(){  
  digitalWrite(IN_1,0);
  digitalWrite(IN_2,0);
  digitalWrite(IN_3,0);
  digitalWrite(IN_4,0);
 }
int time1,currentTime0;
void loop() 
{
  if(is_setting_mode)
  {
    dnsServer.processNextRequest();
    WebServer.handleClient();
  }
  else{
  Blinker.run();
  }
  if(digitalRead(D3)==0 && time1==0)
  {currentTime0=millis();
  time1=1;
  }
  else if(digitalRead(D3)==0 && time1==1)
  { int currentTime1=millis();
    if(currentTime1-currentTime0>3000)
    {Reset();}
  }
}
bool restoreConfig()
{
  EEPROM.begin(4096);
  Serial.println("读取EEPROM中....");
  if (EEPROM.read(EEPROM_START_ADDRESS + 0) != 255)
  {
    EEPROM.get(EEPROM_START_ADDRESS + 0, ssid);
    EEPROM.get(EEPROM_START_ADDRESS + 32, pswd);
    EEPROM.get(EEPROM_START_ADDRESS + 96, auth);
    Serial.printf("SSID:%s\n", ssid);
    Serial.printf("pswd:%s\n", pswd);
    Serial.printf("auth:%s\n", auth);
    EEPROM.end();
    return true;
  }
  else
  {
    Serial.println("读取配置失败!");
    return false;
  }
}

void setupMode()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (int i = 0; i < n; ++i)
  {
    ssid_list += R"(<option value=")" + WiFi.SSID(i) + R"(">)" + WiFi.SSID(i) +
                 R"(</option>)";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(LocalIp, LocalIp, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid_config);
  dnsServer.start(53, "*", LocalIp);
  startWebServer();
  Serial.print("启动无线AP \"");
  Serial.print(ssid_config);
  Serial.println("\"");
}

void startWebServer()
{
  Serial.print("启动网络服务中.... ");
  Serial.println(WiFi.softAPIP());
  WebServer.onNotFound([]()
                       {
      String s =  R"(
        <h1>Wi-Fi 设置</h1>
        <p>请选择你的网络名称,输入密码和Blinker密钥</p>
        <form action="/setap" method="post">
        <label>网络名称: </label><select name="ssid">)" +
                 ssid_list +
                 R"(
        </select><br>
        网络密码: <input name="pass" length=64 type="pswd"><br>
        点灯密钥: <input name="auth" length=12 type="text">
        <input type="submit">
        </form>
        )";
      WebServer.send(200, "text/html", makePage("网络设置", s)); });
  WebServer.on("/setap", HTTP_POST, []()
               {
                /*ssid[]="";
                pswd[]="";
                auth[]="";*/
                
      strcpy(ssid,WebServer.arg("ssid").c_str());
      Serial.print("SSID: ");
      Serial.println(ssid);
      strcpy(pswd, WebServer.arg("pass").c_str());
      Serial.print("pswd: ");
      Serial.println(pswd);
      strcpy(auth,WebServer.arg("auth").c_str());
      Serial.printf("auth:%s\n", auth);

      Serial.println("开始写入配置信息.....");
      EEPROM.put(EEPROM_START_ADDRESS + 0,ssid);
      EEPROM.put(EEPROM_START_ADDRESS + 32,pswd);
      EEPROM.put(EEPROM_START_ADDRESS + 96,auth);
      EEPROM.end();
      String ssid_tmp = ssid;
      Serial.println("配置信息写入完成。");
      String s = R"(
        <h1>配置完成.</h1>
        <p>重启后连接到 ")" +
        ssid_tmp +
        R"(")";
      WebServer.send(200, "text/html", makePage("Wi-Fi 设置完成", s));
      ESP.restart(); });
  WebServer.begin();
}
String makePage(String title, String contents)
{
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}


static void Reset()
{
  Serial.println("正在重置设备......");
  EEPROM.begin(4096);
  EEPROM.write(EEPROM_START_ADDRESS + 0, 255);
  EEPROM.end();
  Serial.print("重置完成,设备正在重启......");
  ESP.restart();
}

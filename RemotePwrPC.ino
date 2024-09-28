#include <ESP8266WiFi.h>                        // 本程序使用ESP8266WiFi库
#include <WiFiManager.h>  

/*************************用户需要修改的-开始******************************************** */


/*巴法云命令*/
const char* LoginStr   = "cmd=1&uid=巴法云KEY&topic=pc006\r\n";
const char* PwrOnState = "cmd=2&uid=巴法云KEY&topic=pc006/up&msg=on\r\n";
const char* PwrOffState = "cmd=2&uid=巴法云KEY&topic=pc006/up&msg=off\r\n";



/**************************用户需要修改的-结束******************************************* */






const char* host = "bemfa.com";
const uint16_t port = 8344;                                    

#define POWERKEY_READ     14    //D5+GND读为低=按下  
#define POWER_MB_SET      13    //D7  拉低按下  抬高松开   接主板电源按钮3.3V  
#define POWER_LED_READ    5     //D1接主板电源LED      
#define PWOER_LED_SET     4     //D2接机箱电源LED


// 初始化 WiFiManager  
WiFiManager wifiManager;  
void setup() {
  Serial.begin(115200);                   //初始化串口通讯波特率为115200
  pinMode(POWERKEY_READ,INPUT_PULLUP);    //原电源按钮
  pinMode(POWER_MB_SET,OUTPUT);           //D7接主板
  pinMode(PWOER_LED_SET,OUTPUT);          //LED设为输出
  pinMode(POWER_LED_READ,INPUT);          //LED输入读取
  digitalWrite(POWER_MB_SET, HIGH);       // 松开按钮

  delay(10);  
  wifiManager.setDebugOutput(true);  
    
  // 尝试连接到之前保存的 Wi-Fi  
  if (!wifiManager.autoConnect("PwrCard")) {  
    Serial.println("Failed to connect");  
    // 停留并监听配置  
    ESP.restart();  
  }  
  
  Serial.println("Connected to WiFi");  
  Serial.println("IP Address: ");  
    
  // 设置当断开连接的时候自动重连
  WiFi.setAutoReconnect(true);
  Serial.println("WiFi connected!");            // NodeMCU将通过串口监视器输出"连接成功"信息。
}


/*主联网状态机*/
typedef enum
{
    no_connect,
    Loging,
    waiting,
    connected,
}NET_STATE;
NET_STATE mNetState;


/*计时与控制*/
uint8_t Timercount = 0;
uint8_t MainTimer = 0;
/* 新建一个WiFiClient类对象，作为TCP客户端对象 */


WiFiClient tcpclient;

/* 设置电源按钮状态 */
void SETPOWERSTATE(uint8_t iostate) {
  if (iostate == 0) {
    digitalWrite(POWER_MB_SET, LOW);   // 按下按钮
    delay(200);                        // 模拟按下延迟
    digitalWrite(POWER_MB_SET, HIGH);  // 松开按钮
  }else
  {
    digitalWrite(POWER_MB_SET, LOW);   // 按下按钮
    delay(3000);                        // 模拟按下延迟
    digitalWrite(POWER_MB_SET, HIGH);  // 松开按钮
  }
}

void loop() {
  /*10ms task*/
  /*电源键透传*/
  digitalWrite(POWER_MB_SET, digitalRead(POWERKEY_READ)); 
  /*LED透传*/
  digitalWrite(PWOER_LED_SET, digitalRead(POWER_LED_READ)); 
  if(millis() - MainTimer > 10)
  {
    MainTimer = millis();
    if(digitalRead(POWERKEY_READ)==LOW)
    {
      Serial.println("按下电源键");
    }
     /*主联网状态机*/
    switch (mNetState)
    {
      case no_connect:
          if (!tcpclient.connect(host, port)) 
          {
            Serial.println("连接中");
            /*未连接上*/
          }
          else
          {
              Serial.println("TCP已连接");
              Timercount = 0;
              mNetState = Loging;
          }
        break;
      case Loging:
            Timercount++;
            if(Timercount > 100)
            {
                tcpclient.print(LoginStr);
              Timercount = 0;
              mNetState = waiting;
            }
        break;
      case waiting:
            if(tcpclient.find("cmd=1&res=1")==true)
            {
              Serial.println("已联平台");
              mNetState = connected;
              Timercount = 0;
            }
            else
            {
              Timercount++;
            }
            if(Timercount > 100)
            {
              Timercount = 0;
              mNetState = no_connect;
            }
        break;
      case connected:
          //TCP状态检查
          if(!tcpclient.connected())
          {
              mNetState = no_connect;
          }
          //定时状态上报
          Timercount++;
          if(Timercount > 100)
          {
              Timercount = 0;
              if( digitalRead(POWER_LED_READ) == HIGH)
              {
                Serial.println("开机状态上报");
                tcpclient.print(PwrOnState);
              }
              else if( digitalRead(POWER_LED_READ) == LOW)
              {
                Serial.println("关机状态上报");
                tcpclient.print(PwrOffState);
              }
          }
          //下行处理
          if (tcpclient.available() > 0) {
            String msg = tcpclient.readStringUntil('\n');
            if (msg.indexOf("msg=on") != -1) 
            {
              SETPOWERSTATE(0);
              Serial.println("开启");
            } 
            else if (msg.indexOf("msg=off") != -1) 
            {
              SETPOWERSTATE(0);
              Serial.println("关闭");
            }
            else if(msg.indexOf("msg=reboot") != -1)
            {
               SETPOWERSTATE(1);
               Serial.println("强制关机");
            }
          }
        break;
    }
  }


 

}

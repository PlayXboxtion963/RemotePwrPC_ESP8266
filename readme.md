### 基于ESP8266(NodeMCU) + 巴法云的远程电脑开机卡

#### 功能
1. 电源键透传
2. 读取电脑电源键LED状态并上报真实开关状态
3. 强制重启
4. 集成一键配网库
5. 断网自动重连

#### 使用说明
1. 在巴法云申请账号并创建名为“pc006”的设备。
2. 获取Key并填入代码中。

#### 软件环境
- **ArduinoIDE**

#### 硬件连接
- **NodeMCU开发板**（淘宝7.9元，自备母对母杜邦线）
- **连接方式**：
  - D5 + GND 连接到机箱电源键
  - D7 接主板电源按钮输入的3.3V（确保接对，不要接到地上）
  - D1 接主板电源LED+
  - D2 接机箱电源LED+

#### 注意事项
- 确保连接正确，以避免短路或损坏设备。

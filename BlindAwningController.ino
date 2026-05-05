#define    WIND      A2     //PC2  风检测   
#define    RAIN_1    A0     //PC0  雨水检测1 
#define    RAIN_2    A1     //PC1  雨水检测2 
#define    SW1       A3     //PC3  开关1入
#define    SW2       A4     //PC4  开关2出
#define    DC_IN     7      //PD7  直流使用
#define    DC_STOP   6      //PD6  直流方向1
#define    DC_OUT    5      //PD5  直流方向2
#define    AC_N      8      //PB0  交流N级
#define    AC_EN     9      //PB1  交流使能
#define    AC_DIR    10     //PB2  交流方向
#define    SENSOR    23     //PE0  数字量检测

const unsigned char WindVolt = 185;  //风传感器触发-电压值(0.9V)
const unsigned int  RainVolt = 820;  //雨传感器触发-电压值(4V)
unsigned char WindFirstRet = 0;      //系统启动后，不知是in还是out状态，所以第一次检测到风触发，需要执行in 
volatile byte sysState = 0;          //记录系统当前运行状态
unsigned char MotorStatus = 0;       //记录电机出、入执行时间

unsigned long previousMillis = 0;    //防止millis溢出导致计算错误
const unsigned long Raininterval = 1200000; // 1200秒雨水检测

void setup() {
  // put your setup code here, to run once:
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);
  pinMode(WIND,INPUT);      //使用ADC模式
  pinMode(RAIN_1,INPUT);    //开关模式 
  pinMode(RAIN_2,INPUT);    //开关模式

  pinMode(DC_IN, OUTPUT);
  pinMode(DC_STOP, OUTPUT);
  pinMode(DC_OUT, OUTPUT);
  pinMode(AC_EN, OUTPUT);
  pinMode(AC_DIR, OUTPUT);
  Serial.begin(115200);

  digitalWrite(DC_IN, LOW);
  digitalWrite(DC_STOP, LOW);
  digitalWrite(DC_OUT, LOW);
  digitalWrite(AC_N, LOW);
  digitalWrite(AC_EN, LOW);
  digitalWrite(AC_DIR, LOW);
}
/*
 * 函数名：ReadWindADCFun
 * 描述  ：读取风传感器引脚ADC数据并判断
 * 输入  ：无
 * 输出  ：是否起风
 */
unsigned char ReadWindADCFun(){
  int sensorValue = analogRead(WIND);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  //float voltage = sensorValue * (5.0 / 1023.0);
  if (sensorValue > WindVolt) //风AD电压大于0.9V，即触发
  {
    return 1;
  }
  else
  {
    return 0;
  }  
}

/*
 * 函数名：ReadRainADCFun
 * 描述  ：读取雨传感器引脚ADC数据并判断
 * 输入  ：雨传感器序号
 * 输出  ：是否下雨
 */
unsigned char ReadRainADCFun(unsigned char rain){
  int sensorValue = analogRead(rain);
  if (sensorValue > RainVolt) //AD电压大于4V，即触发
  {
    return 1;
  }
  else
  {
    return 0;
  }  
}
/*
 * 函数名：MotorTurnsOutFun
 * 描述  ：电机运行，出
 * 输入  ：无
 * 输出  ：无
 */
void MotorTurnsOutFun(){
  digitalWrite(AC_N, HIGH);
  digitalWrite(AC_DIR, LOW);
  delay(50);   
  digitalWrite(AC_EN, HIGH);
  delay(50); 
  digitalWrite(DC_OUT, HIGH);
  digitalWrite(DC_IN, LOW);
  digitalWrite(DC_STOP, LOW);
}
/*
 * 函数名：MotorTurnsInFun
 * 描述  ：电机运行，入
 * 输入  ：无
 * 输出  ：无
 */
void MotorTurnsInFun(){
  digitalWrite(AC_N, HIGH);
  digitalWrite(AC_DIR, HIGH);
  delay(50);   
  digitalWrite(AC_EN, HIGH);
  delay(50);
  digitalWrite(DC_OUT, LOW);
  digitalWrite(DC_IN, HIGH);
  digitalWrite(DC_STOP, LOW);
}
/*
 * 函数名：MotorStopFun
 * 描述  ：电机停止
 * 输入  ：无
 * 输出  ：无
 */
void MotorStopFun(){
  digitalWrite(AC_EN, LOW);
  delay(50); 
  digitalWrite(AC_N, LOW);
  digitalWrite(AC_DIR, LOW);
  delay(50);
  digitalWrite(DC_STOP, LOW);
  digitalWrite(DC_IN, LOW);
  digitalWrite(DC_OUT, LOW);
}

/*
 * 函数名：MotorPauseFun
 * 描述  ：电机暂停
 * 输入  ：无
 * 输出  ：无
 */
void MotorPauseFun(){
  digitalWrite(AC_EN, LOW);
  delay(50); 
  digitalWrite(AC_N, LOW);
  digitalWrite(AC_DIR, LOW);
  delay(50);
  digitalWrite(DC_IN, LOW);
  digitalWrite(DC_OUT, LOW);
  delay(50);
  digitalWrite(DC_STOP, HIGH);
  delay(1000);
  digitalWrite(DC_STOP, LOW);
}


void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(SW1) == LOW) //IN
  {
    delay(100); 
    if (digitalRead(SW1) == LOW) //IN
    {
      MotorStopFun();
      MotorTurnsInFun();
      unsigned long startTime = millis();
      while(digitalRead(SW1) == LOW);
      unsigned long actualTime = millis() - startTime;
      MotorPauseFun();
      actualTime = actualTime/1000;
      MotorStatus =  (actualTime > MotorStatus)?0:MotorStatus-actualTime; 
    }
  }

  if (digitalRead(SW2) == LOW && ReadWindADCFun() == 0) //OUT
  {
    delay(100); 
    if (digitalRead(SW2) == LOW && ReadWindADCFun() == 0) //OUT
    {
      MotorStopFun();
      MotorTurnsOutFun();
      unsigned long startTime = millis();
      while(digitalRead(SW2) == LOW); 
      unsigned long actualTime = millis() - startTime;  
      MotorPauseFun();
      actualTime = actualTime/1000;
      MotorStatus =  MotorStatus+actualTime; 
    }
  }

  if (sysState == 0)
  {
    if (ReadRainADCFun(RAIN_1) == 1 || ReadRainADCFun(RAIN_2) == 1)
    {    
      delay(500); 
      if (ReadRainADCFun(RAIN_1) == 1 || ReadRainADCFun(RAIN_2) == 1)
      {
        MotorTurnsOutFun();
        delay(40000); // 直接延时40秒（40000毫秒）
        MotorStopFun();
        MotorStatus = 40; 
        sysState = 1;
      }
    }   
  }
  else if (sysState == 1)
  {
    if (ReadRainADCFun(RAIN_1) == 0 && ReadRainADCFun(RAIN_2) == 0)
    {
      sysState = 2;
      previousMillis = millis();
    }
  }
  else
  {
    if (ReadRainADCFun(RAIN_1) == 0 && ReadRainADCFun(RAIN_2) == 0)
    {
      unsigned long currentMillis = millis();  
      if (currentMillis - previousMillis >= Raininterval) 
      {
        MotorTurnsInFun();
        delay(40000); // 直接延时40秒（40000毫秒）
        MotorStopFun();
        MotorStatus = 0; 
        sysState = 0;     
      }
    }
    else 
    {
      sysState = 1;
    }   
  }


  if (ReadWindADCFun() == 1 && (MotorStatus > 1 || WindFirstRet == 0))
  {
    delay(500); 
    if (ReadWindADCFun() == 1 && (MotorStatus > 1 || WindFirstRet == 0))
    {
      MotorTurnsInFun();
      delay(40000); // 直接延时40秒（40000毫秒）
      MotorStopFun();
      MotorStatus = 0; 
      WindFirstRet = 1;
    }
  }


}

#include <Arduino.h>
#include <Servo.h>       // 引入伺服馬達庫
#include <ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32.h>

void commandCallback(const std_msgs::Int32& msg);


Servo servo1;  // 第一個伺服馬達物件（右邊馬達）
Servo servo2;  // 第二個伺服馬達物件（左邊馬達）
Servo servo3;  // 第三個伺服馬達物件（旋轉馬達）

const int servoPin1 = 5;  // 第一個伺服馬達的腳位-右邊馬達
const int servoPin2 = 6;  // 第二個伺服馬達的腳位-左邊馬達
const int servoPin3 = 9;  // 第三個伺服馬達的腳位-旋轉馬達
const int vacuumMotorPin = 13;  // 真空馬達的控制腳位

const int rotationMinAngle = 0;  // 旋轉伺服馬達的最小角度
const int rotationMaxAngle = 180;  // 旋轉伺服馬達的最大角度
const int servo1InitAngle = 0;  // 第一軸伺服馬達初始角度
const int servo2InitAngle = 0;  // 第二軸伺服馬達初始角度

int currentAngle1 = servo1InitAngle;
int currentAngle2 = servo2InitAngle;


// ROS NodeHandle and Publisher/Subscriber
ros::NodeHandle nh;
std_msgs::Int32 motorStateMsg;
ros::Publisher motorStatePub("info_back", &motorStateMsg);
ros::Subscriber<std_msgs::Int32> commandSub("information", commandCallback);

void setup() {

  // 初始化伺服馬達
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);

  // 設置真空馬達控制腳位為輸出
  pinMode(vacuumMotorPin, OUTPUT);
  digitalWrite(vacuumMotorPin, LOW);  // 初始狀態為關閉

  // 將伺服馬達移動到初始位置
  servo1.write(servo1InitAngle);  // 第一軸伺服馬達初始位置
  servo2.write(servo2InitAngle);   // 第二軸伺服馬達初始位置
  servo3.write(rotationMinAngle);   // 旋轉馬達初始位置
  // Initialize ROS node
  nh.initNode();
  nh.advertise(motorStatePub);
  nh.subscribe(commandSub);
}

void loop() {
  nh.spinOnce();
}

void setInitialPositions() {
  servo1.write(servo1InitAngle);  // 第一軸伺服馬達回到初始位置
  servo2.write(servo2InitAngle);  // 第二軸伺服馬達回到初始位置
}

void executeSequence() {
  int currentAngle1 = servo1InitAngle;
  int currentAngle2 = servo2InitAngle;
  
  while(true) {// 第一軸伺服馬達逆時針旋轉90度
    // Serial.print(currentAngle1);
    // Serial.print(" ");
    // Serial.print(currentAngle2);
    // Serial.print(" ");
    // Serial.println(" going up down ");
    // 第一軸順時針旋轉直到觸發A0訊號
    currentAngle1 = (currentAngle1 + 1) % 180;
    servo1.write(currentAngle1);
    // 第二軸逆時針旋轉直到觸發A1訊號
    currentAngle2 = (currentAngle2 - 1 + 180) % 180;
    servo2.write(currentAngle2);
    delay(20);  // 控制旋轉速度
    if(currentAngle1 == 170 && currentAngle2 == 10) { 
      // A0訊號已觸發，將第一軸伺服馬達設置為初始位置
      break;
    }

    // 第二軸伺服馬達順時針旋轉90度
    if (analogRead(A1) <= 500) {
    } else {
      // A1訊號已觸發，將第二軸伺服馬達設置為初始位置
      break;
    }
  }
  // Serial.println(" sucking ");
  // 打開真空馬達
  digitalWrite(vacuumMotorPin, LOW);  // 送出5V電壓
  delay(1000);  // 等待1000毫秒


  servo2.write(45);
  delay(500);
  servo1.write(-45);
  delay(4000);  // 等待4000毫秒

  // 第一軸伺服馬達回到初始位置
  //servo1.write(servo1InitAngle);
  //delay(1500);  // 等待1500毫秒

  // 關閉真空馬達
    Serial.println(" unsuck ");
  digitalWrite(vacuumMotorPin, HIGH);  // 停止送出5V電壓
}

void commandCallback(const std_msgs::Int32& msg) {
  // 3 digit: 1st, 2nd digit: theta angle, 3rd digit: execute sequence, example: 45deg, execute = 451;
  int command = msg.data;
  int angle = command / 10;
  int execute = abs(command % 10);

  if (angle >= -60 && angle <= 60) {
    servo3.write(angle);
    delay(1000);  // 延遲以穩定伺服馬達
  }
  if(execute == 1) {
    executeSequence();
  }
  motorStateMsg.data = (execute);
  motorStatePub.publish(&motorStateMsg);
}

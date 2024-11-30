# rosserial_stm32

## 📖 介绍

rosserial_stm32是一个用于STM32的rosserial库，方便移植与使用。

具体教程见：[在STM32里使用rosserial](https://mdlzcool.github.io/post/f3ad7b3e.html)，这里也贴出其用法教程。

如果对你有帮助的话，请点击star⭐！

## 🎉 使用

> 我使用的是Ubuntu 20.04 ROS Noetic搭配STM32F407开发板，不同STM32芯片之间使用起来差异不大

### 准备工作

- Ubuntu安装软件包

```bash
sudo apt-get install ros-noetic-rosserial ros-noetic-rosserial-arduino
```

- （已经配置好的库）[MDLZCOOL/rosserial_stm32](https://github.com/MDLZCOOL/rosserial_stm32)

### 开始配置

1. 新建STM32工程，配置RCC等步骤省略。
2. 打开串口，配置中断和DMA，波特率推荐115200。具体配置见下图

![image-20241129190705711](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129190705711.png)

![image-20241129190741215](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129190741215.png)

3. 生成代码，将`rosserial_stm32_lib`复制到`Core/Inc`，并添加进工程里。

4. 在`Core/Src`添加一个`mainpp.cpp`，添加以下代码

```c
#include <ros.h>
#include <std_msgs/String.h>
#include "mainpp.h"
#include "main.h"

void messageCallback(const std_msgs::String &msg) {
    // Callback
}

ros::NodeHandle nh;
ros::Subscriber<std_msgs::String> sub("/stm32_ros_bridge", &messageCallback);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    nh.getHardware()->flush();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    nh.getHardware()->reset_rbuf();
}

void setup(void) {
    nh.initNode();
    nh.subscribe(sub);
}

void loop(void) {
    nh.spinOnce();
}
```

6. 在`Core/Inc`添加`mainpp.h`，添加以下代码

```c
#ifndef MAINPP_H_
#define MAINPP_H_

#ifdef __cplusplus
extern "C" {
#endif

void setup(void);

void loop(void);


#ifdef __cplusplus
}
#endif

#endif /* MAINPP_H_ */
```

5. 修改`main.c`

```c
int main(void){
...
  /* USER CODE BEGIN 2 */
  setup();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    loop();
  }
  /* USER CODE END 3 */
...

```

6. 修改`STM32Hardware.h`，具体为：

   - 定义自己的设备头（如果预处理部分没有你的设备，请自己加上）

![image-20241129193813695](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129193813695.png)

   - 将所有的huart句柄改为你自己的，比如在`STM32Hardware.h`里默认handler名字叫`huart1`

![image-20241129194059681](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129194059681.png)

7. 至此STM32就配好了
8. 用串口工具连接STM32和电脑USB_HOST，使用以下命令查看USB文件位置（Linux里万物皆为文件）

```bash
ls /dev/ttyUSB*
或者
dmesg | grep tty
```

比如这里我的USB位置就是`/dev/ttyUSB0`

![image-20241129195247309](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129195247309.png)

9. 执行下面命令使ROS连接上STM32，别忘了先roscore

```bash
rosrun rosserial_python serial_node.py _baud:=115200 _port:=/dev/ttyUSB0
```

不出所料的话，现在已经成功连接上设备了

![image-20241129195651213](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129195651213.png)

### STM32执行ROS发来的指令

1. 在`mainpp.cpp`实现下面测试代码：

```c
void messageCallback(const std_msgs::String &msg) {
    SEGGER_RTT_WriteString(0, "\r\nmessageCallback occurred!\r\nYour message is ");
    SEGGER_RTT_WriteString(0, msg.data);
    SEGGER_RTT_WriteString(0, "\r\n");
    if (strcmp("servo_1", msg.data) == 0) {
        SEGGER_RTT_WriteString(0, "Action 'servo_1' begin!\r\n");
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500);
        SEGGER_RTT_WriteString(0, "Action 'servo_1' complete!\r\n");
    }
}
```

2. 写入固件，连接电脑测试

![image-20241129200940086](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129200940086.png)

![image-20241129201020932](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129201020932.png)

可以看到，回调函数正常触发，至此实现了ROS对STM32 publish 消息，并成功接收。

### STM32向ROS发送话题

1. 在`mainpp.cpp`实现下面测试代码：

```c
#include <ros.h>
#include <std_msgs/String.h>
#include "mainpp.h"
#include "main.h"

std_msgs::String str_msg;
ros::NodeHandle nh;
ros::Publisher pub("stm32_ros_bridge_pub", &str_msg);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    nh.getHardware()->flush();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    nh.getHardware()->reset_rbuf();
}

void setup(void) {
    nh.initNode();
    str_msg.data = "Hello, ROS!";
    nh.advertise(pub);
}

void loop(void) {
    pub.publish(&str_msg);
    nh.spinOnce();
    HAL_Delay(1000);
}

```

![image-20241129203923631](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129203923631.png)

> 注意：这里有一个问题，我们用的波特率是115200，发送数据的频率是有限制的，如果超过这个上限，ROS拿到的数据就会丢包，没法通过完整性校验，所以这里在loop()里写了一个delay，但是这种做法不是通常推荐的，**更加一般的做法是把publish写在某个定时器里，这样就不会占用CPU时间去等待。**

贴一张图，发送频率不能太高

![image-20241129204312643](https://cdn.jsdelivr.net/gh/MDLZCOOL/blog-img/img/image-20241129204312643.png)

至此实现了STM32与ROS的**双向通信**，后面可以拿来做一些好玩的东西了。

## 🤩 致谢：

[yoneken/rosserial_stm32](https://github.com/yoneken/rosserial_stm32)
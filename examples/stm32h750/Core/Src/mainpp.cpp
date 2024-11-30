#include <ros.h>
#include <std_msgs/String.h>
#include "mainpp.h"
#include "main.h"
#include "SEGGER_RTT.h"

void messageCallback(const std_msgs::String &msg) {
    SEGGER_RTT_WriteString(0, "\r\nmessageCallback occurred!\r\nYour message is ");
    SEGGER_RTT_WriteString(0, msg.data);
    SEGGER_RTT_WriteString(0, "\r\n");
    if (strcmp("servo_1", msg.data) == 0) {
        SEGGER_RTT_WriteString(0, "Action 'servo_1' begin!\r\n");
        SEGGER_RTT_WriteString(0, "Action 'servo_1' complete!\r\n");
    }
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
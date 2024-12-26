#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int8.h>

rcl_subscription_t subscriber;
std_msgs__msg__Int8 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LED_PIN 2
#define PWMA 32
#define DIR1A 33
#define DIR2A 25

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void subscription_callback(const void * msgin)
{  
  const std_msgs__msg__Int8 * msg = (const std_msgs__msg__Int8 *)msgin;
  digitalWrite(LED_PIN, (msg->data == 0) ? LOW : HIGH);  
  if(msg->data == 0){
    digitalWrite(DIR1A, LOW);
    digitalWrite(DIR2A, LOW);
    analogWrite(PWMA, 0);
  }else if(msg->data == 1){
    digitalWrite(DIR1A, HIGH);
    digitalWrite(DIR2A, LOW);
    analogWrite(PWMA, 200);
  }else if(msg->data == 2){
    digitalWrite(DIR1A, LOW);
    digitalWrite(DIR2A, HIGH);
    analogWrite(PWMA, 200);
  }

}

void setup() {
  set_microros_transports();
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(DIR1A, OUTPUT); 
  pinMode(DIR2A, OUTPUT); 

  //digitalWrite(LED_PIN, HIGH);  
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node_esp32", "", &support));

  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8),
    "LED"));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

#include <micro_ros_arduino.h>

#include <Servo.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <teleop_interfaces/msg/pitch_yaw_motor.h>

rcl_subscription_t subscriber;
teleop_interfaces__msg__PitchYawMotor msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LED_PIN 13
#define RELAY_PIN 3

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

const int STEP_SIZE = 5, NUM_SERVOS = 2;
Servo servo[NUM_SERVOS];
volatile int servo_pos[NUM_SERVOS], servo_goal[NUM_SERVOS];

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void subscription_callback(const void * msgin)
{  
  const teleop_interfaces__msg__PitchYawMotor * msg = (const teleop_interfaces__msg__PitchYawMotor *)msgin;
  servo_goal[0] = msg->pitch;
  servo_goal[1] = msg->yaw;
  digitalWrite(RELAY_PIN, msg->motors);
}

void servo_callback(void) {
  for (int i = 0; i < NUM_SERVOS; ++i) {
    if (abs(servo_goal[i] - servo_pos[i]) >= STEP_SIZE) {
      servo_pos[i] += (servo_goal[i] > servo_pos[i]) ? STEP_SIZE : -STEP_SIZE;
      servo[i].write(servo_pos[i]);
    }
  }
  
}

void setup() {
  set_microros_transports();
  
  servo[0].attach(6, 800, 2800);
  servo[1].attach(5, 800, 2800);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  for (int i = 0; i < NUM_SERVOS; ++i) {
    servo_pos[i] = 0;
    servo_goal[i] = 0;
  }
  
  HardwareTimer *t = new HardwareTimer(TIM1);
  t->setOverflow(10, HERTZ_FORMAT);
  t->attachInterrupt(servo_callback);
  t->resume();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "uros_node", "", &support));

  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(teleop_interfaces, msg, PitchYawMotor),
    "pitch_yaw_motor"));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

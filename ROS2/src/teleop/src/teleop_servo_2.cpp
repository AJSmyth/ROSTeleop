#include <termios.h>
#include <unistd.h> 
#include <chrono>
#include <functional>
#include <memory>
#include <stdlib.h>

#include "rclcpp/rclcpp.hpp"
#include "teleop_interfaces/msg/pitch_yaw_motor.hpp"

const int ROT_LIMIT = 180, SPEED_LOW = 5, SPEED_HIGH = 20, SPEED_STEP = 5;

int getch()
{
  static struct termios oldt, newt;
  tcgetattr( STDIN_FILENO, &oldt);           // save old settings
  newt = oldt;
  newt.c_lflag &= ~(ICANON);                 // disable buffering
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);  // apply new settings

  int c = getchar();  // read character (non-blocking)

  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);  // restore old settings
  return c;
}

using namespace std::chrono_literals;


int main(int argc, char * argv[]) {
	rclcpp::init(argc, argv);
	auto node = std::make_shared<rclcpp::Node>("servo_teleop");
	auto publisher = node->create_publisher<teleop_interfaces::msg::PitchYawMotor>("pitch_yaw_motor", 10);

	teleop_interfaces::msg::PitchYawMotor msg;
	int c, pitch = 0, yaw = 0, speed = 10;
	auto last_rfsh = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	bool motors = false, needs_update = false;
	while (rclcpp::ok()) {
		auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		c = getch();
		if (now - last_rfsh >= 200) {
			last_rfsh = now;
			switch (c) {
				case 'a':
				case 'A':
					pitch += (pitch + speed <= ROT_LIMIT) ? speed : 0;
					needs_update = true;
					break;
				case 'd':
				case 'D':
					pitch -= (pitch - speed >= 0) ? speed : 0;
					needs_update = true;
					break;
				case 'w':
				case 'W':
					yaw += (yaw + speed <= ROT_LIMIT) ? speed : 0;
					needs_update = true;
					break;
				case 's':
				case 'S':
					yaw -= (yaw - speed >= 0) ? speed : 0;
					needs_update = true;
					break;
				case 'e':
				case 'E':
					speed += (speed + SPEED_STEP <= SPEED_HIGH) ? SPEED_STEP : 0;
					needs_update = false;
					break;
				case 'r':
				case 'R':
					motors = !motors;
					needs_update = true;
					break;
				case 'q':
				case 'Q':
					speed -= (speed - SPEED_STEP >= SPEED_LOW) ? SPEED_STEP : 0;
					needs_update = false;
					break;
			}

			if (needs_update) {
				msg.pitch = pitch;
				msg.yaw = yaw;
				msg.motors = motors;
				
				publisher->publish(msg);		
				needs_update = false;
			}
		}
		system("clear");
		RCLCPP_INFO(node->get_logger(), "Pitch: %d | Yaw: %d | Adjustment Speed: %d | Motors: %s", pitch, yaw, speed, (motors) ? "on" : "off");
		rclcpp::spin_some(node);
	}

	rclcpp::shutdown();
	return 0;
}
		

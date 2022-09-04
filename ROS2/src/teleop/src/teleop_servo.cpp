#include <termios.h>
#include <unistd.h> 
#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

const int ROT_LIMIT = 180, SPEED_LIMIT = 10;

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
	auto publisher = node->create_publisher<std_msgs::msg::Int32>("servo_pos", 10);

	std_msgs::msg::Int32 msg;

	int c, pos = 0, speed = 1;	
	while (rclcpp::ok()) {
		c = getch();
		
		switch (c) {
			case 'a':
			case 'A':
				pos += (pos + speed <= ROT_LIMIT) ? speed : 0;
				RCLCPP_INFO(node->get_logger(), "Pos changed to %d", pos);
			     	break;
			case 'd':
			case 'D':
				RCLCPP_INFO(node->get_logger(), "Pos changed to %d", pos);
				pos -= (pos - speed > 0) ? speed : 0;
				break;
			case 'q':
			case 'Q':
				RCLCPP_INFO(node->get_logger(), "Speed changed to %d", speed);
				speed += (speed < SPEED_LIMIT) ? 1 : 0;
				break;
			case 'e':
			case 'E':
				RCLCPP_INFO(node->get_logger(), "Speed changed to %d", speed);
				speed -= (speed > 1) ? 1 : 0;
				break;
		}
		msg.data = pos;

		publisher->publish(msg);		
		rclcpp::spin_some(node);
	}

	rclcpp::shutdown();
	return 0;
}
		

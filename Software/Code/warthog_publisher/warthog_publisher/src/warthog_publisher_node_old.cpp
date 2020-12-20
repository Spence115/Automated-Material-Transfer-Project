#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <warthog_publisher/ARMarker.h>

int main(int argc, char **argv)
{
	ros::init(argc, argv, "warthog_publisher");

	ros::NodeHandle nh;

	ros::Publisher warthog_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel",1000);

	ros::Rate loop_rate(10);

	while(ros::ok())
	{
		geometry_msgs::Twist msg;
		msg.linear.x = 1;
		//msg.linear.y = 0
		//msg.linear.z = 0
		//msg.angular.x = 0
		//msg.angular.y = 0
		msg.angular.z = 0;
	
		warthog_pub.publish(msg);

		ros::spinOnce();

		loop_rate.sleep();
	}
}

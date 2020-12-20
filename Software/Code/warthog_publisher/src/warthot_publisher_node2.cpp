#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <warthog_publisher/ARMarker.h>
#include <tf/transform_listener.h>
#include "std_msgs/String.h"

using namespace std;

int main(int argc, char **argv)
{
	cout << "hello!" << endl;
	ros::init(argc, argv, "warthog_publisher");

	ros::NodeHandle nh;

	//ROS_INFO("I heard you");
	cout << "hello!" << endl;

	ros::Publisher warthog_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel",1000);

	ros::Rate loop_rate(10);

	tf::TransformListener listener;

	while(ros::ok())
	{
		//ROS_INFO("I heard you");
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


		tf::StampedTransform transform;

		listener.lookupTransform("/chassis_link","/world",ros::Time(0),transform);

		//ROS_INFO("I heard: [%s]", msg->data.c_str(transform.getOrigin().x());
		
		
	}
}

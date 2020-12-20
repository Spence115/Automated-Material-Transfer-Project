#include <ros/ros.h>
#include <warthog_publisher/ARMarker.h>
#include <tf/transform_listener.h>
#include "std_msgs/String.h"
#include "gazebo_msgs/ModelStates.h"
#include <math.h>

using namespace std;

float position_x = 0;
float position_y = 0;
float direction = 0;

int gotopoint(float desired_x,float desired_y,float desired_angle, ros::Publisher warthog_pub, int index)
{
	geometry_msgs::Twist msg;//create a msg variable with type geometry_msgs/Twist

	int y_isneg = 1;//desired_x/abs(desired_x);
	int x_isneg = 1;//desired_y/abs(desired_y);

	if(desired_angle < 0){desired_angle += 2*3.14;}
	if(direction < 0){direction += 2*3.14;}

	cout << "the index is: " << index << endl;
	cout << "the des_x is: " << desired_x << endl;
	cout << "the des_y is: " << desired_y << endl;
	cout << "the act_x is: " << position_x << endl;
	cout << "the act_y is: " << position_y << endl;
	cout << "the des_a is: " << desired_angle << endl;
	cout << "the act_a is: " << direction << endl;
	

	if((desired_angle > direction + 3.14/90) &&
	 (((position_x < (desired_x - 0.2))   || 
	   (position_x > (desired_x + 0.2)))  ||
	  ((position_y < (desired_y - 0.2))   || 
	   (position_y > (desired_y + 0.2)))))
	{
		cout << "plus angle" << endl;
		msg.linear.x = 0;
		msg.angular.z = 0.5;
	}
	else if((desired_angle < direction - 3.14/90) &&
	      (((position_x < (desired_x - 0.2))  || 
	        (position_x > (desired_x + 0.2))) ||
	       ((position_y < (desired_y - 0.2))  || 
	        (position_y > (desired_y + 0.2)))))
	{
		cout << "minus angle" << endl;
		msg.linear.x = 0;
		msg.angular.z = -0.5;
	}
	else if((((position_x < (desired_x - 0.2))  || 
	          (position_x > (desired_x + 0.2))) ||
	         ((position_y < (desired_y - 0.2))  || 
	          (position_y > (desired_y + 0.2)))))
	{
		cout << "forward" << endl;
		msg.linear.x = 0.5;
		msg.angular.z = 0;
	}
	else
	{
		msg.linear.x = 0;
		msg.angular.z = 0;
		index++;
	}	

	warthog_pub.publish(msg); //publish msg

	ros::spinOnce(); //spin ros once each time the loop is run
	
	return index;
}

void modelstatesCallback(gazebo_msgs::ModelStates msg)
{
	int index = 0;
	
	while(msg.name[index] != "warthog") //find the modelstate with name warthog
	{
		index++;
	}

	position_x = msg.pose[index].position.x;
	position_y = msg.pose[index].position.y;
	float i = msg.pose[index].orientation.x;
	float j = msg.pose[index].orientation.y;
	float k = msg.pose[index].orientation.z;
	float w = msg.pose[index].orientation.w;
	direction = atan2(2*(w*k+i*j),1-2*(j*j+k*k));

	//cout << "Position [x,y]: " << endl;
	//cout << "[" << position_x << "," << position_y << "]" << endl;	
	//cout << "Yaw: " << direction << endl;
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "warthog_publisher");

	ros::NodeHandle nh; //create a node handle object named nh

	//advertise to topic cmd_vel
	ros::Publisher warthog_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel",1000); 

	ros::Rate loop_rate(50);//set loop rate at 50hz

	//subscribe to gazebo/modelstates
	ros::Subscriber warthog_sub = nh.subscribe<gazebo_msgs::ModelStates>("gazebo/model_states",1000,modelstatesCallback);


	float desired_xs[4] = {2,2,-2,-2};
	float desired_ys[4] = {2,-2,-2,2};
	float desired_x = desired_xs[1];
	float desired_y = desired_ys[1];
	float desired_angle = atan2(desired_x,desired_y);
	int i = 1;

	while(ros::ok())
	{
		cout << "desx: " << desired_x << endl;
		cout << "desy: " << desired_y << endl;
		float desired_angle = atan2(desired_y - position_y,desired_x - position_x);
		
		i = gotopoint(desired_x, desired_y, desired_angle, warthog_pub, i);
		
		if(i > 3){i = 0;}
		desired_x = desired_xs[i];
		desired_y = desired_ys[i];

		loop_rate.sleep(); //sleep until at least 1s has passed since last sleep call
	
	}
}




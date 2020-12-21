#include <ros/ros.h>
#include <warthog_publisher/ARMarker.h>
#include <tf/transform_listener.h>
#include "std_msgs/String.h"
#include "gazebo_msgs/ModelStates.h"
#include <math.h>
#include <unistd.h>
 
using namespace std;

float position_x = 0; //actual x position relative to world frame
float position_y = 0; //actual y position relative to world frame
float direction = 0;  //actual yaw relative to world frame 
bool latch = 0;	      //hold latch value

bool gotopoint(float desired_x,float desired_y,float desired_angle, ros::Publisher warthog_pub)
{
	geometry_msgs::Twist msg;//create a msg variable with type geometry_msgs/Twist

	//if the angle is near pi it can switch from negative to positive due to the way atan2() works
	//to avoid these kinds of scenarios a latch was created to make negative values positive
	//when the desired angles are near +-pi but normal when they are not
	//(theta < -3pi/4) turns latch on
	//(-pi/2 < theta <  3pi/4) turns latch off
	if(latch == 0 && desired_angle < -3*3.14/4){ latch = 1; }
	else if(latch == 1 && (desired_angle > -3.14/2 && desired_angle < 3*3.14/4)){ latch = 0; }
	if(latch == 1 && desired_angle < 0){ desired_angle += 3.14*2; }
	if(latch == 1 && direction < 0){ direction += 2*3.14; }
	
	//compare angle is desired angle rotated so it is +/- pi for more efficient rotation decisions
	float compare_angle = desired_angle;
	if(desired_angle > direction + 3.14){ compare_angle = desired_angle - 2*3.14;}
	if(desired_angle < direction - 3.14){ compare_angle = desired_angle + 2*3.14;}
	
	//see if needs to turn counterclockwise and not in position
	if((compare_angle > direction + 3.14/90) &&
	 (((position_x < (desired_x - 0.2))   || 
	   (position_x > (desired_x + 0.2)))  ||
	  ((position_y < (desired_y - 0.2))   || 
	   (position_y > (desired_y + 0.2)))))
	{
		msg.linear.x = 0;
		msg.angular.z = 0.5;
	}
	//see if needs to turn clockwise and not in position
	else if((compare_angle < direction - 3.14/90) &&
	      (((position_x < (desired_x - 0.2))  || 
	        (position_x > (desired_x + 0.2))) ||
	       ((position_y < (desired_y - 0.2))  || 
	        (position_y > (desired_y + 0.2)))))
	{
		msg.linear.x = 0;
		msg.angular.z = -0.5;
	}
	//if its not near the point move forward
	else if((((position_x < (desired_x - 0.2))  || 
	          (position_x > (desired_x + 0.2))) ||
	         ((position_y < (desired_y - 0.2))  || 
	          (position_y > (desired_y + 0.2)))))
	{
		msg.linear.x = 2;
		msg.angular.z = 0;
	}
	//stop and return a true meaning the point was reached
	else
	{
		msg.linear.x = 0;
		msg.angular.z = 0;
		warthog_pub.publish(msg); //publish msg
		ros::spinOnce(); //spin ros once each time the loop is run
		return 1;
	}	

	warthog_pub.publish(msg); //publish msg

	ros::spinOnce(); //spin ros once each time the loop is run
	
	return 0; //return zero means point has not been reached
}

void modelstatesCallback(gazebo_msgs::ModelStates msg)
{
	int index = 0;

	//find the modelstate with name warthog
	while(msg.name[index] != "warthog") 
	{
		index++;
	}
	//get position values from pose
	position_x = msg.pose[index].position.x;
	position_y = msg.pose[index].position.y;
	//get quaternion values from pose and calculate the direction(yaw)
	float i = msg.pose[index].orientation.x;
	float j = msg.pose[index].orientation.y;
	float k = msg.pose[index].orientation.z;
	float w = msg.pose[index].orientation.w;
	direction = atan2(2*(w*k+i*j),1-2*(j*j+k*k)); 

	//print the current state of the warthog
	ROS_INFO("\nPosition [x,y]: \n[%.2f,%.2f]\nYaw: %.2f",position_x, position_y,direction);

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

	//place desired waypoint coordinates here
	float desired_xs[] = {0.5,-20};
	float desired_ys[] = {-8,-8};
	//initalize as the first of desired coordinates
	float desired_x = desired_xs[0];
	float desired_y = desired_ys[0];

	int i = 0; //initial array index

	while(ros::ok())
	{
		//calculate angle from warthog to waypoint
		float desired_angle = atan2(desired_y - position_y,desired_x - position_x);
		
		//go to the waypoint, returns true if point has been reached
		//iterates to next waypoint on true
		if(gotopoint(desired_x, desired_y, desired_angle, warthog_pub)){i++;}
		
		//notifies the user when the final waypoint has been reached and stops node after 3 seconds
		if(i >= sizeof(desired_xs)/sizeof(desired_xs[0])){
		
			ROS_INFO("Destination has been reached.");
			sleep(3);
			return 0;
		}
		//set waypoint
		desired_x = desired_xs[i];
		desired_y = desired_ys[i];		

		loop_rate.sleep(); //sleep until at least 1/T loop_rate has passed since last sleep call
	}
}




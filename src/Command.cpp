#include <ros/ros.h>
#include <iostream>
#include <fstream>
#include <limits>

#include <tf/transform_datatypes.h>
#include <tf/LinearMath/Transform.h>
#include <tf/transform_listener.h>
#include <math.h>

#include <vector>
#include <string>
#include "hector_navigation/StructKeyMap.h"
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <algorithm>
#include "hector_navigation/Mesh.h"
#include "hector_navigation/Row.h"
#include "hector_navigation/Size.h"

#include <visualization_msgs/Marker.h>
#include "visualization_msgs/MarkerArray.h"

#include <rosbag/bag.h>
#include <std_msgs/Float64.h>
#include <geometry_msgs/Point.h>
#define PI 3.14159265

class ControlCommand
{

public:
    ControlCommand()
    {
    
     sub_mesh = nh.subscribe<hector_navigation::Mesh>("Sphere_Mesh", 1, &ControlCommand::callback_mesh,this);
     pub_vel = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
     pub_LineStrip = nh.advertise<visualization_msgs::Marker>("LineStrip", 1);
     pub_Goalsphere = nh.advertise<visualization_msgs::Marker>("Goalsphere", 1);
     pub_Arrow= nh.advertise<visualization_msgs::Marker>("Arrow",1);
    }

    void callback_mesh(const hector_navigation::Mesh::ConstPtr& inputMesh);

    typedef std::vector<Vector>   direction;
    geometry_msgs::Twist moveCommand;
    visualization_msgs::Marker  line_strip;

private:
    ros::NodeHandle nh;
    ros::Subscriber sub_mesh;
    ros::Publisher pub_vel;
    ros::Publisher pub_LineStrip;
    ros::Publisher pub_Goalsphere;
    ros::Publisher pub_Arrow;
};


void ControlCommand::callback_mesh(const hector_navigation::Mesh::ConstPtr& inputMesh)
{
   
    position goal;
    goal.x=2.5;
    goal.y=5.0;
    goal.z=3.0;

//PUBLISH Goal in RVIZ	
    visualization_msgs::Marker  dummy_goal;
    dummy_goal.header.frame_id = "world";
    dummy_goal.header.stamp = ros::Time::now();
    dummy_goal.ns = "Dummy_Goal";
    dummy_goal.id = 4;
    dummy_goal.type = visualization_msgs::Marker::SPHERE;
    dummy_goal.action = visualization_msgs::Marker::ADD;
    dummy_goal.pose.orientation.w = 1.0;
    dummy_goal.scale.x = 0.1;
    dummy_goal.scale.y = 0.1;
    dummy_goal.scale.z = 0.1;
    dummy_goal.color.r = 1.0f;
    dummy_goal.color.a = 1.0;
    dummy_goal.pose.position.x = goal.x;
    dummy_goal.pose.position.y = goal.y;
    dummy_goal.pose.position.z = goal.z;
    dummy_goal.lifetime = ros::Duration();
    pub_Goalsphere.publish(dummy_goal);

    tf::TransformListener listener;
    tf::StampedTransform transform2;
    tf::Transform transform3;
    tf::Quaternion Q;
    tf::Vector3 V;

    listener.waitForTransform("world", "base_link", ros::Time(0),ros::Duration(3.0));
    listener.lookupTransform("world", "base_link", ros::Time(0), transform2);

    double roll, pitch, yaw, x ,y ,z ;
    transform2.getBasis ().getRPY(roll,pitch,yaw); 
    x= transform2.getOrigin().x();//UAV location in gloabl 
    y= transform2.getOrigin().y();
    z= transform2.getOrigin().z(); 

    /*std::ofstream outfile;
    outfile.open("/home/lin/Desktop/rostime.csv",std::ios_base::app);
    outfile <<ros::Time::now()<<"\n";
	
    std::ofstream outfile1;
    outfile1.open("/home/lin/Desktop/position_x.csv",std::ios_base::app);
    outfile1 <<x<<"\n";

    std::ofstream outfile2;
    outfile2.open("/home/lin/Desktop/position_y.csv",std::ios_base::app);
    outfile2 <<y<<"\n";

    std::ofstream outfile3;
    outfile3.open("/home/lin/Desktop/position_z.csv",std::ios_base::app);
    outfile3 <<z<<"\n";

    std::ofstream outfile4;
    outfile4.open("/home/lin/Desktop/attitude_roll.csv",std::ios_base::app);
    outfile4 <<roll<<"\n";

    std::ofstream outfile5;
    outfile5.open("/home/lin/Desktop/attitude_pitch.csv",std::ios_base::app);
    outfile5 <<pitch<<"\n";

    std::ofstream outfile6;
    outfile6.open("/home/lin/Desktop/attitude_yaw.csv",std::ios_base::app);
    outfile6 <<yaw<<"\n";

    cout << "x: " << x<< " \n";
    cout << "y: " << y << " \n";  
    cout << "z: " << z<<" \n";
    cout << "roll: " << roll<< " \n";
    cout << "pitch: " << pitch << " \n";  
    cout << "yaw: " << yaw<<" \n";*/

//transform between Quadrotor and sphere space
    Q=transform2.getRotation();    
    V.setX(0);
    V.setY(0);
    V.setZ(0);
    transform3.setOrigin(V);
    transform3.setRotation(Q.inverse ());   

		//const char *path5="/home/lin/Desktop/target.dat";
		//ofstream file5(path5);
    double target_theta=acos((goal.z-z)/sqrt(pow((goal.x-x),2.0)+pow((goal.y-y),2.0)+pow((goal.z-z),2.0)));
    double target_phi=atan2((goal.y-y),(goal.x-x));
		//file5<<target_theta<<"\n";
		//file5<<target_phi<<"\n";

//PUBLISH path in RVIZ	

	line_strip.header.frame_id = "world";
	line_strip.header.stamp = ros::Time::now();
	line_strip.ns = "Lines";
	line_strip.id = 0;
	line_strip.type = visualization_msgs::Marker::LINE_STRIP;
	line_strip.action = visualization_msgs::Marker::ADD;
	line_strip.pose.orientation.w = 1.0;
	line_strip.scale.x = 0.02;
	line_strip.color.g = 1.0f;
	line_strip.color.b = 1.0f;
	line_strip.color.a = 1.0;

        geometry_msgs::Point p;
	p.x=x;
	p.y=y;
	p.z=z;
	line_strip.points.push_back(p);
	pub_LineStrip.publish(line_strip);

//STEERING CONTROL
	    hector_navigation::Mesh MESH;
	    MESH.column= inputMesh ->column;
		const char *path="/home/lin/Desktop/mesh.dat";
		ofstream file(path);

	    int mesh[13][36];
	    for (int j=0; j<13;j++)
	    {
	      for(int k=0; k<36;k++)
	       {
		 mesh[j][k]=MESH.column[j].row[k];          
		file<<mesh[j][k];
	       }
		file <<"\n";
	    }



		//const char *path1="/home/lin/Desktop/sub-directions.dat";
		//ofstream file1(path1);

	bool is_ok=0;//pick up safe sub_directions
	direction dire_vector;
	Vector dire_index;
	dire_vector.clear();
		//int bb=0;

	for (int j=1; j<12;j++)
	{
	  for(int k=0; k<36;k++)
	  {

	    if (mesh[j][k]== 1)
	    {
		    if (k==0)
		    {
			   if (((mesh[j-1][35]==1 && mesh[j-1][k]==1)&& (mesh[j-1][k+1]==1&&mesh[j][35]==1))
	 &&((mesh[j][k+1]==1 && mesh[j+1][35]==1)&& (mesh[j+1][k]==1&&mesh[j+1][k+1]==1))  )
			      {
		                           is_ok=1;    
		             }

		    }
		    else if (k==35)
		    {
			   if (((mesh[j-1][k-1]==1 && mesh[j-1][k]==1)&& (mesh[j-1][0]==1&&mesh[j][k-1]==1)) 
	&&((mesh[j][0]==1 && mesh[j+1][k-1]==1)&& (mesh[j+1][k]==1&&mesh[j+1][0]==1))  )

		           {
		                          is_ok=1;
		           }      

		    }
		    else
		    {
			   if (((mesh[j-1][k-1]==1 && mesh[j-1][k]==1)&& (mesh[j-1][k+1]==1&&mesh[j][k-1]==1)) 
	&&((mesh[j][k+1]==1 && mesh[j+1][k-1]==1)&& (mesh[j+1][k]==1&&mesh[j+1][k+1]==1))  )

		           {
		                          is_ok=1;
		           } 

		     }

		     if (is_ok==1)
			{
			  
			  dire_index.THETA=(10*j+30)*PI/180;
			  dire_index.PHI=(10*k+5)*PI/180;
			  dire_vector.push_back(dire_index);

		//file1<<bb<<" ";
		//file1<<dire_index.THETA<<" ";
		//file1<<dire_index.PHI<<"\n";
		//bb=bb+1;

			}

		     is_ok=0;
	      }   
	  }
		     
	}


		//const char *path2="/home/lin/Desktop/angles.dat";
		//ofstream file2(path2);

//choose the closest direction 
	direction::iterator IT;
	vector<double> delta;
	double Dot_product;
	double magnitude;
	double Angle;
		//int aa=0;

	for(IT=dire_vector.begin(); IT != dire_vector.end(); ++IT)
	{   
	     dire_index =*IT;
	     Dot_product=(goal.x-x)*sin(dire_index.THETA)*cos(dire_index.PHI)+(goal.y-y)*sin(dire_index.THETA)*sin(dire_index.PHI)+(goal.z-z)*cos(dire_index.THETA);
	     magnitude=sqrt(pow(goal.x-x,2.0)+pow(goal.y-y,2.0)+pow(goal.z-z,2.0));
	     Angle=acos(Dot_product/magnitude); 
	     delta.push_back(Angle);
		//file2<<aa<<" ";
		//file2<<Angle<<"\n";
		//aa=aa+1;
	}


//transform direction to UAV frame and steer UAV which way to rotate
	int min_index=min_element(delta.begin(),delta.end())- delta.begin();

	double dir_theta=dire_vector[min_index].THETA;
	double dir_phi=dire_vector[min_index].PHI;
        tf::Vector3 S_dir;
	S_dir.setX(sin(dir_theta)*cos(dir_phi));
	S_dir.setY(sin(dir_theta)*sin(dir_phi));	
	S_dir.setZ(cos(dir_theta));
        tf::Vector3 U_dir=transform3*S_dir;

	double dir_THETA=acos(U_dir.getZ()/sqrt(pow(U_dir.getX(),2.0)+pow(U_dir.getY(),2.0)+pow(U_dir.getZ(),2.0)));
	double dir_PHI=atan2(U_dir.getY(),U_dir.getX());

	//cout << "theta: " << dir_THETA*180/PI<< " \n"; 
	//cout << "phi: " <<  dir_PHI*180/PI<< " \n";

//Pubslih direction in RVIZ
    	visualization_msgs::Marker arrow;	
	arrow.header.frame_id = "base_link";
	arrow.header.stamp = ros::Time::now();
	arrow.ns = "Directions";
	arrow.id = 5;
	arrow.type = visualization_msgs::Marker::ARROW;
	arrow.action = visualization_msgs::Marker::ADD;
	arrow.pose.orientation.w = 1.0;
	arrow.scale.x = 0.02;
	arrow.scale.y=0.05;
	arrow.scale.z=0.05; 
	arrow.color.r=1.0;    
	arrow.color.g = 0;
	arrow.color.b = 1.0;
	arrow.color.a = 1.0;

        geometry_msgs::Point p1;
	p1.x=0;
	p1.y=0;
	p1.z=0;
	arrow.points.push_back(p1);
        geometry_msgs::Point p2;
	p2.x=U_dir.getX();
	p2.y=U_dir.getY();
	p2.z=U_dir.getZ();
	arrow.points.push_back(p2);
	pub_Arrow.publish(arrow);

	int rotate;
	double error=0.087266;
	if (dir_PHI >= error )
	{
		rotate=1;
	}
	else if (dir_PHI <= -error )
	{
		rotate=-1;
	}
	else
	{
		rotate=0;
	}

	if (abs(x-goal.x)<=0.3 && abs(y-goal.y)<=0.3  && abs(z-goal.z)<=0.3 )
	{
		moveCommand.linear.x = 0.0;
		moveCommand.linear.z = 0.0;
		moveCommand.angular.z = -0.2;
                pub_vel.publish(moveCommand);
		ROS_INFO_STREAM("Arrive goal !!");
	}
	else
	{
		if(z <0.5)//rise to 0.5m
		{
		   moveCommand.linear.x = 0;
		   moveCommand.linear.z = 0.2;
		   moveCommand.angular.z = 0;
		   pub_vel.publish(moveCommand);
		   ROS_INFO_STREAM("Rise now !");
		}
		else
		{
			moveCommand.linear.x = 0.2;
			moveCommand.linear.z = 0.2*tan(PI/2-dir_THETA);
			moveCommand.angular.z = 0.2*rotate;
			pub_vel.publish(moveCommand);
		}
	}

}

int main(int argc, char **argv)
{
     ros::init(argc, argv, "command_node");

     ControlCommand  controlproject;

     ros::spin();
   

     return 0;
}




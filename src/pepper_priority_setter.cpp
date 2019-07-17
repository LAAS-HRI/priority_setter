//
// Created by gbuisan on 14/06/19.
//
#include <ros/ros.h>
#include <pepper_base_manager_msgs/PrioritizedTwist.h>
#include <resource_management_msgs/MessagePriority.h>

#define NODE_NAME "pepper_priority_setter"
#define WATCHDOG_PERIOD 0.1

class PrioritySetter{
public:
  PrioritySetter(ros::NodeHandlePtr& nh): nh_(nh){
    cmdVelSub_ = nh_->subscribe<geometry_msgs::Twist>("/raw_cmd_vel", 1, &PrioritySetter::onNewSpeed, this);
    navBaseManagerPub_ = nh_->advertise<pepper_base_manager_msgs::PrioritizedTwist>("/pepper_base_manager/navigating/pepper_base_manager_msgs1_PrioritizedTwist", 1);
    watchdog_ = nh_->createTimer(WATCHDOG_PERIOD, &PrioritySetter::onWatchDog, this);

  }

  void onNewSpeed(const geometry_msgs::Twist msg){
    last_updated_ = ros::Time::now();
    pepper_base_manager_msgs::PrioritizedTwist prioritizedTwist;
    prioritizedTwist.priority.value = resource_management_msgs::MessagePriority::URGENT;
    prioritizedTwist.data = msg;
    navBaseManagerPub_.publish(prioritizedTwist);
  }

  void onWatchDog(const ros::TimerEvent& e){
    if (last_updated_ != ros::Time(0) && ros::Time::now() - last_updated_ >= ros::Duration(WATCHDOG_PERIOD)){
      last_updated_ = ros::Time(0);
      pepper_base_manager_msgs::PrioritizedTwist prioritizedTwist;
      prioritizedTwist.priority.value = resource_management_msgs::MessagePriority::VOID;
      navBaseManagerPub_.publish(prioritizedTwist);
    }
  }

protected:
  ros::NodeHandlePtr nh_;

  ros::Subscriber cmdVelSub_;
  ros::Publisher navBaseManagerPub_;
  ros::Timer watchdog_;

  ros::Time last_updated_;
};


int main(int argc, char ** argv){
  ros::init(argc, argv, NODE_NAME);
  ros::NodeHandlePtr pnh(new ros::NodeHandle("~"));

  PrioritySetter prioritySetter(pnh);
  ros::spin();



}

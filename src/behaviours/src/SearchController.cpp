#include <algorithm>
#include "SearchController.h"
#include <angles/angles.h>
#include <ros/ros.h>
#include "ccny_srvs/GetPickup.h"
#include <math.h>

SearchController::SearchController() {
  rng = new random_numbers::RandomNumberGenerator();
  currentLocation.x = 0;
  currentLocation.y = 0;
  currentLocation.theta = 0;

  centerLocation.x = 0;
  centerLocation.y = 0;
  centerLocation.theta = 0;
  result.PIDMode = FAST_PID;

  result.fingerAngle = M_PI/2;
  result.wristAngle = M_PI/4;

  square_angles = { M_PI/4, 3 * M_PI/4, 5 * M_PI/4, 7 * M_PI/4 };
}
  
void SearchController::Reset() {
  result.reset = false;
}

Result SearchController::PickupWork(){
    ros::NodeHandle nm;
    ros::ServiceClient pickup_req = nm.serviceClient<ccny_srvs::GetPickup>("pickupgetter");
    if (!result.wpts.waypoints.empty()) {
      if (hypot(result.wpts.waypoints[0].x-currentLocation.x, result.wpts.waypoints[0].y-currentLocation.y) < 0.3) {
        attemptCount = 0;
      }
    }

    if (attemptCount > 0 && attemptCount < 5) {
      attemptCount++;
      if (succesfullPickup) {
        succesfullPickup = false;
        attemptCount = 1;
      }
      return result;
    }
    else if (attemptCount >= 5 || attemptCount == 0)
    {
      attemptCount = 1;


      result.type = waypoint;
      Point  searchLocation;
      float radius;
      float angle;
      float curr_angle;

    if (first_waypoint)
    {
      first_waypoint = false;
      ccny_srvs::GetPickup msg;
      msg.request.pickup = true;
      pickup_req.call(msg);
      if(!msg.response.empty){
          searchLocation.x=msg.response.point.x;
          searchLocation.y=msg.response.point.y;
          //ROS_WARN("pickup x:%f y:%f",searchLocation.x,searchLocation.y);
      }else{
          if(currentLocation.x!=0){
          curr_angle = atan(currentLocation.y/currentLocation.x);
          }
          radius = +rng->uniformReal(1,3);
          angle = (curr_angle+rng->uniformReal(-M_PI/4,M_PI/4));
       searchLocation.x = radius*cos(angle);
       searchLocation.y = radius*sin(angle);

    }
    }else{
        ccny_srvs::GetPickup msg;
        msg.request.pickup = true;
        pickup_req.call(msg);
        if(!msg.response.empty){
            searchLocation.x=msg.response.point.x;
            searchLocation.y=msg.response.point.y;
            //ROS_WARN("pickup x:%f y:%f",searchLocation.x,searchLocation.y);
        }else{
            if(currentLocation.x!=0){
            curr_angle = atan(currentLocation.y/currentLocation.x);
            }
            radius = +rng->uniformReal(1,3);
            angle = (curr_angle+rng->uniformReal(-M_PI/4,M_PI/4));
         searchLocation.x = radius*cos(angle);
         searchLocation.y = radius*sin(angle);

    }
    }
    result.wpts.waypoints.clear();
    result.wpts.waypoints.insert(result.wpts.waypoints.begin(), searchLocation);
    return result;
}

}
Result SearchController::SearchWork(){

    if (!result.wpts.waypoints.empty()) {
      if (hypot(result.wpts.waypoints[0].x-currentLocation.x, result.wpts.waypoints[0].y-currentLocation.y) < 0.3) {
        attemptCount = 0;
      }
    }

    if (attemptCount > 0 && attemptCount < 5) {
      attemptCount++;
      if (succesfullPickup) {
        succesfullPickup = false;
        attemptCount = 1;
      }
      return result;
    }
    else if (attemptCount >= 5 || attemptCount == 0)
    {
      attemptCount = 1;


      result.type = waypoint;
      Point  searchLocation;
      float radius;
      float angle;
      float curr_angle;
    if (first_waypoint)
    {
      first_waypoint = false;
      if(currentLocation.x!=0){
      curr_angle = atan(currentLocation.y/currentLocation.x);
      }
      radius = +rng->uniformReal(3,5);
      angle = (curr_angle+rng->uniformReal(-M_PI/4,M_PI/4));
      searchLocation.x = radius*cos(angle);
      searchLocation.y = radius*sin(angle);
    }else{
        if(currentLocation.x!=0){
        curr_angle = atan(currentLocation.y/currentLocation.x);
        }
        radius = +rng->uniformReal(3,5);
        angle = (curr_angle+rng->uniformReal(-M_PI/4,M_PI/4));
        searchLocation.x = radius*cos(angle);
        searchLocation.y = radius*sin(angle);
    }
    result.wpts.waypoints.clear();
    result.wpts.waypoints.insert(result.wpts.waypoints.begin(), searchLocation);
    return result;
    }

}

Result SearchController::DoWork() {
return Work(*this);
}

void SearchController::SetCenterLocation(Point centerLocation) {
  
  float diffX = this->centerLocation.x - centerLocation.x;
  float diffY = this->centerLocation.y - centerLocation.y;
  this->centerLocation = centerLocation;
  
  if (!result.wpts.waypoints.empty())
  {
  result.wpts.waypoints.back().x -= diffX;
  result.wpts.waypoints.back().y -= diffY;
  }
  
}

void SearchController::SetCurrentLocation(Point currentLocation) {
  this->currentLocation = currentLocation;
}

void SearchController::ProcessData() {
}

bool SearchController::ShouldInterrupt(){
  ProcessData();

  return false;
}

bool SearchController::HasWork() {
  return true;
}

void SearchController::SetSuccesfullPickup() {
  succesfullPickup = true;
}

void SearchController::setType(){
    Work=&SearchController::SearchWork;
}

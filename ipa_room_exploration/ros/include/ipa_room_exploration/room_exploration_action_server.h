#include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <ros/time.h>
#include <cv_bridge/cv_bridge.h>

#include <dynamic_reconfigure/server.h>
#include <ipa_room_exploration/RoomExplorationConfig.h>

#include <tf/transform_listener.h>

#include <Eigen/Dense>

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <ipa_building_msgs/RoomExplorationAction.h>

#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/Polygon.h>
#include <geometry_msgs/Point32.h>

#include <ipa_room_exploration/grid_point_explorator.h>


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

class RoomExplorationServer
{
protected:

	int path_planning_algorithm_; // variable to specify which algorithm is going to be used to plan a path
									// 1: grid point explorator

	gridPointExplorator grid_point_planner; // object that uses the grid point method to plan a path trough a room

	// parameters for the different planners
	int grid_line_length_; // size of the grid-lines that the grid-point-explorator lays over the map

	// callback function for dynamic reconfigure
	void dynamic_reconfigure_callback(ipa_room_exploration::RoomExplorationConfig &config, uint32_t level);

	// this is the execution function used by action server
	void exploreRoom(const ipa_building_msgs::RoomExplorationGoalConstPtr &goal);

	// function to publish a navigation goal, it returns true if the goal could be reached
	bool publishNavigationGoal(const geometry_msgs::Pose2D& nav_goal, const std::string map_frame,
			const std::string camera_frame, std::vector<geometry_msgs::Pose2D>& robot_poses);

	// function to check if a point is inside a given polygon, using the crossing line algorithm
	int pointInsidePolygonCheck(cv::Point P, std::vector<cv::Point> V);

	// converter-> Pixel to meter for X coordinate
	double convertPixelToMeterForXCoordinate(const int pixel_valued_object_x, const float map_resolution, const cv::Point2d map_origin)
	{
		double meter_value_obj_x = (pixel_valued_object_x * map_resolution) + map_origin.x;
		return meter_value_obj_x;
	}
	// converter-> Pixel to meter for Y coordinate
	double convertPixelToMeterForYCoordinate(int pixel_valued_object_y, const float map_resolution, const cv::Point2d map_origin)
	{
		double meter_value_obj_y = (pixel_valued_object_y * map_resolution) + map_origin.y;
		return meter_value_obj_y;
	}

	// function to transform the given map in a way s.t. the OpenCV and room coordinate system are the same
	//	--> the map_saver from ros saves maps as images with the origin laying in the lower left corner of it, but openCV assumes
	//		that the origin is in the upper left corner, also they are rotated around the image-x-axis about each other
	void transformImageToRoomCordinates(cv::Mat& map)
	{
		cv::Point2f src_center(map.cols/2.0F, map.rows/2.0F);
		cv::Mat rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
		cv::Mat dst;
		cv::warpAffine(map, dst, rot_mat, map.size());
		cv::flip(dst, map, 1);
	}

	// function to draw the points that have been covered by the field of view, when the robot moved trough the room
	//		--> use given Poses and original field of view points to do so
	void drawSeenPoints(cv::Mat& reachable_areas_map, const std::vector<geometry_msgs::Pose2D>& robot_poses,
			const std::vector<geometry_msgs::Point32>& field_of_view_points, const Eigen::Matrix<float, 2, 1> raycasting_corner_1,
			const Eigen::Matrix<float, 2, 1> raycasting_corner_2, const float map_resolution, const cv::Point2d map_origin);

	// !!Important!!
	//  define the Nodehandle before the action server, or else the server won't start
	//
	ros::NodeHandle node_handle_;
	actionlib::SimpleActionServer<ipa_building_msgs::RoomExplorationAction> room_exploration_server_;
	dynamic_reconfigure::Server<ipa_room_exploration::RoomExplorationConfig> room_exploration_dynamic_reconfigure_server_;

public:
	// initialize the action-server
	RoomExplorationServer(ros::NodeHandle nh, std::string name_of_the_action);

	// default destructor for the class
	~RoomExplorationServer(void)
	{
	}
};

How to use this package:

1. Change the algorithm parameters in room_segmentation_action_server_params.yaml in ros/launch to the wanted algorithm.
2. Start the action server using the file /ros/launch/room_segmentation_action_server.launch, which executes the /ros/src/room_segmentation_server.cpp file
3. Start an action client, which sends a goal to the segmentation action server, corresponding to the action/MapSegmentation.action message.

The algorithms are implemented in common/src, using the headers in common/include/ipa_room_segmentation. Each algorithm has its own class, so if you want to use it alone you have to include the header for the algorithm you want to use and make a new object. Then you have to execute the segmentationAlgorithm (semanticLabeling for adaboost_classifier.cpp), which segments the map.

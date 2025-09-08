# hokuyo_slam_ros2
This packages is only included C++ lib and utility requirements.
Please build this packages, and use from shellscript on hokuyo_navigation2 for 3d mapping

## Reference
- p2o: https://github.com/furo-org/p2o
- p2o_fastlio_util: https://github.com/kiyoshiiriemon/p2o_fastlio_util

## Requirements
Ubuntu 22.04 ROS2 Humble
This package is dependent on Eigen3, C++14, and pcl 1.14

## Build

```bash
cd
mkdir github
git clone -b release https://github.com/Hokuyo-RD/hokuyo_slam_ros2.git
cd hokuyo_slam_ros2
chmod +x ./build.bash
./build.bash
```
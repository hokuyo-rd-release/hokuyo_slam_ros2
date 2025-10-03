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
# python
sudo apt-get install python3-pip
pip3 install numpy==1.24.4 (version >=1.17.3 <1.25.0)
pip3 install tqdm
pip3 install open3d

# for vtk
sudo apt-get install libeigen3-dev
sudo apt-get -y install qtbase5-dev
sudo apt-get -y install clang
sudo apt-get -y install qtcreator
sudo apt-get -y install libqt5x11extras5-dev

# vtk
# wget https://www.vtk.org/files/release/8.2/VTK-8.2.0.tar.gz
# tar -xvf VTK-8.2.0.tar.gz
# cd VTK-8.2.0
# cmake -DCMAKE_BUILD_TYPE=Release -DVTK_Group_Qt=ON -DCMAKE_INSTALL_PREFIX=/opt/vtk8 -Bbuild .
# cmake --build build/
# sudo cmake --install build
# export CMAKE_PREFIX_PATH=/opt/vtk8
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/vtk8/lib

# pcl 1.14
wget https://github.com/PointCloudLibrary/pcl/releases/download/pcl-1.14.1/source.tar.gz -O pcl.tar.gz
tar -xvf pcl.tar.gz
cd pcl
cmake -Bbuild -DCMAKE_INSTALL_PREFIX=/opt/pcl .
cmake --build build
sudo cmake --install build
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/opt/pcl

# PROJ (C++)
sudo apt-get install libsqlite3-dev sqlite3
wget https://download.osgeo.org/proj/proj-9.4.1.tar.gz
tar -zxvf proj-9.4.1.tar.gz
cd proj-9.4.1
mkdir build
cd build
cmake ..
cmake --build .
sudo cmake --build . --target install

```

## ros2 sample run
1. 使用するrosbag(.db3,.yamlを含むフォルダ)を `hokuyo_slam_ros2/ros2_sample_script/rosbag/` に移す
1. `hokuyo_slam_ros2/ros2_sample_script/hokuyo_slam.bash/` の11行目に、スクリプトが入っているディレクトリのフルパスを記入する
1. トピック名、GNSS共分散のしきい値[m^2]を `hokuyo_slam_ros2/ros2_sample_script/config/config.csv` に記入
1. スクリプト実行
    ```bash
    cd <hokuyo_slam_ros2_dir>
    bash ./ros2_sample_script/hokuyo_slam.bash <使用するrosbag名> <出力する地図名>
    # ex) bash ./ros2_sample_script/hokuyo_slam.bash toyonaka_2025 toyonaka_map
    ```
1. 実行結果のp2oファイル等は `hokuyo_slam_ros2/ros2_sample_script/data/<出力する地図名>/*`に出力されます
1. 実行結果の点群地図は `hokuyo_slam_ros2/ros2_sample_script/map/<出力する地図名>.pcd`に出力されます

## 定例打ち合わせで報告したテストスクリプト
`hokuyo_slam_ros2/ros2_sample_script/p2o_from_rosbag_ros2_test_z.py/` で、rosbagから読み込む際に2秒おきにz方向を1m上昇させるテストを行った。

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(hokuyo_slam_ros2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 依存パッケージの検索
find_package(PCL REQUIRED COMPONENTS common io visualization)
find_package(Eigen3 REQUIRED)
find_package(PROJ REQUIRED)

# グローバルなインクルードパス設定
# (PCL_INCLUDE_DIRS が空の場合に備え、PCL_LIBRARIES から自動伝播させる設定も後述しています)
include_directories(
  include
  ${PROJ_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# 1. p2o 共有ライブラリのビルド
add_library(p2o SHARED src/p2o.cpp include/p2o.h)
target_link_libraries(p2o PRIVATE Eigen3::Eigen)

# 2. run_p2o のビルド
add_executable(run_p2o src/run_p2o.cpp)
if(TARGET PROJ::proj)
    target_link_libraries(run_p2o PRIVATE p2o Eigen3::Eigen PROJ::proj)
else()
    target_link_libraries(run_p2o PRIVATE p2o Eigen3::Eigen ${PROJ_LIBRARIES})
endif()

# 3. rearrange_pointcloud のビルド（エラーが発生していた箇所）
add_executable(rearrange_pointcloud src/rearrange_pointcloud.cpp)
# PCL_LIBRARIES を LINK することで、ヘッダパス(PCL_INCLUDE_DIRS)が空であっても
# CMakeのモダインターゲット機構を通じてコンパイラにパスが自動適用されます
target_link_libraries(rearrange_pointcloud PRIVATE 
    ${PCL_LIBRARIES} 
    Eigen3::Eigen
)

# 4. p2o_viewer のビルド判定
set(BUILD_VIEWER OFF CACHE BOOL "Build p2o viewer")
if (PCL_FOUND)
    set(BUILD_VIEWER ON)
endif()

if(BUILD_VIEWER)
    add_subdirectory(p2o_viewer)
endif()

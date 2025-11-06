// Copyright (C) 2024 Kiyoshi Irie
// MIT License
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream> 
#include <cmath>   
#include <iomanip> 

#include <pcl/common/transforms.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <Eigen/Geometry>

// JSONライブラリのインクルード (nlohmann/json を使用することを想定)
#include "json.hpp"
using json = nlohmann::json;

// ----------------------------------------------------------------------
// ヘルパー関数: 2つの (x, y) 座標間の距離を計算
// ----------------------------------------------------------------------
double calculate_distance_2d(double x1, double y1, double x2, double y2) {
    return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

int main(int argc, char *argv[]) {
    // ----------------------------------------------------------------------
    // 1. 引数の確認と初期設定
    // ----------------------------------------------------------------------
    if (argc != 4) { 
        std::cerr << "使用法: " << argv[0] << " <ログファイル名> <PCD出力ファイル名のベース> <JSON出力ファイル名>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "エラー: ログファイル " << argv[1] << " を開けません。" << std::endl;
        return 1;
    }

    std::string line;
    
    // PCDの出力ベース名 (argv[2])
    std::string str = argv[2]; 
    std::string outpcd_name = str + "_Acord.pcd";

    // JSONの出力ファイル名 (argv[3]から直接読み込む)
    std::string outjson_name = argv[3]; 

    pcl::PointCloud<pcl::PointXYZ>::Ptr merged_cloud(new pcl::PointCloud<pcl::PointXYZ>);
    
    // Waypoint抽出用の変数
    double last_wp_x = 0.0;
    double last_wp_y = 0.0;
    const double MIN_DISTANCE_M = 4.0; // Waypoint抽出の最小距離間隔 (4.0 m)
    json waypoints_list = json::array(); // JSON配列としてウェイポイントを格納
    
    double first_wp_x = 0.0;
    double first_wp_y = 0.0;

    int cnt = 0;

    // ----------------------------------------------------------------------
    // 2. メインループ: 点群の結合と Waypoint の抽出
    // ----------------------------------------------------------------------
    while (std::getline(file, line)) {
        ++cnt;
        // 点群の結合は10行に1回行う (既存のロジックを維持)
        if (cnt % 10 != 0) continue; 
        
        std::istringstream iss(line);
        std::string filename;
        float x, y, z, qx, qy, qz, qw, rx, ry, rz;

        // データ抽出 (11項目)
        if (!(iss >> filename >> x >> y >> z >> qx >> qy >> qz >> qw >> rx >> ry >> rz)) {
            // ログの形式が想定外の場合、スキップ
            std::cerr << "Warning: Skipping malformed log line: " << line << std::endl; 
            continue;
        }

        // ------------------------------------------------------------------
        // A. PCD結合処理 
        // ------------------------------------------------------------------
        Eigen::Quaterniond quaternion(qw, qx, qy, qz);
        Eigen::Translation3d translation(x, y, z);
        Eigen::Affine3d transform = translation * quaternion;

        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
        if (pcl::io::loadPCDFile<pcl::PointXYZ>(filename, *cloud) == -1) {
            std::cerr << "ERROR: Couldn't read file " << filename << std::endl;
            continue;
        }

        pcl::transformPointCloud(*cloud, *cloud, transform);
        *merged_cloud += *cloud;

        // ------------------------------------------------------------------
        // B. Waypoint 抽出処理
        // ------------------------------------------------------------------
        double current_x = static_cast<double>(x);
        double current_y = static_cast<double>(y);
        
        if (waypoints_list.empty() || 
            calculate_distance_2d(current_x, current_y, last_wp_x, last_wp_y) >= MIN_DISTANCE_M) 
        {
            // Waypointリストが空の場合、このポーズを最初の Waypoint として記録
            if (waypoints_list.empty()) {
                first_wp_x = current_x;
                first_wp_y = current_y;
            }
            
            // 相対座標を計算
            double relative_x = current_x - first_wp_x;
            double relative_y = current_y - first_wp_y;

            // 新しい Waypoint を追加
            json wp = json::array({
                // [x, y, 0.0] の形式で相対位置情報
                json::array({relative_x, relative_y, 0.0}), 
                // [0.0, 0.0, qz, qw] の形式で回転情報 
                json::array({0.0, 0.0, qz, qw}), 
                // デフォルトパラメータ
                json::object({
                    {"type", "normal"},
                    {"value", 0.0},
                    {"xy_tolerance", 1.0},
                    {"yaw_tolerance", 3.14}
                })
            });
            waypoints_list.push_back(wp);

            // 最後の Waypoint の絶対座標を更新 (次の距離計算のために使用)
            last_wp_x = current_x;
            last_wp_y = current_y;
        }
    }

    // ----------------------------------------------------------------------
    // 3. 結果の保存
    // ----------------------------------------------------------------------

    // PCDファイルの保存
    pcl::io::savePCDFileASCII(outpcd_name, *merged_cloud);
    std::cout << "PCDファイルを保存しました: " << outpcd_name << std::endl;

    // ----------------------------------------------------------------------
    // 🔧 修正: Waypointリストの最初2つと最後2つを削除
    // ----------------------------------------------------------------------
    size_t min_waypoints = 4; // 削除対象の合計数 (最初2 + 最後2)

    if (waypoints_list.size() > min_waypoints) {
        // 最初2つを削除 (0番目と1番目)
        // erase(開始イテレータ, 終了イテレータ)
        waypoints_list.erase(waypoints_list.begin(), waypoints_list.begin() + 2);
        
        // 残りのリストの最後2つを削除
        // erase(最後尾から数えたイテレータ, 最後尾のイテレータ)
        // waypoints_list.erase(waypoints_list.end() - 2, waypoints_list.end());

        std::cout << "情報: Waypointリストの最初2つと最後2つの要素を削除しました。" << std::endl;
    } else if (waypoints_list.size() > 0) {
        std::cout << "警告: Waypointの数が少ないため、最初2つと最後2つの削除をスキップしました。"
                  << "（現在の要素数: " << waypoints_list.size() << "）" << std::endl;
    }
    
    // JSONファイルの保存
    std::ofstream ofs(outjson_name);
    if (ofs.is_open()) {
        // インデントをつけて読みやすい形式で保存
        ofs << std::setw(4) << waypoints_list << std::endl; 
        std::cout << "Waypointファイルを保存しました: " << outjson_name << std::endl;
    } else {
        std::cerr << "エラー: Waypointファイルを保存できませんでした: " << outjson_name << std::endl;
    }

    return 0;
}
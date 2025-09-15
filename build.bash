#!/bin/bash

#---------------------------------------
# ディレクトリ設定
#---------------------------------------
if [ "$DOCKER_ENV" = "1" ]; then
    # Docker環境の場合
    GITHUB_DIR="/home/github"
else
    # 通常の環境の場合
    GITHUB_DIR="$HOME/github"
fi

echo "使用するgithubディレクトリ: $GITHUB_DIR"

#---------------------------------------
# 前提となるツールのインストール (初回のみ)
#---------------------------------------
# 必要なシステムパッケージをインストールします。
echo "必要なシステムパッケージをインストールします..."

# apt-get で依存パッケージをインストール
echo "apt-get で依存パッケージをインストールします..."
sudo apt-get install -y $(cat apt_packages.txt)

# pip でPythonパッケージをインストール
echo "pip でPythonパッケージをインストールします..."
pip3 install -r requirements.txt

#---------------------------------------
# PROJ のビルドとインストール (ディレクトリが存在しない場合のみ)
#---------------------------------------
cd $GITHUB_DIR
if [ ! -d "proj-9.4.1" ]; then
  echo "PROJ をビルドおよびインストールします..."
  # ダウンロードと展開
  if [ ! -f "proj-9.4.1.tar.gz" ]; then
    wget https://download.osgeo.org/proj/proj-9.4.1.tar.gz
  fi
  tar -zxvf proj-9.4.1.tar.gz
  
  # ビルドとインストール
  cd proj-9.4.1
  mkdir build
  cd build
  cmake ..
  cmake --build .
  sudo cmake --build . --target install
  echo "PROJ のビルドとインストールが完了しました。"
else
  echo "PROJ はすでに存在します。スキップします。"
fi

#---------------------------------------
# PCL 1.14 のビルドとインストール (ディレクトリが存在しない場合のみ)
#---------------------------------------
cd $GITHUB_DIR
if [ ! -d "pcl" ]; then
  echo "PCL 1.14 をビルドおよびインストールします..."
  # ダウンロードと展開
  if [ ! -f "pcl.tar.gz" ]; then
    wget https://github.com/PointCloudLibrary/pcl/releases/download/pcl-1.14.1/source.tar.gz -O pcl.tar.gz
  fi
  tar -xvf pcl.tar.gz
  
  # ビルドとインストール
  cd pcl
  cmake -Bbuild -DCMAKE_INSTALL_PREFIX=/opt/pcl .
  cmake --build build -j$(nproc)
  sudo cmake --install build
  
  # 環境変数を .bashrc に追記 (重複しないようにチェック)
  if ! grep -q "export CMAKE_PREFIX_PATH=/opt/pcl" ~/.bashrc; then
    echo "PCLのCMAKE_PREFIX_PATHを.bashrcに追記します。"
    echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:/opt/pcl" >> ~/.bashrc
    # 現在のセッションでもパスを有効にする
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/opt/pcl
    echo "CMAKE_PREFIX_PATHが更新されました。"
  else
    echo "PCLのCMAKE_PREFIX_PATHはすでに.bashrcに存在します。"
  fi
  echo "PCL 1.14 のビルドとインストールが完了しました。"
else
  echo "PCL 1.14 はすでに存在します。スキップします。"
fi

#---------------------------------------
# hokuyo_slam_ros2 のビルド (buildディレクトリが存在しない場合のみ)
#---------------------------------------
cd $GITHUB_DIR/hokuyo_slam_ros2
if [ -d "build" ]; then
  echo "hokuyo_slam_ros2 はすでにビルドされています。スキップします。"
else
  echo "hokuyo_slam_ros2 をビルドします..."
  mkdir build
  cd build
  
  # PCLのパスが設定されているか確認し、必要であればsource
  if ! grep -q "/opt/pcl" <<< "$CMAKE_PREFIX_PATH"; then
      echo "PCLのパスが設定されていません。.bashrcから読み込みます。"
      source ~/.bashrc
  fi
  
  cmake ..
  cmake --build .
  echo "hokuyo_slam_ros2 のビルドが完了しました。"
fi

echo "--- 全ての処理が完了しました ---"
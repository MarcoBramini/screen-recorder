# pds-screen-recording

An app to capture and record your screen

# Build

## MacOS

1. Install dependencies

```
brew install ffmpeg
brew install fmt
brew install qt6
```
2. Build project
```
export CMAKE_PREFIX_PATH=/usr/local/Cellar/qt/6.2.2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake .  
```

## Linux

1. Install dependencies

```
sudo apt-get install libavdevice-dev
sudo apt-get install libavfilter-dev
sudo apt-get install libfmt-dev
sudo apt-get install libxrandr-dev
sudo apt-get install pip
pip install -U pip
pip install aqtinstall
aqt install-qt linux desktop 6.2.0
```

2. Build project

```
sudo snap install cmake --classic
export CMAKE_PREFIX_PATH=~/6.2.0/gcc_64
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake .  
```

## Windows

1. Install dependencies

```
cd \
git clone https://github.com/Microsoft/vcpkg.git
	cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install ffmpeg:x64-windows
.\vcpkg install fmt:x64-windows
.\vcpkg install qt:x64-windows
```

"-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
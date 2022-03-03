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
cmake --build .  
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
cmake --build .  
```

## Windows

1. Install CMake (>= 3.22)

2. Install Visual Studio environment for desktop c++ applications

3. Install Qt6 MSVC environment for desktop applications

4. Install dependencies

```
cd \
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install ffmpeg[avcodec,avdevice,avfilter,avformat,avresample,core,gpl,postproc,swresample,swscale,x264]:x64-windows
.\vcpkg install fmt:x64-windows
```

5. Build project

```
cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.2.3/msvc2019_64 ..
cmake --build  . -- /property:Configuration=Release
```

6. Provide dependencies
- Release: 
```
cd build\qt_screen_recorder\Release
C:\Qt\6.2.3\msvc2019_64\bin\windeployqt.exe -qmldir ..\..\..\qt_screen_recorder\components --release appqt_screen_recorder.exe
```
- Debug: 
```
cd build\qt_screen_recorder\Debug
C:\Qt\6.2.3\msvc2019_64\bin\windeployqt.exe -qmldir ..\..\..\qt_screen_recorder\components --debug appqt_screen_recorder.exe
```

7. Run
```
set QSG_RHI_BACKEND=opengl // Might be needed for VMs
appqt_screen_recorder.exe
```

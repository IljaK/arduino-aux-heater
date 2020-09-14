Arduino project for controlling audi a4 b6 auxilary heater with TP42 remote from phone.
Also includes battery monitoring and sms notification for very low level.

Project consists of two sub projects:
* Arduino project (aux-heater sub directory)
* Google Test project for unit testing and emulating arduino functionality (root folder)

To work with project it is necissary to have:
1) Arduino software https://www.arduino.cc/
2) Visual Studio Code https://code.visualstudio.com/
  * C/C++ extension https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
  * Arduino extension https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino
  * CMake tools https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
  * Google Test adapter https://marketplace.visualstudio.com/items?itemName=DavidSchuldenfrei.gtest-adapter

Building Test project:
In Visual Studio Code command promt (F1) launch:
 * CMake: Configure
 * CMake: Build

Project configuration has been tested on Windows and Linux. For Mac OS might require some changes.

On windows it is possible to work in Visual Studio. For that it is necissary to have:
1) Arduino software https://www.arduino.cc/
2) CMake https://cmake.org/ (to get configured project for Visual Studio)
2) Visual Studio https://visualstudio.com/
  * VisualMicro extension https://www.visualmicro.com/

Launching project in Visual Studio:
Build project with cmake using CMakeLists.txt
Inside "build" folder locate Visual Studio project.

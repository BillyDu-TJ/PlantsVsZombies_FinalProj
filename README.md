# Plants vs Zombies - Final Project

A Plants vs Zombies game implementation using Cocos2d-x C++ framework, developed as the final project for the **Programming Paradigms(程序设计范式 )** course at **Tongji University (同济大学)**.

## 项目简介 (Project Introduction)

This project is a recreation of the classic Plants vs Zombies tower defense game, demonstrating various programming paradigms including:
- **Object-Oriented Programming (OOP)**: Entity hierarchy with Plants, Zombies, Bullets, and other game objects
- **Game Loop Pattern**: Continuous game state management and rendering
- **Manager Pattern**: Separate managers for data, levels, audio, and scenes
- **MVC Architecture**:  Separation of game logic, data, and presentation

## 功能特点 (Features)

- 🌻 Multiple plant types with unique abilities
- 🧟 Various zombie types with different behaviors
- 🎮 Interactive gameplay with mouse/touch controls
- 🎵 Background music and sound effects
- 📊 Level progression system
- 🏆 Victory and game over scenes
- ☀️ Sun collection and resource management system

## 项目结构 (Project Structure)

```
PlantsVsZombies_FinalProj/
├── Classes/                 # Core game logic
│   ├── Entities/           # Game entities (Plants, Zombies, Bullets, etc.)
│   ├── Managers/           # Game managers (Data, Level, Audio, Scene)
│   ├── Scenes/             # Game scenes (Start, Game, Victory, GameOver)
│   ├── UI/                 # UI components (SeedCard, etc.)
│   └── Utils/              # Utility classes and helpers
├── Resources/              # Game assets (images, audio, fonts)
├── proj.win32/            # Windows platform project
├── proj.linux/            # Linux platform project
├── proj.android/          # Android platform project
├── proj.ios_mac/          # iOS and macOS platform projects
└── CMakeLists.txt         # CMake build configuration
```

## 环境配置 (Environment Setup)

### Prerequisites

1. **Cocos2d-x Framework** (Version 3.x or 4.x)
   - Download from:  https://www.cocos. com/en/cocos2d-x
   - Or clone the repository and place this project in the Cocos2d-x directory

2. **CMake** (Version 3.6 or higher)
   - Download from: https://cmake.org/download/

3. **C++ Compiler**
   - **Windows**: Visual Studio 2017 or later (with C++ desktop development tools)
   - **Linux**: GCC 5.0+ or Clang 3.3+
   - **macOS**:  Xcode 10 or later

### Windows Setup

1. **Install Cocos2d-x**
   ```bash
   # Download and extract Cocos2d-x
   # Place this project folder in the Cocos2d-x directory
   ```

2. **Install Visual Studio 2017+** with C++ desktop development workload

3. **Configure the project**
   ```bash
   mkdir build
   cd build
   cmake ..  -G "Visual Studio 16 2019" -A Win32
   ```

4. **Build and run**
   - Open `PvzGame.sln` in Visual Studio
   - Set the project as startup project
   - Press F5 to build and run

### Linux Setup

1. **Install dependencies**
   ```bash
   sudo apt-get update
   sudo apt-get install -y \
       build-essential \
       cmake \
       libgl1-mesa-dev \
       libglu1-mesa-dev \
       libcurl4-openssl-dev \
       libfontconfig1-dev \
       libgtk-3-dev \
       libasound2-dev
   ```

2. **Install Cocos2d-x** (follow official guide)

3. **Build the project**
   ```bash
   mkdir build
   cd build
   cmake .. 
   make -j4
   ```

4. **Run the game**
   ```bash
   ./bin/PvzGame
   ```

### macOS Setup

1. **Install Xcode** from the App Store

2. **Install CMake**
   ```bash
   brew install cmake
   ```

3. **Install Cocos2d-x** (follow official guide)

4. **Build the project**
   ```bash
   mkdir build
   cd build
   cmake ..  -G Xcode
   open PvzGame.xcodeproj
   ```

5. **Run from Xcode** (⌘+R)

### Android Setup

1. **Install Android Studio** and Android SDK

2. **Install Android NDK** (r16b or later)

3. **Configure environment variables**
   ```bash
   export ANDROID_SDK_ROOT=/path/to/android/sdk
   export NDK_ROOT=/path/to/android/ndk
   ```

4. **Build APK**
   ```bash
   cd proj.android
   ./gradlew assembleDebug
   ```

## 如何运行 (How to Run)

1.  Ensure all dependencies are properly installed
2. Clone this repository
3. Place the `cocos2d` folder in the project root (or set `COCOS2DX_ROOT_PATH`)
4. Follow the platform-specific build instructions above
5. Run the executable or install the APK

## 游戏玩法 (Gameplay)

1. **Start**:  Launch the game and click "Start" on the main menu
2. **Plant Selection**: Select plants from the seed cards at the top
3. **Plant Placement**: Click on grid cells to plant your defenses
4. **Sun Collection**: Collect falling suns to gather resources
5. **Defend**: Prevent zombies from reaching your house
6. **Victory**: Eliminate all zombies to win the level

## 技术栈 (Technology Stack)

- **Language**: C++ 11/14
- **Framework**: Cocos2d-x
- **Build System**: CMake
- **Platforms**: Windows, Linux, macOS, Android, iOS

## 开发团队 (Development Team)

- **Developer**: BillyDu-TJ,ChangwenZhao
- **Course**: 程序设计范式 (Programming Paradigms)
- **Institution**: 同济大学 (Tongji University)

## 许可证 (License)

This project is created for educational purposes as part of a university course. 

## 致谢 (Acknowledgments)

- Cocos2d-x development team for the excellent game engine
- Plants vs Zombies original game by PopCap Games for inspiration
- Tongji University for providing the learning opportunity

---

**Note**: Make sure you have the necessary game assets (images, sounds) in the `Resources` folder before building the project. 

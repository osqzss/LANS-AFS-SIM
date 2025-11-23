# LimeSuiteNG build from source on Windows 11

1. Install Visual Studio 2026

- During installation, make sure to select `Desktop development with C++`

2. Launch the `x64 Native Tools Command Prompt for VS` from the Start Menu

3. Clone and build LimeSuiteNG

```sh
git clone https://github.com/myriadrf/LimeSuiteNG.git
cd LimeSuiteNG
mkdir build
cd build
cmake ..  -DBUILD_GUI=OFF -DBUILD_PULGINS=OFF
cmake --build . --config Release
```
- This should build all the DLLs, libraries, and tools you need
- You can find them in `LimeSuiteNG/build/bin/Release`

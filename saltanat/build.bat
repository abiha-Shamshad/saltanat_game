@echo off
echo Building Saltanat C++ Project...

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake ..

echo Compiling...
cmake --build . --config Release

echo Build Complete.
echo If successful, you can run the executable from the build/Release directory or build/ directory.
cd ..
pause

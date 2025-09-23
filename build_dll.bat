@echo off
echo Building JsonDiffPatch DLL for GameMaker...

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

REM Build the DLL
cmake --build . --config Release --target JsonDiffPatchDLL

echo.
echo Build complete! 
echo DLL location: build\Release\JsonDiffPatch.dll
echo.
echo Testing the DLL...
cmake --build . --config Release --target test_dll

echo.
echo To test the DLL, run: build\Release\test_dll.exe
echo.
pause
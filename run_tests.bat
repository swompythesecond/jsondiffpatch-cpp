@echo off

echo Building JsonDiffPatch library and tests...

if not exist build mkdir build
cd build

cmake ..
cmake --build . --config Release

echo.
echo Running tests...
run_tests.exe

echo.
echo Test run complete!
pause
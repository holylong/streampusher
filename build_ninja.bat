@ECHO OFF
@SETLOCAL

@REM set to  your own vcpkg path
set VCPKG_PATH="D:\works\vcpkg"
set OpenCV_PATH="D:\commonlib\opencv\opencv3416\opencv\build"

rd /s/q build_ninja
mkdir build_ninja
pushd build_ninja
cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DOpenCV_DIR=%OpenCV_PATH%
cmake --build . --parallel %NUMBER_OF_PROCESSORS%
@REM cmake --build . --target install
popd
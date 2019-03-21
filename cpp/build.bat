md build
cd build
cmake .. -DBUILD_TESTING=OFF -DBUILD_UWS=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 14 2015 Win64"
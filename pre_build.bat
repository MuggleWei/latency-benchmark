:: c++ pre build
:: vcpkg.exe install openssl:x64-windows zlib:x64-windows
git submodule update --init
git submodule update --init --recursive -- cpp/thirdparty/uWebSockets/

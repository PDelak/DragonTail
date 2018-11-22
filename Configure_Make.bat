setlocal

mkdir build-make
pushd build-make
cmake -G "MinGW Makefiles" ..
popd

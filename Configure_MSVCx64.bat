setlocal

mkdir build-msvc
pushd build-msvc
cmake -G "Visual Studio 15 2017 Win64" ..
popd 
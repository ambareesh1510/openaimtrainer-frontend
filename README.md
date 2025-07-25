# Aim Trainer

## Building
Install `cmake` and `conan`.

```sh
conan profile detect --force    # modify as necessary
conan install . \
    --output-folder=build   \
    --build=missing \
    --settings=build_type=Debug       # or Release  \
    -o *:shared=False   # always link statically
cd build
# You may need to specify the generator on Windows
cmake ..    \
    -DCMAKE_TOOLCHAIN_FILE="build/Debug/generators/conan_toolchain.cmake"   \
    -DCMAKE_BUILD_TYPE=Debug  # or Release 
cmake --build .
```

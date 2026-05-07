debug: build
    ./build/wdock

build:
    cmake -Bbuild -GNinja
    cmake --build build

release:
    cmake -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ./build/wdock

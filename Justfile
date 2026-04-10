run: build
    ./build/wdock

build: configure
    cmake --build build

configure:
    cmake -Bbuild -GNinja
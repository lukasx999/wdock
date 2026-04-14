# wdock

a minimal, lightweight desktop dock for wayland.

## Build Instructions

```console
git clone https://github.com/lukasx999/wdock.git
cd wdock
cmake -Bbuild
cmake --build build
sudo cmake --install .
```

## Configuration

The config file is located at `~/.config/config.kdl`.
wdock uses the [KDL](https://kdl.dev/) language for configuration.

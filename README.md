# wdock

a minimal, lightweight desktop dock for wayland.

## Build Instructions

```console
git clone https://github.com/lukasx999/wdock.git
cd wdock
cmake -Bbuild
cmake --build build
sudo cmake --install build
```

## Configuration

The config file is located at `~/.config/config.kdl`. \
wdock uses the [KDL](https://kdl.dev/) language for configuration.

## FAQ

### What about X11?

I do not intend do add support for X11, but I suppose that porting wdock to X11 would not be very hard, as you can just swap out the wayland initialization code, aswell as the Dear ImGui backend for the corresponding Xlib code. If someone wants to do that, then I'll happily merge the PR.
# libretro-rimage

A proof-of-concept of using libretro with raylib's Image API [rimage](https://github.com/robloach/rimage).

## Example

```
ClearBackground(SKYBLUE);
ImageDrawCircle(&core->backBuffer, 200, 150, 30, RED);
ImageDrawRectangle(&core->backBuffer, 400, 200, 100, 140, GREEN);
ImageDrawLine(&core->backBuffer, 50, 300, 550, 380, ORANGE);
```

## Development

```
cmake -B build .
make -C build

```

For some reason it needs a ROM-name, that is an existing file, even though it doesn't read it, or it segfaults:

```
# linux
retroarch -L build/rimage_libretro.so README.md

# mac
/Applications/RetroArch.app/Contents/MacOS/RetroArch -L build/rimage_libretro.dylib README.md

# windows
retroarch -L build/rimage_libretro.dll README.md
```

## License

[zlib](LICENSE)

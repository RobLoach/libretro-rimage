# libretro-rimage

# Deprecated. Used [pntr](https://github.com/robloach/pntr) instead

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
git submodule update --init
make
retroarch -L rimage_libretro.so
```

## License

[zlib](LICENSE)

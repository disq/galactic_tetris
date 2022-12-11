# Galactic Tetris

Tetris on the [Galactic Unicorn](https://shop.pimoroni.com/products/galactic-unicorn)!

![demo](./doc/demo3.gif)

## How to Flash

Copy the [galactic_tetris.uf2](./build/galactic_tetris.uf2) file to the automatically mounted volume when the Unicorn is in `BOOTSEL` mode.

## Usage

Hold the Unicorn sideways with the A/B/C/D buttons on the bottom and use the buttons to play.

- `A` Left
- `B` Down
- `C` Up
- `D` Right

Game starts in demo/attract mode and will (badly) auto-play some games. During this time, the bottom row (when you're holding the device as intended) will light up. Use any of the A/B/C/D keys to stop the demo mode and start playing.

Brightness is automatically taken care of using the light sensor. Use the 'Lux' buttons to adjust the brightness manually.

Volume keys to manually decrease/increase speed or `Zzz` button to freeze the game.

## Build

```bash
mkdir build && cd build
cmake ..
make -j8
cp -X galactic_tetris.uf2 /Volumes/RPI-RP2/ # -X is for macOS
```

### Acknowledgements

- Educational purposes only. Tetris game copyright (c) The Tetris Company. Not intended for commercial use. 
- Tetris code copied from the [Tetris200lines project](https://github.com/najibghadri/Tetris200lines) and modified to work with the Unicorn.
- Code in [util.cpp](./util.cpp) is mostly from [Pimoroni-Pico examples](https://github.com/pimoroni/pimoroni-pico/tree/main/examples/galactic_unicorn)

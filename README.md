# Galactic Tetris

Tetris on the Galactic Unicorn!

![demo](./doc/demo1.gif)

## How to Flash

Copy the [galactic_tetris.uf2](./build/galactic_tetris.uf2) file to the automatically mounted volume when the Unicorn is in `BOOTSEL` mode.

## Usage

Hold the Unicorn sideways with the A/B/C/D buttons on the bottom and use the buttons to play.

- `A` Left
- `B` Down
- `C` Up
- `D` Right

Brightness is automatically taken care of using the light sensor. Use the 'Lux' buttons to adjust the brightness manually.

### Acknowledgements

- Educational purposes only. Tetris game copyright (c) The Tetris Company. Not intended for commercial use. 
- Tetris code copied from the [Tetris200lines project](https://github.com/najibghadri/Tetris200lines) and modified to work with the Unicorn.
- Code in [util.cpp](./util.cpp) is mostly from [Pimoroni-Pico examples](https://github.com/pimoroni/pimoroni-pico/tree/main/examples/galactic_unicorn)

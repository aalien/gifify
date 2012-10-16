# gifify

Create gifs with more than 256 colors

## How to use

Currently, gifify needs external tools to create the final gif file.

1. gifify < picture.ppm
2. ppmtogif -transparent "#ddffff" frameN.ppm > frameN.gif
3. convert -delay 1 frame*.gif picture.gif

## License

MIT

## Copyright

Antti Laine <antti.a.laine@iki.fi>

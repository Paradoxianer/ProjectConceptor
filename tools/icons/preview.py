"""Renders every icon at 20, 32 and 64 pixels, on a light and a dark
toolbar color, into one sheet: python3 preview.py sheet.png"""

import sys

import cairo

import hvif
from icons import ICONS

SIZES = (20, 32, 64)
BACKGROUNDS = ((0.85, 0.85, 0.85), (0.20, 0.21, 0.23))
CELL = 76


def main(target):
    names = list(ICONS)
    width = CELL * len(SIZES) * len(BACKGROUNDS) + 120
    height = CELL * len(names)
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height)
    context = cairo.Context(surface)
    context.set_source_rgb(1, 1, 1)
    context.paint()
    for row, name in enumerate(names):
        shapes = ICONS[name][1]()
        hvif.encode(shapes)
        context.set_source_rgb(0, 0, 0)
        context.move_to(6, row * CELL + CELL / 2)
        context.show_text(name)
        column = 0
        for background in BACKGROUNDS:
            for size in SIZES:
                x = 120 + column * CELL
                y = row * CELL
                context.set_source_rgb(*background)
                context.rectangle(x, y, CELL, CELL)
                context.fill()
                context.save()
                context.translate(x + (CELL - size) / 2, y + (CELL - size) / 2)
                hvif.preview(context, shapes, size)
                context.restore()
                column += 1
    surface.write_to_png(target)


if __name__ == '__main__':
    main(sys.argv[1])

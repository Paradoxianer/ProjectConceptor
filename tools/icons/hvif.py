"""Writes Haiku Vector Icon Format (HVIF) data and previews it with cairo.

The encoding follows Haiku's src/libs/icon/flat_icon (FlatIconImporter.cpp,
FlatIconFormat.cpp): magic "ncif", then styles, paths and shapes, each
section a count byte followed by its entries. Coordinates live in a
64 x 64 box.
"""

import math
import struct


STYLE_SOLID = 1
STYLE_GRADIENT = 2
SHAPE_PATH_SOURCE = 10
TRANSFORMER_STROKE = 23

GRADIENT_FLAG_TRANSFORM = 1 << 1

PATH_FLAG_CLOSED = 1 << 1
PATH_FLAG_NO_CURVES = 1 << 3

SHAPE_FLAG_HAS_TRANSFORMERS = 1 << 4

# agg line caps and joins, as StrokeTransformer takes them
CAP_BUTT, CAP_SQUARE, CAP_ROUND = 0, 1, 2
JOIN_MITER, JOIN_MITER_REVERT, JOIN_ROUND = 0, 1, 2


def rgba(hex_color, alpha=255):
    hex_color = hex_color.lstrip('#')
    return (int(hex_color[0:2], 16), int(hex_color[2:4], 16),
        int(hex_color[4:6], 16), alpha)


class Solid:
    def __init__(self, color):
        self.color = color

    def encode(self):
        return bytes([STYLE_SOLID]) + bytes(self.color)

    def key(self):
        return ('solid', self.color)


class Linear:
    """A linear gradient from p1 to p2 (icon coordinates); stops are
    (offset 0..1, color)."""

    def __init__(self, p1, p2, stops):
        self.p1, self.p2, self.stops = p1, p2, stops

    def matrix(self):
        # gradient space runs from -64 to 64 along x (IconRenderer.cpp)
        dx = self.p2[0] - self.p1[0]
        dy = self.p2[1] - self.p1[1]
        mx = (self.p1[0] + self.p2[0]) / 2
        my = (self.p1[1] + self.p2[1]) / 2
        # agg trans_affine order: sx, shy, shx, sy, tx, ty
        return [dx / 128, dy / 128, -dy / 128, dx / 128, mx, my]

    def encode(self):
        out = bytes([STYLE_GRADIENT, 0, GRADIENT_FLAG_TRANSFORM, len(self.stops)])
        for value in self.matrix():
            out += float_24(value)
        for offset, color in self.stops:
            out += bytes([int(round(offset * 255))]) + bytes(color)
        return out

    def key(self):
        return ('linear', self.p1, self.p2, tuple(self.stops))


class Path:
    """points: (x, y) for corners, or (x, y, in, out) with bezier
    control points for curves."""

    def __init__(self, points, closed=True):
        self.points, self.closed = points, closed

    def has_curves(self):
        return any(len(p) == 4 for p in self.points)

    def full_points(self):
        return [p if len(p) == 4 else (p[0], p[1], (p[0], p[1]), (p[0], p[1]))
            for p in self.points]

    def encode(self):
        flags = PATH_FLAG_CLOSED if self.closed else 0
        if not self.has_curves():
            out = bytes([flags | PATH_FLAG_NO_CURVES, len(self.points)])
            for x, y in self.points:
                out += coord(x) + coord(y)
            return out
        out = bytes([flags, len(self.points)])
        for x, y, pin, pout in self.full_points():
            out += coord(x) + coord(y) + coord(pin[0]) + coord(pin[1]) \
                + coord(pout[0]) + coord(pout[1])
        return out

    def key(self):
        return (tuple(self.points), self.closed)


class Shape:
    """A fill of paths, or with stroke=(width, cap, join) their outline."""

    def __init__(self, style, paths, stroke=None):
        self.style = style
        self.paths = paths if isinstance(paths, list) else [paths]
        self.stroke = stroke


def coord(value):
    # FlatIconFormat.cpp write_coord()
    value = max(-128.0, min(192.0, value))
    if value == int(value) and -32 <= value <= 95:
        return bytes([int(value + 32)])
    packed = int((value + 128.0) * 102.0) | 0x8000
    return bytes([packed >> 8, packed & 0xff])


def float_24(value):
    # FlatIconFormat.cpp write_float_24(): 1 bit sign, 6 exponent, 17 mantissa
    if value == 0:
        return bytes(3)
    bits = struct.unpack('<I', struct.pack('<f', value))[0]
    sign = bits >> 31
    exponent = ((bits >> 23) & 0xff) - 127
    mantissa = bits & 0x7fffff
    if exponent >= 32 or exponent < -32:
        return bytes(3)
    packed = (sign << 23) | ((exponent + 32) << 17) | (mantissa >> 6)
    return bytes([packed >> 16, (packed >> 8) & 0xff, packed & 0xff])


def encode(shapes):
    styles, paths = [], []

    def index(table, item):
        for i, known in enumerate(table):
            if known.key() == item.key():
                return i
        table.append(item)
        return len(table) - 1

    shape_data = b''
    for shape in shapes:
        style_index = index(styles, shape.style)
        path_indices = [index(paths, p) for p in shape.paths]
        data = bytes([SHAPE_PATH_SOURCE, style_index, len(path_indices)]) \
            + bytes(path_indices)
        if shape.stroke:
            width, cap, join = shape.stroke
            data += bytes([SHAPE_FLAG_HAS_TRANSFORMERS, 1, TRANSFORMER_STROKE,
                int(round(width + 128)), (cap << 4) | join, 4])
        else:
            data += bytes([0])
        shape_data += data
    if len(styles) > 255 or len(paths) > 255 or len(shapes) > 255:
        raise ValueError('too many styles, paths or shapes')
    out = b'ncif'
    out += bytes([len(styles)]) + b''.join(s.encode() for s in styles)
    out += bytes([len(paths)]) + b''.join(p.encode() for p in paths)
    out += bytes([len(shapes)]) + shape_data
    return out


def preview(context, shapes, size):
    """Draws shapes into a cairo context, the 64 box scaled to size."""
    import cairo
    context.save()
    context.scale(size / 64.0, size / 64.0)
    for shape in shapes:
        context.new_path()
        for path in shape.paths:
            points = path.full_points()
            context.move_to(points[0][0], points[0][1])
            count = len(points) if path.closed else len(points) - 1
            for i in range(count):
                a = points[i]
                b = points[(i + 1) % len(points)]
                if a[3] == (a[0], a[1]) and b[2] == (b[0], b[1]):
                    context.line_to(b[0], b[1])
                else:
                    context.curve_to(a[3][0], a[3][1], b[2][0], b[2][1], b[0], b[1])
            if path.closed:
                context.close_path()
        style = shape.style
        if isinstance(style, Solid):
            r, g, b, a = style.color
            source = cairo.SolidPattern(r / 255, g / 255, b / 255, a / 255)
        else:
            source = cairo.LinearGradient(style.p1[0], style.p1[1],
                style.p2[0], style.p2[1])
            for offset, (r, g, b, a) in style.stops:
                source.add_color_stop_rgba(offset, r / 255, g / 255, b / 255, a / 255)
        context.set_source(source)
        if shape.stroke:
            width, cap, join = shape.stroke
            context.set_line_width(width)
            context.set_line_cap([cairo.LINE_CAP_BUTT, cairo.LINE_CAP_SQUARE,
                cairo.LINE_CAP_ROUND][cap])
            context.set_line_join([cairo.LINE_JOIN_MITER, cairo.LINE_JOIN_MITER,
                cairo.LINE_JOIN_ROUND][join])
            context.stroke()
        else:
            context.set_fill_rule(cairo.FILL_RULE_EVEN_ODD)
            context.fill()
    context.restore()


# shape helpers, all in the 64 box

def rect(x0, y0, x1, y1):
    return Path([(x0, y0), (x1, y0), (x1, y1), (x0, y1)])


def rounded_rect(x0, y0, x1, y1, r):
    k = r * 0.5523
    return Path([
        (x0 + r, y0, (x0 + r - k, y0), (x0 + r, y0)),
        (x1 - r, y0, (x1 - r, y0), (x1 - r + k, y0)),
        (x1, y0 + r, (x1, y0 + r - k), (x1, y0 + r)),
        (x1, y1 - r, (x1, y1 - r), (x1, y1 - r + k)),
        (x1 - r, y1, (x1 - r + k, y1), (x1 - r, y1)),
        (x0 + r, y1, (x0 + r, y1), (x0 + r - k, y1)),
        (x0, y1 - r, (x0, y1 - r + k), (x0, y1 - r)),
        (x0, y0 + r, (x0, y0 + r), (x0, y0 + r - k)),
    ])


def ellipse(cx, cy, rx, ry):
    kx, ky = rx * 0.5523, ry * 0.5523
    return Path([
        (cx, cy - ry, (cx - kx, cy - ry), (cx + kx, cy - ry)),
        (cx + rx, cy, (cx + rx, cy - ky), (cx + rx, cy + ky)),
        (cx, cy + ry, (cx + kx, cy + ry), (cx - kx, cy + ry)),
        (cx - rx, cy, (cx - rx, cy + ky), (cx - rx, cy - ky)),
    ])


def polygon(*points):
    return Path(list(points))


def line(*points):
    return Path(list(points), closed=False)


def arc(cx, cy, r, start, end, steps=None):
    """An open arc as bezier segments, angles in degrees (0 = right,
    clockwise on screen)."""
    if steps is None:
        steps = max(1, int(math.ceil(abs(end - start) / 90.0)))
    span = math.radians(end - start) / steps
    k = 4.0 / 3.0 * math.tan(span / 4.0) * r
    points = []
    for i in range(steps + 1):
        a = math.radians(start) + span * i
        x, y = cx + r * math.cos(a), cy + r * math.sin(a)
        tx, ty = -math.sin(a), math.cos(a)
        pin = (x - k * tx, y - k * ty) if i > 0 else (x, y)
        pout = (x + k * tx, y + k * ty) if i < steps else (x, y)
        points.append((x, y, pin, pout))
    return Path(points, closed=False)

"""ProjectConceptor's toolbar icons, in the 64 x 64 HVIF box.

Haiku look: light from the top left, gradients from light to dark, a dark
outline around every part. Toolbar icons show at about 20 pixels, so every
part is kept bold: one outline unit is about 1/3 of a pixel there.
"""

from hvif import (Shape, Solid, Linear, rgba, rect, rounded_rect, ellipse,
    polygon, line, arc, CAP_ROUND, CAP_BUTT, JOIN_ROUND, JOIN_MITER)

OUTLINE = rgba('2a303b')
OUTLINE_WIDTH = 3.2


def vertical(y0, y1, light, dark):
    return Linear((0, y0), (0, y1), [(0, rgba(light)), (1, rgba(dark))])


def diagonal(x0, y0, x1, y1, light, dark):
    return Linear((x0, y0), (x1, y1), [(0, rgba(light)), (1, rgba(dark))])


def outlined(style, path, width=OUTLINE_WIDTH):
    return [Shape(style, path), Shape(Solid(OUTLINE), path,
        stroke=(width, CAP_ROUND, JOIN_ROUND))]


def stroked(style, path, width, outline=True):
    """A thick line with a dark rim: the outline drawn wider underneath."""
    shapes = []
    if outline:
        shapes.append(Shape(Solid(OUTLINE), path,
            stroke=(width + 2 * OUTLINE_WIDTH, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(style, path, stroke=(width, CAP_ROUND, JOIN_ROUND)))
    return shapes


PAPER = lambda y0, y1: vertical(y0, y1, 'ffffff', 'd9dee6')
BLUE = lambda y0, y1: vertical(y0, y1, '7fb0f0', '2f63b8')
YELLOW_BACK = lambda y0, y1: vertical(y0, y1, 'f2c14e', 'c98a1c')
YELLOW_FRONT = lambda y0, y1: vertical(y0, y1, 'ffe08a', 'eaa92a')
GREEN = lambda y0, y1: vertical(y0, y1, '8fdc7a', '2f9a3c')
GREY = lambda y0, y1: vertical(y0, y1, 'dfe3e9', '8d96a3')
RED = lambda y0, y1: vertical(y0, y1, 'f08a76', 'c43d2c')


def page(x0, y0, x1, y1, fold):
    return polygon((x0, y0), (x1 - fold, y0), (x1, y0 + fold), (x1, y1), (x0, y1))


def page_with_fold(x0, y0, x1, y1, fold):
    shapes = outlined(PAPER(y0, y1), page(x0, y0, x1, y1, fold))
    corner = polygon((x1 - fold, y0), (x1 - fold, y0 + fold), (x1, y0 + fold))
    shapes += outlined(vertical(y0, y0 + fold, 'eef1f5', 'b9c1cc'), corner)
    return shapes


def plus_badge(cx, cy, r):
    shapes = outlined(GREEN(cy - r, cy + r), ellipse(cx, cy, r, r))
    arm = r * 0.55
    white = Solid(rgba('ffffff'))
    shapes.append(Shape(white, line((cx - arm, cy), (cx + arm, cy)),
        stroke=(4.5, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(white, line((cx, cy - arm), (cx, cy + arm)),
        stroke=(4.5, CAP_ROUND, JOIN_ROUND)))
    return shapes


def icon_new():
    return page_with_fold(10, 4, 46, 58, 12) + plus_badge(45, 45, 14)


def icon_open():
    back = polygon((4, 12), (22, 12), (27, 18), (56, 18), (56, 54), (4, 54))
    front = polygon((12, 28), (62, 28), (54, 54), (4, 54))
    return outlined(YELLOW_BACK(12, 54), back) + outlined(YELLOW_FRONT(28, 54), front)


def floppy(x0, y0, x1, y1):
    w = x1 - x0
    body = polygon((x0, y0), (x1 - 7, y0), (x1, y0 + 7), (x1, y1), (x0, y1))
    shapes = outlined(BLUE(y0, y1), body)
    shutter = rect(x0 + w * 0.25, y0, x0 + w * 0.72, y0 + (y1 - y0) * 0.32)
    shapes += outlined(GREY(y0, y0 + 16), shutter)
    slot = rect(x0 + w * 0.55, y0 + 3, x0 + w * 0.65, y0 + (y1 - y0) * 0.32 - 3)
    shapes.append(Shape(Solid(rgba('3a4250')), slot))
    label = rect(x0 + w * 0.17, y0 + (y1 - y0) * 0.5, x1 - w * 0.17, y1)
    shapes += outlined(PAPER(y0 + 26, y1), label)
    return shapes


def icon_save():
    return floppy(8, 6, 56, 58)


def pencil(x0, y0, x1, y1, width):
    """A pencil from its tip at (x0, y0) to its end at (x1, y1)."""
    import math
    dx, dy = x1 - x0, y1 - y0
    length = math.hypot(dx, dy)
    ux, uy = dx / length, dy / length
    px, py = -uy * width / 2, ux * width / 2
    tip = 11
    bx, by = x0 + ux * tip, y0 + uy * tip
    body = polygon((bx + px, by + py), (x1 + px, y1 + py), (x1 - px, y1 - py),
        (bx - px, by - py))
    point = polygon((x0, y0), (bx + px, by + py), (bx - px, by - py))
    shapes = outlined(diagonal(x0, y0, x1, y1, 'ffd75e', 'e08a12'), body)
    shapes += outlined(vertical(y0, by, 'f6e2c0', 'd4b182'), point)
    return shapes


def icon_save_as():
    return floppy(4, 4, 46, 50) + pencil(30, 60, 60, 30, 10)


def icon_print():
    paper_in = rect(18, 4, 46, 26)
    body = rounded_rect(4, 22, 60, 48, 6)
    slot = rect(14, 40, 50, 44)
    paper_out = rect(16, 40, 48, 60)
    shapes = outlined(PAPER(4, 26), paper_in)
    shapes += outlined(GREY(22, 48), body)
    shapes.append(Shape(Solid(rgba('3a4250')), slot))
    shapes += outlined(PAPER(40, 60), paper_out)
    lines = Solid(rgba('9aa3b0'))
    shapes.append(Shape(lines, line((22, 49), (42, 49)), stroke=(2.5, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(lines, line((22, 54), (36, 54)), stroke=(2.5, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(Solid(rgba('5fd36a')), ellipse(51, 30, 3, 3)))
    return shapes


def mirror(shapes):
    """The same icon flipped left to right."""
    def flip_point(p):
        if len(p) == 2:
            return (64 - p[0], p[1])
        return (64 - p[0], p[1], (64 - p[2][0], p[2][1]), (64 - p[3][0], p[3][1]))

    def flip_style(style):
        if isinstance(style, Linear):
            return Linear((64 - style.p1[0], style.p1[1]),
                (64 - style.p2[0], style.p2[1]), style.stops)
        return style

    out = []
    for shape in shapes:
        paths = [type(p)([flip_point(q) for q in p.points], p.closed) for p in shape.paths]
        out.append(Shape(flip_style(shape.style), paths, shape.stroke))
    return out


def icon_undo():
    # a hook: from the arrow head on the left, around to the bottom
    curve = line((24, 20), (38, 20, (38, 20), (47, 20)),
        (52, 34, (52, 26), (52, 42)), (38, 48, (47, 48), (38, 48)), (26, 48))
    head = polygon((6, 20), (26, 6), (26, 34))
    shapes = stroked(BLUE(14, 54), curve, 8)
    shapes += outlined(BLUE(6, 34), head)
    return shapes


def icon_redo():
    return mirror(icon_undo())


def icon_find():
    shapes = stroked(vertical(36, 60, '6b7380', '2f3540'), line((40, 40), (56, 56)), 9)
    glass = ellipse(26, 26, 18, 18)
    shapes.append(Shape(Solid(OUTLINE), glass, stroke=(9, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(vertical(8, 44, 'e9f3ff', '8fb8ea'), glass))
    shapes.append(Shape(GREY(8, 44), glass, stroke=(4, CAP_ROUND, JOIN_ROUND)))
    shapes.append(Shape(Solid(rgba('ffffff', 200)),
        arc(26, 26, 11, 200, 260), stroke=(3.5, CAP_ROUND, JOIN_ROUND)))
    return shapes


def icon_delete():
    body = polygon((14, 20), (50, 20), (46, 60), (18, 60))
    lid = rounded_rect(8, 12, 56, 20, 2)
    handle = rect(24, 6, 40, 12)
    shapes = outlined(GREY(6, 12), handle)
    shapes += outlined(GREY(20, 60), body)
    shapes += outlined(vertical(12, 20, 'f2f4f7', 'a7b0bc'), lid)
    rib = Solid(rgba('6f7987'))
    for x0, x1 in ((24, 25), (32, 32), (40, 39)):
        shapes.append(Shape(rib, line((x0, 26), (x1, 54)), stroke=(3, CAP_ROUND, JOIN_ROUND)))
    return shapes


def icon_grid():
    shapes = []
    for x in (12, 32, 52):
        for y in (12, 32, 52):
            shapes += outlined(vertical(y - 6, y + 6, 'ffffff', '9aa6b8'),
                ellipse(x, y, 5.5, 5.5), 2.6)
    return shapes


def icon_guides():
    shapes = outlined(BLUE(22, 42), rounded_rect(4, 22, 22, 42, 3))
    shapes += outlined(BLUE(22, 42), rounded_rect(42, 22, 60, 42, 3))
    accent = Solid(rgba('1e90ff'))
    for x in (26, 33):
        shapes.append(Shape(accent, rect(x, 30, x + 5, 34)))
    line_color = Solid(rgba('1e90ff', 150))
    shapes.append(Shape(line_color, line((13, 6), (13, 58)), stroke=(2, CAP_BUTT, JOIN_MITER)))
    shapes.append(Shape(line_color, line((51, 6), (51, 58)), stroke=(2, CAP_BUTT, JOIN_MITER)))
    return shapes


def node_box(x0, y0, x1, y1, light='ffffff', dark='cfd6e0'):
    shapes = outlined(vertical(y0, y1, light, dark), rounded_rect(x0, y0, x1, y1, 3))
    shapes.append(Shape(BLUE(y0, y0 + 4), rect(x0 + 2, y0 + 1.5, x1 - 2, y0 + 4.5)))
    return shapes


def star(cx, cy, r):
    import math
    points = []
    for i in range(10):
        a = math.radians(-90 + i * 36)
        radius = r if i % 2 == 0 else r * 0.45
        points.append((cx + radius * math.cos(a), cy + radius * math.sin(a)))
    return outlined(vertical(cy - r, cy + r, 'fff3a0', 'f0a81c'), polygon(*points), 2.4)


LINK = Solid(rgba('c5ccd6'))


def tree(quarter_turns=0):
    """Root and two children; turned, the boxes themselves stay upright."""
    def turn(x, y):
        for _ in range(quarter_turns % 4):
            x, y = 64 - y, x
        return x, y

    root = turn(32, 12)
    children = [turn(14, 50), turn(50, 50)]
    shapes = []
    for child in children:
        shapes += stroked(LINK, line(root, child), 2.5)
    for x, y in [root] + children:
        shapes += node_box(x - 10, y - 8, x + 10, y + 8)
    return shapes


def icon_auto_layout():
    return tree() + star(52, 14, 11)


EDGE = Solid(rgba('8a95a6'))


def dot(x, y, r=6.5):
    return outlined(vertical(y - r, y + r, '9cc4f6', '2f63b8'), ellipse(x, y, r, r), 2.6)


def edges(*segments):
    return [Shape(EDGE, line(*segment), stroke=(3, CAP_ROUND, JOIN_ROUND))
        for segment in segments]


def connection_icon(path):
    shapes = stroked(LINK, path, 3)
    shapes += node_box(2, 40, 20, 56)
    shapes += node_box(44, 8, 62, 24)
    return shapes


def icon_linear():
    return connection_icon(line((20, 48), (44, 16)))


def icon_bended():
    return connection_icon(line((20, 48, (20, 48), (36, 48)), (44, 16, (28, 16), (44, 16))))


def icon_angled():
    return connection_icon(line((20, 48), (32, 48), (32, 16), (44, 16)))


def arrow_head(tip_x, direction):
    """A filled arrow head on the middle line, pointing left (-1) or right (1)."""
    back = tip_x - direction * 16
    return outlined(BLUE(20, 44), polygon((tip_x, 32), (back, 20), (back, 44)), 2.6)


def arrows_icon(at_source, at_target):
    x0 = 18 if at_source else 6
    x1 = 46 if at_target else 58
    shapes = stroked(LINK, line((x0, 32), (x1, 32)), 3.5)
    if at_target:
        shapes += arrow_head(60, 1)
    if at_source:
        shapes += arrow_head(4, -1)
    return shapes


def icon_topo_dot():
    shapes = edges([(32, 12), (16, 32)], [(32, 12), (48, 32)], [(16, 32), (10, 52)],
        [(16, 32), (24, 52)], [(48, 32), (48, 52)])
    for x, y in ((32, 12), (16, 32), (48, 32), (10, 52), (24, 52), (48, 52)):
        shapes += dot(x, y)
    return shapes


def icon_topo_neato():
    shapes = []
    spring = line((14, 18), (24, 14), (30, 28), (38, 16), (50, 26))
    shapes.append(Shape(EDGE, spring, stroke=(2.6, CAP_ROUND, JOIN_ROUND)))
    shapes += edges([(14, 18), (22, 50)], [(50, 26), (22, 50)], [(50, 26), (52, 52)],
        [(22, 50), (52, 52)])
    for x, y in ((14, 18), (50, 26), (22, 50), (52, 52)):
        shapes += dot(x, y)
    return shapes


def icon_topo_fdp():
    points = ((32, 10), (12, 28), (52, 26), (20, 54), (46, 52), (32, 34))
    shapes = edges([points[0], points[1]], [points[0], points[2]], [points[1], points[3]],
        [points[2], points[4]], [points[3], points[4]], [points[5], points[0]],
        [points[5], points[3]], [points[5], points[4]])
    for x, y in points:
        shapes += dot(x, y, 6)
    return shapes


def icon_topo_sfdp():
    points = ((10, 12), (28, 8), (46, 14), (58, 30), (16, 30), (36, 28), (8, 48),
        (26, 46), (44, 44), (54, 56), (34, 58))
    links = ((0, 1), (1, 2), (2, 3), (0, 4), (1, 5), (4, 5), (5, 2), (4, 6), (4, 7),
        (5, 8), (7, 8), (8, 9), (8, 3), (7, 10), (10, 9))
    shapes = edges(*[[points[a], points[b]] for a, b in links])
    for x, y in points:
        shapes += dot(x, y, 4.5)
    return shapes


def icon_topo_circo():
    import math
    points = [(32 + 22 * math.cos(math.radians(-90 + i * 60)),
        32 + 22 * math.sin(math.radians(-90 + i * 60))) for i in range(6)]
    shapes = edges(*[[points[i], points[(i + 1) % 6]] for i in range(6)])
    for x, y in points:
        shapes += dot(x, y, 6)
    return shapes


def icon_topo_twopi():
    import math
    points = [(32 + 23 * math.cos(math.radians(-90 + i * 60)),
        32 + 23 * math.sin(math.radians(-90 + i * 60))) for i in range(6)]
    shapes = edges(*[[(32, 32), p] for p in points])
    shapes += dot(32, 32, 7.5)
    for x, y in points:
        shapes += dot(x, y, 5.5)
    return shapes


# name -> (where the resource goes, builder)
ICONS = {
    'new':			('app', icon_new),
    'open':			('app', icon_open),
    'save':			('app', icon_save),
    'save as':		('app', icon_save_as),
    'print':		('app', icon_print),
    'undo':			('app', icon_undo),
    'redo':			('app', icon_redo),
    'find':			('app', icon_find),
    'trash':		('app', icon_delete),
    'grid':			('GraphEditor', icon_grid),
    'guides':		('GraphEditor', icon_guides),
    'linear':		('GraphEditor', icon_linear),
    'bended':		('GraphEditor', icon_bended),
    'angeld':		('GraphEditor', icon_angled),
    'arrow-target':	('GraphEditor', lambda: arrows_icon(False, True)),
    'arrow-source':	('GraphEditor', lambda: arrows_icon(True, False)),
    'arrow-both':	('GraphEditor', lambda: arrows_icon(True, True)),
    'arrow-none':	('GraphEditor', lambda: arrows_icon(False, False)),
    'layout':		('LayoutEditor', icon_auto_layout),
    'dir-tb':		('LayoutEditor', tree),
    'dir-lr':		('LayoutEditor', lambda: tree(3)),
    'dir-rl':		('LayoutEditor', lambda: tree(1)),
    'dir-bt':		('LayoutEditor', lambda: tree(2)),
    'topo-dot':		('LayoutEditor', icon_topo_dot),
    'topo-neato':	('LayoutEditor', icon_topo_neato),
    'topo-fdp':		('LayoutEditor', icon_topo_fdp),
    'topo-sfdp':	('LayoutEditor', icon_topo_sfdp),
    'topo-circo':	('LayoutEditor', icon_topo_circo),
    'topo-twopi':	('LayoutEditor', icon_topo_twopi),
}

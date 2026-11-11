import math
import sys
from PIL import Image, ImageDraw

PAL = {
    'K': (24, 20, 36), 'k': (8, 8, 14), 'W': (246, 246, 250), 'w': (204, 206, 218),
    'g': (146, 148, 164), 'G': (86, 88, 104), 'r': (232, 64, 74), 'R': (150, 32, 44),
    'o': (247, 146, 44), 'y': (252, 218, 66), 'Y': (196, 152, 34), 'l': (156, 232, 86),
    'v': (64, 174, 86), 'V': (30, 104, 54), 'c': (86, 222, 232), 'b': (74, 124, 232),
    'B': (40, 62, 142), 'p': (164, 96, 226), 'm': (232, 74, 184), 'P': (255, 158, 204),
    'n': (154, 98, 58), 'N': (92, 56, 30), 's': (242, 204, 164), 'x': (255, 242, 212),
    'q': (128, 204, 255), 'Q': (42, 42, 74), 'u': (255, 120, 120), 'h': (255, 252, 200),
    'z': (70, 50, 110), 'Z': (36, 26, 60), 't': (110, 220, 170), 'T': (40, 120, 110),
    'j': (210, 170, 255), 'a': (255, 196, 90), 'A': (170, 100, 20), 'd': (60, 64, 80),
    'D': (30, 32, 44), 'f': (255, 90, 40), 'F': (255, 200, 60), 'i': (190, 255, 250),
}
TINT = {'1': (1, 1, 1), '2': (2, 2, 2), '3': (3, 3, 3), 'E': (4, 4, 4)}
ALL = dict(PAL)
ALL.update(TINT)
INV = {v: k for k, v in ALL.items()}


class G:
    def __init__(self, w=28, h=32):
        self.w, self.h = w, h
        self.c = [['.'] * w for _ in range(h)]

    def p(self, x, y, ch):
        if 0 <= x < self.w and 0 <= y < self.h and ch != '.':
            self.c[y][x] = ch

    def get(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            return self.c[y][x]
        return '.'

    def mask(self, draw):
        im = Image.new('L', (self.w, self.h), 0)
        draw(ImageDraw.Draw(im))
        return {(x, y) for y in range(self.h) for x in range(self.w) if im.getpixel((x, y))}

    def fill(self, m, ch, edge='K', shade=None):
        for (x, y) in m:
            self.p(x, y, ch)
        if shade:
            dark, light = shade
            xs = [x for x, _ in m] or [0]
            ys = [y for _, y in m] or [0]
            x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
            for (x, y) in m:
                fx = (x - x0) / max(1, x1 - x0)
                fy = (y - y0) / max(1, y1 - y0)
                if fx + fy > 1.35 and dark:
                    self.p(x, y, dark)
                elif fx + fy < 0.55 and light:
                    self.p(x, y, light)
        if edge:
            for (x, y) in m:
                if any((x + dx, y + dy) not in m for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                    self.p(x, y, edge)

    def ell(self, x0, y0, x1, y1, ch, edge='K', shade=None):
        self.fill(self.mask(lambda d: d.ellipse([x0, y0, x1, y1], fill=255)), ch, edge, shade)

    def rect(self, x0, y0, x1, y1, ch, edge='K', shade=None):
        self.fill(self.mask(lambda d: d.rectangle([x0, y0, x1, y1], fill=255)), ch, edge, shade)

    def rrect(self, x0, y0, x1, y1, r, ch, edge='K', shade=None):
        self.fill(self.mask(lambda d: d.rounded_rectangle([x0, y0, x1, y1], r, fill=255)), ch, edge, shade)

    def poly(self, pts, ch, edge='K', shade=None):
        self.fill(self.mask(lambda d: d.polygon(pts, fill=255)), ch, edge, shade)

    def line(self, x0, y0, x1, y1, ch):
        dx, dy = abs(x1 - x0), -abs(y1 - y0)
        sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
        e = dx + dy
        while True:
            self.p(x0, y0, ch)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * e
            if e2 >= dy:
                e += dy
                x0 += sx
            if e2 <= dx:
                e += dx
                y0 += sy

    def paste(self, x, y, rows):
        for j, row in enumerate(rows):
            for i, ch in enumerate(row):
                if ch not in '. ':
                    self.p(x + i, y + j, ch)

    def crop(self):
        pts = [(x, y) for y in range(self.h) for x in range(self.w) if self.c[y][x] != '.']
        if not pts:
            return (0, 0, 0, 0, [])
        x0 = min(x for x, _ in pts)
        x1 = max(x for x, _ in pts)
        y0 = min(y for _, y in pts)
        y1 = max(y for _, y in pts)
        rows = [''.join(self.c[y][x0:x1 + 1]) for y in range(y0, y1 + 1)]
        return (x1 - x0 + 1, y1 - y0 + 1, x0, y0, rows)


SLOTS = ['fond', 'cape', 'jambes', 'armure', 'casque', 'yeux', 'deco', 'arme', 'main', 'familier', 'aura']
ITEMS = []


def item(slot, iid, fr, en, price, base):
    def deco(fn):
        g = G()
        fn(g)
        ITEMS.append((slot, iid, fr, en, price, base, g.crop()))
        return fn
    return deco


def none_item(slot, iid='aucun', fr='Aucun', en='None'):
    ITEMS.append((slot, slot + '_' + iid, fr, en, 0, (0, 0, 0), (0, 0, 0, 0, [])))


none_item('fond')


@item('fond', 'fond_nuit', 'Nuit étoilée', 'Starry night', 60, (30, 30, 70))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'Q' if y < 20 else 'Z')
    for (x, y) in [(3, 3), (20, 5), (12, 2), (25, 12), (6, 14), (17, 9), (9, 22), (23, 19), (2, 27)]:
        g.p(x, y, 'h')
    g.ell(19, 2, 24, 7, 'h', edge=None)
    g.ell(21, 2, 25, 6, 'Q', edge=None)


@item('fond', 'fond_couchant', 'Coucher de soleil', 'Sunset', 80, (255, 120, 60))
def _(g):
    cols = ['m', 'm', 'r', 'r', 'f', 'o', 'o', 'a', 'F', 'y']
    for y in range(32):
        for x in range(28):
            g.p(x, y, cols[min(9, y // 3)] if y < 26 else 'z')
    g.ell(9, 17, 19, 27, 'y', edge=None)
    for y in range(26, 32):
        for x in range(28):
            g.p(x, y, 'z' if (x + y) % 5 else 'p')


@item('fond', 'fond_damier', 'Damier de cantine', 'Cafeteria checks', 50, (230, 230, 240))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'W' if ((x // 4) + (y // 4)) % 2 else 'r')


@item('fond', 'fond_terminal', 'Terminal', 'Terminal', 70, (0, 40, 10))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'D')
    for y in range(1, 32, 3):
        for x in range(1, 4 + (y * 7) % 20):
            g.p(x, y, 'v' if x % 3 else 'l')


@item('fond', 'fond_nuages', 'Nuages en coton', 'Cotton clouds', 60, (140, 200, 255))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'q')
    for (a, b, c, d) in [(1, 4, 11, 9), (14, 9, 26, 14), (3, 20, 15, 25), (17, 25, 27, 30)]:
        g.ell(a, b, c, d, 'W', edge='w')


@item('fond', 'fond_frigo', 'Intérieur de frigo', 'Inside a fridge', 90, (220, 245, 255))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'i')
    for y in (10, 21):
        g.line(0, y, 27, y, 'w')
    g.rect(2, 4, 6, 9, 'y')
    g.ell(20, 14, 25, 20, 'l')
    g.rect(3, 23, 9, 30, 'W', shade=('w', None))


@item('fond', 'fond_lave', 'Volcan en promo', 'Volcano on sale', 110, (120, 20, 10))
def _(g):
    for y in range(32):
        for x in range(28):
            g.p(x, y, 'R' if y < 22 else ('f' if (x + y) % 4 else 'F'))
    g.poly([(4, 22), (14, 6), (24, 22)], 'N')
    g.poly([(12, 9), (14, 6), (16, 9)], 'f', edge=None)


none_item('cape')


@item('cape', 'cape_classique', 'Cape de chevalier', 'Knight cape', 0, (200, 40, 60))
def _(g):
    g.poly([(9, 15), (18, 15), (22, 28), (5, 28)], '1', shade=('2', '3'))


@item('cape', 'cape_chaussette', 'Chaussette géante', 'Giant sock', 70, (90, 160, 240))
def _(g):
    g.poly([(9, 15), (18, 15), (19, 25), (23, 25), (23, 29), (8, 29)], '1')
    for y in (18, 22, 26):
        g.line(9, y, 19, y, '3')
    g.rect(19, 26, 23, 29, 'W')


@item('cape', 'cape_nappe', 'Nappe à carreaux', 'Checkered tablecloth', 60, (220, 50, 60))
def _(g):
    m = g.mask(lambda d: d.polygon([(8, 15), (19, 15), (22, 28), (5, 28)], fill=255))
    for (x, y) in m:
        g.p(x, y, '1' if ((x // 2) + (y // 2)) % 2 else 'W')
    g.fill(m, '.', edge='K')


@item('cape', 'cape_rideau', 'Rideau de douche', 'Shower curtain', 80, (120, 220, 230))
def _(g):
    for i, x in enumerate(range(5, 23)):
        top = 15 if 8 <= x <= 19 else 17
        g.line(x, top, x, 28 - (i % 2), '1' if i % 2 else '3')
    for x in range(6, 22, 3):
        g.p(x, 15, 'W')


@item('cape', 'cape_pirate', 'Drapeau pirate', 'Pirate flag', 120, (30, 30, 40))
def _(g):
    g.poly([(8, 15), (19, 15), (22, 28), (5, 28)], '1')
    g.ell(11, 19, 16, 23, 'W', edge=None)
    g.p(12, 21, 'k')
    g.p(15, 21, 'k')
    g.line(10, 25, 17, 25, 'W')


@item('cape', 'cape_serviette', 'Serviette de plage', 'Beach towel', 70, (255, 200, 60))
def _(g):
    g.poly([(8, 15), (19, 15), (21, 29), (6, 29)], 'W')
    for y in range(16, 29, 3):
        g.line(7, y, 20, y, '1')


@item('cape', 'cape_feuilles', 'Cape en feuilles', 'Leaf cape', 90, (90, 190, 80))
def _(g):
    for (x, y) in [(9, 16), (13, 16), (17, 16), (7, 20), (11, 20), (15, 20), (19, 20), (6, 24), (10, 24), (14, 24), (18, 24), (21, 25)]:
        g.ell(x - 1, y - 1, x + 2, y + 3, '1', edge='2')


@item('cape', 'cape_chauvesouris', 'Ailes de chauve-souris', 'Bat wings', 150, (90, 60, 120))
def _(g):
    g.poly([(10, 16), (1, 12), (3, 17), (0, 21), (5, 20), (6, 24), (10, 20)], '1', shade=('2', None))
    g.poly([(17, 16), (26, 12), (24, 17), (27, 21), (22, 20), (21, 24), (17, 20)], '1', shade=('2', None))


@item('jambes', 'jambes_jambieres', 'Jambières', 'Greaves', 0, (120, 130, 150))
def _(g):
    g.rect(10, 23, 12, 27, '1', shade=('2', '3'))
    g.rect(15, 23, 17, 27, '1', shade=('2', '3'))
    g.rect(9, 28, 12, 29, 'D')
    g.rect(15, 28, 18, 29, 'D')


@item('jambes', 'jambes_short', 'Short à fleurs', 'Flower shorts', 50, (255, 120, 180))
def _(g):
    g.rect(9, 22, 18, 25, '1')
    g.p(11, 23, 'y')
    g.p(16, 24, 'W')
    g.p(13, 24, 'y')
    g.rect(10, 26, 11, 28, 's', edge=None)
    g.rect(16, 26, 17, 28, 's', edge=None)
    g.rect(9, 29, 12, 29, 'K')
    g.rect(15, 29, 18, 29, 'K')


@item('jambes', 'jambes_tutu', 'Tutu', 'Tutu', 80, (255, 160, 210))
def _(g):
    g.poly([(10, 22), (17, 22), (22, 26), (5, 26)], '1', edge='2')
    for x in range(6, 22, 2):
        g.p(x, 25, '3')
    g.rect(10, 27, 11, 29, 'W', edge=None)
    g.rect(16, 27, 17, 29, 'W', edge=None)


@item('jambes', 'jambes_pattes', "Pattes d'eph'", 'Bell-bottoms', 60, (90, 120, 220))
def _(g):
    g.poly([(10, 22), (13, 22), (13, 29), (8, 29)], '1')
    g.poly([(14, 22), (17, 22), (19, 29), (14, 29)], '1')


@item('jambes', 'jambes_palmes', 'Palmes', 'Flippers', 70, (60, 200, 120))
def _(g):
    g.rect(10, 23, 12, 26, 'G')
    g.rect(15, 23, 17, 26, 'G')
    g.poly([(10, 27), (12, 27), (13, 30), (6, 30)], '1')
    g.poly([(15, 27), (17, 27), (21, 30), (14, 30)], '1')


@item('jambes', 'jambes_chaussettes', 'Chaussettes dépareillées', 'Odd socks', 40, (255, 210, 60))
def _(g):
    g.rect(10, 23, 12, 28, '1')
    g.rect(15, 23, 17, 28, 'm')
    for y in (24, 26):
        g.line(10, y, 12, y, '3')
        g.line(15, y, 17, y, 'P')


@item('jambes', 'jambes_bottes', 'Bottes en caoutchouc', 'Rubber boots', 60, (250, 220, 40))
def _(g):
    g.rect(10, 22, 12, 24, 'G')
    g.rect(15, 22, 17, 24, 'G')
    g.rect(9, 25, 12, 29, '1', shade=('2', '3'))
    g.rect(15, 25, 18, 29, '1', shade=('2', '3'))


@item('jambes', 'jambes_rollers', 'Rollers', 'Roller skates', 110, (230, 60, 160))
def _(g):
    g.rect(10, 22, 12, 26, 'G')
    g.rect(15, 22, 17, 26, 'G')
    g.rect(9, 26, 13, 28, '1')
    g.rect(14, 26, 18, 28, '1')
    for x in (9, 12, 14, 17):
        g.p(x, 30, 'y')
        g.p(x + (1 if x in (9, 14) else 0), 29, 'K')


@item('armure', 'armure_cotte', 'Cotte de mailles', 'Chainmail', 0, (110, 130, 170))
def _(g):
    g.rrect(9, 15, 18, 22, 2, '1', shade=('2', '3'))
    g.line(10, 20, 17, 20, 'n')
    g.p(13, 20, 'y')
    g.ell(6, 16, 9, 20, '1')
    g.ell(18, 16, 21, 20, '1')


@item('armure', 'armure_noel', 'Pull de Noël', 'Xmas sweater', 80, (200, 40, 50))
def _(g):
    g.rrect(9, 15, 18, 22, 2, '1')
    for x in range(10, 18, 2):
        g.p(x, 17, 'W')
        g.p(x + 1, 19, 'v')
    g.p(13, 21, 'y')
    g.ell(6, 16, 9, 20, '1')
    g.ell(18, 16, 21, 20, '1')


@item('armure', 'armure_tablier', 'Tablier de cuisine', 'Kitchen apron', 50, (255, 255, 255))
def _(g):
    g.rrect(9, 15, 18, 22, 2, 'b')
    g.rect(10, 16, 17, 22, '1')
    g.p(12, 19, 'r')
    g.p(15, 19, 'r')
    g.ell(6, 16, 9, 20, 'b')
    g.ell(18, 16, 21, 20, 'b')


@item('armure', 'armure_gilet', 'Gilet de sauvetage', 'Life vest', 70, (255, 120, 30))
def _(g):
    g.rrect(8, 15, 19, 23, 3, '1', shade=('2', '3'))
    g.line(13, 15, 13, 23, 'K')
    g.line(9, 18, 18, 18, 'W')
    g.ell(6, 16, 9, 20, 'G')
    g.ell(18, 16, 21, 20, 'G')


@item('armure', 'armure_smoking', 'Smoking', 'Tuxedo', 120, (30, 30, 40))
def _(g):
    g.rrect(9, 15, 18, 22, 2, '1')
    g.poly([(12, 15), (15, 15), (14, 21), (13, 21)], 'W', edge=None)
    g.p(13, 16, 'r')
    g.p(14, 16, 'r')
    g.ell(6, 16, 9, 20, '1')
    g.ell(18, 16, 21, 20, '1')


@item('armure', 'armure_peignoir', 'Peignoir', 'Bathrobe', 60, (255, 180, 210))
def _(g):
    g.rrect(8, 15, 19, 23, 3, '1', shade=('2', '3'))
    g.line(10, 15, 15, 23, '2')
    g.line(9, 20, 18, 20, 'W')
    g.ell(6, 16, 9, 21, '1')
    g.ell(18, 16, 21, 21, '1')


@item('armure', 'armure_carton', 'Armure en carton', 'Cardboard armor', 40, (190, 140, 80))
def _(g):
    g.rect(8, 15, 19, 22, '1', shade=('2', None))
    g.p(10, 17, 'K')
    g.p(11, 18, 'K')
    g.line(15, 19, 17, 17, 'K')
    g.rect(5, 16, 8, 19, '1')
    g.rect(19, 16, 22, 19, '1')


@item('armure', 'armure_foot', 'Maillot de foot', 'Football jersey', 70, (40, 90, 220))
def _(g):
    g.rrect(9, 15, 18, 22, 1, '1')
    g.rect(12, 17, 15, 20, 'W', edge=None)
    g.p(13, 18, '1')
    g.p(14, 19, '1')
    g.ell(6, 16, 9, 19, '1')
    g.ell(18, 16, 21, 19, '1')


@item('armure', 'armure_poubelle', 'Sac poubelle', 'Trash bag', 30, (40, 40, 50))
def _(g):
    g.poly([(10, 14), (17, 14), (20, 23), (7, 23)], '1', shade=(None, '3'))
    g.p(13, 13, 'y')
    g.p(14, 13, 'y')


@item('armure', 'armure_kimono', 'Kimono', 'Kimono', 100, (240, 240, 245))
def _(g):
    g.rrect(9, 15, 18, 22, 1, '1')
    g.line(10, 15, 14, 20, 'K')
    g.line(17, 15, 13, 20, 'K')
    g.line(9, 20, 18, 20, 'r')
    g.line(9, 21, 18, 21, 'r')
    g.poly([(9, 16), (4, 21), (8, 21)], '1')
    g.poly([(18, 16), (23, 21), (19, 21)], '1')


@item('casque', 'casque_rond', 'Casque rond', 'Round helmet', 0, (214, 218, 230))
def _(g):
    g.ell(7, 3, 20, 15, '1', shade=('2', '3'))
    g.rect(9, 8, 18, 12, 'k', edge=None)
    g.line(9, 8, 18, 8, 'K')


@item('casque', 'casque_seau', 'Seau', 'Bucket', 40, (90, 150, 230))
def _(g):
    g.poly([(7, 4), (20, 4), (19, 15), (8, 15)], '1', shade=('2', '3'))
    g.line(7, 3, 20, 3, 'G')
    g.rect(9, 8, 18, 11, 'k', edge=None)
    g.line(6, 5, 4, 9, 'G')
    g.line(21, 5, 23, 9, 'G')


@item('casque', 'casque_casserole', 'Casserole', 'Saucepan', 60, (170, 175, 190))
def _(g):
    g.rrect(7, 5, 20, 15, 2, '1', shade=('2', '3'))
    g.rect(20, 6, 26, 7, 'N', edge=None)
    g.rect(9, 9, 18, 12, 'k', edge=None)


@item('casque', 'casque_citrouille', 'Citrouille', 'Pumpkin', 90, (250, 140, 30))
def _(g):
    g.ell(6, 4, 21, 16, '1', shade=('2', '3'))
    g.line(13, 5, 13, 15, '2')
    g.rect(13, 1, 14, 4, 'v', edge=None)
    g.poly([(9, 8), (12, 8), (10, 11)], 'k', edge=None)
    g.poly([(15, 8), (18, 8), (17, 11)], 'k', edge=None)
    g.line(10, 13, 17, 13, 'k')


@item('casque', 'casque_bocal', 'Bocal à cornichons', 'Pickle jar', 100, (190, 240, 220))
def _(g):
    g.rrect(7, 4, 20, 16, 3, '1', edge='w')
    g.rect(8, 2, 19, 4, 'y')
    g.rect(9, 9, 18, 12, 'k', edge=None)
    g.ell(9, 13, 11, 15, 'v', edge=None)
    g.ell(16, 5, 18, 7, 'v', edge=None)


@item('casque', 'casque_ampoule', 'Ampoule', 'Light bulb', 120, (255, 250, 170))
def _(g):
    g.ell(6, 1, 21, 15, '1', shade=(None, 'W'))
    g.rect(10, 14, 17, 16, 'g')
    g.rect(9, 7, 18, 10, 'k', edge=None)


@item('casque', 'casque_cone', 'Cône de chantier', 'Traffic cone', 70, (255, 120, 30))
def _(g):
    g.poly([(13, 0), (14, 0), (21, 15), (6, 15)], '1')
    g.line(10, 6, 17, 6, 'W')
    g.line(9, 12, 18, 12, 'W')
    g.rect(10, 8, 17, 10, 'k', edge=None)


@item('casque', 'casque_pot', 'Pot de fleurs', 'Flower pot', 60, (200, 110, 70))
def _(g):
    g.poly([(7, 5), (20, 5), (18, 15), (9, 15)], '1', shade=('2', None))
    g.rect(6, 3, 21, 5, '1')
    g.rect(10, 8, 17, 11, 'k', edge=None)
    g.line(13, 2, 13, 0, 'v')
    g.p(12, 0, 'l')
    g.p(14, 1, 'l')


@item('casque', 'casque_conserve', 'Boîte de conserve', 'Tin can', 50, (200, 205, 215))
def _(g):
    g.rect(7, 3, 20, 15, '1', shade=('2', '3'))
    g.rect(7, 6, 20, 12, 'r', edge=None)
    g.rect(9, 8, 18, 10, 'k', edge=None)
    g.line(7, 3, 20, 3, 'w')


@item('casque', 'casque_cloche', 'Cloche de plat', 'Food cloche', 140, (230, 230, 240))
def _(g):
    g.fill(g.mask(lambda d: d.pieslice([5, 3, 22, 22], 180, 360, fill=255)), '1', shade=('2', 'W'))
    g.line(5, 13, 22, 13, 'g')
    g.ell(12, 1, 15, 4, '1')
    g.rect(9, 8, 18, 11, 'k', edge=None)
    g.rect(8, 13, 19, 15, 'k', edge=None)


@item('yeux', 'yeux_points', 'Points blancs', 'White dots', 0, (255, 255, 255))
def _(g):
    g.rect(10, 9, 11, 10, 'E', edge=None)
    g.rect(16, 9, 17, 10, 'E', edge=None)


@item('yeux', 'yeux_fente', 'Fente', 'Slit', 30, (255, 220, 80))
def _(g):
    g.line(10, 10, 17, 10, 'E')


@item('yeux', 'yeux_ko', 'Croix (KO)', 'Crosses (KO)', 40, (255, 90, 90))
def _(g):
    for cx in (11, 16):
        for d in (-1, 0, 1):
            g.p(cx + d, 10 + d, 'E')
            g.p(cx + d, 10 - d, 'E')


@item('yeux', 'yeux_coeurs', 'Cœurs', 'Hearts', 60, (255, 90, 160))
def _(g):
    for cx in (10, 15):
        g.paste(cx, 9, ['E.E', 'EEE', '.E.'])


@item('yeux', 'yeux_etoiles', 'Étoiles', 'Stars', 60, (255, 230, 80))
def _(g):
    for cx in (10, 15):
        g.paste(cx, 9, ['.E.', 'EEE', '.E.'])


@item('yeux', 'yeux_3d', 'Lunettes 3D', '3D glasses', 80, (255, 255, 255))
def _(g):
    g.rect(9, 9, 12, 11, 'r', edge=None)
    g.rect(15, 9, 18, 11, 'c', edge=None)
    g.line(13, 9, 14, 9, 'W')


@item('yeux', 'yeux_cyclope', 'Un seul œil', 'One eye', 70, (120, 255, 200))
def _(g):
    g.ell(11, 8, 16, 12, 'E', edge=None)
    g.rect(13, 9, 14, 11, 'k', edge=None)


@item('yeux', 'yeux_dodo', 'Endormis', 'Sleepy', 30, (200, 200, 255))
def _(g):
    g.line(9, 10, 12, 10, 'E')
    g.line(15, 10, 18, 10, 'E')
    g.paste(19, 5, ['EEE', '.E.', 'EEE'])


@item('yeux', 'yeux_dollars', 'Dollars', 'Dollars', 90, (120, 255, 120))
def _(g):
    for cx in (9, 15):
        g.paste(cx, 8, ['.E.', 'EE.', '.EE', 'EE.'])


@item('yeux', 'yeux_laser', 'Laser', 'Laser', 150, (255, 40, 40))
def _(g):
    g.rect(9, 9, 18, 10, 'E', edge=None)
    g.line(18, 10, 27, 12, 'E')


none_item('deco')


@item('deco', 'deco_fleur', 'Fleur', 'Flower', 30, (255, 120, 180))
def _(g):
    g.line(13, 3, 13, 1, 'v')
    g.paste(11, -2 + 2, ['.1.1.', '11y11', '.111.'])


@item('deco', 'deco_etiquette', 'Étiquette -50%', '-50% tag', 40, (255, 230, 90))
def _(g):
    g.line(14, 4, 16, 2, 'K')
    g.rect(16, 0, 22, 3, '1')
    g.p(18, 1, 'r')
    g.p(20, 2, 'r')
    g.p(19, 1, 'K')


@item('deco', 'deco_helice', 'Hélice', 'Propeller', 60, (80, 160, 255))
def _(g):
    g.line(13, 3, 13, 2, 'G')
    g.line(8, 1, 12, 1, '1')
    g.line(14, 1, 18, 1, 'r')
    g.p(13, 1, 'K')


@item('deco', 'deco_bougie', "Bougie d'anniversaire", 'Birthday candle', 50, (120, 200, 255))
def _(g):
    g.rect(13, 0, 14, 3, '1', edge=None)
    g.p(13, 1, 'W')
    g.p(14, 2, 'W')
    g.p(13, -1 + 1, 'F')


@item('deco', 'deco_antenne', 'Antenne', 'Antenna', 40, (255, 80, 80))
def _(g):
    g.line(13, 3, 15, 0, 'G')
    g.ell(14, -1, 17, 2, '1')


@item('deco', 'deco_plume', 'Plume', 'Feather', 50, (255, 230, 120))
def _(g):
    g.poly([(15, 4), (19, 0), (21, 0), (16, 4)], '1', edge='2')


@item('deco', 'deco_glace', 'Cornet de glace', 'Ice cream cone', 80, (255, 170, 210))
def _(g):
    g.poly([(12, 3), (15, 3), (13, 6)], 'a', edge='A')
    g.ell(11, 0, 16, 3, '1')


@item('deco', 'deco_canard', 'Canard en plastique', 'Rubber duck', 90, (255, 220, 40))
def _(g):
    g.ell(11, 1, 18, 4, '1', shade=('2', None))
    g.ell(10, -1, 14, 2, '1')
    g.p(9, 1, 'o')
    g.p(11, 0, 'k')


@item('deco', 'deco_crete', 'Crête de punk', 'Punk mohawk', 70, (255, 60, 160))
def _(g):
    for i, x in enumerate(range(10, 18)):
        g.line(x, 4, x, 1 - (i % 2), '1' if i % 2 else '3')


@item('deco', 'deco_couronne', 'Couronne en carton', 'Cardboard crown', 100, (255, 210, 60))
def _(g):
    g.poly([(9, 4), (9, 0), (11, 2), (13, 0), (15, 2), (17, 0), (18, 0), (18, 4)], '1')
    g.p(13, 3, 'r')


@item('deco', 'deco_ventouse', 'Ventouse', 'Plunger', 60, (230, 50, 60))
def _(g):
    g.ell(10, 2, 17, 5, '1')
    g.line(13, 1, 13, -5, 'n')
    g.line(14, 1, 14, -5, 'n')


@item('deco', 'deco_champignon', 'Champignon', 'Mushroom', 70, (230, 50, 60))
def _(g):
    g.ell(9, 0, 18, 3, '1')
    g.p(11, 1, 'W')
    g.p(15, 1, 'W')
    g.rect(12, 3, 15, 4, 'x', edge=None)


@item('deco', 'deco_noeud', 'Nœud géant', 'Giant bow', 50, (255, 100, 180))
def _(g):
    g.poly([(13, 2), (8, 0), (8, 4)], '1')
    g.poly([(14, 2), (19, 0), (19, 4)], '1')
    g.rect(13, 1, 14, 3, '2', edge=None)


@item('deco', 'deco_wip', 'Panneau WIP', 'WIP sign', 80, (255, 220, 40))
def _(g):
    g.line(13, 4, 13, 2, 'G')
    g.rect(9, -3 + 3, 18, 3, '1')
    g.line(10, 1, 17, 1, 'K')


@item('deco', 'deco_arcenciel', 'Arc-en-ciel', 'Rainbow', 160, (255, 255, 255))
def _(g):
    for i, ch in enumerate('royvbp'):
        g.fill(g.mask(lambda d, i=i: d.arc([4 + i, 0 + i, 23 - i, 12 - i], 180, 360, fill=255)), ch, edge=None)


none_item('arme')


@item('arme', 'arme_epee', 'Épée', 'Sword', 0, (220, 225, 240))
def _(g):
    g.poly([(21, 5), (22, 3), (23, 3), (24, 5), (24, 17), (21, 17)], '1', edge='K')
    g.line(22, 5, 22, 16, '3')
    g.line(23, 5, 23, 16, '1')
    g.rect(20, 18, 25, 18, 'Y', edge=None)
    g.rect(22, 19, 23, 21, 'n', edge=None)


@item('arme', 'arme_baguette', 'Baguette de pain', 'Baguette', 30, (220, 160, 80))
def _(g):
    g.rrect(21, 3, 24, 22, 2, '1', shade=('2', None))
    for y in (6, 10, 14, 18):
        g.line(22, y, 23, y + 1, 'x')


@item('arme', 'arme_poisson', 'Poisson', 'Fish', 60, (120, 180, 240))
def _(g):
    g.ell(20, 6, 25, 19, '1', shade=('2', '3'))
    g.poly([(22, 19), (20, 23), (25, 23)], '1')
    g.p(23, 8, 'k')


@item('arme', 'arme_fourchette', 'Fourchette géante', 'Giant fork', 50, (210, 215, 230))
def _(g):
    for x in (20, 22, 24):
        g.line(x, 3, x, 8, '1')
    g.rect(20, 8, 24, 10, '1', edge=None)
    g.rect(22, 10, 22, 22, '1', edge=None)


@item('arme', 'arme_parapluie', 'Parapluie', 'Umbrella', 80, (230, 60, 120))
def _(g):
    g.fill(g.mask(lambda d: d.pieslice([17, 1, 27, 11], 180, 360, fill=255)), '1', edge='K')
    g.line(22, 6, 22, 21, 'G')
    g.p(21, 21, 'G')


@item('arme', 'arme_ventouse', 'Ventouse de combat', 'Battle plunger', 40, (230, 50, 60))
def _(g):
    g.ell(19, 3, 26, 7, '1')
    g.rect(22, 7, 23, 21, 'n', edge=None)


@item('arme', 'arme_sucette', 'Sucette géante', 'Giant lollipop', 70, (255, 120, 200))
def _(g):
    g.ell(18, 2, 27, 11, '1')
    g.line(19, 7, 26, 5, 'W')
    g.line(20, 9, 25, 4, '3')
    g.rect(22, 11, 23, 21, 'W', edge=None)


@item('arme', 'arme_usb', 'Clé USB géante', 'Giant USB stick', 90, (60, 60, 70))
def _(g):
    g.rect(20, 3, 25, 6, 'w')
    g.rect(21, 4, 21, 4, 'k', edge=None)
    g.rect(24, 4, 24, 4, 'k', edge=None)
    g.rect(19, 7, 26, 20, '1', shade=(None, '3'))
    g.p(22, 12, 'b')


@item('arme', 'arme_balai', 'Balai', 'Broom', 50, (200, 150, 60))
def _(g):
    g.rect(22, 2, 23, 17, 'n', edge=None)
    g.poly([(20, 17), (25, 17), (27, 24), (18, 24)], '1', edge='2')


@item('arme', 'arme_plume', "Plume d'écrivain", 'Quill', 60, (255, 255, 255))
def _(g):
    g.poly([(25, 2), (27, 3), (23, 18), (21, 18)], '1', edge='w')
    g.line(22, 18, 21, 22, 'k')


@item('arme', 'arme_spatule', 'Spatule', 'Spatula', 40, (240, 100, 60))
def _(g):
    g.rect(20, 3, 25, 9, '1')
    g.line(21, 5, 24, 5, '2')
    g.line(21, 7, 24, 7, '2')
    g.rect(22, 10, 23, 21, 'k', edge=None)


@item('arme', 'arme_guitare', 'Guitare', 'Guitar', 120, (200, 60, 50))
def _(g):
    g.rect(22, 1, 23, 13, 'N', edge=None)
    g.ell(19, 13, 26, 18, '1')
    g.ell(18, 17, 27, 23, '1', shade=('2', None))
    g.p(22, 19, 'k')


none_item('main')


@item('main', 'main_bouclier', 'Bouclier', 'Shield', 0, (200, 60, 70))
def _(g):
    g.poly([(1, 15), (8, 15), (8, 21), (4, 25), (1, 21)], '1', shade=('2', '3'))
    g.line(4, 16, 4, 23, 'y')
    g.line(2, 19, 7, 19, 'y')


@item('main', 'main_poubelle', 'Couvercle de poubelle', 'Trash can lid', 30, (150, 155, 170))
def _(g):
    g.ell(0, 14, 8, 24, '1', shade=('2', '3'))
    g.ell(3, 18, 5, 20, 'G')


@item('main', 'main_pizza', 'Part de pizza', 'Pizza slice', 60, (255, 200, 80))
def _(g):
    g.poly([(1, 14), (8, 16), (3, 25)], '1', edge='A')
    g.line(1, 14, 8, 16, 'n')
    g.p(4, 17, 'r')
    g.p(5, 20, 'r')
    g.p(3, 19, 'v')


@item('main', 'main_poele', 'Poêle', 'Frying pan', 50, (60, 60, 70))
def _(g):
    g.ell(0, 13, 7, 20, '1', shade=(None, '3'))
    g.ell(2, 15, 4, 17, 'y')
    g.p(3, 16, 'F')
    g.line(5, 20, 7, 23, 'n')


@item('main', 'main_livre', 'Livre « man 3 »', '"man 3" book', 70, (60, 100, 200))
def _(g):
    g.rect(1, 15, 7, 23, '1', shade=('2', None))
    g.line(2, 15, 2, 23, '3')
    g.line(4, 18, 6, 18, 'W')
    g.line(4, 20, 6, 20, 'W')


@item('main', 'main_ballon', 'Ballon de baudruche', 'Balloon', 40, (255, 70, 90))
def _(g):
    g.ell(0, 3, 6, 10, '1', shade=('2', '3'))
    g.line(3, 11, 6, 18, 'W')


@item('main', 'main_lanterne', 'Lanterne', 'Lantern', 90, (255, 220, 120))
def _(g):
    g.rect(1, 17, 6, 24, 'G')
    g.rect(2, 18, 5, 23, '1', edge=None)
    g.p(3, 20, 'W')
    g.line(3, 16, 4, 16, 'G')


@item('main', 'main_cafe', 'Tasse de café', 'Coffee mug', 40, (255, 255, 255))
def _(g):
    g.rect(1, 17, 6, 23, '1', shade=('2', None))
    g.rect(2, 17, 5, 17, 'N', edge=None)
    g.ell(6, 18, 8, 21, '1')
    g.p(3, 15, 'w')
    g.p(4, 14, 'w')


@item('main', 'main_postit', 'Post-it', 'Sticky note', 20, (255, 240, 100))
def _(g):
    g.rect(1, 16, 7, 22, '1', edge='2')
    g.line(2, 18, 6, 18, 'K')
    g.line(2, 20, 5, 20, 'K')


@item('main', 'main_bouquet', 'Bouquet de brocolis', 'Broccoli bouquet', 80, (80, 180, 80))
def _(g):
    g.rect(3, 19, 4, 24, 'l', edge=None)
    for (x, y) in [(0, 14), (3, 13), (5, 15), (1, 17)]:
        g.ell(x, y, x + 3, y + 3, '1', edge='2')


none_item('familier')


@item('familier', 'fam_pointeur', 'Pointeur', 'Pointer', 60, (255, 255, 255))
def _(g):
    g.paste(21, 24, ['1....', '11...', '111..', '1111.', '11111', '..11.', '...11'])


@item('familier', 'fam_canard', 'Canard', 'Duck', 50, (255, 220, 40))
def _(g):
    g.ell(21, 27, 27, 31, '1', shade=('2', None))
    g.ell(22, 24, 25, 27, '1')
    g.p(26, 25, 'o')
    g.p(24, 25, 'k')


@item('familier', 'fam_escargot', 'Escargot', 'Snail', 50, (200, 140, 220))
def _(g):
    g.ell(22, 25, 27, 30, '1')
    g.p(24, 27, '2')
    g.p(25, 28, '2')
    g.rect(19, 29, 26, 31, 'x', edge='a')
    g.p(19, 27, 'K')
    g.p(20, 28, 'x')


@item('familier', 'fam_slime', 'Slime', 'Slime', 70, (90, 230, 140))
def _(g):
    g.poly([(21, 31), (21, 28), (24, 25), (27, 28), (27, 31)], '1', shade=('2', '3'))
    g.p(23, 28, 'k')
    g.p(25, 28, 'k')


@item('familier', 'fam_chat', 'Chat', 'Cat', 90, (240, 150, 60))
def _(g):
    g.ell(21, 26, 27, 31, '1')
    g.poly([(22, 26), (22, 24), (24, 26)], '1')
    g.poly([(24, 26), (26, 24), (26, 26)], '1')
    g.p(23, 28, 'k')
    g.p(25, 28, 'k')


@item('familier', 'fam_bug', 'Bug (segfault)', 'Bug (segfault)', 80, (60, 200, 80))
def _(g):
    g.ell(22, 26, 27, 31, '1', shade=('2', None))
    for y in (27, 29, 31):
        g.p(21, y, 'K')
        g.p(27, y, 'K')
    g.line(23, 25, 22, 24, 'K')
    g.line(26, 25, 27, 24, 'K')


@item('familier', 'fam_poisson', 'Poisson rouge en bocal', 'Goldfish bowl', 100, (180, 230, 255))
def _(g):
    g.ell(20, 24, 27, 31, '1', edge='w')
    g.ell(22, 27, 24, 29, 'o', edge=None)
    g.p(25, 28, 'o')


@item('familier', 'fam_champignon', 'Champignon qui marche', 'Walking mushroom', 60, (230, 60, 70))
def _(g):
    g.ell(20, 24, 27, 28, '1')
    g.p(22, 25, 'W')
    g.p(25, 26, 'W')
    g.rect(22, 28, 25, 30, 'x', edge='a')
    g.p(22, 31, 'K')
    g.p(25, 31, 'K')


@item('familier', 'fam_miniboss', 'Mini-boss', 'Mini boss', 150, (120, 220, 120))
def _(g):
    g.rect(21, 25, 27, 31, '1')
    g.p(23, 27, 'r')
    g.p(25, 27, 'r')
    g.line(22, 30, 26, 30, 'y')


@item('familier', 'fam_nuage', 'Nuage de pluie', 'Rain cloud', 70, (200, 205, 220))
def _(g):
    g.ell(20, 22, 27, 26, '1', edge='g')
    for x in (21, 23, 25):
        g.p(x, 28, 'q')
        g.p(x + 1, 30, 'q')


AURAS = [('aura_aucune', 'Aucune', 'None', 0), ('aura_etincelles', 'Étincelles', 'Sparks', 80),
         ('aura_flammes', 'Flammes', 'Flames', 120), ('aura_bulles', 'Bulles', 'Bubbles', 60),
         ('aura_notes', 'Notes de musique', 'Music notes', 90), ('aura_coeurs', 'Cœurs', 'Hearts', 70),
         ('aura_binaire', 'Zéros et uns', 'Zeros and ones', 100), ('aura_flocons', 'Flocons', 'Snowflakes', 80)]
for iid, fr, en, price in AURAS:
    ITEMS.append(('aura', iid, fr, en, price, (255, 255, 255), (0, 0, 0, 0, [])))


def boss_gardien():
    g = G(40, 36)
    g.rrect(12, 15, 27, 31, 2, 'v', shade=('V', 'l'))
    for y in range(17, 30, 3):
        for x in range(14, 26, 3):
            g.rect(x, y, x + 1, y + 1, 'K' if (x + y) % 2 else 'D', edge=None)
    for y in range(16, 31, 2):
        g.p(11, y, 'y')
        g.p(28, y, 'y')
    g.ell(8, 2, 31, 17, 'W', shade=('w', None))
    g.ell(9, -3, 30, 7, 'P', shade=('m', None))
    for (x, y) in [(12, 2), (17, 0), (23, 3), (27, 1), (20, 4)]:
        g.p(x, y, 'W')
    g.ell(12, 7, 18, 13, 'W', edge='K')
    g.ell(21, 7, 27, 13, 'W', edge='K')
    g.rect(15, 10, 16, 11, 'E', edge=None)
    g.rect(22, 9, 23, 10, 'E', edge=None)
    g.line(15, 15, 24, 15, 'K')
    for x in range(16, 24, 2):
        g.p(x, 16, 'y')
    g.ell(1, 19, 11, 24, 'v', shade=('V', None))
    g.ell(28, 19, 38, 24, 'v', shade=('V', None))
    g.ell(0, 23, 5, 28, 'y', shade=('Y', None))
    g.p(0, 24, 'o')
    g.p(2, 24, 'k')
    g.rect(14, 32, 16, 34, 'D', edge=None)
    g.rect(23, 32, 25, 34, 'D', edge=None)
    g.rect(12, 35, 17, 35, 'm', edge=None)
    g.rect(22, 35, 27, 35, 'm', edge=None)
    g.poly([(10, 31), (29, 31), (33, 34), (6, 34)], 'P', edge='m')
    return g.crop()


def boss_alchimiste():
    g = G(40, 36)
    g.poly([(20, 0), (27, 11), (13, 11)], 'p', shade=('z', 'j'))
    g.line(19, 0, 21, 0, 'K')
    g.line(16, -1, 24, -1, 'K')
    g.line(14, 0, 18, 0, 'c')
    g.line(22, 0, 26, 0, 'r')
    for (x, y, ch) in [(18, 5, 'y'), (21, 7, 'W'), (16, 9, 'c'), (23, 9, 'y')]:
        g.p(x, y, ch)
    g.ell(8, 10, 32, 13, 'p', shade=('z', None))
    g.poly([(20, 12), (33, 34), (7, 34)], 'z', shade=('Z', 'p'))
    g.ell(13, 12, 27, 22, 'Z', edge='K')
    for cx in (17, 23):
        g.paste(cx - 2, 15, ['EEEE.', 'E..E.', 'E.EE.', 'E....', 'EEEEE'])
    g.ell(7, 22, 33, 28, 'y', shade=('Y', 'h'))
    g.ell(12, 23, 28, 27, 'z', edge=None)
    g.ell(29, 17, 35, 22, 'y')
    g.p(35, 19, 'o')
    g.p(32, 18, 'k')
    g.rrect(0, 14, 6, 25, 2, 'i', edge='w')
    g.rect(1, 18, 5, 24, 'l', edge=None)
    g.rect(2, 11, 4, 14, 'n')
    g.p(2, 20, 'W')
    g.poly([(9, 34), (2, 35), (14, 35)], 'm')
    g.poly([(31, 34), (38, 35), (26, 35)], 'm')
    return g.crop()


def boss_hydre():
    g = G(40, 36)
    g.rrect(3, 24, 36, 32, 4, 'W', shade=('w', None))
    g.rect(2, 23, 37, 24, 'w', edge='K')
    for x in (6, 33):
        g.rect(x, 33, x + 2, 35, 'Y', edge=None)
    for (x0, y0, x1, y1) in [(9, 24, 6, 12), (20, 24, 20, 9), (31, 24, 34, 13)]:
        n = 5
        for i in range(n):
            t = i / n
            x = round(x0 + (x1 - x0) * t)
            y = round(y0 + (y1 - y0) * t)
            g.rect(x - 1, y - 1, x + 1, y + 1, 'g' if i % 2 else 'G', edge='K')
        g.p(x1, y1 + 2, 'c')
    heads = [(6, 10, 'r'), (20, 6, 'v'), (34, 11, 'b')]
    for (cx, cy, col) in heads:
        g.ell(cx - 5, cy - 3, cx + 5, cy + 4, col, shade=({'r': 'R', 'v': 'V', 'b': 'B'}[col], None))
        g.p(cx - 2, cy, 'W')
        g.p(cx + 2, cy, 'W')
        g.p(cx - 2, cy, 'E')
        g.p(cx + 2, cy, 'E')
        g.line(cx - 2, cy + 3, cx + 2, cy + 3, 'k')
        g.p(cx, cy + 4, 'P')
    g.poly([(6, 2), (3, 7), (9, 7)], 'y', edge='K')
    g.p(6, 1, 'm')
    g.rrect(16, -2, 24, 3, 2, 'W', edge='w')
    g.rect(17, 2, 23, 3, 'W', edge='K')
    g.poly([(34, 2), (30, 8), (38, 8)], 'o', edge='K')
    g.line(32, 5, 36, 5, 'W')
    for (x, y) in [(12, 26), (18, 28), (27, 26), (24, 29), (8, 29)]:
        g.ell(x, y, x + 2, y + 2, 'q', edge=None)
    return g.crop()


BOSSES = [boss_gardien(), boss_alchimiste(), boss_hydre()]


def cstr(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'


def spr_c(name, spr):
    w, h, ox, oy, rows = spr
    if not rows:
        return '', (0, 0, 0, 0, 'NULL')
    body = ',\n\t'.join(cstr(r) for r in rows)
    return f'static const char *const {name}[] = {{\n\t{body}\n}};\n', (w, h, ox, oy, name)


def write_c(path):
    out = ['#include <stddef.h>', '#include "fous.h"', '']
    keys = ''.join(PAL.keys())
    out.append('const char\tg_pal_keys[] = ' + cstr(keys) + ';')
    out.append('const t_rgb\tg_pal_vals[] = {')
    out.append(',\n'.join('\t{%d, %d, %d}' % PAL[k] for k in PAL))
    out.append('};')
    out.append('')
    defs = []
    for i, spr in enumerate(BOSSES):
        code, d = spr_c(f'g_boss{i}', spr)
        out.append(code)
        defs.append(d)
    out.append('const t_spr\tg_boss_spr[3] = {')
    out.append(',\n'.join('\t{%d, %d, %d, %d, %s}' % d for d in defs))
    out.append('};')
    out.append('')
    idefs = []
    for i, (slot, iid, fr, en, price, base, spr) in enumerate(ITEMS):
        code, d = spr_c(f'g_it{i}', spr)
        out.append(code)
        idefs.append((slot, iid, fr, en, price, base, d))
    out.append('const t_idef\tg_items[] = {')
    lines = []
    for (slot, iid, fr, en, price, base, d) in idefs:
        lines.append('\t{%s, %s, %s, %d, %d, {%d, %d, %d}, {%d, %d, %d, %d, %s}}' % (
            cstr(iid), cstr(fr), cstr(en), SLOTS.index(slot), price, base[0], base[1], base[2],
            d[0], d[1], d[2], d[3], d[4] if d[0] else 'NULL'))
    out.append(',\n'.join(lines))
    out.append('};')
    out.append('const int\tg_nitems = %d;' % len(ITEMS))
    open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(out) + '\n')


def preview(path):
    sc = 6
    variants = [(214, 218, 230), (230, 60, 70), (90, 200, 120), (90, 140, 240), (250, 200, 60)]
    defaults = {'cape': 'cape_classique', 'jambes': 'jambes_jambieres', 'armure': 'armure_cotte',
                'casque': 'casque_rond', 'yeux': 'yeux_points', 'arme': 'arme_epee', 'main': 'main_bouclier'}
    by_id = {it[1]: it for it in ITEMS}

    def compose(over):
        layers = dict(defaults)
        layers.update(over)
        img = Image.new('RGB', (28, 32), (12, 13, 24))
        eye = by_id[layers.get('yeux', 'yeux_points')][5]
        for slot in SLOTS:
            iid = layers.get(slot)
            if not iid:
                continue
            _, _, _, _, _, base, (w, h, ox, oy, rows) = by_id[iid]
            for j, row in enumerate(rows):
                for i, ch in enumerate(row):
                    if ch == '.':
                        continue
                    x, y = ox + i, oy + j
                    if not (0 <= x < 28 and 0 <= y < 32):
                        continue
                    if ch == '1':
                        c = base
                    elif ch == '2':
                        c = tuple(int(v * 0.55) for v in base)
                    elif ch == '3':
                        c = tuple(int(v + (255 - v) * 0.45) for v in base)
                    elif ch == 'E':
                        c = eye
                    else:
                        c = PAL[ch]
                    img.putpixel((x, y), c)
        return img

    cells = []
    for it in ITEMS:
        if it[0] == 'aura' or not it[6][4]:
            continue
        cells.append((it[1], compose({it[0]: it[1]})))
    cols = 12
    rows_n = (len(cells) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * 30 * sc, rows_n * 34 * sc + 40 * sc), (40, 40, 50))
    for n, (iid, img) in enumerate(cells):
        sheet.paste(img.resize((28 * sc, 32 * sc), Image.NEAREST), ((n % cols) * 30 * sc, (n // cols) * 34 * sc))
    x = 0
    for (w, h, ox, oy, rows) in BOSSES:
        img = Image.new('RGB', (w, h), (12, 13, 24))
        for j, row in enumerate(rows):
            for i, ch in enumerate(row):
                if ch != '.':
                    img.putpixel((i, j), PAL.get(ch, (255, 255, 0)) if ch not in '123E' else (255, 230, 80))
        sheet.paste(img.resize((w * sc, h * sc), Image.NEAREST), (x, rows_n * 34 * sc))
        x += (w + 4) * sc
    sheet.save(path)


if __name__ == '__main__':
    write_c(sys.argv[1])
    if len(sys.argv) > 2:
        preview(sys.argv[2])
    print(len(ITEMS), 'objets')

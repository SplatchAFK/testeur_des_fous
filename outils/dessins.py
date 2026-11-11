#!/usr/bin/env python3
"""
Le testeur des fous : dessine tous les décors et personnages des cinématiques
en pixel art (PNG avec transparence), à la taille exacte de l'écran du jeu.

    python3 dessins.py <dossier assets> libft|printf|tout

Les scènes font 140 x 84 pixels : une fenêtre de 140 x 42 caractères, deux
pixels par caractère. Le jeu les dessine à taille entière (x1, x2...), jamais
déformées. Ce script n'a besoin que de Pillow.
"""
import math
import os
import random
import sys

from PIL import Image, ImageDraw

W, H = 140, 84
BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]
OUTLINE = (24, 20, 34, 255)


# ------------------------------------------------------------------ outils

def new(w, h, col=(0, 0, 0, 0)):
    return Image.new("RGBA", (w, h), col)


def mix(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(3)) + (255,)


def ramp(cols, t):
    """couleur le long d'une rampe de plusieurs couleurs"""
    t = max(0.0, min(0.9999, t))
    n = len(cols) - 1
    i = int(t * n)
    return mix(cols[i], cols[i + 1], t * n - i)


def dither_pick(cols, t, x, y):
    """rampe quantifiée avec un tramage ordonné : des dégradés sans bandes"""
    n = len(cols) - 1
    v = max(0.0, min(0.9999, t)) * n
    i = int(v)
    f = v - i
    thr = (BAYER[y % 4][x % 4] + 0.5) / 16
    c = cols[min(n, i + (1 if f > thr else 0))]
    return tuple(c[:3]) + (255,)


def vgrad(img, box, cols, dither=True):
    x0, y0, x1, y1 = box
    for y in range(y0, y1):
        t = (y - y0) / max(1, y1 - y0 - 1)
        for x in range(x0, x1):
            img.putpixel((x, y), dither_pick(cols, t, x, y) if dither else ramp(cols, t))


def put(img, x, y, c):
    if 0 <= x < img.width and 0 <= y < img.height:
        img.putpixel((x, y), tuple(c[:3]) + (c[3] if len(c) > 3 else 255,))


def get(img, x, y):
    return img.getpixel((x, y))


def blend(img, x, y, c, k):
    if 0 <= x < img.width and 0 <= y < img.height and k > 0:
        o = img.getpixel((x, y))
        if o[3] == 0:
            return
        put(img, x, y, mix(o, c, k))


def glow(img, cx, cy, r, c, k, dither=True):
    for y in range(int(cy - r), int(cy + r) + 1):
        for x in range(int(cx - r), int(cx + r) + 1):
            d = math.hypot(x - cx, y - cy) / r
            if d < 1:
                a = (1 - d) ** 2 * k
                if dither:
                    a = a + ((BAYER[y % 4][x % 4] + 0.5) / 16 - 0.5) * 0.12
                blend(img, x, y, c, a)


def outline(img, col=OUTLINE):
    """contour de 1 pixel autour de tout ce qui est opaque"""
    w, h = img.size
    src = img.copy()
    for y in range(h):
        for x in range(w):
            if src.getpixel((x, y))[3]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h and src.getpixel((nx, ny))[3] > 128:
                    img.putpixel((x, y), col)
                    break
    return img


def grid(rows, pal):
    """sprite à partir de lignes de caractères ; '.' = transparent"""
    h = len(rows)
    w = max(len(r) for r in rows)
    img = new(w, h)
    for y, r in enumerate(rows):
        for x, ch in enumerate(r):
            if ch != "." and ch in pal:
                img.putpixel((x, y), tuple(pal[ch]) + (255,))
    return img


def sheet(frames):
    w = sum(f.width for f in frames)
    h = max(f.height for f in frames)
    out = new(w, h)
    x = 0
    for f in frames:
        out.paste(f, (x, 0), f)
        x += f.width
    return out


FONT3 = {
    "A": ["111", "101", "111", "101", "101"], "B": ["110", "101", "110", "101", "110"],
    "C": ["111", "100", "100", "100", "111"], "D": ["110", "101", "101", "101", "110"],
    "E": ["111", "100", "110", "100", "111"], "F": ["111", "100", "110", "100", "100"],
    "G": ["111", "100", "101", "101", "111"], "I": ["111", "010", "010", "010", "111"],
    "L": ["100", "100", "100", "100", "111"], "M": ["101", "111", "111", "101", "101"],
    "N": ["110", "101", "101", "101", "101"], "O": ["111", "101", "101", "101", "111"],
    "R": ["110", "101", "110", "101", "101"], "S": ["111", "100", "111", "001", "111"],
    "T": ["111", "010", "010", "010", "010"], "U": ["101", "101", "101", "101", "111"],
    "2": ["111", "001", "111", "100", "111"], "5": ["111", "100", "111", "001", "111"],
    "!": ["010", "010", "010", "000", "010"], "%": ["101", "001", "010", "100", "101"],
    " ": ["000", "000", "000", "000", "000"],
}


def text3(img, x, y, s, c):
    for ch in s:
        g = FONT3.get(ch.upper(), FONT3[" "])
        for j, row in enumerate(g):
            for i, b in enumerate(row):
                if b == "1":
                    put(img, x + i, y + j, c)
        x += 4


# ------------------------------------------------------------------ Moulinette

CAT_PAL = {
    "K": (36, 34, 48), "D": (96, 98, 114), "G": (148, 150, 164), "L": (190, 192, 206),
    "W": (246, 242, 234), "P": (255, 150, 176), "E": (110, 222, 122), "e": (18, 18, 24),
    "h": (255, 255, 255), "B": (255, 190, 200),
}

CAT = [
    "...KK..............KK...",
    "..KGGK............KGGK..",
    "..KGPGKKKKKKKKKKKKGPGK..",
    "..KGPPGGGGGGGGGGGGPPGK..",
    ".KGGGLGGGGGGGGGGGGLGGGK.",
    ".KGGLLGGGGGGGGGGGGLLGGK.",
    ".KGGKEEKGGGGGGGGKEEKGGK.",
    ".KGGKEeKGGGGGGGGKeEKGGK.",
    ".KGGGKKGGGGWWGGGGKKGGGK.",
    ".KGBLLGGGWWPPWWGGGLLBGK.",
    "..KGGGGGWWKWWKWWGGGGGK..",
    "...KGGGGWWWWWWWWGGGGK...",
    "....KKGGGWWWWWWGGGKK....",
    "...KGGGGGWWWWWWGGGGGK...",
    "..KGGGGGGWWWWWWGGGGGGK..",
    "..KGGDGGGWWWWWWGGGDGGKK.",
    "..KGDGGGGWWWWWWGGGGDGKGK",
    "..KGGGGGGWWWWWWGGGGGGKGK",
    "..KGGGGGWWWWWWWWGGGGGKGK",
    "..KGGGKWWKWWWWKWWKGGGKK.",
    "..KKKKKWWKKKKKKWWKKKKK..",
    "........................",
]


def cat_frames():
    base = [list(r) for r in CAT]

    def build(rows):
        return grid(["".join(r) for r in rows], CAT_PAL)

    f_idle = build(base)
    # respiration : la tête descend d'un pixel
    breathe = [list("." * 24)] + [r[:] for r in base[:12]] + [r[:] for r in base[13:]]
    f_breathe = build(breathe)
    # clignement
    blink = [r[:] for r in base]
    blink[6] = list(".KGGGGGGGGGGGGGGGGGGGGK.")
    blink[7] = list(".KGGKKKKGGGGGGGGKKKKGGK.")
    f_blink = build(blink)
    # contente : yeux en ^ ^
    happy = [r[:] for r in base]
    happy[6] = list(".KGGGKKGGGGGGGGGGKKGGGK.")
    happy[7] = list(".KGGKGGKGGGGGGGGKGGKGGK.")
    happy[8] = list(".KGGGGGGGGGWWGGGGGGGGGK.")
    f_happy = build(happy)
    # surprise : bouche ouverte
    surp = [r[:] for r in base]
    surp[10] = list("..KGGGGGWWKPPKWWGGGGGK..")
    surp[11] = list("...KGGGGWWKPPKWWGGGGK...")
    f_surp = build(surp)
    frames = [f_idle, f_breathe, f_blink, f_happy, f_surp]
    for f in frames:
        for y in range(f.height):
            for x in range(f.width):
                p = f.getpixel((x, y))
                if p[3] and p[:3] == CAT_PAL["E"] and (x + y) % 5 == 0:
                    f.putpixel((x, y), CAT_PAL["h"] + (255,))
    return sheet(frames)


def cat_face(blink=False):
    """visage de Moulinette en gros plan (vue subjective)"""
    img = new(112, 84)
    d = ImageDraw.Draw(img)
    fur = [(70, 72, 88), (120, 122, 138), (158, 160, 174), (196, 198, 210)]
    # oreilles
    for s in (-1, 1):
        cx = 56 + s * 34
        d.polygon([(cx - s * 18, 34), (cx + s * 6, 0), (cx + s * 16, 30)], fill=fur[1] + (255,))
        d.polygon([(cx - s * 10, 30), (cx + s * 5, 8), (cx + s * 11, 28)], fill=(255, 160, 186, 255))
    # tête avec ombrage tramé (lumière en haut à gauche)
    for y in range(10, 84):
        for x in range(0, 112):
            dx, dy = (x - 56) / 52, (y - 52) / 40
            r = dx * dx + dy * dy
            if r <= 1:
                light = 0.75 - 0.45 * ((x - 30) / 112 + (y - 20) / 84) - r * 0.25
                img.putpixel((x, y), dither_pick(fur, light, x, y))
    # rayures du front
    for k in (-1, 0, 1):
        d.line([(56 + k * 9, 14), (56 + k * 7, 26)], fill=fur[0] + (255,), width=2)
    # museau blanc
    for y in range(48, 84):
        for x in range(28, 84):
            dx, dy = (x - 56) / 26, (y - 66) / 18
            if dx * dx + dy * dy <= 1:
                img.putpixel((x, y), dither_pick([(214, 210, 204), (246, 242, 234), (255, 252, 248)],
                                                 0.8 - (y - 48) / 60, x, y))
    # yeux
    for s in (-1, 1):
        ex, ey = 56 + s * 21, 42
        if blink:
            d.line([(ex - 10, ey + 2), (ex + 10, ey + 2)], fill=(30, 28, 40, 255), width=2)
            continue
        for y in range(ey - 11, ey + 12):
            for x in range(ex - 11, ex + 12):
                dx, dy = (x - ex) / 11, (y - ey) / 11
                rr = dx * dx + dy * dy
                if rr <= 1:
                    img.putpixel((x, y), dither_pick([(40, 140, 60), (90, 200, 100), (170, 245, 160)],
                                                     0.9 - (y - ey + 11) / 30, x, y))
                if rr > 0.82 and rr <= 1:
                    img.putpixel((x, y), (30, 28, 40, 255))
        d.ellipse([ex - 3, ey - 9, ex + 3, ey + 9], fill=(16, 16, 22, 255))
        d.ellipse([ex - 7, ey - 7, ex - 3, ey - 3], fill=(255, 255, 255, 255))
        put(img, ex + 4, ey + 5, (255, 255, 255))
    # nez, bouche, joues, moustaches
    d.polygon([(51, 60), (61, 60), (56, 66)], fill=(255, 140, 170, 255))
    d.line([(56, 66), (51, 71), (47, 70)], fill=(110, 70, 70, 255), width=1)
    d.line([(56, 66), (61, 71), (65, 70)], fill=(110, 70, 70, 255), width=1)
    for s in (-1, 1):
        glow(img, 56 + s * 30, 62, 8, (255, 150, 170), 0.5, dither=True)
        for k in (-1, 0, 1):
            d.line([(56 + s * 18, 62 + k * 3), (56 + s * 54, 58 + k * 7)], fill=(250, 250, 255, 255), width=1)
    return img


# ------------------------------------------------------------------ petits sprites

def plush_frames():
    pal = {"K": (36, 34, 48), "P": (255, 170, 200), "W": (240, 236, 228), "G": (90, 190, 110),
           "Y": (250, 210, 90), "V": (170, 110, 240), "D": (46, 40, 66), "U": (96, 64, 140),
           "R": (240, 90, 90), "B": (90, 140, 240), "T": (90, 170, 200), "E": (255, 255, 255)}
    gardien = [
        "....PPPPP....",
        "...PPPPPPP...",
        "..PPPPPPPPP..",
        "..WWWWWWWWW..",
        "..WWKWWWKWW..",
        "..WWWWWWWWW..",
        "...WWWPWWW...",
        "..GGGGGGGGG..",
        ".GGGYGGGYGGG.",
        ".GGGGGGGGGGG.",
        "..GGGGGGGGG..",
        "..GG.....GG..",
        ".............",
    ]
    alchimiste = [
        "......V......",
        ".....VVV.....",
        "....VVYVV....",
        "...VVVVVVV...",
        ".VVVVVVVVVVV.",
        "...DDDDDDD...",
        "...DYDDDYD...",
        "...DDDDDDD...",
        "..UUUUUUUUU..",
        ".UUUUYUUUUUU.",
        "..UUUUUUUUU..",
        "..UU.....UU..",
        ".............",
    ]
    hydre = [
        ".RR..GG..BB..",
        "RERRGEGGBEBB.",
        ".RR..GG..BB..",
        "..R..G...B...",
        "..R..G..B....",
        "...R.G.B.....",
        "..TTTTTTTT...",
        ".TTTTTTTTTT..",
        ".TWWTTTTWWT..",
        ".TTTTTTTTTT..",
        "..TTTTTTTT...",
        "..TT....TT...",
        ".............",
    ]
    return sheet([outline(grid(g, pal)) for g in (gardien, alchimiste, hydre)])


def mug():
    pal = {"W": (240, 236, 228), "S": (200, 194, 186), "C": (110, 64, 40), "c": (150, 96, 60), "H": (255, 255, 255)}
    g = grid([
        "............",
        ".WWWWWWWW...",
        ".WcCCCCcW...",
        ".WWWWWWWWWW.",
        ".WHWWWWWW.W.",
        ".WHWWWWWW.W.",
        ".WWWWWWWWWW.",
        ".SWWWWWWS...",
        "..SSSSSS....",
        "............",
    ], pal)
    return outline(g)


def gear_frames(r, teeth, base):
    frames = []
    size = r * 2 + 6
    for k in range(4):
        img = new(size, size)
        d = ImageDraw.Draw(img)
        c = size / 2
        ang0 = k * (2 * math.pi / teeth) / 4
        pts = []
        for i in range(teeth * 2):
            a = ang0 + i * math.pi / teeth
            rr = r + 2 if i % 2 == 0 else r - 1
            for e in (-0.18, 0.18):
                pts.append((c + math.cos(a + e) * rr, c + math.sin(a + e) * rr))
        d.polygon(pts, fill=base + (255,))
        for y in range(size):
            for x in range(size):
                p = img.getpixel((x, y))
                if p[3]:
                    light = 0.5 + 0.5 * (-(x - c) - (y - c)) / (r * 1.6)
                    img.putpixel((x, y), dither_pick([mix(base, (0, 0, 0), 0.45), base, mix(base, (255, 255, 255), 0.35)],
                                                     light, x, y))
        d.ellipse([c - r * 0.55, c - r * 0.55, c + r * 0.55, c + r * 0.55], fill=mix(base, (0, 0, 0), 0.5))
        d.ellipse([c - r * 0.28, c - r * 0.28, c + r * 0.28, c + r * 0.28], fill=mix(base, (255, 255, 255), 0.25))
        for i in range(4):
            a = ang0 * 2 + i * math.pi / 2
            put(img, int(c + math.cos(a) * r * 0.42), int(c + math.sin(a) * r * 0.42), mix(base, (255, 255, 255), 0.5))
        frames.append(outline(img))
    return sheet(frames)


def stamp_frames():
    out = []
    for k in range(2):
        img = new(16, 22)
        d = ImageDraw.Draw(img)
        d.rectangle([6, 0, 9, 9], fill=(150, 110, 70, 255))
        d.rectangle([5, 0, 10, 2], fill=(190, 150, 100, 255))
        d.rectangle([1, 10, 14, 19], fill=(176, 50, 64, 255))
        d.rectangle([1, 10, 14, 11], fill=(220, 90, 100, 255))
        d.rectangle([1, 18, 14, 19], fill=(120, 30, 44, 255))
        text3(img, 2 if k == 0 else 3, 13, "E!", (255, 230, 230))
        out.append(outline(img))
    return sheet(out)


def cage_frames():
    """cage de 28 x 30 : barreaux fins (on voit Moulinette entre eux) qui remontent"""
    out = []
    for k in range(4):
        img = new(28, 30)
        cap = new(28, 30)
        d = ImageDraw.Draw(cap)
        d.rectangle([1, 2, 26, 4], fill=(120, 118, 140, 255))
        d.rectangle([1, 2, 26, 2], fill=(200, 200, 216, 255))
        d.rectangle([12, 0, 15, 1], fill=(150, 150, 170, 255))
        cap = outline(cap, (30, 28, 40, 255))
        lift = k * 8
        bars = ImageDraw.Draw(img)
        for i, x in enumerate((1, 6, 11, 16, 21, 26)):
            if 29 - lift > 5:
                bars.line([(x, 5), (x, 29 - lift)], fill=(206, 206, 222, 255))
                if i in (0, 5):
                    bars.line([(x + (1 if i == 0 else -1), 5), (x + (1 if i == 0 else -1), 29 - lift)],
                              fill=(96, 94, 116, 255))
        if k == 0:
            bars.rectangle([1, 28, 26, 29], fill=(120, 118, 140, 255))
            bars.line([(1, 28), (26, 28)], fill=(180, 180, 200, 255))
        img.paste(cap, (0, 0), cap)
        out.append(img)
    return sheet(out)


def torch_frames():
    out = []
    pal = [(255, 250, 200), (255, 210, 90), (255, 140, 40), (200, 70, 30)]
    for k in range(4):
        img = new(9, 16)
        d = ImageDraw.Draw(img)
        d.rectangle([3, 9, 5, 15], fill=(110, 76, 50, 255))
        d.rectangle([2, 9, 6, 10], fill=(80, 70, 70, 255))
        random.seed(k * 7 + 3)
        for y in range(9):
            w = [0.5, 1, 1.6, 2.2, 2.6, 2.8, 2.6, 2.2, 1.6][y] + random.uniform(-0.4, 0.4)
            cx = 4 + math.sin(k * 1.6 + y * 0.7) * 0.6
            for x in range(9):
                dd = abs(x - cx) / max(0.5, w)
                if dd <= 1:
                    img.putpixel((x, y), ramp(pal, dd * 0.6 + (8 - y) / 16) )
        out.append(outline(img, (40, 26, 20, 255)))
    return sheet(out)


# ------------------------------------------------------------------ décors printf

def wall_bricks(img, box, base, mortar, rows=5, seed=1):
    random.seed(seed)
    x0, y0, x1, y1 = box
    for y in range(y0, y1):
        for x in range(x0, x1):
            row = (y - y0) // rows
            off = (row % 2) * 6
            bx = (x - x0 + off) // 12
            if (y - y0) % rows == 0 or (x - x0 + off) % 12 == 0:
                img.putpixel((x, y), tuple(mortar) + (255,))
            else:
                random.seed(row * 131 + bx * 17 + seed)
                v = random.uniform(-0.12, 0.12)
                c = mix(base, (255, 255, 255) if v > 0 else (0, 0, 0), abs(v))
                if (y - y0) % rows == 1:
                    c = mix(c, (255, 255, 255), 0.08)
                img.putpixel((x, y), c)


def splash_bg():
    img = new(W, H)
    vgrad(img, (0, 0, W, H - 20), [(8, 4, 20), (22, 8, 38), (46, 14, 62)])
    vgrad(img, (0, H - 20, W, H), [(40, 14, 56), (20, 8, 30), (10, 4, 16)])
    for x in range(W):
        if x % 3 == 0:
            put(img, x, H - 20, (90, 40, 120))
    glow(img, W / 2, 26, 46, (255, 80, 200), 0.55)
    glow(img, W / 2, H - 14, 34, (180, 90, 255), 0.35)
    # vignette
    for y in range(H):
        for x in range(W):
            d = math.hypot((x - W / 2) / (W / 2), (y - H / 2) / (H / 2))
            if d > 0.75:
                blend(img, x, y, (0, 0, 0), (d - 0.75) * 1.4)
    return img


def previously_bg():
    img = new(W, H)
    vgrad(img, (0, 0, W, 60), [(70, 46, 54), (150, 90, 70), (230, 150, 90), (250, 200, 130)])
    glow(img, 98, 52, 26, (255, 220, 150), 0.6)
    d = ImageDraw.Draw(img)
    d.ellipse([88, 42, 108, 62], fill=(255, 226, 160, 255))
    # collines en trois plans
    for k, (col, base, amp) in enumerate([((120, 76, 70), 50, 5), ((94, 60, 58), 56, 4), ((72, 48, 46), 62, 3)]):
        for x in range(W):
            y = int(base + math.sin(x / (14 + k * 6) + k) * amp + math.sin(x / 5.3 + k * 2) * 1.2)
            for yy in range(y, H):
                img.putpixel((x, yy), col + (255,))
    # la tour au loin (à gauche)
    d.rectangle([12, 14, 26, 60], fill=(56, 38, 40, 255))
    for i in range(4):
        d.rectangle([11 + i * 4, 10, 13 + i * 4, 14], fill=(56, 38, 40, 255))
    for y in (24, 36, 48):
        d.rectangle([18, y, 20, y + 3], fill=(255, 200, 110, 255))
    # chemin
    vgrad(img, (0, 66, W, H), [(84, 62, 48), (64, 46, 38)])
    for x in range(W):
        for y in range(66, H):
            if abs(x - (60 + (y - 66) * 1.8)) < 6 + (y - 66) * 0.6:
                img.putpixel((x, y), dither_pick([(150, 112, 80), (120, 88, 64)], (y - 66) / 18, x, y))
    return img


def machine_bg():
    img = new(W, H)
    wall_bricks(img, (0, 0, W, 64), (58, 44, 74), (34, 26, 46), rows=5, seed=3)
    for y in range(0, 64):
        for x in range(W):
            blend(img, x, y, (10, 6, 18), 0.55 * (1 - y / 64))
    # tuyaux
    d = ImageDraw.Draw(img)
    for py, col in ((10, (120, 108, 96)), (52, (104, 94, 86))):
        d.rectangle([0, py, W, py + 3], fill=col + (255,))
        d.line([(0, py), (W, py)], fill=mix(col, (255, 255, 255), 0.3))
        d.line([(0, py + 3), (W, py + 3)], fill=mix(col, (0, 0, 0), 0.4))
        for x in range(8, W, 30):
            d.rectangle([x, py - 1, x + 3, py + 4], fill=mix(col, (0, 0, 0), 0.25))
    # la Machine de la Norminette : grand cadre de fer riveté
    d.rectangle([36, 4, 104, 62], fill=(52, 50, 64, 255))
    d.rectangle([40, 8, 100, 58], fill=(30, 26, 40, 255))
    for x in range(36, 105, 6):
        put(img, x, 5, (170, 170, 190))
        put(img, x, 61, (170, 170, 190))
    for y in range(5, 62, 6):
        put(img, 37, y, (170, 170, 190))
        put(img, 103, y, (170, 170, 190))
    # plaque
    d.rectangle([48, 0, 92, 8], fill=(150, 120, 70, 255))
    d.rectangle([48, 0, 92, 0], fill=(200, 170, 110, 255))
    text3(img, 51, 2, "NORMINETTE", (60, 40, 20))
    # voyants sur les montants du cadre (loin des chaînes)
    for i, gy in enumerate((16, 24, 32, 40, 48)):
        put(img, 38, gy, (255, 60, 70) if i % 2 else (90, 255, 120))
        put(img, 102, gy, (255, 60, 70) if i % 2 == 0 else (90, 255, 120))
    # règle « 25 » au mur
    d.rectangle([4, 20, 30, 24], fill=(200, 182, 120, 255))
    for i in range(26):
        d.line([(4 + i, 20), (4 + i, 21 if i % 5 else 22)], fill=(100, 84, 50, 255))
    text3(img, 12, 26, "25", (200, 182, 120))
    # sol en planches
    vgrad(img, (0, 64, W, H), [(70, 50, 52), (50, 36, 40), (34, 24, 30)])
    for y in (64, 69, 75, 82):
        d.line([(0, y), (W, y)], fill=(30, 20, 26, 255))
    for x in range(0, W, 22):
        for y in range(64, H):
            if (y - 64) % 11 < 5:
                put(img, x + (y > 69) * 11, y, (40, 28, 34))
    return img


def restaurant_bg():
    img = new(W, H)
    # papier peint à rayures
    for y in range(0, 50):
        for x in range(W):
            stripe = (x // 4) % 2
            base = (150, 84, 64) if stripe else (134, 74, 58)
            img.putpixel((x, y), dither_pick([mix(base, (0, 0, 0), 0.3), base], y / 50 + 0.2, x, y))
    # lambris
    vgrad(img, (0, 50, W, 62), [(110, 66, 40), (86, 50, 32)])
    d = ImageDraw.Draw(img)
    for x in range(0, W, 14):
        d.line([(x, 50), (x, 61)], fill=(66, 38, 26, 255))
    d.line([(0, 50), (W, 50)], fill=(170, 110, 70, 255))
    # fenêtre : nuit, lumières de la ville
    wx0, wy0, wx1, wy1 = 92, 8, 132, 46
    vgrad(img, (wx0, wy0, wx1, wy1), [(18, 24, 50), (30, 40, 80), (50, 60, 100)])
    random.seed(9)
    for _ in range(26):
        x, y = random.randint(wx0 + 1, wx1 - 2), random.randint(wy0 + 18, wy1 - 2)
        put(img, x, y, random.choice([(255, 220, 140), (255, 190, 120), (200, 220, 255)]))
    d.rectangle([wx0 - 2, wy0 - 2, wx1 + 1, wy1 + 1], outline=(150, 100, 60, 255))
    d.rectangle([wx0 - 1, wy0 - 1, wx1, wy1], outline=(110, 72, 44, 255))
    d.line([((wx0 + wx1) // 2, wy0), ((wx0 + wx1) // 2, wy1)], fill=(150, 100, 60, 255))
    d.line([(wx0, (wy0 + wy1) // 2), (wx1, (wy0 + wy1) // 2)], fill=(150, 100, 60, 255))
    # rideaux
    for cx in (wx0 - 6, wx1 + 2):
        for y in range(wy0 - 4, wy1 + 6):
            for x in range(cx, cx + 6):
                fold = (x - cx) % 3
                img.putpixel((x, y), [(170, 40, 50, 255), (200, 60, 70, 255), (140, 30, 44, 255)][fold])
    # lampe suspendue et halo
    d.line([(46, 0), (46, 8)], fill=(40, 30, 30, 255))
    d.polygon([(38, 14), (54, 14), (50, 8), (42, 8)], fill=(220, 170, 90, 255))
    d.line([(38, 14), (54, 14)], fill=(255, 220, 140, 255))
    glow(img, 46, 18, 40, (255, 210, 140), 0.45)
    # table au premier plan
    vgrad(img, (0, 62, W, H), [(176, 116, 70), (140, 90, 54), (104, 66, 40)])
    d.line([(0, 62), (W, 62)], fill=(210, 150, 96, 255))
    random.seed(4)
    for _ in range(14):
        y = random.randint(64, H - 2)
        x = random.randint(0, W - 30)
        d.line([(x, y), (x + random.randint(10, 30), y)], fill=(120, 76, 46, 255))
    return img


# ------------------------------------------------------------------ décors libft

def sky(img, box, cols, seed, stars=60, moon=None):
    vgrad(img, box, cols)
    random.seed(seed)
    x0, y0, x1, y1 = box
    for _ in range(stars):
        x, y = random.randint(x0, x1 - 1), random.randint(y0, int(y0 + (y1 - y0) * 0.8))
        c = random.choice([(255, 255, 240), (200, 210, 255), (255, 230, 200)])
        put(img, x, y, c)
        if random.random() < 0.12:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                blend(img, x + dx, y + dy, c, 0.5)
    if moon:
        mx, my, mr = moon
        glow(img, mx, my, mr * 3, (255, 245, 210), 0.4)
        d = ImageDraw.Draw(img)
        d.ellipse([mx - mr, my - mr, mx + mr, my + mr], fill=(250, 244, 222, 255))
        for cx, cy, cr in ((-2, -1, 2), (2, 2, 1), (1, -3, 1)):
            d.ellipse([mx + cx - cr, my + cy - cr, mx + cx + cr, my + cy + cr], fill=(220, 212, 190, 255))


def stone(img, box, base, seed, light_right=True):
    x0, y0, x1, y1 = box
    wall_bricks(img, box, base, mix(base, (0, 0, 0), 0.4)[:3], rows=4, seed=seed)
    for y in range(y0, y1):
        for x in range(x0, x1):
            t = (x - x0) / max(1, x1 - x0)
            k = (0.35 * (1 - t)) if light_right else (0.35 * t)
            blend(img, x, y, (10, 8, 24), k)


def tower_bg():
    th = 260
    img = new(W, th)
    sky(img, (0, 0, W, th), [(14, 10, 40), (32, 20, 70), (40, 34, 96), (30, 44, 100)], seed=5, stars=110,
        moon=(112, 20, 6))
    d = ImageDraw.Draw(img)
    # montagnes lointaines
    for x in range(W):
        y = int(206 + math.sin(x / 13) * 6 + math.sin(x / 5.7) * 2)
        for yy in range(y, th):
            img.putpixel((x, yy), (30, 34, 70, 255))
    # tour
    tx0, tx1, top = 52, 88, 66
    stone(img, (tx0, top, tx1, 242), (104, 96, 132), seed=7)
    for i in range(10):
        x = tx0 - 2 + i * 4
        if i % 2 == 0:
            stone(img, (x, top - 6, x + 4, top), (104, 96, 132), seed=8 + i)
    d.rectangle([tx0 - 3, top - 1, tx1 + 2, top + 1], fill=(80, 74, 104, 255))
    # fenêtres éclairées
    for wy in (90, 150, 196):
        d.rectangle([68, wy, 72, wy + 7], fill=(255, 206, 110, 255))
        d.ellipse([68, wy - 2, 72, wy + 2], fill=(255, 206, 110, 255))
        put(img, 70, wy + 3, (255, 240, 190))
        glow(img, 70, wy + 3, 9, (255, 200, 110), 0.35)
    # balcon des gardiens
    d.rectangle([22, 124, 118, 127], fill=(84, 78, 108, 255))
    d.line([(22, 124), (118, 124)], fill=(130, 124, 160, 255))
    for x in range(24, 118, 6):
        d.line([(x, 117), (x, 123)], fill=(84, 78, 108, 255))
    d.line([(22, 117), (118, 117)], fill=(84, 78, 108, 255))
    # sol, herbe, chemin, porte
    vgrad(img, (0, 240, W, th), [(36, 80, 54), (26, 60, 42), (18, 40, 30)])
    random.seed(11)
    for x in range(0, W, 2):
        if random.random() < 0.6:
            put(img, x, 239, (60, 120, 70))
            put(img, x + 1, 238, (70, 140, 80))
    for y in range(242, th):
        for x in range(W):
            if abs(x - 70) < 4 + (y - 242) * 1.2:
                img.putpixel((x, y), dither_pick([(120, 104, 90), (90, 78, 68)], (y - 242) / 18, x, y))
    d.rectangle([64, 226, 76, 241], fill=(70, 42, 30, 255))
    d.ellipse([64, 221, 76, 232], fill=(70, 42, 30, 255))
    for x in (67, 70, 73):
        d.line([(x, 226), (x, 241)], fill=(50, 30, 22, 255))
    put(img, 74, 234, (230, 200, 90))
    return img


def room_bg():
    img = new(W, H)
    stone(img, (0, 0, W, 58), (74, 64, 84), seed=21, light_right=False)
    for y in range(58):
        for x in range(W):
            blend(img, x, y, (6, 4, 10), 0.4 * (1 - y / 58) + 0.25 * abs(x - W / 2) / (W / 2))
    d = ImageDraw.Draw(img)
    # arche centrale
    d.rectangle([52, 18, 88, 58], fill=(14, 10, 18, 255))
    d.ellipse([52, 4, 88, 34], fill=(14, 10, 18, 255))
    for a in range(0, 181, 12):
        r = math.radians(a)
        put(img, int(70 + math.cos(r) * 19), int(19 - math.sin(r) * 16), (120, 110, 130))
    # bannières
    for bx in (34, 100):
        d.rectangle([bx, 8, bx + 8, 30], fill=(120, 30, 50, 255))
        d.polygon([(bx, 30), (bx + 4, 26), (bx + 8, 30), (bx + 8, 34), (bx + 4, 30), (bx, 34)], fill=(120, 30, 50, 255))
        d.line([(bx - 1, 8), (bx + 9, 8)], fill=(200, 170, 90, 255))
        put(img, bx + 4, 16, (240, 210, 110))
    # supports de torches
    for tx in (18, 120):
        d.rectangle([tx - 1, 34, tx + 2, 36], fill=(60, 56, 64, 255))
    # sol en dalles
    vgrad(img, (0, 58, W, H), [(64, 56, 70), (48, 42, 56), (32, 28, 40)])
    for y in (58, 63, 70, 79):
        d.line([(0, y), (W, y)], fill=(26, 22, 32, 255))
    for i in range(-8, 9):
        d.line([(70 + i * 9, 58), (70 + i * 22, H)], fill=(26, 22, 32, 255))
    return img


def towertop_bg():
    img = new(W, H)
    sky(img, (0, 0, W, H), [(14, 10, 40), (40, 24, 80), (70, 46, 110)], seed=31, stars=70, moon=(24, 14, 5))
    stone(img, (0, 58, W, H), (110, 100, 138), seed=33)
    for i in range(0, W, 16):
        stone(img, (i, 50, i + 9, 58), (110, 100, 138), seed=40 + i)
    d = ImageDraw.Draw(img)
    d.line([(0, 58), (W, 58)], fill=(150, 140, 176, 255))
    return img


# ------------------------------------------------------------------ ft_printf

def colossus_frames():
    """ft_printf : un colosse encapuchonné fait de conversions qui défilent"""
    fw, fh = 60, 66
    pal_glyph = [(255, 90, 200), (190, 110, 255), (255, 150, 230)]
    frames = []
    for k in range(3):
        img = new(fw, fh)
        d = ImageDraw.Draw(img)
        body = (64, 20, 84, 255)
        # robe large, épaules, capuche
        d.polygon([(10, 22), (50, 22), (58, 65), (2, 65)], fill=body)
        d.ellipse([12, 16, 48, 32], fill=body)
        d.polygon([(30, 0), (44, 10), (46, 24), (14, 24), (16, 10)], fill=body)
        # bras
        d.polygon([(6, 24), (14, 24), (12, 48), (4, 48)], fill=body)
        d.polygon([(46, 24), (54, 24), (56, 48), (48, 48)], fill=body)
        d.ellipse([2, 45, 13, 54], fill=(80, 26, 100, 255))
        d.ellipse([47, 45, 58, 54], fill=(80, 26, 100, 255))
        # volume : tramage sombre en bas, lumière rose à gauche
        for y in range(fh):
            for x in range(fw):
                pp = img.getpixel((x, y))
                if not pp[3]:
                    continue
                light = 0.75 - y / fh * 0.55 - (x - 18) / fw * 0.35
                img.putpixel((x, y), dither_pick([(26, 8, 40), (52, 16, 72), (86, 28, 112), (128, 46, 150)], light, x, y))
        # intérieur de la capuche
        d.ellipse([20, 7, 40, 23], fill=(14, 4, 22, 255))
        # conversions qui défilent dans le corps
        random.seed(100 + k)
        glyphs = "%SDCPUXI"
        for gy in range(26 + (k * 2) % 6, fh - 6, 7):
            for gx in range(8 + (gy * 3 + k * 5) % 9, fw - 8, 9):
                if img.getpixel((gx, gy))[3] and img.getpixel((gx + 2, gy + 4))[3] and random.random() < 0.7:
                    text3(img, gx, gy, random.choice(glyphs), pal_glyph[(gx + gy + k) % 3])
        # yeux : deux petits % lumineux
        for ex in (24, 33):
            for (dx, dy) in ((0, 0), (3, 3), (2, 0), (1, 1), (1, 2), (0, 3)):
                put(img, ex + dx, 12 + dy, (255, 240, 255) if k != 1 else (255, 200, 245))
        frames.append(outline(img, (14, 4, 20, 255)))
    return sheet(frames)


# ------------------------------------------------------------------ export

def save(img, out, name):
    img.save(os.path.join(out, name + ".png"))


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "assets"
    which = sys.argv[2] if len(sys.argv) > 2 else "tout"
    os.makedirs(out, exist_ok=True)
    save(cat_frames(), out, "chat")
    if which in ("libft", "tout"):
        save(tower_bg(), out, "tour")
        save(room_bg(), out, "salle")
        save(towertop_bg(), out, "sommet")
        save(cage_frames(), out, "cage")
        save(torch_frames(), out, "torche")
        save(sheet([cat_face(False), cat_face(True)]), out, "chat_visage")
    if which in ("printf", "tout"):
        save(splash_bg(), out, "scene_titre")
        save(previously_bg(), out, "scene_avant")
        save(machine_bg(), out, "machine")
        save(restaurant_bg(), out, "restaurant")
        save(gear_frames(15, 10, (104, 98, 128)), out, "engrenage")
        save(gear_frames(9, 8, (92, 86, 114)), out, "engrenage_petit")
        save(stamp_frames(), out, "tampon")
        save(plush_frames(), out, "peluches")
        save(mug(), out, "tasse")
        save(colossus_frames(), out, "ft_printf")
    print("dessins écrits dans", out)


if __name__ == "__main__":
    main()

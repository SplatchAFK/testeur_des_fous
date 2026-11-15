#!/usr/bin/env python3
"""
Le testeur des fous : décors du Puits, de l'Épreuve du Silence et des deux fins,
dans le même pixel art que la Tour de la Libft et la Fonderie de ft_printf.

    python3 dessins_cite.py <racine du testeur>

Écrit les PNG dans modules/gnl/assets, modules/exam/assets et modules/printf/assets.
"""
import math
import os
import random
import sys

from PIL import ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dessins import (W, H, new, mix, vgrad, put, blend, glow, outline, sheet, sky, stone,
                     wall_bricks, dither_pick)


def vignette(img, k=1.2, d0=0.7):
    for y in range(img.height):
        for x in range(img.width):
            d = math.hypot((x - img.width / 2) / (img.width / 2), (y - img.height / 2) / (img.height / 2))
            if d > d0:
                blend(img, x, y, (0, 0, 0), min(0.85, (d - d0) * k))


def puits_bg():
    th = 250
    img = new(W, th)
    sky(img, (0, 0, W, 34), [(10, 8, 30), (24, 18, 56), (34, 30, 80)], seed=51, stars=40)
    d = ImageDraw.Draw(img)
    for x in range(W):
        y = int(26 + math.sin(x / 11) * 2)
        for yy in range(y, 34):
            put(img, x, yy, (26, 30, 60))
    stone(img, (0, 34, W, th), (70, 76, 96), seed=53, light_right=False)
    for y in range(34, th):
        for x in range(W):
            t = abs(x - W / 2) / (W / 2)
            depth = (y - 34) / (th - 34)
            blend(img, x, y, (4, 8, 16), 0.15 + 0.55 * (1 - t) ** 1.6 + 0.3 * depth)
    for side in (0, 1):
        x0 = 26 if side == 0 else 108
        for y in range(34, th):
            for x in range(x0, x0 + 6):
                put(img, x, y, dither_pick([(40, 46, 62), (70, 78, 98), (104, 112, 132)],
                                            (x - x0) / 6 if side else 1 - (x - x0) / 6, x, y))
            put(img, x0 + (6 if side == 0 else -1), y, (16, 18, 28))
    d.rectangle([16, 30, 124, 35], fill=(96, 84, 70, 255))
    d.line([(16, 30), (124, 30)], fill=(150, 130, 104, 255))
    d.rectangle([68, 18, 71, 30], fill=(96, 84, 70, 255))
    d.rectangle([22, 22, 118, 24], fill=(110, 96, 78, 255))
    d.line([(22, 22), (118, 22)], fill=(160, 140, 110, 255))
    d.rectangle([20, 22, 22, 35], fill=(96, 84, 70, 255))
    d.rectangle([118, 22, 120, 35], fill=(96, 84, 70, 255))
    vgrad(img, (32, th - 14, 108, th), [(30, 96, 110), (16, 60, 76), (8, 30, 44)])
    for y in range(th - 14, th):
        for x in range(32, 108):
            if (x + y * 3) % 11 == 0:
                blend(img, x, y, (120, 220, 230), 0.45)
    d.rectangle([108, th - 30, 126, th - 12], fill=(8, 10, 16, 255))
    d.ellipse([108, th - 38, 126, th - 22], fill=(8, 10, 16, 255))
    for i, (px, py) in enumerate(((96, th - 16), (101, th - 18), (106, th - 16), (111, th - 19))):
        put(img, px, py, (176, 168, 150))
        put(img, px + 1, py, (176, 168, 150))
    return img


def couloir_bg():
    img = new(W, H)
    vgrad(img, (0, 0, W, H), [(10, 12, 22), (18, 20, 34)])
    vx, vy = 70, 36
    for y in range(H):
        for x in range(W):
            if y < vy - 22 + abs(x - vx) * 0.0:
                pass
    stone(img, (0, 0, 46, H), (62, 66, 84), seed=61, light_right=True)
    stone(img, (94, 0, W, H), (62, 66, 84), seed=62, light_right=False)
    stone(img, (46, 8, 94, 60), (52, 56, 74), seed=63)
    for y in range(H):
        for x in range(W):
            if x < 46:
                top = 8 * x / 46
                bot = 60 + (H - 60) * (1 - x / 46)
                if y < top:
                    put(img, x, y, (8, 8, 16))
                elif y > bot:
                    put(img, x, y, dither_pick([(30, 32, 46), (46, 48, 64)], (x / 46), x, y))
                else:
                    blend(img, x, y, (6, 6, 14), 0.5 * (1 - x / 46))
            elif x >= 94:
                u = (W - 1 - x) / 46
                top = 8 * u
                bot = 60 + (H - 60) * (1 - u)
                if y < top:
                    put(img, x, y, (8, 8, 16))
                elif y > bot:
                    put(img, x, y, dither_pick([(30, 32, 46), (46, 48, 64)], u, x, y))
                else:
                    blend(img, x, y, (6, 6, 14), 0.5 * (1 - u))
            elif y < 8:
                put(img, x, y, (8, 8, 16))
            elif y >= 60:
                put(img, x, y, dither_pick([(28, 30, 44), (44, 46, 62)], (y - 60) / (H - 60), x, y))
    d = ImageDraw.Draw(img)
    for i in range(1, 7):
        y = 60 + i * i * 0.65
        d.line([(int(46 * (1 - (y - 60) / (H - 60))), int(y)), (int(W - 1 - 46 * (1 - (y - 60) / (H - 60))), int(y))],
               fill=(22, 24, 36, 255))
    d.rectangle([60, 28, 80, 60], fill=(6, 6, 12, 255))
    d.ellipse([60, 18, 80, 38], fill=(6, 6, 12, 255))
    for a in range(0, 181, 15):
        r = math.radians(a)
        put(img, int(70 + math.cos(r) * 11), int(28 - math.sin(r) * 11), (96, 100, 120))
    d.rectangle([58, 59, 82, 60], fill=(70, 74, 92, 255))
    for x in (52, 88):
        d.rectangle([x - 1, 30, x + 1, 32], fill=(50, 52, 64, 255))
    vignette(img)
    return img


def portes_bg():
    img = new(W, H)
    stone(img, (0, 0, W, 62), (60, 64, 82), seed=71, light_right=False)
    for y in range(62):
        for x in range(W):
            blend(img, x, y, (6, 6, 14), 0.35 * (1 - y / 62) + 0.2 * abs(x - W / 2) / (W / 2))
    vgrad(img, (0, 62, W, H), [(46, 48, 64), (34, 36, 50), (22, 24, 34)])
    d = ImageDraw.Draw(img)
    for y in (62, 66, 72, 80):
        d.line([(0, y), (W, y)], fill=(18, 20, 30, 255))
    for i in range(-8, 9):
        d.line([(70 + i * 10, 62), (70 + i * 22, H)], fill=(18, 20, 30, 255))
    for cx in (25, 55, 85, 115):
        d.rectangle([cx - 11, 26, cx + 11, 62], fill=(84, 88, 108, 255))
        d.ellipse([cx - 11, 14, cx + 11, 38], fill=(84, 88, 108, 255))
        d.rectangle([cx - 9, 27, cx + 9, 61], fill=(30, 26, 26, 255))
        d.ellipse([cx - 9, 16, cx + 9, 36], fill=(30, 26, 26, 255))
        for y in range(18, 62):
            for x in range(cx - 8, cx + 9):
                p = img.getpixel((x, y))
                if p[:3] == (30, 26, 26):
                    plank = (x - cx + 8) // 4
                    c = dither_pick([(54, 40, 32), (82, 60, 44), (104, 78, 56)], 0.3 + 0.4 * ((plank % 2) * 0.5) +
                                    0.3 * (1 - (y - 18) / 44), x, y)
                    if (x - cx + 8) % 4 == 0:
                        c = (40, 30, 24, 255)
                    put(img, x, y, c)
        for yy in (32, 50):
            d.line([(cx - 8, yy), (cx + 8, yy)], fill=(60, 60, 70, 255))
            for xx in (cx - 6, cx + 6):
                put(img, xx, yy, (140, 140, 150))
        put(img, cx + 5, 42, (200, 180, 90))
        put(img, cx + 5, 43, (150, 130, 60))
        for k in range(3):
            put(img, cx - 2 + k * 2, 10, (110, 220, 230) if False else (70, 76, 92))
    vignette(img, 1.0, 0.8)
    return img


def mecanisme_bg():
    img = new(W, H)
    stone(img, (0, 0, W, 64), (52, 60, 78), seed=81, light_right=False)
    for y in range(64):
        for x in range(W):
            blend(img, x, y, (4, 8, 16), 0.45 * (1 - y / 64) + 0.25 * abs(x - W / 2) / (W / 2))
    vgrad(img, (0, 64, W, H), [(40, 46, 60), (28, 32, 44), (16, 18, 28)])
    d = ImageDraw.Draw(img)
    for y in (64, 68, 74, 82):
        d.line([(0, y), (W, y)], fill=(14, 16, 26, 255))
    for x in (30, 110):
        d.rectangle([x - 2, 6, x + 2, 64], fill=(70, 50, 34, 255))
        d.line([(x - 2, 6), (x - 2, 64)], fill=(110, 80, 54, 255))
    d.rectangle([28, 8, 112, 11], fill=(70, 50, 34, 255))
    d.line([(28, 8), (112, 8)], fill=(110, 80, 54, 255))
    for x in range(20, W - 10, 18):
        put(img, x, 2, (110, 220, 230))
        glow(img, x, 2, 4, (110, 220, 230), 0.3)
    vignette(img, 1.0, 0.75)
    return img


def cite_bg():
    img = new(W, H)
    sky(img, (0, 0, W, H), [(10, 8, 34), (30, 22, 70), (60, 40, 100), (110, 70, 120)], seed=91, stars=90,
        moon=(112, 14, 6))
    d = ImageDraw.Draw(img)
    random.seed(93)
    x = 0
    while x < W:
        w = random.randint(8, 16)
        h = random.randint(14, 34)
        top = 60 - h
        col = (34, 26, 60)
        d.rectangle([x, top, x + w, 64], fill=col + (255,))
        if random.random() < 0.5:
            d.polygon([(x, top), (x + w // 2, top - random.randint(4, 8)), (x + w, top)], fill=col + (255,))
        for wy in range(top + 3, 58, 5):
            for wx in range(x + 2, x + w - 1, 4):
                put(img, wx, wy, (20, 16, 36))
        x += w + random.randint(0, 3)
    d.rectangle([60, 18, 80, 64], fill=(40, 30, 70, 255))
    d.polygon([(58, 18), (70, 4), (82, 18)], fill=(40, 30, 70, 255))
    stone(img, (0, 62, W, H), (104, 96, 132), seed=95)
    for i in range(0, W, 16):
        stone(img, (i, 56, i + 9, 62), (104, 96, 132), seed=96 + i)
    d.line([(0, 62), (W, 62)], fill=(150, 140, 176, 255))
    return img


def lanterne_frames():
    out = []
    for k in range(2):
        img = new(9, 13)
        d = ImageDraw.Draw(img)
        d.ellipse([3, 0, 5, 2], outline=(150, 150, 160, 255))
        d.rectangle([1, 3, 7, 4], fill=(70, 74, 90, 255))
        d.rectangle([1, 11, 7, 12], fill=(70, 74, 90, 255))
        d.rectangle([2, 5, 6, 10], fill=(150, 236, 244, 255) if k else (40, 52, 64, 255))
        if k:
            d.rectangle([3, 6, 5, 9], fill=(230, 255, 255, 255))
        d.line([(1, 5), (1, 10)], fill=(70, 74, 90, 255))
        d.line([(7, 5), (7, 10)], fill=(70, 74, 90, 255))
        out.append(outline(img, (14, 16, 24, 255)))
    return sheet(out)


def main():
    r = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
    g = os.path.join(r, "modules", "gnl", "assets")
    e = os.path.join(r, "modules", "exam", "assets")
    p = os.path.join(r, "modules", "printf", "assets")
    puits_bg().save(os.path.join(g, "puits_pixel.png"))
    mecanisme_bg().save(os.path.join(g, "mecanisme.png"))
    lanterne_frames().save(os.path.join(g, "lanterne.png"))
    couloir_bg().save(os.path.join(e, "epreuve_couloir.png"))
    portes_bg().save(os.path.join(e, "epreuve_portes.png"))
    cite_bg().save(os.path.join(p, "cite_nuit.png"))
    print("décors écrits")


if __name__ == "__main__":
    main()

import io
import re
import sys
import tokenize


def lire(chemin):
    with io.open(chemin, encoding="utf-8", newline="") as f:
        return f.read()


def ecrire(chemin, texte):
    with io.open(chemin, "w", encoding="utf-8", newline="") as f:
        f.write(texte)


def nettoyer_lignes(avant, apres):
    lignes_avant = avant.split("\n")
    lignes_apres = apres.split("\n")
    sortie = []
    for i, ligne in enumerate(lignes_apres):
        ligne = ligne.rstrip(" \t")
        origine = lignes_avant[i] if i < len(lignes_avant) else ""
        if ligne == "" and origine.strip() != "":
            continue
        sortie.append(ligne)
    texte = "\n".join(sortie)
    texte = re.sub(r"\n{3,}", "\n\n", texte)
    return texte.lstrip("\n")


def sans_c(src):
    out = []
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        d = src[i + 1] if i + 1 < n else ""
        if c == '"' or c == "'":
            j = i + 1
            while j < n and src[j] != c:
                if src[j] == "\\":
                    j += 1
                j += 1
            out.append(src[i:j + 1])
            i = j + 1
        elif c == "/" and d == "/":
            while i < n and src[i] != "\n":
                i += 1
        elif c == "/" and d == "*":
            j = src.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * src.count("\n", i, j))
            if out and i > 0 and src[i - 1] not in " \t\n" and j < n and src[j] not in " \t\n":
                out.append(" ")
            i = j
        else:
            out.append(c)
            i += 1
    return nettoyer_lignes(src, "".join(out))


HEREDOC = re.compile(r"<<-?\s*(['\"]?)([A-Za-z_][A-Za-z0-9_]*)\1")


def sans_sh(src):
    lignes = src.split("\n")
    sortie = []
    etat = ""
    attente = []
    fin = None
    tirets = False
    for num, ligne in enumerate(lignes):
        if fin is not None:
            sortie.append(ligne)
            test = ligne.lstrip("\t") if tirets else ligne
            if test == fin:
                if attente:
                    fin, tirets = attente.pop(0)
                else:
                    fin = None
            continue
        if num == 0 and ligne.startswith("#!"):
            sortie.append(ligne)
            continue
        i = 0
        n = len(ligne)
        coupe = None
        ansi = False
        while i < n:
            c = ligne[i]
            if etat == "'":
                if ansi and c == "\\":
                    i += 2
                    continue
                if c == "'":
                    etat = ""
                    ansi = False
                i += 1
                continue
            if etat == '"':
                if c == "\\":
                    i += 2
                    continue
                if c == '"':
                    etat = ""
                i += 1
                continue
            if c == "\\":
                i += 2
                continue
            if c == "'":
                etat = "'"
                ansi = i > 0 and ligne[i - 1] == "$"
                i += 1
                continue
            if c == '"':
                etat = '"'
                i += 1
                continue
            if c == "#" and (i == 0 or ligne[i - 1] in " \t;"):
                coupe = i
                break
            if c == "<" and ligne.startswith("<<", i) and not ligne.startswith("<<<", i):
                m = HEREDOC.match(ligne, i)
                if m:
                    attente.append((m.group(2), ligne.startswith("<<-", i)))
                    i = m.end()
                    continue
            i += 1
        if coupe is not None:
            reste = ligne[:coupe].rstrip(" \t")
            if reste.strip() != "":
                sortie.append(reste)
            else:
                sortie.append("\0")
        else:
            sortie.append(ligne)
        if attente and etat == "":
            fin, tirets = attente.pop(0)
    texte = "\n".join(l for l in sortie if l != "\0")
    texte = re.sub(r"\n{3,}", "\n\n", texte)
    return texte


def sans_py(src):
    jetons = []
    for tok in tokenize.generate_tokens(io.StringIO(src).readline):
        if tok.type == tokenize.COMMENT:
            continue
        jetons.append(tok)
    lignes = src.split("\n")
    garder = {}
    for tok in tokenize.generate_tokens(io.StringIO(src).readline):
        if tok.type == tokenize.COMMENT:
            ligne, col = tok.start
            if ligne == 1 and tok.string.startswith("#!"):
                continue
            garder[ligne] = col
    sortie = []
    for num, ligne in enumerate(lignes, 1):
        if num in garder:
            reste = ligne[:garder[num]].rstrip(" \t")
            if reste.strip() == "":
                continue
            sortie.append(reste)
        else:
            sortie.append(ligne)
    texte = "\n".join(sortie)
    return re.sub(r"\n{4,}", "\n\n\n", texte)


def main():
    for chemin in sys.argv[1:]:
        src = lire(chemin)
        if chemin.endswith((".c", ".h")):
            neuf = sans_c(src)
        elif chemin.endswith(".sh"):
            neuf = sans_sh(src)
        elif chemin.endswith(".py"):
            neuf = sans_py(src)
        else:
            continue
        if neuf != src:
            ecrire(chemin, neuf)
            print("nettoyé", chemin)


main()

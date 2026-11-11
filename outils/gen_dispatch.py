#!/usr/bin/env python3
"""Régénère src/tdf_dispatch.c : appelle printf/ft_printf avec 0 à 4 arguments
typés (int, unsigned int, char *, void *) sans comportement indéfini de type."""
import itertools, os

K = ['v[%d].i', 'v[%d].u', 'v[%d].s', 'v[%d].p']
out = ['/* Le testeur des fous · version printf - généré par tools/gen_dispatch.py : NE PAS ÉDITER */',
       '#include "tdf.h"', '',
       'static int\td0(t_pf f, const char *s)\n{\n\treturn (f(s));\n}', '']
for n in range(1, 5):
    out.append('static int\td%d(t_pf f, const char *s, int id, const t_val *v)\n{\n\tswitch (id)\n\t{' % n)
    for combo in itertools.product(range(4), repeat=n):
        idv = 0
        for i in reversed(range(n)):
            idv = idv * 4 + combo[i]
        args = ', '.join(K[k] % i for i, k in enumerate(combo))
        out.append('\t\tcase %d: return (f(s, %s));' % (idv, args))
    out.append('\t}\n\treturn (-12345);\n}\n')
out.append('''int	tdf_dispatch(t_pf fn, const char *fmt, int n, const int *k, const t_val *v)
{
	int	id;
	int	i;

	id = 0;
	i = n;
	while (--i >= 0)
		id = id * 4 + k[i];
	if (n == 1)
		return (d1(fn, fmt, id, v));
	if (n == 2)
		return (d2(fn, fmt, id, v));
	if (n == 3)
		return (d3(fn, fmt, id, v));
	if (n == 4)
		return (d4(fn, fmt, id, v));
	return (d0(fn, fmt));
}
''')
dst = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'tdf_dispatch.c')
open(dst, 'w').write('\n'.join(out))

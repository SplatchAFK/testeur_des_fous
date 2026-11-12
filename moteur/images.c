#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

typedef struct s_img
{
	int				w;
	int				h;
	unsigned char	*px;
	unsigned char	*a;
}	t_img;

static unsigned char	*read_all(const char *path, size_t *n)
{
	FILE			*f;
	unsigned char	*b;
	long			sz;

	f = fopen(path, "rb");
	if (!f)
		return (NULL);
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (sz <= 0 || sz > 40 * 1024 * 1024)
	{
		fclose(f);
		return (NULL);
	}
	b = malloc(sz);
	if (b && fread(b, 1, sz, f) != (size_t)sz)
	{
		free(b);
		b = NULL;
	}
	fclose(f);
	*n = sz;
	return (b);
}

typedef struct s_bits
{
	const unsigned char	*p;
	size_t				n;
	size_t				pos;
	unsigned int		bit;
	int					bad;
}	t_bits;

typedef struct s_huff
{
	unsigned short	count[16];
	unsigned short	sym[320];
}	t_huff;

static int	getbit(t_bits *b)
{
	int	v;

	if (b->pos >= b->n)
	{
		b->bad = 1;
		return (0);
	}
	v = (b->p[b->pos] >> b->bit) & 1;
	if (++b->bit == 8)
	{
		b->bit = 0;
		b->pos++;
	}
	return (v);
}

static int	getbits(t_bits *b, int n)
{
	int	v;
	int	i;

	v = 0;
	for (i = 0; i < n; i++)
		v |= getbit(b) << i;
	return (v);
}

static void	huff_build(t_huff *h, const unsigned char *len, int n)
{
	unsigned short	offs[16];
	int				i;

	memset(h->count, 0, sizeof(h->count));
	for (i = 0; i < n; i++)
		h->count[len[i]]++;
	h->count[0] = 0;
	offs[1] = 0;
	for (i = 1; i < 15; i++)
		offs[i + 1] = offs[i] + h->count[i];
	for (i = 0; i < n; i++)
		if (len[i])
			h->sym[offs[len[i]]++] = i;
}

static int	huff_dec(t_bits *b, const t_huff *h)
{
	int	code;
	int	first;
	int	index;
	int	len;
	int	count;

	code = 0;
	first = 0;
	index = 0;
	for (len = 1; len < 16; len++)
	{
		code |= getbit(b);
		count = h->count[len];
		if (code - count < first)
			return (h->sym[index + (code - first)]);
		index += count;
		first += count;
		first <<= 1;
		code <<= 1;
		if (b->bad)
			return (-1);
	}
	b->bad = 1;
	return (-1);
}

static const unsigned short	g_lbase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const unsigned short	g_lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3,
	3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const unsigned short	g_dbase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129,
	193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const unsigned short	g_dext[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
	8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

static int	codes(t_bits *b, unsigned char *out, size_t cap, size_t *o, t_huff *lt, t_huff *dt)
{
	int		s;
	int		len;
	int		dist;

	while (!b->bad)
	{
		s = huff_dec(b, lt);
		if (s < 0)
			return (-1);
		if (s < 256)
		{
			if (*o >= cap)
				return (-1);
			out[(*o)++] = s;
		}
		else if (s == 256)
			return (0);
		else
		{
			s -= 257;
			if (s >= 29)
				return (-1);
			len = g_lbase[s] + getbits(b, g_lext[s]);
			s = huff_dec(b, dt);
			if (s < 0 || s >= 30)
				return (-1);
			dist = g_dbase[s] + getbits(b, g_dext[s]);
			if ((size_t)dist > *o || *o + len > cap)
				return (-1);
			while (len--)
			{
				out[*o] = out[*o - dist];
				(*o)++;
			}
		}
	}
	return (-1);
}

static int	dynamic(t_bits *b, t_huff *lt, t_huff *dt)
{
	static const unsigned char	ord[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	unsigned char				len[320];
	t_huff						ct;
	int							nl;
	int							nd;
	int							nc;
	int							i;
	int							s;
	int							r;

	nl = getbits(b, 5) + 257;
	nd = getbits(b, 5) + 1;
	nc = getbits(b, 4) + 4;
	memset(len, 0, sizeof(len));
	for (i = 0; i < nc; i++)
		len[ord[i]] = getbits(b, 3);
	huff_build(&ct, len, 19);
	memset(len, 0, sizeof(len));
	i = 0;
	while (i < nl + nd && !b->bad)
	{
		s = huff_dec(b, &ct);
		if (s < 0)
			return (-1);
		if (s < 16)
			len[i++] = s;
		else
		{
			r = s == 16 ? 3 + getbits(b, 2) : (s == 17 ? 3 + getbits(b, 3) : 11 + getbits(b, 7));
			if ((s == 16 && i == 0) || i + r > nl + nd)
				return (-1);
			while (r--)
			{
				len[i] = s == 16 ? len[i - 1] : 0;
				i++;
			}
		}
	}
	huff_build(lt, len, nl);
	huff_build(dt, len + nl, nd);
	return (b->bad ? -1 : 0);
}

static int	inflate_z(const unsigned char *in, size_t n, unsigned char *out, size_t cap, size_t *got)
{
	t_bits			b;
	t_huff			lt;
	t_huff			dt;
	unsigned char	len[320];
	int				last;
	int				type;
	int				i;
	size_t			o;
	unsigned int	l;

	if (n < 2)
		return (-1);
	memset(&b, 0, sizeof(b));
	b.p = in + 2;
	b.n = n - 2;
	o = 0;
	do
	{
		last = getbit(&b);
		type = getbits(&b, 2);
		if (type == 0)
		{
			if (b.bit)
			{
				b.bit = 0;
				b.pos++;
			}
			if (b.pos + 4 > b.n)
				return (-1);
			l = b.p[b.pos] | (b.p[b.pos + 1] << 8);
			b.pos += 4;
			if (b.pos + l > b.n || o + l > cap)
				return (-1);
			memcpy(out + o, b.p + b.pos, l);
			o += l;
			b.pos += l;
		}
		else if (type == 1)
		{
			for (i = 0; i < 144; i++)
				len[i] = 8;
			for (; i < 256; i++)
				len[i] = 9;
			for (; i < 280; i++)
				len[i] = 7;
			for (; i < 288; i++)
				len[i] = 8;
			huff_build(&lt, len, 288);
			for (i = 0; i < 30; i++)
				len[i] = 5;
			huff_build(&dt, len, 30);
			if (codes(&b, out, cap, &o, &lt, &dt))
				return (-1);
		}
		else if (type == 2)
		{
			if (dynamic(&b, &lt, &dt) || codes(&b, out, cap, &o, &lt, &dt))
				return (-1);
		}
		else
			return (-1);
	} while (!last && !b.bad);
	*got = o;
	return (b.bad ? -1 : 0);
}

static unsigned int	be32(const unsigned char *p)
{
	return ((unsigned int)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
}

static int	paeth(int a, int b, int c)
{
	int	p;
	int	pa;
	int	pb;
	int	pc;

	p = a + b - c;
	pa = abs(p - a);
	pb = abs(p - b);
	pc = abs(p - c);
	if (pa <= pb && pa <= pc)
		return (a);
	return (pb <= pc ? b : c);
}

static int	png_decode(const unsigned char *d, size_t n, t_img *im, const char **err)
{
	unsigned char	*idat;
	unsigned char	*raw;
	unsigned char	pal[768];
	unsigned char	trns[256];
	size_t			ni;
	size_t			pos;
	size_t			got;
	unsigned int	len;
	int				ct;
	int				bd;
	int				ch;
	int				bpp;
	size_t			stride;
	int				x;
	int				y;
	int				i;
	unsigned char	*cur;
	unsigned char	*prv;
	int				v;

	idat = malloc(n);
	ni = 0;
	pos = 8;
	ct = -1;
	bd = 0;
	memset(pal, 0, sizeof(pal));
	memset(trns, 255, sizeof(trns));
	while (idat && pos + 8 <= n)
	{
		len = be32(d + pos);
		if (pos + 12 + len > n)
			break ;
		if (!memcmp(d + pos + 4, "IHDR", 4) && len >= 13)
		{
			im->w = be32(d + pos + 8);
			im->h = be32(d + pos + 12);
			bd = d[pos + 16];
			ct = d[pos + 17];
			if (d[pos + 20])
				return (free(idat), *err = "PNG entrelacé (Adam7) non pris en charge", -1);
		}
		else if (!memcmp(d + pos + 4, "PLTE", 4))
			memcpy(pal, d + pos + 8, len < 768 ? len : 768);
		else if (!memcmp(d + pos + 4, "tRNS", 4))
			memcpy(trns, d + pos + 8, len < 256 ? len : 256);
		else if (!memcmp(d + pos + 4, "IDAT", 4))
		{
			memcpy(idat + ni, d + pos + 8, len);
			ni += len;
		}
		else if (!memcmp(d + pos + 4, "IEND", 4))
			break ;
		pos += 12 + len;
	}
	ch = ct == 0 ? 1 : (ct == 2 ? 3 : (ct == 3 ? 1 : (ct == 4 ? 2 : (ct == 6 ? 4 : 0))));
	if (!idat || !ch || im->w <= 0 || im->h <= 0 || im->w > 8000 || im->h > 8000
		|| (bd != 8 && !(ct == 3 && (bd == 1 || bd == 2 || bd == 4)) && !(ct == 0 && bd < 8)))
		return (free(idat), *err = "format PNG non pris en charge (8 bits ou palette)", -1);
	bpp = ch * bd;
	stride = ((size_t)im->w * bpp + 7) / 8;
	raw = malloc((stride + 1) * im->h);
	if (!raw || inflate_z(idat, ni, raw, (stride + 1) * im->h, &got) || got < (stride + 1) * im->h)
		return (free(idat), free(raw), *err = "données PNG illisibles", -1);
	free(idat);
	im->px = malloc((size_t)im->w * im->h * 3);
	im->a = malloc((size_t)im->w * im->h);
	bpp = (bpp + 7) / 8;
	for (y = 0; y < im->h; y++)
	{
		cur = raw + y * (stride + 1) + 1;
		prv = y ? raw + (y - 1) * (stride + 1) + 1 : NULL;
		for (i = 0; i < (int)stride; i++)
		{
			int a = i >= bpp ? cur[i - bpp] : 0;
			int b = prv ? prv[i] : 0;
			int c = (prv && i >= bpp) ? prv[i - bpp] : 0;
			v = cur[-1];
			if (v == 1)
				cur[i] += a;
			else if (v == 2)
				cur[i] += b;
			else if (v == 3)
				cur[i] += (a + b) / 2;
			else if (v == 4)
				cur[i] += paeth(a, b, c);
		}
		for (x = 0; x < im->w; x++)
		{
			unsigned char	*o = im->px + ((size_t)y * im->w + x) * 3;
			unsigned char	*al = im->a + (size_t)y * im->w + x;
			int				g;

			*al = 255;

			if (ct == 3 || (ct == 0 && bd < 8))
			{
				g = (cur[x * bd / 8] >> (8 - bd - (x * bd) % 8)) & ((1 << bd) - 1);
				if (ct == 3)
				{
					memcpy(o, pal + g * 3, 3);
					*al = trns[g];
				}
				else
					memset(o, g * 255 / ((1 << bd) - 1), 3);
			}
			else if (ch <= 2)
			{
				memset(o, cur[x * ch], 3);
				if (ch == 2)
					*al = cur[x * 2 + 1];
			}
			else
			{
				memcpy(o, cur + x * ch, 3);
				if (ch == 4)
					*al = cur[x * 4 + 3];
				if (ch == 4 && cur[x * 4 + 3] < 128)
					memset(o, 0, 3);
			}
		}
	}
	free(raw);
	return (0);
}

typedef struct s_jh
{
	unsigned char	bits[17];
	unsigned char	val[256];
	int				mincode[17];
	int				maxcode[18];
	int				valptr[17];
}	t_jh;

typedef struct s_jcomp
{
	int		id;
	int		h;
	int		v;
	int		tq;
	int		td;
	int		ta;
	int		dc;
	float	*plane;
	int		pw;
	int		ph;
}	t_jcomp;

typedef struct s_jpg
{
	const unsigned char	*d;
	size_t				n;
	size_t				pos;
	unsigned int		buf;
	int					cnt;
	unsigned short		q[4][64];
	t_jh				hdc[4];
	t_jh				hac[4];
	t_jcomp				c[3];
	int					nc;
	int					w;
	int					h;
	int					hmax;
	int					vmax;
	int					restart;
}	t_jpg;

static const unsigned char	g_zz[64] = {0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
	19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22,
	15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

static void	jh_build(t_jh *h)
{
	int	code;
	int	k;
	int	l;

	code = 0;
	k = 0;
	for (l = 1; l <= 16; l++)
	{
		h->valptr[l] = k;
		h->mincode[l] = code;
		code += h->bits[l];
		k += h->bits[l];
		h->maxcode[l] = h->bits[l] ? code - 1 : -1;
		code <<= 1;
	}
	h->maxcode[17] = 0x7fffffff;
}

static int	jbit(t_jpg *j)
{
	int	b;

	if (j->cnt == 0)
	{
		if (j->pos >= j->n)
			return (0);
		b = j->d[j->pos++];
		if (b == 0xff)
		{
			if (j->pos < j->n && j->d[j->pos] == 0)
				j->pos++;
		}
		j->buf = b;
		j->cnt = 8;
	}
	j->cnt--;
	return ((j->buf >> j->cnt) & 1);
}

static int	jbits(t_jpg *j, int n)
{
	int	v;

	v = 0;
	while (n--)
		v = (v << 1) | jbit(j);
	return (v);
}

static int	jdec(t_jpg *j, t_jh *h)
{
	int	code;
	int	l;

	code = jbit(j);
	for (l = 1; l <= 16; l++)
	{
		if (h->maxcode[l] >= 0 && code <= h->maxcode[l] && code >= h->mincode[l])
			return (h->val[h->valptr[l] + code - h->mincode[l]]);
		code = (code << 1) | jbit(j);
	}
	return (0);
}

static int	extend(int v, int t)
{
	return (t && v < (1 << (t - 1)) ? v - (1 << t) + 1 : v);
}

static void	idct(const float *in, float *out)
{
	static float	ct[8][8];
	static int		ready;
	float			tmp[64];
	float			s;
	int				x;
	int				u;
	int				y;

	if (!ready)
	{
		for (x = 0; x < 8; x++)
			for (u = 0; u < 8; u++)
				ct[x][u] = (u ? 1.0f : 0.70710678f) * cosf((2 * x + 1) * u * 3.14159265f / 16) / 2;
		ready = 1;
	}
	for (y = 0; y < 8; y++)
		for (x = 0; x < 8; x++)
		{
			for (s = 0, u = 0; u < 8; u++)
				s += ct[x][u] * in[y * 8 + u];
			tmp[y * 8 + x] = s;
		}
	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
		{
			for (s = 0, u = 0; u < 8; u++)
				s += ct[y][u] * tmp[u * 8 + x];
			out[y * 8 + x] = s + 128;
		}
}

static void	jblock(t_jpg *j, t_jcomp *c, int bx, int by)
{
	float	co[64];
	float	px[64];
	int		k;
	int		t;
	int		r;
	int		s;
	int		x;
	int		y;

	memset(co, 0, sizeof(co));
	t = jdec(j, &j->hdc[c->td]);
	c->dc += extend(jbits(j, t), t);
	co[0] = c->dc * (float)j->q[c->tq][0];
	for (k = 1; k < 64; k++)
	{
		t = jdec(j, &j->hac[c->ta]);
		r = t >> 4;
		s = t & 15;
		if (!s)
		{
			if (r != 15)
				break ;
			k += 15;
			continue ;
		}
		k += r;
		if (k > 63)
			break ;
		co[g_zz[k]] = extend(jbits(j, s), s) * (float)j->q[c->tq][k];
	}
	idct(co, px);
	for (y = 0; y < 8; y++)
		for (x = 0; x < 8; x++)
			if (bx + x < c->pw && by + y < c->ph)
				c->plane[(by + y) * c->pw + bx + x] = px[y * 8 + x];
}

static int	jpg_scan(t_jpg *j)
{
	int	mx;
	int	my;
	int	i;
	int	bx;
	int	by;
	int	mcus;
	int	m;

	mx = (j->w + 8 * j->hmax - 1) / (8 * j->hmax);
	my = (j->h + 8 * j->vmax - 1) / (8 * j->vmax);
	mcus = 0;
	for (m = 0; m < mx * my; m++)
	{
		if (j->restart && mcus == j->restart)
		{
			j->cnt = 0;
			while (j->pos + 1 < j->n && !(j->d[j->pos] == 0xff && j->d[j->pos + 1] >= 0xd0
					&& j->d[j->pos + 1] <= 0xd7))
				j->pos++;
			j->pos += 2;
			for (i = 0; i < j->nc; i++)
				j->c[i].dc = 0;
			mcus = 0;
		}
		for (i = 0; i < j->nc; i++)
			for (by = 0; by < j->c[i].v; by++)
				for (bx = 0; bx < j->c[i].h; bx++)
					jblock(j, &j->c[i], ((m % mx) * j->c[i].h + bx) * 8, ((m / mx) * j->c[i].v + by) * 8);
		mcus++;
		if (j->pos >= j->n)
			break ;
	}
	return (0);
}

static int	jpg_decode(const unsigned char *d, size_t n, t_img *im, const char **err)
{
	t_jpg	j;
	size_t	len;
	int		mk;
	int		i;
	int		k;
	int		x;
	int		y;

	memset(&j, 0, sizeof(j));
	j.d = d;
	j.n = n;
	j.pos = 2;
	while (j.pos + 4 <= n)
	{
		if (d[j.pos] != 0xff)
		{
			j.pos++;
			continue ;
		}
		mk = d[j.pos + 1];
		len = (d[j.pos + 2] << 8) | d[j.pos + 3];
		if (mk == 0xc2 || mk == 0xc1 || mk == 0xc3 || (mk >= 0xc5 && mk <= 0xcf && mk != 0xc8 && mk != 0xcc))
			return (*err = "JPEG progressif ou non standard : enregistre-le en JPEG classique ou en PNG", -1);
		if (mk == 0xdb)
			for (i = j.pos + 4; i < (int)(j.pos + 2 + len); i += 65)
				for (k = 0; k < 64; k++)
					j.q[d[i] & 3][k] = d[i + 1 + k];
		else if (mk == 0xc4)
			for (i = j.pos + 4; i < (int)(j.pos + 2 + len);)
			{
				t_jh	*h = (d[i] >> 4) ? &j.hac[d[i] & 3] : &j.hdc[d[i] & 3];
				int		tot = 0;

				for (k = 1; k <= 16; k++)
					tot += (h->bits[k] = d[i + k]);
				memcpy(h->val, d + i + 17, tot < 256 ? tot : 256);
				jh_build(h);
				i += 17 + tot;
			}
		else if (mk == 0xc0)
		{
			j.h = (d[j.pos + 5] << 8) | d[j.pos + 6];
			j.w = (d[j.pos + 7] << 8) | d[j.pos + 8];
			j.nc = d[j.pos + 9];
			if (j.nc != 1 && j.nc != 3)
				return (*err = "JPEG : seules les images en gris ou en couleurs (3 composantes) sont prises en charge", -1);
			for (i = 0; i < j.nc; i++)
			{
				j.c[i].id = d[j.pos + 10 + i * 3];
				j.c[i].h = d[j.pos + 11 + i * 3] >> 4;
				j.c[i].v = d[j.pos + 11 + i * 3] & 15;
				j.c[i].tq = d[j.pos + 12 + i * 3] & 3;
				if (j.c[i].h > j.hmax)
					j.hmax = j.c[i].h;
				if (j.c[i].v > j.vmax)
					j.vmax = j.c[i].v;
			}
		}
		else if (mk == 0xdd)
			j.restart = (d[j.pos + 4] << 8) | d[j.pos + 5];
		else if (mk == 0xda)
		{
			for (i = 0; i < d[j.pos + 4] && i < 3; i++)
				for (k = 0; k < j.nc; k++)
					if (j.c[k].id == d[j.pos + 5 + i * 2])
					{
						j.c[k].td = d[j.pos + 6 + i * 2] >> 4;
						j.c[k].ta = d[j.pos + 6 + i * 2] & 15;
					}
			j.pos += 2 + len;
			break ;
		}
		else if (mk == 0xd9)
			break ;
		j.pos += 2 + len;
	}
	if (!j.w || !j.h || !j.nc || j.w > 8000 || j.h > 8000 || !j.hmax || !j.vmax)
		return (*err = "JPEG illisible", -1);
	for (i = 0; i < j.nc; i++)
	{
		j.c[i].pw = ((j.w + 8 * j.hmax - 1) / (8 * j.hmax)) * j.c[i].h * 8;
		j.c[i].ph = ((j.h + 8 * j.vmax - 1) / (8 * j.vmax)) * j.c[i].v * 8;
		j.c[i].plane = calloc((size_t)j.c[i].pw * j.c[i].ph, sizeof(float));
		if (!j.c[i].plane)
			return (*err = "image trop grande", -1);
	}
	jpg_scan(&j);
	im->w = j.w;
	im->h = j.h;
	im->px = malloc((size_t)j.w * j.h * 3);
	for (y = 0; im->px && y < j.h; y++)
		for (x = 0; x < j.w; x++)
		{
			float	v[3];
			float	rgbv[3];

			for (i = 0; i < j.nc; i++)
				v[i] = j.c[i].plane[(y * j.c[i].v / j.vmax) * j.c[i].pw + x * j.c[i].h / j.hmax];
			if (j.nc == 1)
				rgbv[0] = rgbv[1] = rgbv[2] = v[0];
			else
			{
				rgbv[0] = v[0] + 1.402f * (v[2] - 128);
				rgbv[1] = v[0] - 0.344136f * (v[1] - 128) - 0.714136f * (v[2] - 128);
				rgbv[2] = v[0] + 1.772f * (v[1] - 128);
			}
			for (i = 0; i < 3; i++)
				im->px[((size_t)y * j.w + x) * 3 + i] = rgbv[i] < 0 ? 0 : (rgbv[i] > 255 ? 255 : (int)rgbv[i]);
		}
	for (i = 0; i < j.nc; i++)
		free(j.c[i].plane);
	return (im->px ? 0 : -1);
}

static void	to_hex(const t_img *im, int tw, int th, char *dst)
{
	static const char	*hx = "0123456789abcdef";
	double				cw;
	double				chh;
	double				ox;
	double				oy;
	int					x;
	int					y;
	int					sx;
	int					sy;
	long				acc[3];
	int					nb;

	cw = im->w;
	chh = im->w * (double)th / tw;
	if (chh > im->h)
	{
		chh = im->h;
		cw = im->h * (double)tw / th;
	}
	ox = (im->w - cw) / 2;
	oy = (im->h - chh) / 2;
	for (y = 0; y < th; y++)
		for (x = 0; x < tw; x++)
		{
			memset(acc, 0, sizeof(acc));
			nb = 0;
			for (sy = (int)(oy + y * chh / th); sy < (int)(oy + (y + 1) * chh / th) || sy == (int)(oy + y * chh / th); sy++)
				for (sx = (int)(ox + x * cw / tw); sx < (int)(ox + (x + 1) * cw / tw) || sx == (int)(ox + x * cw / tw); sx++)
				{
					if (sx < 0 || sy < 0 || sx >= im->w || sy >= im->h)
						continue ;
					acc[0] += im->px[((size_t)sy * im->w + sx) * 3];
					acc[1] += im->px[((size_t)sy * im->w + sx) * 3 + 1];
					acc[2] += im->px[((size_t)sy * im->w + sx) * 3 + 2];
					nb++;
				}
			nb = nb ? nb : 1;
			dst[(y * tw + x) * 3] = hx[(acc[0] / nb) >> 4];
			dst[(y * tw + x) * 3 + 1] = hx[(acc[1] / nb) >> 4];
			dst[(y * tw + x) * 3 + 2] = hx[(acc[2] / nb) >> 4];
		}
	dst[tw * th * 3] = '\0';
}

typedef struct s_slot
{
	char	name[48];
	t_pic	pic;
	int		ok;
}	t_slot;

static t_slot	g_pics[96];
static int		g_npics;

const t_pic	*art_pic(const char *name)
{
	char			path[700];
	unsigned char	*d;
	size_t			n;
	t_img			im;
	const char		*err;
	int				i;

	for (i = 0; i < g_npics; i++)
		if (!strcmp(g_pics[i].name, name))
			return (g_pics[i].ok ? &g_pics[i].pic : NULL);
	if (g_npics >= 96)
		return (NULL);
	i = g_npics++;
	snprintf(g_pics[i].name, sizeof(g_pics[i].name), "%s", name);
	snprintf(path, sizeof(path), "%s/assets/%s.png", g_tester_dir, name);
	memset(&im, 0, sizeof(im));
	d = read_all(path, &n);
	if (!d && getenv("TDF_RACINE"))
	{
		snprintf(path, sizeof(path), "%s/moteur/images/%s.png", getenv("TDF_RACINE"), name);
		d = read_all(path, &n);
	}
	if (!d || n < 8 || memcmp(d, "\x89PNG\r\n\x1a\n", 8) || png_decode(d, n, &im, &err))
	{
		free(d);
		free(im.px);
		free(im.a);
		return (NULL);
	}
	free(d);
	g_pics[i].pic.w = im.w;
	g_pics[i].pic.h = im.h;
	g_pics[i].pic.rgb = im.px;
	g_pics[i].pic.a = im.a;
	g_pics[i].ok = 1;
	return (&g_pics[i].pic);
}

void	hd_pic(t_hd *d, const t_pic *p, int fw, int frame, int x, int y, int k, int flip)
{
	int				i;
	int				j;
	int				sx;
	int				sy;
	int				u;
	int				v;
	unsigned char	al;
	unsigned char	*c;

	if (!p)
		return ;
	if (fw <= 0)
		fw = p->w;
	k = k < 1 ? 1 : k;
	for (j = 0; j < p->h; j++)
		for (i = 0; i < fw; i++)
		{
			sx = frame * fw + (flip ? fw - 1 - i : i);
			if (sx < 0 || sx >= p->w)
				continue ;
			al = p->a[(size_t)j * p->w + sx];
			if (al < 16)
				continue ;
			c = p->rgb + ((size_t)j * p->w + sx) * 3;
			for (v = 0; v < k; v++)
				for (u = 0; u < k; u++)
				{
					sy = y + j * k + v;
					if (al > 240)
						hd_px(d, x + i * k + u, sy, rgb(c[0], c[1], c[2]));
					else
						hd_blend(d, x + i * k + u, sy, rgb(c[0], c[1], c[2]), al / 255.0);
				}
		}
}

int	image_import(const char *path, int banner, const char **err)
{
	unsigned char	*d;
	size_t			n;
	t_img			im;
	int				r;

	memset(&im, 0, sizeof(im));
	*err = "fichier introuvable ou illisible";
	d = read_all(path, &n);
	if (!d)
		return (-1);
	r = -1;
	if (n > 8 && !memcmp(d, "\x89PNG\r\n\x1a\n", 8))
		r = png_decode(d, n, &im, err);
	else if (n > 3 && d[0] == 0xff && d[1] == 0xd8)
		r = jpg_decode(d, n, &im, err);
	else
		*err = "format non reconnu : PNG ou JPG seulement";
	free(d);
	free(im.a);
	if (r == 0 && im.px)
	{
		if (banner)
			to_hex(&im, BAN_W, BAN_H, g_save.banner);
		else
			to_hex(&im, PFP_W, PFP_H, g_save.pfp);
	}
	free(im.px);
	return (r);
}

static int	hexv(char c)
{
	return (c >= 'a' ? c - 'a' + 10 : c - '0');
}

void	image_draw(const char *hex, int w, int h, int cx, int cy)
{
	int		x;
	int		y;
	t_rgb	a;
	t_rgb	b;

	if (!hex[0])
		return ;
	for (y = 0; y < h; y += 2)
	{
		at(cx, cy + y / 2);
		for (x = 0; x < w; x++)
		{
			a = rgb(hexv(hex[(y * w + x) * 3]) * 17, hexv(hex[(y * w + x) * 3 + 1]) * 17,
					hexv(hex[(y * w + x) * 3 + 2]) * 17);
			b = y + 1 < h ? rgb(hexv(hex[((y + 1) * w + x) * 3]) * 17, hexv(hex[((y + 1) * w + x) * 3 + 1]) * 17,
					hexv(hex[((y + 1) * w + x) * 3 + 2]) * 17) : theme()->bg;
			fg(a);
			bgc(b);
			out("▀");
		}
	}
	bgc(theme()->bg);
}

int	profile_header(int x, int y)
{
	if (!g_save.banner[0] && !g_save.pfp[0])
		return (0);
	if (g_save.banner[0])
		image_draw(g_save.banner, BAN_W, BAN_H, x + (g_save.pfp[0] ? 18 : 10), y);
	if (g_save.pfp[0])
		image_draw(g_save.pfp, PFP_W, PFP_H, x, y);
	return (BAN_H / 2 + 1);
}

void	screen_images(void)
{
	static int	sel;
	const char	*it[5];
	const char	*err;
	char		path[560];
	char		full[600];
	int			k;
	int			i;
	int			x;

	cls();
	while (1)
	{
		it[0] = L("Choisir une bannière (PNG ou JPG)", "Pick a banner (PNG or JPG)");
		it[1] = L("Choisir une photo de profil (PNG ou JPG)", "Pick a profile picture (PNG or JPG)");
		it[2] = L("Retirer la bannière", "Remove the banner");
		it[3] = L("Retirer la photo", "Remove the picture");
		it[4] = L("Retour", "Back");
		x = (term_cols() - 76) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("B A N N I È R E   E T   P H O T O", "B A N N E R   A N D   P I C T U R E"));
		if (g_save.banner[0])
			image_draw(g_save.banner, BAN_W, BAN_H, x + 8, 3);
		else
			text_at(x + 8, 6, theme()->dim, L("(pas de bannière)", "(no banner)"));
		if (g_save.pfp[0])
			image_draw(g_save.pfp, PFP_W, PFP_H, x, 3);
		else
			text_at(x, 6, theme()->dim, L("(photo)", "(picture)"));
		text_at(x + 18, 12, pseudo_color(), g_save.login);
		for (i = 0; i < 5; i++)
		{
			at(x, 14 + i);
			bgc(i == sel ? mix(theme()->bg, theme()->accent, 0.2) : theme()->bg);
			fg(i == sel ? theme()->accent2 : theme()->text);
			outf(" %s %-50s ", i == sel ? "▶" : " ", it[i]);
		}
		bgc(theme()->bg);
		text_clip(x, 20, theme()->dim, L("L'image est réduite en pixel art et rangée dans ta sauvegarde : elle suit l'export.",
				"The image is turned into pixel art and stored in your save: it follows the export."), 76);
		foot_line(x, L("▲ ▼ choisir   Entrée : valider   Échap : retour",
				"▲ ▼ pick   Enter: confirm   Esc: back"));
		flush();
		k = key_dir(key_read(80));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			sel = (sel + 4) % 5;
		else if (k == 1001)
			sel = (sel + 1) % 5;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			if (sel == 4)
				break ;
			if (sel == 2 || sel == 3)
			{
				if (sel == 2)
					g_save.banner[0] = '\0';
				else
					g_save.pfp[0] = '\0';
				save_write();
			}
			else
			{
				path[0] = '\0';
				if (ask_text(sel == 0 ? L("Chemin de ta bannière", "Path of your banner")
						: L("Chemin de ta photo", "Path of your picture"),
						L("Un fichier PNG ou JPG, par exemple ~/Images/moi.png", "A PNG or JPG file, e.g. ~/Pictures/me.png"),
						path, sizeof(path)))
				{
					if (path[0] == '~' && getenv("HOME"))
						snprintf(full, sizeof(full), "%s%s", getenv("HOME"), path + 1);
					else
						snprintf(full, sizeof(full), "%s", path);
					if (image_import(full, sel == 0, &err) == 0)
					{
						save_write();
						toast(L("IMAGE AJOUTÉE", "IMAGE ADDED"), L("ton profil a changé de tête", "your profile got a new look"),
							rgb(80, 230, 140));
					}
					else
						toast(L("IMAGE REFUSÉE", "IMAGE REFUSED"), err, rgb(255, 90, 100));
				}
			}
			cls();
		}
	}
	cls();
}

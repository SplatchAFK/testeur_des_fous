#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cle_publique.h"

#define NL TDF_CLE_LIMBS

static const uint32_t	g_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t	rotr(uint32_t x, int n)
{
	return ((x >> n) | (x << (32 - n)));
}

static void	sha_block(uint32_t *h, const unsigned char *p)
{
	uint32_t	w[64];
	uint32_t	v[8];
	uint32_t	t1;
	uint32_t	t2;
	int			i;

	for (i = 0; i < 16; i++)
		w[i] = (uint32_t)p[i * 4] << 24 | (uint32_t)p[i * 4 + 1] << 16 | (uint32_t)p[i * 4 + 2] << 8 | p[i * 4 + 3];
	for (i = 16; i < 64; i++)
		w[i] = (rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10)) + w[i - 7]
			+ (rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3)) + w[i - 16];
	memcpy(v, h, sizeof(v));
	for (i = 0; i < 64; i++)
	{
		t1 = v[7] + (rotr(v[4], 6) ^ rotr(v[4], 11) ^ rotr(v[4], 25)) + ((v[4] & v[5]) ^ (~v[4] & v[6])) + g_k[i] + w[i];
		t2 = (rotr(v[0], 2) ^ rotr(v[0], 13) ^ rotr(v[0], 22)) + ((v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]));
		memmove(v + 1, v, sizeof(uint32_t) * 7);
		v[4] += t1;
		v[0] = t1 + t2;
	}
	for (i = 0; i < 8; i++)
		h[i] += v[i];
}

void	tdf_sha256(const unsigned char *msg, size_t len, unsigned char *out)
{
	uint32_t		h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	unsigned char	b[128];
	size_t			i;
	size_t			rest;
	uint64_t		bits;

	for (i = 0; i + 64 <= len; i += 64)
		sha_block(h, msg + i);
	rest = len - i;
	memset(b, 0, sizeof(b));
	memcpy(b, msg + i, rest);
	b[rest] = 0x80;
	bits = (uint64_t)len * 8;
	i = rest < 56 ? 64 : 128;
	for (rest = 0; rest < 8; rest++)
		b[i - 1 - rest] = (unsigned char)(bits >> (8 * rest));
	sha_block(h, b);
	if (i == 128)
		sha_block(h, b + 64);
	for (i = 0; i < 8; i++)
	{
		out[i * 4] = h[i] >> 24;
		out[i * 4 + 1] = h[i] >> 16;
		out[i * 4 + 2] = h[i] >> 8;
		out[i * 4 + 3] = h[i];
	}
}

static int	geq(const uint32_t *a, const uint32_t *b)
{
	int	i;

	for (i = NL - 1; i >= 0; i--)
		if (a[i] != b[i])
			return (a[i] > b[i]);
	return (1);
}

static void	sub(uint32_t *a, const uint32_t *b)
{
	int64_t	c;
	int		i;

	c = 0;
	for (i = 0; i < NL; i++)
	{
		c += (int64_t)a[i] - b[i];
		a[i] = (uint32_t)c;
		c >>= 32;
	}
}

static void	mont(uint32_t *r, const uint32_t *a, const uint32_t *b)
{
	uint32_t	t[NL + 2];
	uint64_t	c;
	uint64_t	s;
	uint32_t	m;
	int			i;
	int			j;

	memset(t, 0, sizeof(t));
	for (i = 0; i < NL; i++)
	{
		c = 0;
		for (j = 0; j < NL; j++)
		{
			s = (uint64_t)t[j] + (uint64_t)a[j] * b[i] + c;
			t[j] = (uint32_t)s;
			c = s >> 32;
		}
		s = (uint64_t)t[NL] + c;
		t[NL] = (uint32_t)s;
		t[NL + 1] = (uint32_t)(s >> 32);
		m = t[0] * TDF_CLE_N0INV;
		c = ((uint64_t)t[0] + (uint64_t)m * g_cle_n[0]) >> 32;
		for (j = 1; j < NL; j++)
		{
			s = (uint64_t)t[j] + (uint64_t)m * g_cle_n[j] + c;
			t[j - 1] = (uint32_t)s;
			c = s >> 32;
		}
		s = (uint64_t)t[NL] + c;
		t[NL - 1] = (uint32_t)s;
		t[NL] = t[NL + 1] + (uint32_t)(s >> 32);
		t[NL + 1] = 0;
	}
	if (t[NL] || geq(t, g_cle_n))
		sub(t, g_cle_n);
	memcpy(r, t, sizeof(uint32_t) * NL);
}

static int	b32(char c)
{
	if (c >= 'a' && c <= 'z')
		c -= 32;
	if (c >= 'A' && c <= 'Z')
		return (c - 'A');
	if (c >= '2' && c <= '7')
		return (c - '2' + 26);
	return (-1);
}

static int	decode(const char *code, unsigned char *out, size_t n)
{
	size_t	k;
	int		acc;
	int		bits;
	int		v;

	k = 0;
	acc = 0;
	bits = 0;
	if (!strncmp(code, "TDF1", 4) || !strncmp(code, "tdf1", 4))
		code += 4;
	for (; *code; code++)
	{
		if (*code == '-' || *code == ' ' || *code == '\t' || *code == '\n' || *code == '\r')
			continue ;
		v = b32(*code);
		if (v < 0)
			return (0);
		acc = (acc << 5 | v) & 0xffff;
		bits += 5;
		if (bits >= 8)
		{
			if (k >= n)
				return (0);
			out[k++] = (unsigned char)(acc >> (bits - 8));
			bits -= 8;
		}
	}
	return (k == n);
}

int	tdf_code_ok(const char *login, const char *code)
{
	static const unsigned char	info[19] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
		0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
	unsigned char				sig[NL * 4];
	unsigned char				em[NL * 4];
	unsigned char				msg[200];
	uint32_t					s[NL];
	uint32_t					x[NL];
	uint32_t					y[NL];
	uint32_t					one[NL];
	size_t						len;
	int							i;

	if (!login || !*login || !code || strlen(login) > 64)
		return (0);
	if (!decode(code, sig, sizeof(sig)))
		return (0);
	for (i = 0; i < NL; i++)
		s[i] = (uint32_t)sig[(NL - 1 - i) * 4] << 24 | (uint32_t)sig[(NL - 1 - i) * 4 + 1] << 16
			| (uint32_t)sig[(NL - 1 - i) * 4 + 2] << 8 | sig[(NL - 1 - i) * 4 + 3];
	if (geq(s, g_cle_n))
		return (0);
	mont(x, s, g_cle_r2);
	memcpy(y, x, sizeof(y));
	for (i = 0; i < 16; i++)
		mont(y, y, y);
	mont(y, y, x);
	memset(one, 0, sizeof(one));
	one[0] = 1;
	mont(y, y, one);
	for (i = 0; i < NL; i++)
	{
		em[(NL - 1 - i) * 4] = y[i] >> 24;
		em[(NL - 1 - i) * 4 + 1] = y[i] >> 16;
		em[(NL - 1 - i) * 4 + 2] = y[i] >> 8;
		em[(NL - 1 - i) * 4 + 3] = y[i];
	}
	len = (size_t)snprintf((char *)msg, sizeof(msg), "le-testeur-des-fous|code|v1|%s", login);
	if (em[0] != 0 || em[1] != 1)
		return (0);
	for (i = 2; i < NL * 4 - 52; i++)
		if (em[i] != 0xff)
			return (0);
	if (em[NL * 4 - 52] != 0 || memcmp(em + NL * 4 - 51, info, 19))
		return (0);
	tdf_sha256(msg, len, sig);
	return (memcmp(em + NL * 4 - 32, sig, 32) == 0);
}

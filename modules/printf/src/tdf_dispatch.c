#include "tdf.h"

static int	d0(t_pf f, const char *s)
{
	return (f(s));
}

static int	d1(t_pf f, const char *s, int id, const t_val *v)
{
	switch (id)
	{
		case 0: return (f(s, v[0].i));
		case 1: return (f(s, v[0].u));
		case 2: return (f(s, v[0].s));
		case 3: return (f(s, v[0].p));
	}
	return (-12345);
}

static int	d2(t_pf f, const char *s, int id, const t_val *v)
{
	switch (id)
	{
		case 0: return (f(s, v[0].i, v[1].i));
		case 4: return (f(s, v[0].i, v[1].u));
		case 8: return (f(s, v[0].i, v[1].s));
		case 12: return (f(s, v[0].i, v[1].p));
		case 1: return (f(s, v[0].u, v[1].i));
		case 5: return (f(s, v[0].u, v[1].u));
		case 9: return (f(s, v[0].u, v[1].s));
		case 13: return (f(s, v[0].u, v[1].p));
		case 2: return (f(s, v[0].s, v[1].i));
		case 6: return (f(s, v[0].s, v[1].u));
		case 10: return (f(s, v[0].s, v[1].s));
		case 14: return (f(s, v[0].s, v[1].p));
		case 3: return (f(s, v[0].p, v[1].i));
		case 7: return (f(s, v[0].p, v[1].u));
		case 11: return (f(s, v[0].p, v[1].s));
		case 15: return (f(s, v[0].p, v[1].p));
	}
	return (-12345);
}

static int	d3(t_pf f, const char *s, int id, const t_val *v)
{
	switch (id)
	{
		case 0: return (f(s, v[0].i, v[1].i, v[2].i));
		case 16: return (f(s, v[0].i, v[1].i, v[2].u));
		case 32: return (f(s, v[0].i, v[1].i, v[2].s));
		case 48: return (f(s, v[0].i, v[1].i, v[2].p));
		case 4: return (f(s, v[0].i, v[1].u, v[2].i));
		case 20: return (f(s, v[0].i, v[1].u, v[2].u));
		case 36: return (f(s, v[0].i, v[1].u, v[2].s));
		case 52: return (f(s, v[0].i, v[1].u, v[2].p));
		case 8: return (f(s, v[0].i, v[1].s, v[2].i));
		case 24: return (f(s, v[0].i, v[1].s, v[2].u));
		case 40: return (f(s, v[0].i, v[1].s, v[2].s));
		case 56: return (f(s, v[0].i, v[1].s, v[2].p));
		case 12: return (f(s, v[0].i, v[1].p, v[2].i));
		case 28: return (f(s, v[0].i, v[1].p, v[2].u));
		case 44: return (f(s, v[0].i, v[1].p, v[2].s));
		case 60: return (f(s, v[0].i, v[1].p, v[2].p));
		case 1: return (f(s, v[0].u, v[1].i, v[2].i));
		case 17: return (f(s, v[0].u, v[1].i, v[2].u));
		case 33: return (f(s, v[0].u, v[1].i, v[2].s));
		case 49: return (f(s, v[0].u, v[1].i, v[2].p));
		case 5: return (f(s, v[0].u, v[1].u, v[2].i));
		case 21: return (f(s, v[0].u, v[1].u, v[2].u));
		case 37: return (f(s, v[0].u, v[1].u, v[2].s));
		case 53: return (f(s, v[0].u, v[1].u, v[2].p));
		case 9: return (f(s, v[0].u, v[1].s, v[2].i));
		case 25: return (f(s, v[0].u, v[1].s, v[2].u));
		case 41: return (f(s, v[0].u, v[1].s, v[2].s));
		case 57: return (f(s, v[0].u, v[1].s, v[2].p));
		case 13: return (f(s, v[0].u, v[1].p, v[2].i));
		case 29: return (f(s, v[0].u, v[1].p, v[2].u));
		case 45: return (f(s, v[0].u, v[1].p, v[2].s));
		case 61: return (f(s, v[0].u, v[1].p, v[2].p));
		case 2: return (f(s, v[0].s, v[1].i, v[2].i));
		case 18: return (f(s, v[0].s, v[1].i, v[2].u));
		case 34: return (f(s, v[0].s, v[1].i, v[2].s));
		case 50: return (f(s, v[0].s, v[1].i, v[2].p));
		case 6: return (f(s, v[0].s, v[1].u, v[2].i));
		case 22: return (f(s, v[0].s, v[1].u, v[2].u));
		case 38: return (f(s, v[0].s, v[1].u, v[2].s));
		case 54: return (f(s, v[0].s, v[1].u, v[2].p));
		case 10: return (f(s, v[0].s, v[1].s, v[2].i));
		case 26: return (f(s, v[0].s, v[1].s, v[2].u));
		case 42: return (f(s, v[0].s, v[1].s, v[2].s));
		case 58: return (f(s, v[0].s, v[1].s, v[2].p));
		case 14: return (f(s, v[0].s, v[1].p, v[2].i));
		case 30: return (f(s, v[0].s, v[1].p, v[2].u));
		case 46: return (f(s, v[0].s, v[1].p, v[2].s));
		case 62: return (f(s, v[0].s, v[1].p, v[2].p));
		case 3: return (f(s, v[0].p, v[1].i, v[2].i));
		case 19: return (f(s, v[0].p, v[1].i, v[2].u));
		case 35: return (f(s, v[0].p, v[1].i, v[2].s));
		case 51: return (f(s, v[0].p, v[1].i, v[2].p));
		case 7: return (f(s, v[0].p, v[1].u, v[2].i));
		case 23: return (f(s, v[0].p, v[1].u, v[2].u));
		case 39: return (f(s, v[0].p, v[1].u, v[2].s));
		case 55: return (f(s, v[0].p, v[1].u, v[2].p));
		case 11: return (f(s, v[0].p, v[1].s, v[2].i));
		case 27: return (f(s, v[0].p, v[1].s, v[2].u));
		case 43: return (f(s, v[0].p, v[1].s, v[2].s));
		case 59: return (f(s, v[0].p, v[1].s, v[2].p));
		case 15: return (f(s, v[0].p, v[1].p, v[2].i));
		case 31: return (f(s, v[0].p, v[1].p, v[2].u));
		case 47: return (f(s, v[0].p, v[1].p, v[2].s));
		case 63: return (f(s, v[0].p, v[1].p, v[2].p));
	}
	return (-12345);
}

static int	d4(t_pf f, const char *s, int id, const t_val *v)
{
	switch (id)
	{
		case 0: return (f(s, v[0].i, v[1].i, v[2].i, v[3].i));
		case 64: return (f(s, v[0].i, v[1].i, v[2].i, v[3].u));
		case 128: return (f(s, v[0].i, v[1].i, v[2].i, v[3].s));
		case 192: return (f(s, v[0].i, v[1].i, v[2].i, v[3].p));
		case 16: return (f(s, v[0].i, v[1].i, v[2].u, v[3].i));
		case 80: return (f(s, v[0].i, v[1].i, v[2].u, v[3].u));
		case 144: return (f(s, v[0].i, v[1].i, v[2].u, v[3].s));
		case 208: return (f(s, v[0].i, v[1].i, v[2].u, v[3].p));
		case 32: return (f(s, v[0].i, v[1].i, v[2].s, v[3].i));
		case 96: return (f(s, v[0].i, v[1].i, v[2].s, v[3].u));
		case 160: return (f(s, v[0].i, v[1].i, v[2].s, v[3].s));
		case 224: return (f(s, v[0].i, v[1].i, v[2].s, v[3].p));
		case 48: return (f(s, v[0].i, v[1].i, v[2].p, v[3].i));
		case 112: return (f(s, v[0].i, v[1].i, v[2].p, v[3].u));
		case 176: return (f(s, v[0].i, v[1].i, v[2].p, v[3].s));
		case 240: return (f(s, v[0].i, v[1].i, v[2].p, v[3].p));
		case 4: return (f(s, v[0].i, v[1].u, v[2].i, v[3].i));
		case 68: return (f(s, v[0].i, v[1].u, v[2].i, v[3].u));
		case 132: return (f(s, v[0].i, v[1].u, v[2].i, v[3].s));
		case 196: return (f(s, v[0].i, v[1].u, v[2].i, v[3].p));
		case 20: return (f(s, v[0].i, v[1].u, v[2].u, v[3].i));
		case 84: return (f(s, v[0].i, v[1].u, v[2].u, v[3].u));
		case 148: return (f(s, v[0].i, v[1].u, v[2].u, v[3].s));
		case 212: return (f(s, v[0].i, v[1].u, v[2].u, v[3].p));
		case 36: return (f(s, v[0].i, v[1].u, v[2].s, v[3].i));
		case 100: return (f(s, v[0].i, v[1].u, v[2].s, v[3].u));
		case 164: return (f(s, v[0].i, v[1].u, v[2].s, v[3].s));
		case 228: return (f(s, v[0].i, v[1].u, v[2].s, v[3].p));
		case 52: return (f(s, v[0].i, v[1].u, v[2].p, v[3].i));
		case 116: return (f(s, v[0].i, v[1].u, v[2].p, v[3].u));
		case 180: return (f(s, v[0].i, v[1].u, v[2].p, v[3].s));
		case 244: return (f(s, v[0].i, v[1].u, v[2].p, v[3].p));
		case 8: return (f(s, v[0].i, v[1].s, v[2].i, v[3].i));
		case 72: return (f(s, v[0].i, v[1].s, v[2].i, v[3].u));
		case 136: return (f(s, v[0].i, v[1].s, v[2].i, v[3].s));
		case 200: return (f(s, v[0].i, v[1].s, v[2].i, v[3].p));
		case 24: return (f(s, v[0].i, v[1].s, v[2].u, v[3].i));
		case 88: return (f(s, v[0].i, v[1].s, v[2].u, v[3].u));
		case 152: return (f(s, v[0].i, v[1].s, v[2].u, v[3].s));
		case 216: return (f(s, v[0].i, v[1].s, v[2].u, v[3].p));
		case 40: return (f(s, v[0].i, v[1].s, v[2].s, v[3].i));
		case 104: return (f(s, v[0].i, v[1].s, v[2].s, v[3].u));
		case 168: return (f(s, v[0].i, v[1].s, v[2].s, v[3].s));
		case 232: return (f(s, v[0].i, v[1].s, v[2].s, v[3].p));
		case 56: return (f(s, v[0].i, v[1].s, v[2].p, v[3].i));
		case 120: return (f(s, v[0].i, v[1].s, v[2].p, v[3].u));
		case 184: return (f(s, v[0].i, v[1].s, v[2].p, v[3].s));
		case 248: return (f(s, v[0].i, v[1].s, v[2].p, v[3].p));
		case 12: return (f(s, v[0].i, v[1].p, v[2].i, v[3].i));
		case 76: return (f(s, v[0].i, v[1].p, v[2].i, v[3].u));
		case 140: return (f(s, v[0].i, v[1].p, v[2].i, v[3].s));
		case 204: return (f(s, v[0].i, v[1].p, v[2].i, v[3].p));
		case 28: return (f(s, v[0].i, v[1].p, v[2].u, v[3].i));
		case 92: return (f(s, v[0].i, v[1].p, v[2].u, v[3].u));
		case 156: return (f(s, v[0].i, v[1].p, v[2].u, v[3].s));
		case 220: return (f(s, v[0].i, v[1].p, v[2].u, v[3].p));
		case 44: return (f(s, v[0].i, v[1].p, v[2].s, v[3].i));
		case 108: return (f(s, v[0].i, v[1].p, v[2].s, v[3].u));
		case 172: return (f(s, v[0].i, v[1].p, v[2].s, v[3].s));
		case 236: return (f(s, v[0].i, v[1].p, v[2].s, v[3].p));
		case 60: return (f(s, v[0].i, v[1].p, v[2].p, v[3].i));
		case 124: return (f(s, v[0].i, v[1].p, v[2].p, v[3].u));
		case 188: return (f(s, v[0].i, v[1].p, v[2].p, v[3].s));
		case 252: return (f(s, v[0].i, v[1].p, v[2].p, v[3].p));
		case 1: return (f(s, v[0].u, v[1].i, v[2].i, v[3].i));
		case 65: return (f(s, v[0].u, v[1].i, v[2].i, v[3].u));
		case 129: return (f(s, v[0].u, v[1].i, v[2].i, v[3].s));
		case 193: return (f(s, v[0].u, v[1].i, v[2].i, v[3].p));
		case 17: return (f(s, v[0].u, v[1].i, v[2].u, v[3].i));
		case 81: return (f(s, v[0].u, v[1].i, v[2].u, v[3].u));
		case 145: return (f(s, v[0].u, v[1].i, v[2].u, v[3].s));
		case 209: return (f(s, v[0].u, v[1].i, v[2].u, v[3].p));
		case 33: return (f(s, v[0].u, v[1].i, v[2].s, v[3].i));
		case 97: return (f(s, v[0].u, v[1].i, v[2].s, v[3].u));
		case 161: return (f(s, v[0].u, v[1].i, v[2].s, v[3].s));
		case 225: return (f(s, v[0].u, v[1].i, v[2].s, v[3].p));
		case 49: return (f(s, v[0].u, v[1].i, v[2].p, v[3].i));
		case 113: return (f(s, v[0].u, v[1].i, v[2].p, v[3].u));
		case 177: return (f(s, v[0].u, v[1].i, v[2].p, v[3].s));
		case 241: return (f(s, v[0].u, v[1].i, v[2].p, v[3].p));
		case 5: return (f(s, v[0].u, v[1].u, v[2].i, v[3].i));
		case 69: return (f(s, v[0].u, v[1].u, v[2].i, v[3].u));
		case 133: return (f(s, v[0].u, v[1].u, v[2].i, v[3].s));
		case 197: return (f(s, v[0].u, v[1].u, v[2].i, v[3].p));
		case 21: return (f(s, v[0].u, v[1].u, v[2].u, v[3].i));
		case 85: return (f(s, v[0].u, v[1].u, v[2].u, v[3].u));
		case 149: return (f(s, v[0].u, v[1].u, v[2].u, v[3].s));
		case 213: return (f(s, v[0].u, v[1].u, v[2].u, v[3].p));
		case 37: return (f(s, v[0].u, v[1].u, v[2].s, v[3].i));
		case 101: return (f(s, v[0].u, v[1].u, v[2].s, v[3].u));
		case 165: return (f(s, v[0].u, v[1].u, v[2].s, v[3].s));
		case 229: return (f(s, v[0].u, v[1].u, v[2].s, v[3].p));
		case 53: return (f(s, v[0].u, v[1].u, v[2].p, v[3].i));
		case 117: return (f(s, v[0].u, v[1].u, v[2].p, v[3].u));
		case 181: return (f(s, v[0].u, v[1].u, v[2].p, v[3].s));
		case 245: return (f(s, v[0].u, v[1].u, v[2].p, v[3].p));
		case 9: return (f(s, v[0].u, v[1].s, v[2].i, v[3].i));
		case 73: return (f(s, v[0].u, v[1].s, v[2].i, v[3].u));
		case 137: return (f(s, v[0].u, v[1].s, v[2].i, v[3].s));
		case 201: return (f(s, v[0].u, v[1].s, v[2].i, v[3].p));
		case 25: return (f(s, v[0].u, v[1].s, v[2].u, v[3].i));
		case 89: return (f(s, v[0].u, v[1].s, v[2].u, v[3].u));
		case 153: return (f(s, v[0].u, v[1].s, v[2].u, v[3].s));
		case 217: return (f(s, v[0].u, v[1].s, v[2].u, v[3].p));
		case 41: return (f(s, v[0].u, v[1].s, v[2].s, v[3].i));
		case 105: return (f(s, v[0].u, v[1].s, v[2].s, v[3].u));
		case 169: return (f(s, v[0].u, v[1].s, v[2].s, v[3].s));
		case 233: return (f(s, v[0].u, v[1].s, v[2].s, v[3].p));
		case 57: return (f(s, v[0].u, v[1].s, v[2].p, v[3].i));
		case 121: return (f(s, v[0].u, v[1].s, v[2].p, v[3].u));
		case 185: return (f(s, v[0].u, v[1].s, v[2].p, v[3].s));
		case 249: return (f(s, v[0].u, v[1].s, v[2].p, v[3].p));
		case 13: return (f(s, v[0].u, v[1].p, v[2].i, v[3].i));
		case 77: return (f(s, v[0].u, v[1].p, v[2].i, v[3].u));
		case 141: return (f(s, v[0].u, v[1].p, v[2].i, v[3].s));
		case 205: return (f(s, v[0].u, v[1].p, v[2].i, v[3].p));
		case 29: return (f(s, v[0].u, v[1].p, v[2].u, v[3].i));
		case 93: return (f(s, v[0].u, v[1].p, v[2].u, v[3].u));
		case 157: return (f(s, v[0].u, v[1].p, v[2].u, v[3].s));
		case 221: return (f(s, v[0].u, v[1].p, v[2].u, v[3].p));
		case 45: return (f(s, v[0].u, v[1].p, v[2].s, v[3].i));
		case 109: return (f(s, v[0].u, v[1].p, v[2].s, v[3].u));
		case 173: return (f(s, v[0].u, v[1].p, v[2].s, v[3].s));
		case 237: return (f(s, v[0].u, v[1].p, v[2].s, v[3].p));
		case 61: return (f(s, v[0].u, v[1].p, v[2].p, v[3].i));
		case 125: return (f(s, v[0].u, v[1].p, v[2].p, v[3].u));
		case 189: return (f(s, v[0].u, v[1].p, v[2].p, v[3].s));
		case 253: return (f(s, v[0].u, v[1].p, v[2].p, v[3].p));
		case 2: return (f(s, v[0].s, v[1].i, v[2].i, v[3].i));
		case 66: return (f(s, v[0].s, v[1].i, v[2].i, v[3].u));
		case 130: return (f(s, v[0].s, v[1].i, v[2].i, v[3].s));
		case 194: return (f(s, v[0].s, v[1].i, v[2].i, v[3].p));
		case 18: return (f(s, v[0].s, v[1].i, v[2].u, v[3].i));
		case 82: return (f(s, v[0].s, v[1].i, v[2].u, v[3].u));
		case 146: return (f(s, v[0].s, v[1].i, v[2].u, v[3].s));
		case 210: return (f(s, v[0].s, v[1].i, v[2].u, v[3].p));
		case 34: return (f(s, v[0].s, v[1].i, v[2].s, v[3].i));
		case 98: return (f(s, v[0].s, v[1].i, v[2].s, v[3].u));
		case 162: return (f(s, v[0].s, v[1].i, v[2].s, v[3].s));
		case 226: return (f(s, v[0].s, v[1].i, v[2].s, v[3].p));
		case 50: return (f(s, v[0].s, v[1].i, v[2].p, v[3].i));
		case 114: return (f(s, v[0].s, v[1].i, v[2].p, v[3].u));
		case 178: return (f(s, v[0].s, v[1].i, v[2].p, v[3].s));
		case 242: return (f(s, v[0].s, v[1].i, v[2].p, v[3].p));
		case 6: return (f(s, v[0].s, v[1].u, v[2].i, v[3].i));
		case 70: return (f(s, v[0].s, v[1].u, v[2].i, v[3].u));
		case 134: return (f(s, v[0].s, v[1].u, v[2].i, v[3].s));
		case 198: return (f(s, v[0].s, v[1].u, v[2].i, v[3].p));
		case 22: return (f(s, v[0].s, v[1].u, v[2].u, v[3].i));
		case 86: return (f(s, v[0].s, v[1].u, v[2].u, v[3].u));
		case 150: return (f(s, v[0].s, v[1].u, v[2].u, v[3].s));
		case 214: return (f(s, v[0].s, v[1].u, v[2].u, v[3].p));
		case 38: return (f(s, v[0].s, v[1].u, v[2].s, v[3].i));
		case 102: return (f(s, v[0].s, v[1].u, v[2].s, v[3].u));
		case 166: return (f(s, v[0].s, v[1].u, v[2].s, v[3].s));
		case 230: return (f(s, v[0].s, v[1].u, v[2].s, v[3].p));
		case 54: return (f(s, v[0].s, v[1].u, v[2].p, v[3].i));
		case 118: return (f(s, v[0].s, v[1].u, v[2].p, v[3].u));
		case 182: return (f(s, v[0].s, v[1].u, v[2].p, v[3].s));
		case 246: return (f(s, v[0].s, v[1].u, v[2].p, v[3].p));
		case 10: return (f(s, v[0].s, v[1].s, v[2].i, v[3].i));
		case 74: return (f(s, v[0].s, v[1].s, v[2].i, v[3].u));
		case 138: return (f(s, v[0].s, v[1].s, v[2].i, v[3].s));
		case 202: return (f(s, v[0].s, v[1].s, v[2].i, v[3].p));
		case 26: return (f(s, v[0].s, v[1].s, v[2].u, v[3].i));
		case 90: return (f(s, v[0].s, v[1].s, v[2].u, v[3].u));
		case 154: return (f(s, v[0].s, v[1].s, v[2].u, v[3].s));
		case 218: return (f(s, v[0].s, v[1].s, v[2].u, v[3].p));
		case 42: return (f(s, v[0].s, v[1].s, v[2].s, v[3].i));
		case 106: return (f(s, v[0].s, v[1].s, v[2].s, v[3].u));
		case 170: return (f(s, v[0].s, v[1].s, v[2].s, v[3].s));
		case 234: return (f(s, v[0].s, v[1].s, v[2].s, v[3].p));
		case 58: return (f(s, v[0].s, v[1].s, v[2].p, v[3].i));
		case 122: return (f(s, v[0].s, v[1].s, v[2].p, v[3].u));
		case 186: return (f(s, v[0].s, v[1].s, v[2].p, v[3].s));
		case 250: return (f(s, v[0].s, v[1].s, v[2].p, v[3].p));
		case 14: return (f(s, v[0].s, v[1].p, v[2].i, v[3].i));
		case 78: return (f(s, v[0].s, v[1].p, v[2].i, v[3].u));
		case 142: return (f(s, v[0].s, v[1].p, v[2].i, v[3].s));
		case 206: return (f(s, v[0].s, v[1].p, v[2].i, v[3].p));
		case 30: return (f(s, v[0].s, v[1].p, v[2].u, v[3].i));
		case 94: return (f(s, v[0].s, v[1].p, v[2].u, v[3].u));
		case 158: return (f(s, v[0].s, v[1].p, v[2].u, v[3].s));
		case 222: return (f(s, v[0].s, v[1].p, v[2].u, v[3].p));
		case 46: return (f(s, v[0].s, v[1].p, v[2].s, v[3].i));
		case 110: return (f(s, v[0].s, v[1].p, v[2].s, v[3].u));
		case 174: return (f(s, v[0].s, v[1].p, v[2].s, v[3].s));
		case 238: return (f(s, v[0].s, v[1].p, v[2].s, v[3].p));
		case 62: return (f(s, v[0].s, v[1].p, v[2].p, v[3].i));
		case 126: return (f(s, v[0].s, v[1].p, v[2].p, v[3].u));
		case 190: return (f(s, v[0].s, v[1].p, v[2].p, v[3].s));
		case 254: return (f(s, v[0].s, v[1].p, v[2].p, v[3].p));
		case 3: return (f(s, v[0].p, v[1].i, v[2].i, v[3].i));
		case 67: return (f(s, v[0].p, v[1].i, v[2].i, v[3].u));
		case 131: return (f(s, v[0].p, v[1].i, v[2].i, v[3].s));
		case 195: return (f(s, v[0].p, v[1].i, v[2].i, v[3].p));
		case 19: return (f(s, v[0].p, v[1].i, v[2].u, v[3].i));
		case 83: return (f(s, v[0].p, v[1].i, v[2].u, v[3].u));
		case 147: return (f(s, v[0].p, v[1].i, v[2].u, v[3].s));
		case 211: return (f(s, v[0].p, v[1].i, v[2].u, v[3].p));
		case 35: return (f(s, v[0].p, v[1].i, v[2].s, v[3].i));
		case 99: return (f(s, v[0].p, v[1].i, v[2].s, v[3].u));
		case 163: return (f(s, v[0].p, v[1].i, v[2].s, v[3].s));
		case 227: return (f(s, v[0].p, v[1].i, v[2].s, v[3].p));
		case 51: return (f(s, v[0].p, v[1].i, v[2].p, v[3].i));
		case 115: return (f(s, v[0].p, v[1].i, v[2].p, v[3].u));
		case 179: return (f(s, v[0].p, v[1].i, v[2].p, v[3].s));
		case 243: return (f(s, v[0].p, v[1].i, v[2].p, v[3].p));
		case 7: return (f(s, v[0].p, v[1].u, v[2].i, v[3].i));
		case 71: return (f(s, v[0].p, v[1].u, v[2].i, v[3].u));
		case 135: return (f(s, v[0].p, v[1].u, v[2].i, v[3].s));
		case 199: return (f(s, v[0].p, v[1].u, v[2].i, v[3].p));
		case 23: return (f(s, v[0].p, v[1].u, v[2].u, v[3].i));
		case 87: return (f(s, v[0].p, v[1].u, v[2].u, v[3].u));
		case 151: return (f(s, v[0].p, v[1].u, v[2].u, v[3].s));
		case 215: return (f(s, v[0].p, v[1].u, v[2].u, v[3].p));
		case 39: return (f(s, v[0].p, v[1].u, v[2].s, v[3].i));
		case 103: return (f(s, v[0].p, v[1].u, v[2].s, v[3].u));
		case 167: return (f(s, v[0].p, v[1].u, v[2].s, v[3].s));
		case 231: return (f(s, v[0].p, v[1].u, v[2].s, v[3].p));
		case 55: return (f(s, v[0].p, v[1].u, v[2].p, v[3].i));
		case 119: return (f(s, v[0].p, v[1].u, v[2].p, v[3].u));
		case 183: return (f(s, v[0].p, v[1].u, v[2].p, v[3].s));
		case 247: return (f(s, v[0].p, v[1].u, v[2].p, v[3].p));
		case 11: return (f(s, v[0].p, v[1].s, v[2].i, v[3].i));
		case 75: return (f(s, v[0].p, v[1].s, v[2].i, v[3].u));
		case 139: return (f(s, v[0].p, v[1].s, v[2].i, v[3].s));
		case 203: return (f(s, v[0].p, v[1].s, v[2].i, v[3].p));
		case 27: return (f(s, v[0].p, v[1].s, v[2].u, v[3].i));
		case 91: return (f(s, v[0].p, v[1].s, v[2].u, v[3].u));
		case 155: return (f(s, v[0].p, v[1].s, v[2].u, v[3].s));
		case 219: return (f(s, v[0].p, v[1].s, v[2].u, v[3].p));
		case 43: return (f(s, v[0].p, v[1].s, v[2].s, v[3].i));
		case 107: return (f(s, v[0].p, v[1].s, v[2].s, v[3].u));
		case 171: return (f(s, v[0].p, v[1].s, v[2].s, v[3].s));
		case 235: return (f(s, v[0].p, v[1].s, v[2].s, v[3].p));
		case 59: return (f(s, v[0].p, v[1].s, v[2].p, v[3].i));
		case 123: return (f(s, v[0].p, v[1].s, v[2].p, v[3].u));
		case 187: return (f(s, v[0].p, v[1].s, v[2].p, v[3].s));
		case 251: return (f(s, v[0].p, v[1].s, v[2].p, v[3].p));
		case 15: return (f(s, v[0].p, v[1].p, v[2].i, v[3].i));
		case 79: return (f(s, v[0].p, v[1].p, v[2].i, v[3].u));
		case 143: return (f(s, v[0].p, v[1].p, v[2].i, v[3].s));
		case 207: return (f(s, v[0].p, v[1].p, v[2].i, v[3].p));
		case 31: return (f(s, v[0].p, v[1].p, v[2].u, v[3].i));
		case 95: return (f(s, v[0].p, v[1].p, v[2].u, v[3].u));
		case 159: return (f(s, v[0].p, v[1].p, v[2].u, v[3].s));
		case 223: return (f(s, v[0].p, v[1].p, v[2].u, v[3].p));
		case 47: return (f(s, v[0].p, v[1].p, v[2].s, v[3].i));
		case 111: return (f(s, v[0].p, v[1].p, v[2].s, v[3].u));
		case 175: return (f(s, v[0].p, v[1].p, v[2].s, v[3].s));
		case 239: return (f(s, v[0].p, v[1].p, v[2].s, v[3].p));
		case 63: return (f(s, v[0].p, v[1].p, v[2].p, v[3].i));
		case 127: return (f(s, v[0].p, v[1].p, v[2].p, v[3].u));
		case 191: return (f(s, v[0].p, v[1].p, v[2].p, v[3].s));
		case 255: return (f(s, v[0].p, v[1].p, v[2].p, v[3].p));
	}
	return (-12345);
}

int	tdf_dispatch(t_pf fn, const char *fmt, int n, const int *k, const t_val *v)
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

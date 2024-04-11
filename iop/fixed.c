#ifndef FIXED_H_
#define FIXED_H_

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

typedef int fixed_t;

fixed_t
fixed_mul(fixed_t a, fixed_t b)
{
	fixed_t res;

	__asm__("mult %1, %2 \n"
			"mflo %0 \n"
			"srl %0, %3\n"
			"mfhi $v1 \n"
			"sll $v1, %3\n"
			"or %0, $v1 \n"
			: "=d"(res)
			: "d"(a), "d"(b), "i"(FRACBITS)
			: "3");

	return res;
}

static inline int
nlz(unsigned x)
{
	int n;

	if (x == 0)
		return (32);
	n = 0;
	if (x <= 0x0000FFFF) {
		n = n + 16;
		x = x << 16;
	}
	if (x <= 0x00FFFFFF) {
		n = n + 8;
		x = x << 8;
	}
	if (x <= 0x0FFFFFFF) {
		n = n + 4;
		x = x << 4;
	}
	if (x <= 0x3FFFFFFF) {
		n = n + 2;
		x = x << 2;
	}
	if (x <= 0x7FFFFFFF) {
		n = n + 1;
	}
	return n;
}

/* courtesy of hackers delight
 * unsigned div!
 */

fixed_t
fixed_div(fixed_t u, fixed_t v)
{
	unsigned u0 = u << FRACBITS;
	unsigned u1 = (u >> FRACBITS) & (FRACUNIT - 1);

	const unsigned b = 65536; // Number base (16 bits).
	unsigned un1, un0,		  // Norm. dividend LSD's.
	  vn1, vn0,				  // Norm. divisor digits.
	  q1, q0,				  // Quotient digits.
	  un32, un21, un10,		  // Dividend digit pairs.
	  rhat;					  // A remainder.
	int s;					  // Shift amount for norm.

	s = nlz(v);		  // 0 <= s <= 31.
	v = v << s;		  // Normalize divisor.
	vn1 = v >> 16;	  // Break divisor up into
	vn0 = v & 0xFFFF; // two 16-bit digits.

	un32 = (u1 << s) | ((u0 >> (32 - s)) & (-s >> 31));
	un10 = u0 << s; // Shift dividend left.

	un1 = un10 >> 16;	 // Break right half of
	un0 = un10 & 0xFFFF; // dividend into two digits.

	q1 = un32 / vn1;		// Compute the first
	rhat = un32 - q1 * vn1; // quotient digit, q1.

	while (q1 >= b || q1 * vn0 > b * rhat + un1) {
		q1 = q1 - 1;
		rhat = rhat + vn1;
		if (rhat >= b)
			break;
	}

	un21 = un32 * b + un1 - q1 * v; // Multiply and subtract.

	q0 = un21 / vn1;		// Compute the second
	rhat = un21 - q0 * vn1; // quotient digit, q0.

	while (q0 >= b || q0 * vn0 > b * rhat + un0) {
		q0 = q0 - 1;
		rhat = rhat + vn1;
		if (rhat >= b)
			break;
	}

	return q1 * b + q0;
}

#endif // FIXED_H_

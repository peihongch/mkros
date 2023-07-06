#ifndef __MATH_H_
#define __MATH_H_

#define do_div(num, base)         \
	({                            \
		int rem = (num) % (base); \
		(num) /= (base);          \
		rem;                      \
	})

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#endif /* __MATH_H_ */

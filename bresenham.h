typedef struct {
	unsigned fixed; // For fractions greater than one.
	int error;		// The state
	unsigned numerator;
	unsigned denominator;
	} tInterpKernel;

void interp_reset(tInterpKernel *kernel);
void interp_init(tInterpKernel *kernel, unsigned num, unsigned denom);
unsigned interp_next(tInterpKernel *kernel);

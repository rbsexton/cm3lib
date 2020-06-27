/**
 * @file bresenham.c
 * @brief Interpolate using Bresenhams line drawing algorothm.
 * Interpolate one point at a time.
 *
 * See: http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * For a good primer.
 *
 */

#include "bresenham.h"

// Call to reset the kernel.
void interp_reset(tInterpKernel *kernel) {
    kernel->error = kernel->denominator / 2;
    }

void interp_init(tInterpKernel *kernel, unsigned num, unsigned denom) {
    if ( num > denom ) {
        kernel->fixed = num / denom;
        num -= kernel->fixed * denom;
        }
    else {
        kernel->fixed = 0;
        }

    kernel->numerator = num;
    kernel->denominator = denom;
    interp_reset(kernel);
    }

unsigned interp_next(tInterpKernel *kernel) {
    unsigned ret = kernel->fixed;

    kernel->error -= kernel->numerator;

    if ( kernel->error < 0 ) {
        ret++;
        kernel->error += kernel->denominator;
        }

    return(ret);
    }

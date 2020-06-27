#include <stdio.h>
#include "bresenham.h"

void showstate(tInterpKernel *k) {
    printf("err=%d fixed=%d, n=%d d=%d \n",
           k->error, k->fixed,
           k->numerator, k->denominator);
    }

int main() {
    int total = 0;
    int val;
    tInterpKernel kern;

    total = 0;
    interp_init(&kern, 7, 32);
    showstate(&kern);

    for(int i = 0; i < 70; i++) {
        val = interp_next(&kern);
        total += val;
        printf("%03d, inc=%d tot=%d\n", i, val, total);
        }


    printf("---- Next!\n");

    total = 0;
    interp_init(&kern, 41, 32);
    showstate(&kern);

    for(int i = 0; i < 70; i++) {
        val = interp_next(&kern);
        total += val;
        printf("%03d, inc=%d tot=%d\n", i, val, total);
        }

    printf("Done!\n");

    }
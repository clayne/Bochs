/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

/*----------------------------------------------------------------------------
| Extracts the mantissa of single-precision floating-point value 'a' and
| returns the result as a single-precision floating-point after applying
| the mantissa interval normalization and sign control. The operation is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float32 f32_getMant(float32 a, softfloat_status_t *status, int sign_ctrl, int interv)
{
    bool signA;
    int16_t expA;
    uint32_t sigA;
    struct exp16_sig32 normExpSig;

    signA = signF32UI(a);
    expA  = expF32UI(a);
    sigA  = fracF32UI(a);

    if (expA == 0xFF) {
        if (sigA) return softfloat_propagateNaNF32UI(a, 0, status);
        if (signA) {
            if (sign_ctrl & 0x2) {
                softfloat_raiseFlags(status, softfloat_flag_invalid);
                return defaultNaNF32UI;
            }
        }
        return packToF32UI(~sign_ctrl & signA, 0x7F, 0);
    }

    if (! expA && (! sigA || softfloat_denormalsAreZeros(status))) {
        return packToF32UI(~sign_ctrl & signA, 0x7F, 0);
    }

    if (signA) {
        if (sign_ctrl & 0x2) {
            softfloat_raiseFlags(status, softfloat_flag_invalid);
            return defaultNaNF32UI;
        }
    }

    if (! expA) {
        softfloat_raiseFlags(status, softfloat_flag_denormal);
        normExpSig = softfloat_normSubnormalF32Sig(sigA);
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
        sigA &= 0x7FFFFF;
    }

    switch(interv) {
    case 0x0: // interval [1,2)
        expA = 0x7F;
        break;
    case 0x1: // interval [1/2,2)
        expA -= 0x7F;
        expA  = 0x7F - (expA & 0x1);
        break;
    case 0x2: // interval [1/2,1)
        expA = 0x7E;
        break;
    case 0x3: // interval [3/4,3/2)
        expA = 0x7F - ((sigA >> 22) & 0x1);
        break;
    }

    return packToF32UI(~sign_ctrl & signA, expA, sigA);
}

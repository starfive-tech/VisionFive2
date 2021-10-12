/********************************************************************
*                                                                  *
* THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
* USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
* GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
* IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
*                                                                  *
* THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
* by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
*                                                                  *
********************************************************************

function:
last mod: $Id: ocintrin.h 16503 2009-08-22 18:14:02Z giles $

********************************************************************/

/*Some common macros for potential platform-specific optimization.*/
#include <math.h>
#if !defined(_ocintrin_H)
# define _ocintrin_H (1)

/*Some specific platforms may have optimized intrinsic or inline assembly
versions of these functions which can substantially improve performance.
We define macros for them to allow easy incorporation of these non-ANSI
features.*/

/*Note that we do not provide a macro for abs(), because it is provided as a
library function, which we assume is translated into an intrinsic to avoid
the function call overhead and then implemented in the smartest way for the
target platform.
With modern gcc (4.x), this is true: it uses cmov instructions if the
architecture supports it and branchless bit-twiddling if it does not (the
speed difference between the two approaches is not measurable).
Interestingly, the bit-twiddling method was patented in 2000 (US 6,073,150)
by Sun Microsystems, despite prior art dating back to at least 1996:
http://web.archive.org/web/19961201174141/www.x86.org/ftp/articles/pentopt/PENTOPT.TXT
On gcc 3.x, however, our assumption is not true, as abs() is translated to a
conditional jump, which is horrible on deeply piplined architectures (e.g.,
all consumer architectures for the past decade or more).
Also be warned that -C*abs(x) where C is a constant is mis-optimized as
abs(C*x) on every gcc release before 4.2.3.
See bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34130 */


#endif

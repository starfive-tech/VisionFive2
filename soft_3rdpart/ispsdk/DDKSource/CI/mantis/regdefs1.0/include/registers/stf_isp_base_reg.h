/**
  ******************************************************************************
  * @file  stf_isp_base_reg.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/14/2019
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */
#ifndef __STF_ISP_BASE_REG_H__
#define __STF_ISP_BASE_REG_H__


#include "registers/isp_reg.h"
#include "registers/stf_isp_csi_in_rdma.h"
#include "registers/stf_isp_isp_in_rdma.h"
#include "registers/stf_isp_crop_rdma.h"
#include "registers/stf_isp_sud_csi_rdma.h"
#include "registers/stf_isp_sud_isp_rdma.h"


/* add ISP and CSI base registers structure */
//=============================================================================
#define EN_CSI                  (1LL << 0)  // 0x0000 - REG_ENABLE_STATUS: (1 << 0).
//-----------------------------------------------------------------------------
#define STS_CSI_BUSY            (1LL << 1)  // 0x0000 - REG_ENABLE_STATUS: (1 << 1).
//-----------------------------------------------------------------------------
#define STS_SCD_DONE            (1LL << 2)  // 0x0000 - REG_ENABLE_STATUS: (1 << 2).
//-----------------------------------------------------------------------------
#define STS_ITU656_FIELD_2      (1LL << 3)  // 0x0000 - REG_ENABLE_STATUS: (1 << 3).
#define STS_ITU656_ERROR        (1LL << 4)  // 0x0000 - REG_ENABLE_STATUS: (1 << 4).
//-----------------------------------------------------------------------------
#define STS_SCD_ERROR           (1LL << 6)  // 0x0000 - REG_ENABLE_STATUS: (1 << 6).
//-----------------------------------------------------------------------------
#define STS_SCD_ALL             (STS_SCD_DONE | STS_SCD_ERROR)
#define STS_ITU656_ALL          (STS_ITU656_FIELD_2 | STS_ITU656_ERROR)
#define STS_CSI_ALL             (STS_CSI_BUSY | STS_SCD_ALL | STS_ITU656_ALL)
//=============================================================================

//=============================================================================
#define EN_ISP                  (1LL << 0)  // 0x0A00 - REG_ISPCTRL0: (1 << 0).
#define RST_ISP                 (1LL << 1)  // 0x0A00 - REG_ISPCTRL0: (1 << 0).
//-----------------------------------------------------------------------------
#define EN_ISP_CONTINUE_MODE    (1LL << 17) // 0x0A00 - REG_ISPCTRL0: (1 << 17), Enable ISP multiple frame mode.
//=============================================================================

//=============================================================================
#define STS_ISP_BUSY            (1LL << 0)  // 0x0A04 - REG_ISPSTAT0: (1 << 0).
#define STS_ISP_UO_BUSY         (1LL << 1)  // 0x0A04 - REG_ISPSTAT0: (1 << 1).
#define STS_ISP_SS0_BUSY        (1LL << 2)  // 0x0A04 - REG_ISPSTAT0: (1 << 2).
#define STS_ISP_SS1_BUSY        (1LL << 3)  // 0x0A04 - REG_ISPSTAT0: (1 << 3).
//-----------------------------------------------------------------------------
#define STS_CSI_BUFFER_ERROR    (1LL << 9)  // 0x0A04 - REG_ISPSTAT0: (1 << 9).
//-----------------------------------------------------------------------------
#define STS_OUT_ALL             (STS_ISP_UO_BUSY | STS_ISP_SS0_BUSY | STS_ISP_SS1_BUSY)
#define STS_ISP_AND_OUT_ALL     (STS_ISP_BUSY | STS_OUT_ALL)
#define STS_ISP_ALL             (STS_ISP_BUSY | STS_OUT_ALL | STS_CSI_BUFFER_ERROR)
//=============================================================================

//=============================================================================
typedef enum _EN_KRN_MOD_ID {
    //---------- NONE ----------
    EN_KRN_MOD_ID_SUD_OFF = 0,      // 00       // 12 bits
    //---------- 0x0010 - REG_FUNCTION_MODE ----------
    EN_KRN_MOD_ID_DC,               // 01       // 12 bits
    EN_KRN_MOD_ID_DEC,              // 02       // 12 bits
    EN_KRN_MOD_ID_OBC,              // 03       // 12 bits
    EN_KRN_MOD_ID_LCBQ,             // 04       // 12 bits
    EN_KRN_MOD_ID_OECF,             // 05       // 12 bits
    EN_KRN_MOD_ID_OECFHM,           // 06       // 12 bits
    EN_KRN_MOD_ID_LCCF,             // 07       // 12 bits
    EN_KRN_MOD_ID_AWB,              // 08       // 12 bits
    EN_KRN_MOD_ID_OBA,              // 09       // 12 bits
    EN_KRN_MOD_ID_SC,               // 10       // 12 bits
    EN_KRN_MOD_ID_DUMP,             // 11       // 12 bits
    //---------- 0x0A08 - REG_ISPCTRL1 ----------
    EN_KRN_MOD_ID_DBC,              // 12       // 12 bits
    EN_KRN_MOD_ID_CTC,              // 13       // 12 bits
    EN_KRN_MOD_ID_CFA,              // 14       // 12 bits
    EN_KRN_MOD_ID_CAR,              // 15       // 12 bits
    EN_KRN_MOD_ID_CCM,              // 16       // 12 bits
    EN_KRN_MOD_ID_GMARGB,           // 17       // 12 bits
    EN_KRN_MOD_ID_R2Y,              // 18       // 12 bits
    EN_KRN_MOD_ID_YCRV,             // 19       // 12 bits
    EN_KRN_MOD_ID_SHRP,             // 20       // 12 bits
    EN_KRN_MOD_ID_DNYUV,            // 21       // 12 bits
    EN_KRN_MOD_ID_SAT,              // 22       // 12 bits
    EN_KRN_MOD_ID_YHIST,            // 23
    //---------- 0x0A00 - REG_ISPCTRL0 ----------
    EN_KRN_MOD_ID_OUT,              // 24       // 8 bits
    EN_KRN_MOD_ID_UO,               // 25       // 8 bits
    EN_KRN_MOD_ID_SS0,              // 26       // 8 bits
    EN_KRN_MOD_ID_SS1,              // 27       // 8 bits
    //---------- 0x0B48 - REG_ITIDPSR  ----------
    EN_KRN_MOD_ID_TIL,              // 28       // 10 - 12 bits
    EN_KRN_MOD_ID_TIL_1_RD,         // 29       // 10 - 12 bits
    EN_KRN_MOD_ID_TIL_1_WR,         // 30       // 10 - 12 bits
    //---------- 0x0008 - REG_CSIINTS and 0x0A50 - REG_IESHD ----------
    EN_KRN_MOD_ID_SUD,              // 31
    //---------- 0x0008 - REG_CSIINTS ----------
    EN_KRN_MOD_ID_SUD_CSI,          // 32
    //---------- 0x0A50 - REG_IESHD ----------
    EN_KRN_MOD_ID_SUD_ISP,          // 33
    //---------- 0x0000 - REG_ENABLE_STATUS ----------
    EN_KRN_MOD_ID_CSI,              // 34
    //---------- 0x0A00 - REG_ISPCTRL0 ----------
    EN_KRN_MOD_ID_ISP,              // 35
    //---------- 0x060C - CSICBMD0 ----------
    EN_KRN_MOD_ID_CBAR,             // 36
    //---------- NONE ----------
    EN_KRN_MOD_ID_BUF,              // 37
    EN_KRN_MOD_ID_MAX               // 38
} EN_KRN_MOD_ID, *PEN_KRN_MOD_ID;

//=============================================================================
#define EN_KRN_MOD_SUD_OFF      (1LL << EN_KRN_MOD_ID_SUD_OFF)
//=============================================================================
#define EN_KRN_MOD_DC           (1LL << EN_KRN_MOD_ID_DC)       // 0x0010 - REG_FUNCTION_MODE: (1 << 0).
#define EN_KRN_MOD_DEC          (1LL << EN_KRN_MOD_ID_DEC)      // 0x0010 - REG_FUNCTION_MODE: (1 << 1).
#define EN_KRN_MOD_OBC          (1LL << EN_KRN_MOD_ID_OBC)      // 0x0010 - REG_FUNCTION_MODE: (1 << 2).
#define EN_KRN_MOD_LCBQ         (1LL << EN_KRN_MOD_ID_LCBQ)     // 0x0010 - REG_FUNCTION_MODE: (1 << 3).
#define EN_KRN_MOD_OECF         (1LL << EN_KRN_MOD_ID_OECF)     // 0x0010 - REG_FUNCTION_MODE: (1 << 4).
#define EN_KRN_MOD_OECFHM       (1LL << EN_KRN_MOD_ID_OECFHM)   // 0x0010 - REG_FUNCTION_MODE: (1 << 5).
#define EN_KRN_MOD_LCCF         (1LL << EN_KRN_MOD_ID_LCCF)     // 0x0010 - REG_FUNCTION_MODE: (1 << 6).
#define EN_KRN_MOD_AWB          (1LL << EN_KRN_MOD_ID_AWB)      // 0x0010 - REG_FUNCTION_MODE: (1 << 7).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_OBA          (1LL << EN_KRN_MOD_ID_OBA)      // 0x0010 - REG_FUNCTION_MODE: (1 << 16).
#define EN_KRN_MOD_SC           (1LL << EN_KRN_MOD_ID_SC)       // 0x0010 - REG_FUNCTION_MODE: (1 << 17).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_DUMP         (1LL << EN_KRN_MOD_ID_DUMP)     // 0x0010 - REG_FUNCTION_MODE: (1 << 19).
//=============================================================================
#define EN_KRN_MOD_DBC          (1LL << EN_KRN_MOD_ID_DBC)      // 0x0A08 - REG_ISPCTRL1: (1 << 22).
#define EN_KRN_MOD_CTC          (1LL << EN_KRN_MOD_ID_CTC)      // 0x0A08 - REG_ISPCTRL1: (1 << 21).
#define EN_KRN_MOD_CFA          (1LL << EN_KRN_MOD_ID_CFA)      // 0x0A08 - REG_ISPCTRL1: (1 << 1).
#define EN_KRN_MOD_CAR          (1LL << EN_KRN_MOD_ID_CAR)      // 0x0A08 - REG_ISPCTRL1: (1 << 2).
#define EN_KRN_MOD_CCM          (1LL << EN_KRN_MOD_ID_CCM)      // 0x0A08 - REG_ISPCTRL1: (1 << 3).
#define EN_KRN_MOD_GMARGB       (1LL << EN_KRN_MOD_ID_GMARGB)   // 0x0A08 - REG_ISPCTRL1: (1 << 4).
#define EN_KRN_MOD_R2Y          (1LL << EN_KRN_MOD_ID_R2Y)      // 0x0A08 - REG_ISPCTRL1: (1 << 5).
#define EN_KRN_MOD_YCRV         (1LL << EN_KRN_MOD_ID_YCRV)     // 0x0A08 - REG_ISPCTRL1: (1 << 19).
#define EN_KRN_MOD_SHRP         (1LL << EN_KRN_MOD_ID_SHRP)     // 0x0A08 - REG_ISPCTRL1: (1 << 7).
#define EN_KRN_MOD_DNYUV        (1LL << EN_KRN_MOD_ID_DNYUV)    // 0x0A08 - REG_ISPCTRL1: (1 << 17).
#define EN_KRN_MOD_SAT          (1LL << EN_KRN_MOD_ID_SAT)      // 0x0A08 - REG_ISPCTRL1: (1 << 8).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_YHIST        (1LL << EN_KRN_MOD_ID_YHIST)    // 0x0A08 - REG_ISPCTRL1: (1 << 20).
//=============================================================================
#define EN_KRN_MOD_OUT          (1LL << EN_KRN_MOD_ID_OUT)      // 0x0A00 - REG_ISPCTRL0: ((1 << 20) | (1 << 11) | (1 << 12)).
#define EN_KRN_MOD_UO           (1LL << EN_KRN_MOD_ID_UO)       // 0x0A00 - REG_ISPCTRL0: (1 << 20).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_SS0          (1LL << EN_KRN_MOD_ID_SS0)      // 0x0A00 - REG_ISPCTRL0: (1 << 11).
#define EN_KRN_MOD_SS1          (1LL << EN_KRN_MOD_ID_SS1)      // 0x0A00 - REG_ISPCTRL0: (1 << 12).
//=============================================================================
#define EN_KRN_MOD_TIL          (1LL << EN_KRN_MOD_ID_TIL)      // 0x0B48 - REG_ITIDPSR: ((1 << 16) | (1 << 17)).
#define EN_KRN_MOD_TIL_1_RD     (1LL << EN_KRN_MOD_ID_TIL_1_RD) // 0x0B48 - REG_ITIDPSR: (1 << 16).
#define EN_KRN_MOD_TIL_1_WR     (1LL << EN_KRN_MOD_ID_TIL_1_WR) // 0x0B48 - REG_ITIDPSR: (1 << 17).
//=============================================================================
#define EN_KRN_MOD_SUD          (1LL << EN_KRN_MOD_ID_SUD)      // 0x0008 - REG_CSIINTS: (1 << 16), 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_SUD_CSI      (1LL << EN_KRN_MOD_ID_SUD_CSI)  // 0x0008 - REG_CSIINTS: (1 << 16).
//=============================================================================
#define EN_KRN_MOD_SUD_ISP      (1LL << EN_KRN_MOD_ID_SUD_ISP)  // 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_CSI          (1LL << EN_KRN_MOD_ID_CSI)      // 0x0000 - REG_ENABLE_STATUS: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_ISP          (1LL << EN_KRN_MOD_ID_ISP)      // 0x0A00 - REG_ISPCTRL0: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_CBAR         (1LL << EN_KRN_MOD_ID_CBAR)     // 0x060C - CSICBMD0: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_BUF          (1LL << EN_KRN_MOD_ID_BUF)      // 0xXXXX - XXXX.
//=============================================================================
#define EN_KRN_MOD_ALL          (0x00000000             \
                                 | EN_KRN_MOD_DC        \
                                 | EN_KRN_MOD_DEC       \
                                 | EN_KRN_MOD_OBC       \
                                 | EN_KRN_MOD_LCBQ      \
                                 | EN_KRN_MOD_OECF      \
                                 | EN_KRN_MOD_OECFHM    \
                                 | EN_KRN_MOD_LCCF      \
                                 | EN_KRN_MOD_AWB       \
                                 | EN_KRN_MOD_OBA       \
                                 | EN_KRN_MOD_SC        \
                                 | EN_KRN_MOD_DUMP      \
                                 | EN_KRN_MOD_DBC       \
                                 | EN_KRN_MOD_CTC       \
                                 | EN_KRN_MOD_CFA       \
                                 | EN_KRN_MOD_CAR       \
                                 | EN_KRN_MOD_CCM       \
                                 | EN_KRN_MOD_GMARGB    \
                                 | EN_KRN_MOD_R2Y       \
                                 | EN_KRN_MOD_YCRV      \
                                 | EN_KRN_MOD_SHRP      \
                                 | EN_KRN_MOD_DNYUV     \
                                 | EN_KRN_MOD_SAT       \
                                 | EN_KRN_MOD_YHIST     \
                                 | EN_KRN_MOD_OUT       \
                                 | EN_KRN_MOD_UO        \
                                 | EN_KRN_MOD_SS0       \
                                 | EN_KRN_MOD_SS1       \
                                 | EN_KRN_MOD_TIL       \
                                 | EN_KRN_MOD_TIL_1_RD  \
                                 | EN_KRN_MOD_TIL_1_WR  \
                                 | EN_KRN_MOD_SUD       \
                                 | EN_KRN_MOD_SUD_CSI   \
                                 | EN_KRN_MOD_SUD_ISP   \
                                 | EN_KRN_MOD_CSI       \
                                 | EN_KRN_MOD_ISP       \
                                 | EN_KRN_MOD_BUF       \
                                )
//=============================================================================

#define ISP_MOD_CHECK           (0x0FLL << 60)
#define ISP_MOD_CHECK_ENA       (0x00LL << 60)                          // 0x0000 - REG_ENABLE_STATUS
#define ISP_MOD_CHECK_CSI       (0x01LL << 60)                          // 0x0010 - REG_FUNCTION_MODE
#define ISP_MOD_CHECK_ISP_1     (0x02LL << 60)                          // 0x0A08 - REG_ISPCTRL1
#define ISP_MOD_CHECK_OUT       (0x03LL << 60)                          // 0x0A00 - REG_ISPCTRL0
#define ISP_MOD_CHECK_TIL       (0x04LL << 60)                          // 0x0B48 - REG_ITIDPSR
#define ISP_MOD_CHECK_TIL_1     (0x05LL << 60)                          // 0x0B48 - REG_ITIDPSR
#define ISP_MOD_CHECK_SUD       (0x06LL << 60)                          // 0x0008 - REG_CSIINTS and 0x0A50 - REG_IESHD
#define ISP_MOD_CHECK_SUD_CSI   (0x07LL << 60)                          // 0x0008 - REG_CSIINTS
#define ISP_MOD_CHECK_SUD_ISP   (0x08LL << 60)                          // 0x0A50 - REG_IESHD
#define ISP_MOD_CHECK_CBAR      (0x09LL << 60)                          // 0x060C - CSICBMD0
#define ISP_MOD_CHECK_MAX       (0x0A)
//=============================================================================
#define ISP_MOD_DC              (ISP_MOD_CHECK_CSI | (1LL << 0))        // 0x0010 - REG_FUNCTION_MODE: (1 << 0).
#define ISP_MOD_DEC             (ISP_MOD_CHECK_CSI | (1LL << 1))        // 0x0010 - REG_FUNCTION_MODE: (1 << 1).
#define ISP_MOD_OBC             (ISP_MOD_CHECK_CSI | (1LL << 2))        // 0x0010 - REG_FUNCTION_MODE: (1 << 2).
#define ISP_MOD_LCBQ            (ISP_MOD_CHECK_CSI | (1LL << 3))        // 0x0010 - REG_FUNCTION_MODE: (1 << 3).
#define ISP_MOD_OECF            (ISP_MOD_CHECK_CSI | (1LL << 4))        // 0x0010 - REG_FUNCTION_MODE: (1 << 4).
#define ISP_MOD_OECFHM          (ISP_MOD_CHECK_CSI | (1LL << 5))        // 0x0010 - REG_FUNCTION_MODE: (1 << 5).
#define ISP_MOD_LCCF            (ISP_MOD_CHECK_CSI | (1LL << 6))        // 0x0010 - REG_FUNCTION_MODE: (1 << 6).
#define ISP_MOD_AWB             (ISP_MOD_CHECK_CSI | (1LL << 7))        // 0x0010 - REG_FUNCTION_MODE: (1 << 7).
//-----------------------------------------------------------------------------
#define ISP_MOD_OBA             (ISP_MOD_CHECK_CSI | (1LL << 16))       // 0x0010 - REG_FUNCTION_MODE: (1 << 16).
#define ISP_MOD_SC              (ISP_MOD_CHECK_CSI | (1LL << 17))       // 0x0010 - REG_FUNCTION_MODE: (1 << 17).
//-----------------------------------------------------------------------------
#define ISP_MOD_DUMP            (ISP_MOD_CHECK_CSI | (1LL << 19))       // 0x0010 - REG_FUNCTION_MODE: (1 << 19).
//=============================================================================
#define ISP_MOD_DBC             (ISP_MOD_CHECK_ISP_1 | (1LL << 22))     // 0x0A08 - REG_ISPCTRL1: (1 << 22).
#define ISP_MOD_CTC             (ISP_MOD_CHECK_ISP_1 | (1LL << 21))     // 0x0A08 - REG_ISPCTRL1: (1 << 21).
#define ISP_MOD_CFA             (ISP_MOD_CHECK_ISP_1 | (1LL << 1))      // 0x0A08 - REG_ISPCTRL1: (1 << 1).
#define ISP_MOD_CAR             (ISP_MOD_CHECK_ISP_1 | (1LL << 2))      // 0x0A08 - REG_ISPCTRL1: (1 << 2).
#define ISP_MOD_CCM             (ISP_MOD_CHECK_ISP_1 | (1LL << 3))      // 0x0A08 - REG_ISPCTRL1: (1 << 3).
#define ISP_MOD_GMARGB          (ISP_MOD_CHECK_ISP_1 | (1LL << 4))      // 0x0A08 - REG_ISPCTRL1: (1 << 4).
#define ISP_MOD_R2Y             (ISP_MOD_CHECK_ISP_1 | (1LL << 5))      // 0x0A08 - REG_ISPCTRL1: (1 << 5).
#define ISP_MOD_YCRV            (ISP_MOD_CHECK_ISP_1 | (1LL << 19))     // 0x0A08 - REG_ISPCTRL1: (1 << 19).
#define ISP_MOD_SHRP            (ISP_MOD_CHECK_ISP_1 | (1LL << 7))      // 0x0A08 - REG_ISPCTRL1: (1 << 7).
#define ISP_MOD_DNYUV           (ISP_MOD_CHECK_ISP_1 | (1LL << 17))     // 0x0A08 - REG_ISPCTRL1: (1 << 17).
#define ISP_MOD_SAT             (ISP_MOD_CHECK_ISP_1 | (1LL << 8))      // 0x0A08 - REG_ISPCTRL1: (1 << 8).
//-----------------------------------------------------------------------------
#define ISP_MOD_YHIST           (ISP_MOD_CHECK_ISP_1 | (1LL << 20))     // 0x0A08 - REG_ISPCTRL1: (1 << 20).
//=============================================================================
#define ISP_MOD_OUT             (ISP_MOD_CHECK_OUT | (1LL << 20) | (1LL << 11) | (1LL << 12))       // 0x0A00 - REG_ISPCTRL0: ((1 << 20) | (1 << 11) | (1 << 12)).
#define ISP_MOD_UO              (ISP_MOD_CHECK_OUT | (1LL << 20))       // 0x0A00 - REG_ISPCTRL0: (1 << 20).
//-----------------------------------------------------------------------------
#define ISP_MOD_SS0             (ISP_MOD_CHECK_OUT | (1LL << 11))       // 0x0A00 - REG_ISPCTRL0: (1 << 11).
#define ISP_MOD_SS1             (ISP_MOD_CHECK_OUT | (1LL << 12))       // 0x0A00 - REG_ISPCTRL0: (1 << 12).
//=============================================================================
#define ISP_MOD_TIL             (ISP_MOD_CHECK_TIL | (1LL << 16) | (1LL << 17))       // 0x0B48 - REG_ITIDPSR: ((1 << 16) | (1 << 17)).
#define ISP_MOD_TIL_1_RD        (ISP_MOD_CHECK_TIL_1 | (1LL << 16))     // 0x0B48 - REG_ITIDPSR: (1 << 16).
#define ISP_MOD_TIL_1_WR        (ISP_MOD_CHECK_TIL_1 | (1LL << 17))     // 0x0B48 - REG_ITIDPSR: (1 << 17).
//=============================================================================
#define ISP_MOD_SUD_OFF         (ISP_MOD_CHECK_SUD)                     // 0x0008 - REG_CSIINTS: (1 << 16) and // 0x0A50 - REG_IESHD: (1 << 0).
#define ISP_MOD_SUD             (ISP_MOD_CHECK_SUD | (1LL << 0) | (1LL << 48)) // 0x0008 - REG_CSIINTS: (1 << 16) and // 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define ISP_MOD_SUD_CSI         (ISP_MOD_CHECK_SUD_CSI | (1LL << 16))   // 0x0008 - REG_CSIINTS: (1 << 16).
//=============================================================================
#define ISP_MOD_SUD_ISP         (ISP_MOD_CHECK_SUD_ISP | (1LL << 0))    // 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define ISP_MOD_CSI             (ISP_MOD_CHECK_ENA | (1LL << 0))        // 0x0000 - REG_ENABLE_STATUS: (1 << 0).
//=============================================================================
#define ISP_MOD_ISP             (ISP_MOD_CHECK_ISP_1 | (1LL << 0))      // 0x0A00 - REG_ISPCTRL0: (1 << 0).
//=============================================================================
#define ISP_MOD_CBAR            (ISP_MOD_CHECK_CBAR | (1LL << 0))       // 0x060C - CSICBMD0: (1 << 0).
//=============================================================================
#define ISP_MOD_BUF             ((0))                                   //
//=============================================================================

#ifdef UNUSED_CODE_AND_VARIABLE
/* ISP clock gate control. */
#define ISP_MOD_CLK_GATE_CTL_AWB        (0)
#define ISP_MOD_CLK_GATE_CTL_GAMF       (0)
#define ISP_MOD_CLK_GATE_CTL_CFA        (0)
#define ISP_MOD_CLK_GATE_CTL_CAR        (0)
#define ISP_MOD_CLK_GATE_CTL_CCM        (0)
#define ISP_MOD_CLK_GATE_CTL_GAMRGB     (0)
#define ISP_MOD_CLK_GATE_CTL_R2Y        (0)
#define ISP_MOD_CLK_GATE_CTL_NWED       (0)
#define ISP_MOD_CLK_GATE_CTL_SC0        (0)
#define ISP_MOD_CLK_GATE_CTL_SC1        (0)

#define ISP_MOD_CLK_GATE_CTL_DC         (1<<0)
#define ISP_MOD_CLK_GATE_CTL_DEC        (1<<1)
#define ISP_MOD_CLK_GATE_CTL_OBC        (1<<2)
#define ISP_MOD_CLK_GATE_CTL_LCBQ       (1<<3)
#define ISP_MOD_CLK_GATE_CTL_OECF       (1<<4)
#define ISP_MOD_CLK_GATE_CTL_LCBQ       (1<<5)
#define ISP_MOD_CLK_GATE_CTL_OECFHM     (1<<6)
#define ISP_MOD_CLK_GATE_CTL_LCCF       (1<<7)
#define ISP_MOD_CLK_GATE_CTL_AWB        (1<<8)
#define ISP_MOD_CLK_GATE_CTL_OBA        (1<<9)
#define ISP_MOD_CLK_GATE_CTL_SC         (1<<10)
#define ISP_MOD_CLK_GATE_CTL_DUMP       (1<<11)
#define ISP_MOD_CLK_GATE_CTL_DBC        (1<<12)
#define ISP_MOD_CLK_GATE_CTL_CTC        (1<<13)
#define ISP_MOD_CLK_GATE_CTL_CFA        (1<<14)
#define ISP_MOD_CLK_GATE_CTL_CAR        (1<<15)
#define ISP_MOD_CLK_GATE_CTL_CCM        (1<<16)
#define ISP_MOD_CLK_GATE_CTL_GMARGB     (1<<17)
#define ISP_MOD_CLK_GATE_CTL_R2Y        (1<<18)
#define ISP_MOD_CLK_GATE_CTL_YCRV       (1<<19)
#define ISP_MOD_CLK_GATE_CTL_SHRP       (1<<20)
#define ISP_MOD_CLK_GATE_CTL_DNYUV      (1<<21)
#define ISP_MOD_CLK_GATE_CTL_SAT        (1<<22)
#define ISP_MOD_CLK_GATE_CTL_YHIST      (1<<23)
#define ISP_MOD_CLK_GATE_CTL_OUT        (1<<24)
#define ISP_MOD_CLK_GATE_CTL_UO         (1<<25)
#define ISP_MOD_CLK_GATE_CTL_SS0        (1<<26)
#define ISP_MOD_CLK_GATE_CTL_SS1        (1<<27)
#define ISP_MOD_CLK_GATE_CTL_TIL        (1<<28)
#define ISP_MOD_CLK_GATE_CTL_TIL_1_RD   (1<<29)
#define ISP_MOD_CLK_GATE_CTL_TIL_1_WR   (1<<30)
#define ISP_MOD_CLK_GATE_CTL_SUD        (1<<31)
#define ISP_MOD_CLK_GATE_CTL_SUD_CSI    (1<<32)
#define ISP_MOD_CLK_GATE_CTL_SUD_ISP    (1<<33)
#define ISP_MOD_CLK_GATE_CTL_CSI        (1<<34)
#define ISP_MOD_CLK_GATE_CTL_ISP        (1<<35)
#define ISP_MOD_CLK_GATE_CTL_BUF        (1<<36)

//typedef enum _EN_CLK_GATE_CTL {
//    EN_CLK_GATE_CTL_DC = 0,             (1<<0)
//    EN_CLK_GATE_CTL_DEC,                // 01
//    EN_CLK_GATE_CTL_OBC,                // 02
//    EN_CLK_GATE_CTL_LCBQ,               // 03
//    EN_CLK_GATE_CTL_OECF,               // 04
//    EN_CLK_GATE_CTL_LCBQ,               // 05
//    EN_CLK_GATE_CTL_OECFHM,             // 06
//    EN_CLK_GATE_CTL_LCCF,               // 07
//    EN_CLK_GATE_CTL_AWB,                // 08
//    EN_CLK_GATE_CTL_OBA,                // 09
//    EN_CLK_GATE_CTL_SC,                 // 10
//    EN_CLK_GATE_CTL_DUMP,               // 11
//    EN_CLK_GATE_CTL_DBC,                // 12
//    EN_CLK_GATE_CTL_CTC,                // 13
//    EN_CLK_GATE_CTL_CFA,                // 14
//    EN_CLK_GATE_CTL_CAR,                // 15
//    EN_CLK_GATE_CTL_CCM,                // 16
//    EN_CLK_GATE_CTL_GMARGB,             // 17
//    EN_CLK_GATE_CTL_R2Y,                // 18
//    EN_CLK_GATE_CTL_YCRV,               // 19
//    EN_CLK_GATE_CTL_SHRP,               // 20
//    EN_CLK_GATE_CTL_DNYUV,              // 21
//    EN_CLK_GATE_CTL_SAT,                // 22
//    EN_CLK_GATE_CTL_YHIST,              // 23
//    EN_CLK_GATE_CTL_OUT,                // 24
//    EN_CLK_GATE_CTL_UO,                 // 25
//    EN_CLK_GATE_CTL_SS0,                // 26
//    EN_CLK_GATE_CTL_SS1,                // 27
//    EN_CLK_GATE_CTL_TIL,                // 28
//    EN_CLK_GATE_CTL_TIL_1_RD,           // 29
//    EN_CLK_GATE_CTL_TIL_1_WR,           // 30
//    EN_CLK_GATE_CTL_SUD,                // 31
//    EN_CLK_GATE_CTL_SUD_CSI,            // 32
//    EN_CLK_GATE_CTL_SUD_ISP,            // 33
//    EN_CLK_GATE_CTL_CSI,                // 34
//    EN_CLK_GATE_CTL_ISP,                // 35
//    EN_CLK_GATE_CTL_BUF,                // 36
//} EN_CLK_GATE_CTL, *PEN_CLK_GATE_CTL;

#endif //UNUSED_CODE_AND_VARIABLE

typedef enum _EN_INTERRUPT {
    //-----------------        -------------------------------------------------------------------
    EN_INT_NONE                 = 0,
    EN_INT_ISP_DONE             = (1LL << 24),      // 0x0A00, [24] INTS - ISP module done interrupt. No trigger if none of scalar outputs is enabled.
                                                    //                     This bit will be set by HW when ISP has finished processing and must be cleared by writing 0 to the register.
                                                    //                     Register write can also set the INT bit (for test purposes). HW set has precedence over register write.
    EN_INT_CSI_DONE             = (1LL << 25),      // 0x0A00, [25] VSINT - CSI capture end interrupt bit.
    EN_INT_SC_DONE              = (1LL << 26),      // 0x0A00, [26] VSFWINT - SC dump done interrupt bit. No trigger if SC is disabled.
    EN_INT_LINE_INT             = (1LL << 27),      // 0x0A00, [27] LBERRINT - Line interrupt bit.
    EN_INT_ISP_CSI_DONE         = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_CSI_DONE
                                  ),
    EN_INT_ISP_SC_DONE          = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_SC_DONE
                                  ),
    EN_INT_ISP_LINE_DNOE        = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_CSI_SC_DONE          = (
                                    EN_INT_CSI_DONE
                                  | EN_INT_SC_DONE
                                  ),
    EN_INT_CSI_LINE_DNOE        = (
                                    EN_INT_CSI_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_SC_LINE_DNOE         = (
                                    EN_INT_SC_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_ISP_CSI_SC_DONE    = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_CSI_DONE
                                  | EN_INT_SC_DONE
                                  ),
    EN_INT_ISP_CSI_LINE_DONE    = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_CSI_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_ISP_SC_LINE_DONE     = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_SC_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_CSI_SC_LINE_DONE     = (
                                    EN_INT_CSI_DONE
                                  | EN_INT_SC_DONE
                                  | EN_INT_LINE_INT
                                  ),
    EN_INT_ALL                  = (
                                    EN_INT_ISP_DONE
                                  | EN_INT_CSI_DONE
                                  | EN_INT_SC_DONE
                                  | EN_INT_LINE_INT
                                  ),
} EN_ISP_INTERRUPT, *PEN_ISP_INTERRUPT;


#pragma pack(push, 1)
typedef struct _ST_TEMP_INFO {
    STF_U16 u16MeasureTemperature;              /** AWB measure temperature degree. */
    STF_U8 u8TemperatureIdx;                    /** Indicate CCM temperature table index, this index will plus 100 if measure temperature out of temperature table. */
    STF_DOUBLE dTemperatureFactor;              /** Temperature factor. */
} ST_TEMP_INFO, *PST_TEMP_INFO;

typedef struct _ST_ISP_BASE_ENABLE {
    STF_U8 u8IspIdx;
    STF_BOOL8 bEnable;
} ST_ISP_BASE_ENABLE, *PST_ISP_BASE_ENABLE;

typedef struct _ST_ISP_BASE_RESET {
    STF_U8 u8IspIdx;
    STF_BOOL8 bReset;
} ST_ISP_BASE_RESET, *PST_ISP_BASE_RESET;

typedef struct _ST_ISP_BASE_BOOL8_STATUS {
    STF_U8 u8IspIdx;
    STF_BOOL8 bStatus;
} ST_ISP_BASE_BOOL8_STATUS, *PST_ISP_BASE_BOOL8_STATUS;

typedef struct _ST_ISP_BASE_U32_STATUS {
    STF_U8 u8IspIdx;
    STF_U32 u32Status;
} ST_ISP_BASE_U32_STATUS, *PST_ISP_BASE_U32_STATUS;

typedef struct _ST_ISP_BASE_INTERRUPT {
    STF_U8 u8IspIdx;
    EN_ISP_INTERRUPT enInterrupt;
} ST_ISP_BASE_INTERRUPT, *PST_ISP_BASE_INTERRUPT;

typedef struct _ST_ISP_BASE_COUNT {
    STF_U8 u8IspIdx;
    STF_U16 u16Count;
} ST_ISP_BASE_COUNT, *PST_ISP_BASE_COUNT;

typedef struct _ST_MODULE_EN_PARAM {
    STF_U64 u64Module;
    STF_BOOL8 bEnable;
} ST_MODULE_EN_PARAM, *PST_MODULE_EN_PARAM;

typedef struct _ST_ISP_MODULE_EN_PARAM {
    STF_U8 u8IspIdx;
    ST_MODULE_EN_PARAM stModulesEnableParam;
} ST_ISP_MODULE_EN_PARAM, *PST_ISP_MODULE_EN_PARAM;

typedef struct _ST_MODULES_EN_PARAM {
    STF_U64 u64Modules;
    STF_BOOL8 bEnable;
} ST_MODULES_EN_PARAM, *PST_MODULES_EN_PARAM;

typedef struct _ST_ISP_MODULES_EN_PARAM {
    STF_U8 u8IspIdx;
    ST_MODULES_EN_PARAM stModulesEnableParam;
} ST_ISP_MODULES_EN_PARAM, *PST_ISP_MODULES_EN_PARAM;

typedef struct _ST_SENSOR_PARAM {
    EN_IMAGER_SEL enImagerSel;
    EN_DVP_ITU656 enItu656;
    EN_DVP_SYNC_POLARITY enPolarity;
} ST_SENSOR_PARAM, *PST_SENSOR_PARAM;

typedef struct _ST_ISP_SENSOR_PARAM {
    STF_U8 u8IspIdx;
    ST_SENSOR_PARAM stSensorParam;
} ST_ISP_SENSOR_PARAM, *PST_ISP_SENSOR_PARAM;

typedef struct _ST_SENSOR_RAW_FORMAT {
    STF_U8 u8IspIdx;
    EN_CFA_RAW_FMT enRawFormat;
} ST_SENSOR_RAW_FORMAT, *PST_SENSOR_RAW_FORMAT;

typedef struct _ST_ISP_CSI_BASE_PARAM {
    ST_SENSOR_PARAM stSensorParam;
    EN_CFA_RAW_FMT enRawFormat;
    ST_RECT stRect;
    EN_ISP_1ST_PXL enFirstPixel;
    EN_ISP_MULTI_FRAME_MODE enMultiFrameMode;
} ST_ISP_CSI_BASE_PARAM, *PST_ISP_CSI_BASE_PARAM;

typedef struct _ST_ISP_CSI_BASE_PARAMETERS {
    STF_U8 u8IspIdx;
    ST_ISP_CSI_BASE_PARAM stIspCsiBaseParam;
} ST_ISP_CSI_BASE_PARAMETERS, *PST_ISP_CSI_BASE_PARAMETERS;

//#if defined(SUPPORT_VIC_2_0)
typedef struct _ST_HW_VER {
    union {
        STF_U32 u32Version;
        struct {
            STF_U8 SubVer   : 4;
            STF_U8 Ver      : 4;
            STF_U8 ID[3];
        };
    };
} ST_HW_VER, *PST_HW_VER;

typedef struct _ST_HW_VERSION {
    STF_CHAR szName[4];
    STF_U8 u8Version;
    STF_U8 u8SubVersion;
} ST_HW_VERSION, *PST_HW_VERSION;

typedef struct _ST_ISP_HW_VERSION {
    STF_U8 u8IspIdx;
    ST_HW_VERSION stHwVersion;
} ST_ISP_HW_VERSION, *PST_ISP_HW_VERSION;

//#endif //#if defined(SUPPORT_VIC_2_0)
#if 0
typedef struct _ST_CSI_CAP_WIN {
    REG_CAPTURE_WINDOW_START TopLeft;               // 0x001C, input picture capture start cfg(x/y start location), each 16bit (cropping).
    REG_CAPTURE_WINDOW_END BottomRight;             // 0x0020, input picture capture end cfg(x/y end location), each 16bit (cropping).
} ST_CSI_CAP_WIN, *PST_CSI_CAP_WIN;

typedef struct _ST_ISP_CTRL {
    REG_ISPCTRL0 Ctrl0;                             // 0x0A00, ISP Control Register 0.
    REG_ISPCTRL1 Ctrl1;                             // 0x0A08, ISP Control Register 1.
} ST_ISP_CTRL, *PST_ISP_CTRL;

typedef struct _ST_ISP_BASE_REG {
    REG_FUNCTION_MODE CsiModule;                    // 0x0010, csi module enable register.
    REG_SENSOR  Sensor;                             // 0x0014, input dvp format & signal polarity cfg.
    REG_CFA CfaRawFormat;                           // 0x0018, input RAW format cfg.
    ST_CSI_CAP_WIN stCsiCapWin;                     // 0x001C - 0x0020, input picture capture region cfg(x/y start and end location), each 16bit (cropping).
    ST_ISP_CTRL stIspCtrl;                          // 0x0A00 and 0x0A08, ISP Control Register.
    REG_IFSZ IspCapSize;                            // 0x0A0C, isp pipeline X/Y size, each 13bits(after decimation).
} ST_ISP_BASE_REG, *PST_ISP_BASE_REG;
#elif 1
typedef struct _ST_ISP_BASE_REG {
    REG_FUNCTION_MODE CsiModule;                    // 0x0010, csi module enable register.
    REG_SENSOR  Sensor;                             // 0x0014, input dvp format & signal polarity cfg.
    REG_CFA CfaRawFormat;                           // 0x0018, input RAW format cfg.
    REG_CAPTURE_WINDOW_START CsiCapWinStart;        // 0x001C, input picture capture start cfg(x/y start location), each 16bit (cropping).
    REG_CAPTURE_WINDOW_END CsiCapWinEnd;            // 0x0020, input picture capture end cfg(x/y end location), each 16bit (cropping).
    REG_ISPCTRL0 IspCtrl0;                          // 0x0A00, ISP Control Register 0.
    REG_ISPCTRL1 IspCtrl1;                          // 0x0A08, ISP Control Register 1.
    REG_IFSZ IspCapSize;                            // 0x0A0C, isp pipeline X/Y size, each 13bits(after decimation).
} ST_ISP_BASE_REG, *PST_ISP_BASE_REG;
#endif

typedef struct _ST_ISP_BASE_REG_PARAM {
    STF_U8 u8IspIdx;
    ST_ISP_BASE_REG stIspBaseReg;
} ST_ISP_BASE_REG_PARAM, *PST_ISP_BASE_REG_PARAM;
#pragma pack(pop)


///* ISP and CSI base registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiEnable(
//    STF_U8 u8IspIdx,
//    STF_BOOL8 bEnable
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsCsiBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsScdDone(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsScdError(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsItu656Field(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsItu656Error(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_U32 STFHAL_ISP_BASE_GetCsiAllStatus(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_U16 STFHAL_ISP_BASE_GetCSTFCount(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_U16 STFHAL_ISP_BASE_GetCsiHCount(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_U16 STFHAL_ISP_BASE_GetCsiLineIntLine(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiLineIntLine(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Line
//    );
////-----------------------------------------------------------------------------
//#if defined(ISP_MODULE_CLOCK_GATE_CONTROL)
//extern
//STF_VOID STFHAL_ISP_BASE_SetModulesClockGateControlEnable(
//    STF_U8 u8IspIdx,
//    ST_MODULES_EN_PARAM *pstModulesEnableParam
//    );
////-----------------------------------------------------------------------------
//#endif //ISP_MODULE_CLOCK_GATE_CONTROL
//extern
//STF_VOID STFHAL_ISP_BASE_SetModuleEnable(
//    STF_U8 u8IspIdx,
//    STF_U64 u64Module,
//    STF_BOOL8 bEnable
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetModulesEnable(
//    STF_U8 u8IspIdx,
//    ST_MODULES_EN_PARAM *pstModulesEnableParam
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BASE_SetSensorParameter(
//    STF_U8 u8IspIdx,
//    EN_IMAGER_SEL enImagerSel,
//    EN_DVP_ITU656 enItu656,
//    EN_DVP_SYNC_POLARITY enPolarity
//    );
//extern
//STF_U32 STFHAL_ISP_BASE_GetSensorRawFormat(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetSensorRawFormat(
//    STF_U8 u8IspIdx,
//    EN_CFA_RAW_FMT enRawFormat
//    );
////-----------------------------------------------------------------------------
//extern
//ST_SIZE STFHAL_ISP_BASE_GetIspCaptureSize(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiCaptureWindowAndIspCaptureSize(
//    STF_U8 u8IspIdx,
//    ST_RECT *pstRect
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspFirstFixel(
//    STF_U8 u8IspIdx,
//    EN_ISP_1ST_PXL enFirstPixel
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspMultiFrameMode(
//    STF_U8 u8IspIdx,
//    EN_ISP_MULTI_FRAME_MODE enMultiFrameMode
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspAndCsiBaseParameters(
//    STF_U8 u8IspIdx,
//    ST_ISP_CSI_BASE_PARAM *pstIspCsiBaseParam
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspEnable(
//    STF_U8 u8IspIdx,
//    STF_BOOL8 bEnable
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspReset(
//    STF_U8 u8IspIdx,
//    STF_BOOL8 bReset
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_IspReset(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_IspResetAndEnable(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsIspBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsUoBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsSs0Busy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsSs1Busy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsOutBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsIspOrOutBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsCsiBufferOverrun(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_U32 STFHAL_ISP_BASE_GetIspAllStatus(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BASE_SetInterrupt(
//    STF_U8 u8IspIdx,
//    EN_ISP_INTERRUPT enInterrupt
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetInterruptClear(
//    STF_U8 u8IspIdx,
//    EN_ISP_INTERRUPT enInterrupt
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsIspDoneInterrupt(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsCsiDoneInterrupt(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsScDoneInterrupt(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_BASE_IsCsiLineInterrupt(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_U32 STFHAL_ISP_BASE_GetInterruptStatus(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//// Since the shadow update enable bit cannot be read back,
//// therefore disable below feature support.
//#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadowMode(
//    STF_U8 u8IspIdx,
//    EN_SUD_MODE enShadowMode
//    );
//#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadow(
//    STF_U8 u8IspIdx,
//    ST_SHADOW_PARAM *pstShadowParam
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadowImmediatelyAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadowNextVSyncAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadowImmediatelyAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetCsiShadowNextVSyncAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_TriggerCsiShadow(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//// Since the shadow update enable bit cannot be read back,
//// therefore disable below feature support.
//#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadowMode(
//    STF_U8 u8IspIdx,
//    EN_SUD_MODE enShadowMode
//    );
//#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadow(
//    STF_U8 u8IspIdx,
//    ST_SHADOW_PARAM *pstShadowParam
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadowImmediatelyAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadowNextVSyncAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadowImmediatelyAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_SetIspShadowNextVSyncAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_TriggerIspShadow(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BASE_CsiAndIspStart(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_BASE_CsiAndIspStop(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_BASE_REG_H__


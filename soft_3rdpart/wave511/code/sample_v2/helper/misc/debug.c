/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "wave/wave5_regdefine.h"
#include "wave/wave5.h"
#include "coda9/coda9_regdefine.h"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#endif
#include "main_helper.h"
#include "misc/debug.h"
#include "vpuconfig.h"


enum { False, True };

void ExecuteDebugger(void)
{
    VLOG(INFO, "Starting the debugger....\n");
}

void InitializeDebugEnv(Uint32  option)
{
    switch(option) {
    case CNMQC_ENV_GDBSERVER: ExecuteDebugger(); break;
    default: break;
    }
}

void ReleaseDebugEnv(void)
{
}

Int32 checkLineFeedInHelp(
    struct OptionExt *opt
    )
{
    int i;

    for (i=0;i<MAX_GETOPT_OPTIONS;i++) {
        if (opt[i].name==NULL)
            break;
        if (!strstr(opt[i].help, "\n")) {
            VLOG(INFO, "(%s) doesn't have \\n in options struct in main function. please add \\n\n", opt[i].help);
            return FALSE;
        }
    }
    return TRUE;
}

RetCode PrintVpuProductInfo(
    Uint32   coreIdx,
    VpuAttr* productInfo
    )
{
    BOOL verbose = FALSE;
    RetCode ret = RETCODE_SUCCESS;

    if ((ret = VPU_GetProductInfo(coreIdx, productInfo)) != RETCODE_SUCCESS) {
        //PrintVpuStatus(coreIdx, productInfo->productId);//TODO
        return ret;
    }

    VLOG(INFO, "VPU coreNum : [%d]\n", coreIdx);
    VLOG(INFO, "Firmware : CustomerCode: %04x | version : rev.%d\n", productInfo->customerId, productInfo->fwVersion);
    VLOG(INFO, "Hardware : %04x\n", productInfo->productId);
    VLOG(INFO, "API      : %d.%d.%d\n\n", API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    if (PRODUCT_ID_W_SERIES(productInfo->productId))
    {
        VLOG(INFO, "productId       : %08x\n", productInfo->productId);
        VLOG(INFO, "fwVersion       : %08x(r%d)\n", productInfo->fwVersion, productInfo->fwVersion);
        VLOG(INFO, "productName     : %s%4x\n", productInfo->productName, productInfo->productVersion);
        if ( verbose == TRUE )
        {
            Uint32 stdDef0          = productInfo->hwConfigDef0;
            Uint32 stdDef1          = productInfo->hwConfigDef1;
            Uint32 confFeature      = productInfo->hwConfigFeature;
            BOOL supportDownScaler  = FALSE;
            BOOL supportAfbce       = FALSE;
            char ch_ox[2]           = {'X', 'O'};

            VLOG(INFO, "==========================\n");
            VLOG(INFO, "stdDef0           : %08x\n", stdDef0);
            /* checking ONE AXI BIT FILE */
            VLOG(INFO, "MAP CONVERTER REG : %d\n", (stdDef0>>31)&1);
            VLOG(INFO, "MAP CONVERTER SIG : %d\n", (stdDef0>>30)&1);
            VLOG(INFO, "PVRIC FBC EN      : %d\n", (stdDef0>>27)&1);
            VLOG(INFO, "PVRIC FBC ID      : %d\n", (stdDef0>>24)&7);
            VLOG(INFO, "SCALER 2ALIGNED   : %d\n", (stdDef0>>23)&1);
            VLOG(INFO, "VCORE BACKBONE    : %d\n", (stdDef0>>22)&1);
            VLOG(INFO, "STD SWITCH EN     : %d\n", (stdDef0>>21)&1);
            VLOG(INFO, "BG_DETECT         : %d\n", (stdDef0>>20)&1);
            VLOG(INFO, "3D NR             : %d\n", (stdDef0>>19)&1);
            VLOG(INFO, "ONE-PORT AXI      : %d\n", (stdDef0>>18)&1);
            VLOG(INFO, "2nd AXI           : %d\n", (stdDef0>>17)&1);
            VLOG(INFO, "GDI               : %d\n", !((stdDef0>>16)&1));//no-gdi
            VLOG(INFO, "AFBC              : %d\n", (stdDef0>>15)&1);
            VLOG(INFO, "AFBC VERSION      : %d\n", (stdDef0>>12)&7);
            VLOG(INFO, "FBC               : %d\n", (stdDef0>>11)&1);
            VLOG(INFO, "FBC  VERSION      : %d\n", (stdDef0>>8)&7);
            VLOG(INFO, "SCALER            : %d\n", (stdDef0>>7)&1);
            VLOG(INFO, "SCALER VERSION    : %d\n", (stdDef0>>4)&7);
            VLOG(INFO, "BWB               : %d\n", (stdDef0>>3)&1);
            VLOG(INFO, "==========================\n");
            VLOG(INFO, "stdDef1           : %08x\n", stdDef1);
            VLOG(INFO, "CyclePerTick      : %d\n", (stdDef1>>27)&1); //0:32768, 1:256
            VLOG(INFO, "MULTI CORE EN     : %d\n", (stdDef1>>26)&1);
            VLOG(INFO, "GCU EN            : %d\n", (stdDef1>>25)&1);
            VLOG(INFO, "CU REPORT         : %d\n", (stdDef1>>24)&1);
            VLOG(INFO, "VCORE ID 3        : %d\n", (stdDef1>>19)&1);
            VLOG(INFO, "VCORE ID 2        : %d\n", (stdDef1>>18)&1);
            VLOG(INFO, "VCORE ID 1        : %d\n", (stdDef1>>17)&1);
            VLOG(INFO, "VCORE ID 0        : %d\n", (stdDef1>>16)&1);
            VLOG(INFO, "BW OPT            : %d\n", (stdDef1>>15)&1);
            VLOG(INFO, "CODEC STD AV1     : %d\n", (stdDef1>>4)&1);
            VLOG(INFO, "CODEC STD AVS2    : %d\n", (stdDef1>>3)&1);
            VLOG(INFO, "CODEC STD AVC     : %d\n", (stdDef1>>2)&1);
            VLOG(INFO, "CODEC STD VP9     : %d\n", (stdDef1>>1)&1);
            VLOG(INFO, "CODEC STD HEVC    : %d\n", (stdDef1>>0)&1);

            VLOG(INFO, "==========================\n");
            VLOG(INFO, "confFeature       : %08x\n", confFeature);
            if ( productInfo->hwConfigRev > 167455 ) {//20190321
                VLOG(INFO, "AVC  ENC MAIN10   : %d\n", (confFeature>>11)&1);
                VLOG(INFO, "AVC  ENC MAIN     : %d\n", (confFeature>>10)&1);
                VLOG(INFO, "AVC  DEC MAIN10   : %d\n", (confFeature>>9)&1);
                VLOG(INFO, "AVC  DEC MAIN     : %d\n", (confFeature>>8)&1);
            }
            else {
                VLOG(INFO, "AVC  ENC          : %d\n", (confFeature>>9)&1);
                VLOG(INFO, "AVC  DEC          : %d\n", (confFeature>>8)&1);
            }
            VLOG(INFO, "AV1  ENC PROF     : %d\n", (confFeature>>14)&1);
            VLOG(INFO, "AV1  DEC HIGH     : %d\n", (confFeature>>13)&1);
            VLOG(INFO, "AV1  DEC MAIN     : %d\n", (confFeature>>12)&1);

            VLOG(INFO, "AVC  ENC MAIN10   : %d\n", (confFeature>>11)&1);
            VLOG(INFO, "AVC  ENC MAIN     : %d\n", (confFeature>>10)&1);
            VLOG(INFO, "AVC  DEC MAIN10   : %d\n", (confFeature>>9)&1);
            VLOG(INFO, "AVC  DEC MAIN     : %d\n", (confFeature>>8)&1);

            VLOG(INFO, "VP9  ENC Profile2 : %d\n", (confFeature>>7)&1);
            VLOG(INFO, "VP9  ENC Profile0 : %d\n", (confFeature>>6)&1);
            VLOG(INFO, "VP9  DEC Profile2 : %d\n", (confFeature>>5)&1);
            VLOG(INFO, "VP9  DEC Profile0 : %d\n", (confFeature>>4)&1);
            VLOG(INFO, "HEVC ENC MAIN10   : %d\n", (confFeature>>3)&1);
            VLOG(INFO, "HEVC ENC MAIN     : %d\n", (confFeature>>2)&1);
            VLOG(INFO, "HEVC DEC MAIN10   : %d\n", (confFeature>>1)&1);
            VLOG(INFO, "HEVC DEC MAIN     : %d\n", (confFeature>>0)&1);
            VLOG(INFO, "==========================\n");
            VLOG(INFO, "configDate        : %d\n", productInfo->hwConfigDate);
            VLOG(INFO, "HW version        : r%d\n", productInfo->hwConfigRev);

            supportDownScaler = (BOOL)((stdDef0>>7)&1);
            supportAfbce      = (BOOL)((stdDef0>>15)&1);

            VLOG (INFO, "------------------------------------\n");
            VLOG (INFO, "VPU CONF| SCALER | AFBCE  |\n");
            VLOG (INFO, "        |   %c    |    %c   |\n", ch_ox[supportDownScaler], ch_ox[supportAfbce]);
            VLOG (INFO, "------------------------------------\n");
        }
        else {
            VLOG(INFO, "==========================\n");
            VLOG(INFO, "stdDef0          : %08x\n", productInfo->hwConfigDef0);
            VLOG(INFO, "stdDef1          : %08x\n", productInfo->hwConfigDef1);
            VLOG(INFO, "confFeature      : %08x\n", productInfo->hwConfigFeature);
            VLOG(INFO, "configDate       : %08x\n", productInfo->hwConfigDate);
            VLOG(INFO, "configRevision   : %08x\n", productInfo->hwConfigRev);
            VLOG(INFO, "configType       : %08x\n", productInfo->hwConfigType);
            VLOG(INFO, "==========================\n");
        }
    }
    return ret;
}

#define FIO_DBG_IRB_ADDR    0x8074
#define FIO_DBG_IRB_DATA    0x8078
#define FIO_DBG_IRB_STATUS  0x807C
void vdi_irb_write_register(
    unsigned long coreIdx,
    unsigned int  vcore_idx,
    unsigned int  irb_addr,
    unsigned int  irb_data)
{
    vdi_fio_write_register(coreIdx, FIO_DBG_IRB_DATA + 0x1000*vcore_idx, irb_data);
    vdi_fio_write_register(coreIdx, FIO_DBG_IRB_ADDR + 0x1000*vcore_idx, irb_addr);
}

unsigned int vdi_irb_read_register(
    unsigned long coreIdx,
    unsigned int  vcore_idx,
    unsigned int  irb_addr
    )
{
    unsigned int irb_rdata = 0;

    unsigned int irb_rd_cmd = 0;

    irb_rd_cmd = (1<<20)| (1<<16) | irb_addr; // {dbgMode, Read, Addr}
    vdi_fio_write_register(coreIdx, FIO_DBG_IRB_ADDR + 0x1000*vcore_idx, irb_rd_cmd);
    while((vdi_fio_read_register(coreIdx, FIO_DBG_IRB_STATUS + 0x1000*vcore_idx) & 0x1) == 0);

    irb_rdata = vdi_fio_read_register(coreIdx, FIO_DBG_IRB_DATA + 0x1000*vcore_idx);
    return irb_rdata;
}

char dumpTime[200];

/******************************************************************************
* help function                                                               *
******************************************************************************/

void DisplayHex(void *mem, Uint32 len, char* name)
{
    Uint32   i, j;

    VLOG(INFO, "  addr  : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F %s \n", name);
    for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
        /* print offset */
        if(i % HEXDUMP_COLS == 0) {
            VLOG(INFO, "0x%06x: ", i);
        }

        /* print hex data */
        if(i < len) {
            VLOG(INFO, "%02x ", 0xFF & ((char*)mem)[i]);
        }
        else /* end of block, just aligning for ASCII dump */ {
            VLOG(INFO, "   ");
        }

        /* print ASCII dump */
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= len) { /* end of block, not really printing */
                    VLOG(INFO, " ");
                }
                else if(isprint(((char*)mem)[j])) { /* printable char */
                    VLOG(INFO, "%c", 0xFF & ((char*)mem)[j]);
                }
                else { /* other char */
                    VLOG(INFO, ".");
                }
            }
            VLOG(INFO, "\n");
        }
    }
}


Uint32 ReadRegVCE(
    Uint32 coreIdx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    )
{//lint !e18
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000*vce_core_idx;

    SetClockGate(coreIdx, 1);
    vdi_fio_write_register(coreIdx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(coreIdx, VCORE_DBG_ADDR(vce_core_idx),vcpu_reg_addr + vce_core_base);

    if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) == 1)
        udata= vdi_fio_read_register(0, VCORE_DBG_DATA(vce_core_idx));
    else {
        VLOG(ERR, "failed to read VCE register: %d, 0x%04x\n", vce_core_idx, vce_addr);
        udata = -2;//-1 can be a valid value
    }

    SetClockGate(coreIdx, 0);
    return udata;
}

void WriteRegVCE(
    Uint32   coreIdx,
    Uint32   vce_core_idx,
    Uint32   vce_addr,
    Uint32   udata
    )
{
    int vcpu_reg_addr;

    SetClockGate(coreIdx, 1);

    vdi_fio_write_register(coreIdx, VCORE_DBG_READY(vce_core_idx),0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(coreIdx, VCORE_DBG_DATA(vce_core_idx),udata);
    vdi_fio_write_register(coreIdx, VCORE_DBG_ADDR(vce_core_idx),(vcpu_reg_addr) & 0x00007FFF);

    while (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) == 0xffffffff) {
        VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);
    }
    SetClockGate(coreIdx, 0);
}

#define VCE_DEC_CHECK_SUM0         0x110
#define VCE_DEC_CHECK_SUM1         0x114
#define VCE_DEC_CHECK_SUM2         0x118
#define VCE_DEC_CHECK_SUM3         0x11C
#define VCE_DEC_CHECK_SUM4         0x120
#define VCE_DEC_CHECK_SUM5         0x124
#define VCE_DEC_CHECK_SUM6         0x128
#define VCE_DEC_CHECK_SUM7         0x12C
#define VCE_DEC_CHECK_SUM8         0x130
#define VCE_DEC_CHECK_SUM9         0x134
#define VCE_DEC_CHECK_SUM10        0x138
#define VCE_DEC_CHECK_SUM11        0x13C

#define READ_BIT(val,high,low) ((((high)==31) && ((low) == 0)) ?  (val) : (((val)>>(low)) & (((1<< ((high)-(low)+1))-1))))


static void	DisplayVceEncDebugCommon521(int coreIdx, int vcore_idx, int set_mode, int debug0, int debug1, int debug2)
{
    int reg_val;
    VLOG(ERR, "---------------Common Debug INFO-----------------\n");

    WriteRegVCE(coreIdx, vcore_idx, set_mode,0 );

    reg_val = ReadRegVCE(coreIdx, vcore_idx, debug0);
    VLOG(ERR,"\t- subblok_done    :  0x%x\n", READ_BIT(reg_val,30,23));
    VLOG(ERR,"\t- pipe_on[4]      :  0x%x\n", READ_BIT(reg_val,20,20));
    VLOG(ERR,"\t- cur_s2ime       :  0x%x\n", READ_BIT(reg_val,19,16));
    VLOG(ERR,"\t- cur_pipe        :  0x%x\n", READ_BIT(reg_val,15,12));
    VLOG(ERR,"\t- pipe_on[3:0]    :  0x%x\n", READ_BIT(reg_val,11, 8));

    reg_val = ReadRegVCE(coreIdx, vcore_idx, debug1);
    VLOG(ERR,"\t- i_avc_rdo_debug :  0x%x\n", READ_BIT(reg_val,31,31));
    VLOG(ERR,"\t- curbuf_prp      :  0x%x\n", READ_BIT(reg_val,28,25));
    VLOG(ERR,"\t- curbuf_s2       :  0x%x\n", READ_BIT(reg_val,24,21));
    VLOG(ERR,"\t- curbuf_s0       :  0x%x\n", READ_BIT(reg_val,20,17));
    VLOG(ERR,"\t- cur_s2ime_sel   :  0x%x\n", READ_BIT(reg_val,16,16));
    VLOG(ERR,"\t- cur_mvp         :  0x%x\n", READ_BIT(reg_val,15,14));
    VLOG(ERR,"\t- cmd_ready       :  0x%x\n", READ_BIT(reg_val,13,13));
    VLOG(ERR,"\t- rc_ready        :  0x%x\n", READ_BIT(reg_val,12,12));
    VLOG(ERR,"\t- pipe_cmd_cnt    :  0x%x\n", READ_BIT(reg_val,11, 9));
    VLOG(ERR,"\t- subblok_done    :  LF_PARAM 0x%x SFU 0x%x LF 0x%x RDO 0x%x IMD 0x%x FME 0x%x IME 0x%x\n",
        READ_BIT(reg_val, 6, 6), READ_BIT(reg_val, 5, 5), READ_BIT(reg_val, 4, 4), READ_BIT(reg_val, 3, 3),
        READ_BIT(reg_val, 2, 2), READ_BIT(reg_val, 1, 1), READ_BIT(reg_val, 0, 0));

    reg_val = ReadRegVCE(coreIdx, vcore_idx, debug2);
    //VLOG(ERR,"\t- reserved          :  0x%x\n", READ_BIT(reg_val,31, 23));
    VLOG(ERR,"\t- cur_prp_dma_state :  0x%x\n", READ_BIT(reg_val,22, 20));
    VLOG(ERR,"\t- cur_prp_state     :  0x%x\n", READ_BIT(reg_val,19, 18));
    VLOG(ERR,"\t- main_ctu_xpos     :  0x%x\n", READ_BIT(reg_val,17,  9));
    VLOG(ERR,"\t- main_ctu_ypos     :  0x%x\n", READ_BIT(reg_val, 8,  0));
}

static void	DisplayVceEncDebugMode2(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 2 : ----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode, 2);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- s2fme_info_full    :  0x%x\n", READ_BIT(reg_val,26,26));
    VLOG(ERR,"\t- ime_cmd_ref_full   :  0x%x\n", READ_BIT(reg_val,25,25));
    VLOG(ERR,"\t- ime_cmd_ctb_full   :  0x%x\n", READ_BIT(reg_val,24,24));
    VLOG(ERR,"\t- ime_load_info_full :  0x%x\n", READ_BIT(reg_val,23,23));
    VLOG(ERR,"\t- mvp_nb_info_full   :  0x%x\n", READ_BIT(reg_val,22,22));
    VLOG(ERR,"\t- ime_final_mv_full  :  0x%x\n", READ_BIT(reg_val,21,21));
    VLOG(ERR,"\t- ime_mv_full        :  0x%x\n", READ_BIT(reg_val,20,20));
    VLOG(ERR,"\t- cur_fme_fsm[3:0]   :  0x%x\n", READ_BIT(reg_val,19,16));
    VLOG(ERR,"\t- cur_s2me_fsm[3:0]  :  0x%x\n", READ_BIT(reg_val,15,12));
    VLOG(ERR,"\t- cur_s2mvp_fsm[3:0] :  0x%x\n", READ_BIT(reg_val,11, 8));
    VLOG(ERR,"\t- cur_ime_fsm[3:0]   :  0x%x\n", READ_BIT(reg_val, 7, 4));
    VLOG(ERR,"\t- cur_sam_fsm[3:0]   :  0x%x\n", READ_BIT(reg_val, 3, 0));
}

#define VCE_LF_PARAM               0xA6c
#define VCE_BIN_WDMA_CUR_ADDR      0xB1C
#define VCE_BIN_PIC_PARAM          0xB20
#define VCE_BIN_WDMA_BASE          0xB24
#define VCE_BIN_WDMA_END           0xB28
static void	DisplayVceEncReadVCE(int coreIdx, int vcore_idx)
{
    int reg_val;

    VLOG(ERR, "---------------DisplayVceEncReadVCE-----------------\n");
    reg_val = ReadRegVCE(coreIdx, vcore_idx, VCE_LF_PARAM);
    VLOG(ERR,"\t- VCE_LF_PARAM             :  0x%x\n", reg_val);

    reg_val = ReadRegVCE(coreIdx, vcore_idx, VCE_BIN_WDMA_CUR_ADDR);
    VLOG(ERR,"\t- VCE_BIN_WDMA_CUR_ADDR    :  0x%x\n", reg_val);
    reg_val = ReadRegVCE(coreIdx, vcore_idx, VCE_BIN_PIC_PARAM);
    VLOG(ERR,"\t- VCE_BIN_PIC_PARAM        :  0x%x\n", reg_val);
    reg_val = ReadRegVCE(coreIdx, vcore_idx, VCE_BIN_WDMA_BASE);
    VLOG(ERR,"\t- VCE_BIN_WDMA_BASE        :  0x%x\n", reg_val);
    reg_val = ReadRegVCE(coreIdx, vcore_idx, VCE_BIN_WDMA_END);
    VLOG(ERR,"\t- VCE_BIN_WDMA_END         :  0x%x\n", reg_val);
}


void PrintWave5xxDecSppStatus(
    Uint32 coreIdx
    )
{
    Uint32  regVal;
    //DECODER SDMA INFO
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5000);
    VLOG(ERR,"C_SDMA_LOAD_CMD    = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5004);
    VLOG(ERR,"C_SDMA_AUTO_MODE  = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5008);
    VLOG(ERR,"C_SDMA_START_ADDR  = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x500C);
    VLOG(ERR,"C_SDMA_END_ADDR   = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5010);
    VLOG(ERR,"C_SDMA_ENDIAN     = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5014);
    VLOG(ERR,"C_SDMA_IRQ_CLEAR  = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5018);
    VLOG(ERR,"C_SDMA_BUSY       = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x501C);
    VLOG(ERR,"C_SDMA_LAST_ADDR  = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5020);
    VLOG(ERR,"C_SDMA_SC_BASE_ADDR = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5400);
    VLOG(ERR,"C_SHU_INIT = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x5404);
    VLOG(ERR,"C_SHU_SEEK_NXT_NAL = 0x%x\n",regVal);
    regVal = vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x540C);
    VLOG(ERR,"C_SHU_SATUS = 0x%x\n",regVal);
}

void PrintVpuStatus(
    Uint32 coreIdx,
    Uint32 productId
    )
{

}

void PrintDecVpuStatus(
    DecHandle   handle
    )
{
    Int32       coreIdx;
    coreIdx   = handle->coreIdx;

    SetClockGate(coreIdx, 1);
    vdi_print_vpu_status(coreIdx);
    vdi_print_vpu_status_dec(coreIdx);
    SetClockGate(coreIdx, 0);
}

void PrintEncVpuStatus(
    EncHandle   handle
    )
{
    Int32       coreIdx;
    coreIdx   = handle->coreIdx;

    SetClockGate(coreIdx, 1);
    vdi_print_vpu_status(coreIdx);
    vdi_print_vpu_status_enc(coreIdx);
    SetClockGate(coreIdx, 0);
}


void HandleEncoderError(
    EncHandle       handle,
    Uint32          encPicCnt,
    EncOutputInfo*  outputInfo
    )
{
/*lint -save -e527 */
    UNREFERENCED_PARAMETER(handle);
    /*lint -restore */
}

void DumpCodeBuffer(
    const char* path
    )
{
    Uint8*          buffer;
    vpu_buffer_t    vb;
    PhysicalAddress addr;
    osal_file_t     ofp;

    VLOG(ERR,"DUMP CODE AREA into %s ", path);

    buffer = (Uint8*)osal_malloc(1024*1024);
    if ((ofp=osal_fopen(path, "wb")) == NULL) {
        VLOG(ERR,"[FAIL]\n");
    }
    else {
        vdi_get_common_memory(0, &vb);

        addr   = vb.phys_addr;
        VpuReadMem(0, addr, buffer, WAVE5_MAX_CODE_BUF_SIZE, VDI_128BIT_LITTLE_ENDIAN);
        osal_fwrite(buffer, 1, WAVE5_MAX_CODE_BUF_SIZE, ofp);
        osal_fclose(ofp);
        VLOG(ERR,"[OK]\n");
    }
    osal_free(buffer);
}

void HandleDecoderError(
    DecHandle       handle,
    Uint32          frameIdx,
    DecOutputInfo*  outputInfo
    )
{
    UNREFERENCED_PARAMETER(handle);
    UNREFERENCED_PARAMETER(outputInfo);
}

void PrintMemoryAccessViolationReason(
    Uint32          coreIdx,
    void            *outp
    )
{
    UNREFERENCED_PARAMETER(coreIdx);
    UNREFERENCED_PARAMETER(outp);
}

/**
* \brief           Handle error cases depending on product
* \return  -1      SEQUENCE ERROR
*/
Int32 HandleDecInitSequenceError(DecHandle handle, Uint32 productId, DecOpenParam* openParam, DecInitialInfo* seqInfo, RetCode apiErrorCode)
{
    if (apiErrorCode == RETCODE_MEMORY_ACCESS_VIOLATION) {
        PrintMemoryAccessViolationReason(handle->coreIdx, NULL);
        return -1;
    }

    if (PRODUCT_ID_W_SERIES(productId)) {
        if (seqInfo->seqInitErrReason == WAVE5_ETCERR_INIT_SEQ_SPS_NOT_FOUND) {
            return -2;
        } else {
            if (seqInfo->seqInitErrReason == WAVE5_SPECERR_OVER_PICTURE_WIDTH_SIZE) {
                VLOG(ERR, "Not supported picture width: MAX_SIZE(8192): %d\n", seqInfo->picWidth);
            }
            if (seqInfo->seqInitErrReason == WAVE5_SPECERR_OVER_PICTURE_HEIGHT_SIZE) {
                VLOG(ERR, "Not supported picture height: MAX_SIZE(8192): %d\n", seqInfo->picHeight);
            }
            if (seqInfo->seqInitErrReason == WAVE5_SPECERR_OVER_CHROMA_FORMAT) {
                VLOG(ERR, "Not supported chroma idc: %d\n", seqInfo->chromaFormatIDC);
            }
            if (seqInfo->seqInitErrReason == WAVE5_SPECERR_OVER_BIT_DEPTH) {
                VLOG(ERR, "Not supported Luma or Chroma bitdepth: L(%d), C(%d)\n", seqInfo->lumaBitdepth, seqInfo->chromaBitdepth);
            }
            if (seqInfo->warnInfo == WAVE5_SPECWARN_OVER_PROFILE) {
                VLOG(INFO, "SPEC over profile: %d\n", seqInfo->profile);
            }
            if (seqInfo->warnInfo == WAVE5_ETCWARN_INIT_SEQ_VCL_NOT_FOUND) {
                VLOG(INFO, "VCL Not found : RD_PTR(0x%08x), WR_PTR(0x%08x)\n", seqInfo->rdPtr, seqInfo->wrPtr);
            }
            return -1;
        }
    }
    else {
        if (openParam->bitstreamMode == BS_MODE_PIC_END && (seqInfo->seqInitErrReason&(1UL<<31))) {
            VLOG(ERR, "SEQUENCE HEADER NOT FOUND\n");
            return -1;
        }
        else {
            return -1;
        }
    }
}



enum {
    VDI_PRODUCT_ID_980,
    VDI_PRODUCT_ID_960
};

static int read_pinfo_buffer(int coreIdx, int addr)
{
    int ack;
    int rdata;
#define VDI_LOG_GDI_PINFO_ADDR  (0x1068)
#define VDI_LOG_GDI_PINFO_REQ   (0x1060)
#define VDI_LOG_GDI_PINFO_ACK   (0x1064)
#define VDI_LOG_GDI_PINFO_DATA  (0x106c)
    //------------------------------------------
    // read pinfo - indirect read
    // 1. set read addr     (GDI_PINFO_ADDR)
    // 2. send req          (GDI_PINFO_REQ)
    // 3. wait until ack==1 (GDI_PINFO_ACK)
    // 4. read data         (GDI_PINFO_DATA)
    //------------------------------------------
    vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_ADDR, addr);
    vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_REQ, 1);

    ack = 0;
    while (ack == 0)
    {
        ack = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_ACK);
    }

    rdata = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_DATA);

    //VLOG(INFO, "[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
    return rdata;
}

static void printf_gdi_info(int coreIdx, int num, int reset)
{
    int i;
    int bus_info_addr;
    int tmp;
    int val;
    int productId = 0;

    val = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);
    if ((val&0xff00) == 0x3200) val = 0x3200;

    if (PRODUCT_CODE_W_SERIES(val)) {
        return;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(val)) {
        if (val == CODA960_CODE || val == BODA950_CODE)
            productId = VDI_PRODUCT_ID_960;
        else if (val == CODA980_CODE)
            productId = VDI_PRODUCT_ID_980;
        else
            return;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", val);
        return;
    }

    if (productId == VDI_PRODUCT_ID_980)
        VLOG(INFO, "\n**GDI information for GDI_20\n");
    else
        VLOG(INFO, "\n**GDI information for GDI_10\n");

    for (i=0; i < num; i++)
    {

#define VDI_LOG_GDI_INFO_CONTROL 0x1400
        if (productId == VDI_PRODUCT_ID_980)
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*(0x20);
        else
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*0x14;
        if (reset)
        {
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);

            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);
            }

        }
        else
        {
            VLOG(INFO, "index = %02d", i);

            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);	//TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
            VLOG(INFO, " control = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " pic_size = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " y-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " cb-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " cr-top = 0x%08x", tmp);
            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
                VLOG(INFO, " y-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
                VLOG(INFO, " cb-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
                VLOG(INFO, " cr-bot = 0x%08x", tmp);
            }
            VLOG(INFO, "\n");
        }
    }
}

void vdi_make_log(unsigned long coreIdx, const char *str, int step)
{ //lint !e578
    Uint32 val;

    val = VpuReadReg(coreIdx, W5_CMD_INSTANCE_INFO);
    val &= 0xffff;
    if (step == 1)
        VLOG(INFO, "\n**%s start(%d)\n", str, val);
    else if (step == 2)	//
        VLOG(INFO, "\n**%s timeout(%d)\n", str, val);
    else
        VLOG(INFO, "\n**%s end(%d)\n", str, val);
}

void vdi_log(unsigned long coreIdx, int cmd, int step)
{ //lint !e578
    int i;
    int productId;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return ;

    productId = VPU_GetProductId(coreIdx);

    if (PRODUCT_ID_W_SERIES(productId))
    {
        switch(cmd)
        {
        case W5_INIT_VPU:
            vdi_make_log(coreIdx, "INIT_VPU", step);
            break;
        case W5_ENC_SET_PARAM:
            vdi_make_log(coreIdx, "ENC_SET_PARAM", step);
            break;
        case W5_INIT_SEQ:
            vdi_make_log(coreIdx, "DEC INIT_SEQ", step);
            break;
        case W5_DESTROY_INSTANCE:
            vdi_make_log(coreIdx, "DESTROY_INSTANCE", step);
            break;
        case W5_DEC_PIC://ENC_PIC for ENC
            vdi_make_log(coreIdx, "DEC_PIC(ENC_PIC)", step);
            break;
        case W5_SET_FB:
            vdi_make_log(coreIdx, "SET_FRAMEBUF", step);
            break;
        case W5_FLUSH_INSTANCE:
            vdi_make_log(coreIdx, "FLUSH INSTANCE", step);
            break;
        case W5_QUERY:
            vdi_make_log(coreIdx, "QUERY", step);
            break;
        case W5_SLEEP_VPU:
            vdi_make_log(coreIdx, "SLEEP_VPU", step);
            break;
        case W5_WAKEUP_VPU:
            vdi_make_log(coreIdx, "WAKEUP_VPU", step);
            break;
        case W5_UPDATE_BS:
            vdi_make_log(coreIdx, "UPDATE_BS", step);
            break;
        case W5_CREATE_INSTANCE:
            vdi_make_log(coreIdx, "CREATE_INSTANCE", step);
            break;
        default:
            vdi_make_log(coreIdx, "ANY_CMD", step);
            break;
        }
    }
    else if (PRODUCT_ID_NOT_W_SERIES(productId))
    {
        switch(cmd)
        {
        case ENC_SEQ_INIT://DEC_SEQ_INNT
            vdi_make_log(coreIdx, "SEQ_INIT", step);
            break;
        case ENC_SEQ_END://DEC_SEQ_END
            vdi_make_log(coreIdx, "SEQ_END", step);
            break;
        case PIC_RUN:
            vdi_make_log(coreIdx, "PIC_RUN", step);
            break;
        case SET_FRAME_BUF:
            vdi_make_log(coreIdx, "SET_FRAME_BUF", step);
            break;
        case ENCODE_HEADER:
            vdi_make_log(coreIdx, "ENCODE_HEADER", step);
            break;
        case RC_CHANGE_PARAMETER:
            vdi_make_log(coreIdx, "RC_CHANGE_PARAMETER", step);
            break;
        case DEC_BUF_FLUSH:
            vdi_make_log(coreIdx, "DEC_BUF_FLUSH", step);
            break;
        case FIRMWARE_GET:
            vdi_make_log(coreIdx, "FIRMWARE_GET", step);
            break;
        case ENC_PARA_SET:
            vdi_make_log(coreIdx, "ENC_PARA_SET", step);
            break;
        case DEC_PARA_SET:
            vdi_make_log(coreIdx, "DEC_PARA_SET", step);
            break;
        default:
            vdi_make_log(coreIdx, "ANY_CMD", step);
            break;
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", productId);
        return;
    }

    for (i=0x0; i<0x200; i=i+16) { // host IF register 0x100 ~ 0x200
        VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            vdi_read_register(coreIdx, i), vdi_read_register(coreIdx, i+4),
            vdi_read_register(coreIdx, i+8), vdi_read_register(coreIdx, i+0xc));
    }

    if (PRODUCT_ID_W_SERIES(productId))
    {
        if (cmd == W5_INIT_VPU || cmd == W5_RESET_VPU || cmd == W5_CREATE_INSTANCE)
        {
            vdi_print_vpu_status(coreIdx);
        }
    }
    else if (PRODUCT_ID_NOT_W_SERIES(productId))
    {
        if (cmd == PIC_RUN && step== 0)
        {
            printf_gdi_info(coreIdx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE	 0x0C00
#define VDI_LOG_MC_BUSY	 0x0C04
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
#define VDI_LOG_ROT_SRC_IDX	 (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX	 (0x400 + 0x110)

            VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MBC_BUSY));
            VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MC_BUSY));
            VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n", (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 20) & 0x3F, (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
            VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(coreIdx, VDI_LOG_GDI_BUS_STATUS));

            VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_SRC_IDX));
            VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_DST_IDX));

            VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x200));
            VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x20c));
            VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x218));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x230));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x23C));
            VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x248));
            VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x254));
            VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x260));
            VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x26C));
            VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x278));
            VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x284));
            VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(coreIdx, MC_BASE+0x290));
            VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(coreIdx, MC_BASE+0x29C));
            VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2A8));
            VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2B4));

            VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x028));
            VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x02C));
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", productId);
        return;
    }
}

void print_busy_timeout_status(Uint32 coreIdx, Uint32 product_code, Uint32 pc)
{
    if (PRODUCT_CODE_W_SERIES(product_code)) {
        wave5xx_vcpu_status(coreIdx);
    } else {
        Uint32 idx;
        for (idx=0; idx<20; idx++) {
            VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%lx\n", vdi_read_register(coreIdx, pc));
        }
    }
}

void wave5xx_vcore_status(
    Uint32 coreIdx
    )
{
    Uint32 i;
    Uint32 temp;

    VLOG(INFO,"[+] BPU REG Dump\n");
    for (i=0;i < 20; i++) {
        temp = vdi_fio_read_register(coreIdx, (W5_REG_BASE + 0x8000 + 0x18));
        VLOG(ERR,"BITPC = 0x%08x\n", temp);
    }

    for(i = 0x8000; i < 0x80FC; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }

    VLOG(INFO,"[-] BPU REG Dump\n");




    VLOG(INFO,"[+] MIB REG Dump\n");
    for (i=0x110 ; i < 0x118 ; i++) {
        temp  = vdi_irb_read_register(coreIdx, 0, i);
        VLOG(ERR,"MIB 0x%08x Core0=0x%08x\n", i, temp);
    }
    VLOG(INFO,"[-] MIB REG Dump\n");

    // --------- VCE register Dump
    VLOG(INFO,"[+] VCE REG Dump Core0\n");
    for (i=0x000; i<0x1fc; i+=16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            ReadRegVCE(coreIdx, 0, (i+0x00)),
            ReadRegVCE(coreIdx, 0, (i+0x04)),
            ReadRegVCE(coreIdx, 0, (i+0x08)),
            ReadRegVCE(coreIdx, 0, (i+0x0c)));
    }
    VLOG(INFO,"[-] VCE REG Dump\n");
}


void wave5xx_bpu_status(
    Uint32 coreIdx
    )
{
    Uint32 i;
    Uint32 temp;

    VLOG(INFO,"[+] BPU REG Dump\n");
    for (i=0;i < 20; i++) {
        temp = vdi_fio_read_register(coreIdx, (W5_REG_BASE + 0x8000 + 0x18));
        VLOG(ERR,"BITPC = 0x%08x\n", temp);
    }

    temp = vdi_fio_read_register(coreIdx, (W5_REG_BASE + 0x8000 + 0x30));
    VLOG(ERR,"BIT_BUSY Core0=0x%08x \n", temp);

    for (i=0; i < 8; i += 4 ) {
        temp = vdi_fio_read_register(coreIdx, (W5_REG_BASE + 0x8000 + 0x80 + i));
        VLOG(ERR,"stack[%d] Core0=0x%08x\n", temp);
    }



    for(i = 0x8000; i < 0x81FC; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }

    VLOG(INFO,"[-] BPU REG Dump\n");

    VLOG(INFO,"[+] MIB REG Dump\n");
    for (i=0x110 ; i < 0x118 ; i++) {
        temp  = vdi_irb_read_register(coreIdx, 0, i);
        VLOG(ERR,"MIB 0x%08x Core0=0x%08x\n", i, temp);
    }
    VLOG(INFO,"[-] MIB REG Dump\n");


    VLOG(INFO,"[-] BPU MSG REG Dump\n");

    VLOG(INFO,"[MSG_0:0x%08x], [MSG_1:0x%08x],[MSG_2:0x%08x],[MSG_3:0x%08x],[MSG_4:0x%08x],[MSG_5:0x%08x] \n",
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1A8),
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1AC),
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1B0),
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1B4),
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1B8),
        vdi_fio_read_register(coreIdx, W5_REG_BASE + 0x8000 + 0x1BC));

    VLOG(INFO,"[-] BPU MSG REG Dump\n");

}





void vdi_print_vpu_status_enc(unsigned long coreIdx)
{
    int       vce_enc_debug[12] = {0, };
    int       set_mode;
    int       vcore_num, vcore_idx;
    int i;

    VLOG(INFO,"-------------------------------------------------------------------------------\n");
    VLOG(INFO,"------                           Encoder only                                                         -----\n");
    VLOG(INFO,"-------------------------------------------------------------------------------\n");
    VLOG(ERR,"BS_OPT: 0x%08x\n", VpuReadReg(coreIdx, W5_BS_OPTION));

    VLOG(ERR,"[+] VCPU DMA Dump\n");
    for(i = 0x2000; i < 0x2018; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }
    VLOG(ERR,"[-] VCPU DMA Dump\n");

    VLOG(ERR,"[+] VCPU HOST REG Dump\n");
    for(i = 0x3000; i < 0x30fc; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }
    VLOG(ERR,"[-] VCPU HOST REG Dump\n");

    VLOG(ERR,"[+] VCPU ENT ENC REG Dump\n");
    for(i = 0x6800; i < 0x7000; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }
    VLOG(ERR,"[-] VCPU ENT ENC REG Dump\n");

    VLOG(ERR,"[+] VCPU HOST MEM Dump\n");
    for(i = 0x7000; i < 0x70fc; i += 16) {
        VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
            vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
    }
    VLOG(ERR,"[-] VCPU SPP Dump\n");

    VLOG(ERR,"vce run flag = %d\n", VpuReadReg(coreIdx, 0x1E8));


    VLOG(ERR,"-------------------------------------------------------------------------------\n");
    VLOG(ERR,"------                            VCE DUMP(ENC)                           -----\n");
    VLOG(ERR,"-------------------------------------------------------------------------------\n");
    vce_enc_debug[0] = 0x0ba0;//MODE SEL //parameter VCE_ENC_DEBUG0            = 9'h1A0;
    vce_enc_debug[1] = 0x0ba4;
    vce_enc_debug[2] = 0x0ba8;
    vce_enc_debug[3] = 0x0bac;
    vce_enc_debug[4] = 0x0bb0;
    vce_enc_debug[5] = 0x0bb4;
    vce_enc_debug[6] = 0x0bb8;
    vce_enc_debug[7] = 0x0bbc;
    vce_enc_debug[8] = 0x0bc0;
    vce_enc_debug[9] = 0x0bc4;
    set_mode              = 0x0ba0;
    vcore_num            = 1;


    for (vcore_idx = 0; vcore_idx < vcore_num ; vcore_idx++) {
        VLOG(ERR,"==========================================\n");
        VLOG(ERR,"[+] VCE REG Dump VCORE_IDX : %d\n",vcore_idx);
        VLOG(ERR,"==========================================\n");
        DisplayVceEncReadVCE             (coreIdx, vcore_idx);
        DisplayVceEncDebugCommon521      (coreIdx, vcore_idx, set_mode, vce_enc_debug[0], vce_enc_debug[1], vce_enc_debug[2]);
        DisplayVceEncDebugMode2          (coreIdx, vcore_idx, set_mode, vce_enc_debug);
    }
}

void vdi_print_vpu_status_dec(unsigned long coreIdx)
{
        Uint32 i;

        VLOG(INFO,"-------------------------------------------------------------------------------\n");
        VLOG(INFO,"------                           Decoder only                             -----\n");
        VLOG(INFO,"-------------------------------------------------------------------------------\n");

        /// -- VCPU ENTROPY PERI DECODE Common
        VLOG(INFO,"[+] VCPU ENT DEC REG Dump\n");
        for(i = 0x6000; i < 0x6800; i += 16) {
            VLOG(INFO,"0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", (W5_REG_BASE + i),
                vdi_fio_read_register(coreIdx, (W5_REG_BASE + i)),
                vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 4 )),
                vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 8 )),
                vdi_fio_read_register(coreIdx, (W5_REG_BASE + i + 12)));
        }
        VLOG(INFO,"[-] VCPU ENT DEC REG Dump\n");
}


void wave5xx_vcpu_status (unsigned long coreIdx)
{
    Uint32 vcpu_reg[31]= {0,};
    Uint32 i;

        VLOG(INFO,"-------------------------------------------------------------------------------\n");
        VLOG(INFO,"------                            VCPU STATUS                                                     -----\n");
        VLOG(INFO,"-------------------------------------------------------------------------------\n");

   // --------- VCPU register Dump
    VLOG(INFO,"[+] VCPU REG Dump\n");
    for (i = 0; i < 25; i++) {
        VpuWriteReg (coreIdx, 0x14, (1<<9) | (i & 0xff));
        vcpu_reg[i] = VpuReadReg (coreIdx, 0x1c);

        if (i < 16) {
            VLOG(INFO,"0x%08x\t",  vcpu_reg[i]);
            if ((i % 4) == 3) VLOG(INFO,"\n");
        }
        else {
            switch (i) {
            case 16: VLOG(INFO,"CR0: 0x%08x\t", vcpu_reg[i]); break;
            case 17: VLOG(INFO,"CR1: 0x%08x\n", vcpu_reg[i]); break;
            case 18: VLOG(INFO,"ML:  0x%08x\t", vcpu_reg[i]); break;
            case 19: VLOG(INFO,"MH:  0x%08x\n", vcpu_reg[i]); break;
            case 21: VLOG(INFO,"LR:  0x%08x\n", vcpu_reg[i]); break;
            case 22: VLOG(INFO,"PC:  0x%08x\n", vcpu_reg[i]); break;
            case 23: VLOG(INFO,"SR:  0x%08x\n", vcpu_reg[i]); break;
            case 24: VLOG(INFO,"SSP: 0x%08x\n", vcpu_reg[i]); break;
            default: break;
            }
        }
    }
    for ( i = 0 ; i < 20 ; i++) {
        VLOG(ERR, "PC=0x%x\n", vdi_read_register(coreIdx, W5_VCPU_CUR_PC));
    }
    VLOG(INFO,"[-] VCPU REG Dump\n");
}

void vdi_print_vpu_status(unsigned long coreIdx)
{
    Uint32 productCode;

    productCode = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);

    if (PRODUCT_CODE_W_SERIES(productCode)) {
        wave5xx_vcpu_status(coreIdx);
        wave5xx_vcore_status(coreIdx);
	wave5xx_bpu_status(coreIdx);
        VLOG(INFO,"-------------------------------------------------------------------------------\n");
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(productCode)) {
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", productCode);
    }
}

void ChekcAndPrintDebugInfo(VpuHandle handle, BOOL isEnc, RetCode result)
{
    if ( result == RETCODE_VPU_RESPONSE_TIMEOUT || result == RETCODE_ERROR_FW_FATAL ) {
        VPUDebugInfo* DebugInfo;
        if ( isEnc == TRUE )
            VPU_EncGiveCommand(handle, GET_DEBUG_INFORM, &DebugInfo);
        else
            VPU_DecGiveCommand(handle, GET_DEBUG_INFORM, &DebugInfo);
        VLOG(ERR, "result=%x, priReason:%d \n", result, DebugInfo->priReason);
    }
}

unsigned int GetFileSize(const char *path)
{
    unsigned int fileSize = 0;
    FILE* fp = NULL;

    fp = osal_fopen(path, "r");
    if (NULL != fp) {
        osal_fseek(fp, 0, SEEK_END);
        fileSize = osal_ftell(fp);
        //VLOG(INFO, "file size : %d\n", fileSize);
        osal_fclose(fp);
    }
    return fileSize;
}

int ExistOfFile(const char* path)
{
    int resOf = -1;
    if(NULL != path) {
        resOf = access(path, 0);
    }
    return resOf;
}




//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/wave5/wave5_regdefine.h"
#include "wave/wave5/wave5.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_vpuconfig.h"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#endif
#include "main_helper.h"
#include "misc/debug.h"

enum { False, True };

void InitializeDebugEnv(
    Uint32  productId,
    Uint32  options
    )
{
    UNREFERENCED_PARAMETER(productId);
}

void ReleaseDebugEnv(
    void
    )
{
}


static void	DisplayVceEncDebugCommon(int core_idx, int vcore_idx, int set_mode, int debug0, int debug1, int debug2)
{
    int reg_val;
    VLOG(ERR, "---------------Common Debug INFO-----------------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,0 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug0);
    VLOG(ERR,"\t- pipe_on       :  0x%x\n", ((reg_val >> 8) & 0xf));
    VLOG(ERR,"\t- cur_pipe      :  0x%x\n", ((reg_val >> 12) & 0xf));
    VLOG(ERR,"\t- cur_s2ime     :  0x%x\n", ((reg_val >> 16) & 0xf));
    VLOG(ERR,"\t- cur_s2cache   :  0x%x\n", ((reg_val >> 20) & 0x7));
    VLOG(ERR,"\t- subblok_done  :  0x%x\n", ((reg_val >> 24) & 0x7f));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug1);
    VLOG(ERR,"\t- subblok_done  :  SFU 0x%x LF 0x%x RDO 0x%x IMD 0x%x FME 0x%x IME 0x%x\n", ((reg_val >> 5) & 0x1), ((reg_val >> 4) & 0x1), ((reg_val >> 3) & 0x1), ((reg_val >> 2) & 0x1), ((reg_val >> 1) & 0x1), ((reg_val >> 0) & 0x1));
    VLOG(ERR,"\t- pipe_on       :  0x%x\n", ((reg_val >> 8) & 0xf));
    VLOG(ERR,"\t- cur_pipe      :  0x%x\n", ((reg_val >> 12) & 0xf));
    VLOG(ERR,"\t- cur_s2cache   :  0x%x\n", ((reg_val >> 16) & 0x7));
    VLOG(ERR,"\t- cur_s2ime     :  0x%x\n", ((reg_val >> 24) & 0xf));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug2);
    VLOG(ERR,"\t- main_ctu_ypos :  0x%x\n", ((reg_val >> 0) & 0xff));
    VLOG(ERR,"\t- main_ctu_xpos :  0x%x\n", ((reg_val >> 8) & 0xff));
    VLOG(ERR,"\t- o_prp_ctu_ypos:  0x%x\n", ((reg_val >> 16) & 0xff));
    VLOG(ERR,"\t- o_prp_ctu_xpos:  0x%x\n", ((reg_val >> 24) & 0xff));

    SetClockGate(0, 1);
    reg_val  = vdi_fio_read_register(core_idx, W4_GDI_VCORE0_BUS_STATUS);
    VLOG(ERR,"\t- =========== GDI_BUS_STATUS ==========  \n");
    VLOG(ERR,"\t- pri_bus_busy:  0x%x\n", ((reg_val >> 0) & 0x1));
    VLOG(ERR,"\t- sec_bus_busy:  0x%x\n", ((reg_val >> 16) & 0x1));
    reg_val= VpuReadReg(core_idx, W4_RET_ENC_PIC_TYPE);
    VLOG(ERR,"[DEBUG] ret_core1_init : %d\n", (reg_val >> 16) & 0x3ff);
    reg_val = VpuReadReg(core_idx, W4_RET_ENC_PIC_FLAG);
    VLOG(ERR,"[DEBUG] ret_core0_init : %d\n", (reg_val >> 5) & 0x3ff);
    SetClockGate(0, 0);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugDCI(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 0 : DCI DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,0 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_rd_tuh       :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_tuh       :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_rd_cu        :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_cu        :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_rd_ctu       :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_2d_ctu       :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_rd_coef      :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_coef      :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_dci_full_empty_flag  :  0x%x\n", reg_val);

    VLOG(ERR,"----------- MODE 0 : VCE_CTRL DEBUG INFO----------\n");
    // HW_PARAM
    reg_val = ReadRegVCE(core_idx, vcore_idx, 0x0b08);
    VLOG(ERR,"\t- r_cnt_enable           :  0x%x\n", (reg_val >> 8) & 0x1);
    VLOG(ERR,"\t- r_pic_done_sel         :  0x%x\n", (reg_val >> 9) & 0x1);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- vce_cnt                :  0x%x\n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- prp_cnt                :  0x%x\n", reg_val);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugRDO(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    int reg_val_sub;

    VLOG(ERR,"----------- MODE 1 : RDO DEBUG INFO ----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,1 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- o_rdo_cu_root_cb                    :  0x%x\n", ((reg_val >>  0) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_y                      :  0x%x\n", ((reg_val >>  1) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_u                      :  0x%x\n", ((reg_val >>  2) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_v                      :  0x%x\n", ((reg_val >>  3) & 0x1));
    VLOG(ERR,"\t- w_rdo_wdma_wait                     :  0x%x\n", ((reg_val >>  4) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_luma_csbf[63: 0]       :  0x%x\n", ((reg_val >>  5) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_chro_csbf[31:16]       :  0x%x\n", ((reg_val >>  6) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_chro_csbf[15: 0]       :  0x%x\n", ((reg_val >>  7) & 0x1));
    VLOG(ERR,"\t- o_sub_ctu_coe_ready                 :  0x%x\n", ((reg_val >>  8) & 0x1));
    VLOG(ERR,"\t- o_sub_ctu_rec_ready                 :  0x%x\n", ((reg_val >>  9) & 0x1));
    VLOG(ERR,"\t- o_rdo_wdma_busy                     :  0x%x\n", ((reg_val >> 10) & 0x1));
    VLOG(ERR,"\t- w_rdo_rdma_wait                     :  0x%x\n", ((reg_val >> 11) & 0x1));
    VLOG(ERR,"\t- o_log2_cu_size[07:06]               :  0x%x\n", ((reg_val >> 12) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[15:14]               :  0x%x\n", ((reg_val >> 14) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[23:22]               :  0x%x\n", ((reg_val >> 16) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[31:30]               :  0x%x\n", ((reg_val >> 18) & 0x3));
    VLOG(ERR,"\t- o_rdo_dbk_valid                     :  0x%x\n", ((reg_val >> 20) & 0x1));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- debug_status_ctrl                   :  0x%x\n", ((reg_val >>  0) & 0xff));
    reg_val_sub = (reg_val >>  0) & 0xff; 
    VLOG(ERR,"\t- debug_status_ctrl.fsm_main_cur      :  0x%x\n", ((reg_val_sub >>  0) & 0x7));
    VLOG(ERR,"\t- debug_status_ctrl.i_rdo_wdma_wait   :  0x%x\n", ((reg_val_sub >>  3) & 0x1));
    VLOG(ERR,"\t- debug_status_ctrl.fsm_cu08_cur      :  0x%x\n", ((reg_val_sub >>  4) & 0x7));
    VLOG(ERR,"\t- debug_status_ctrl.init_hold         :  0x%x\n", ((reg_val_sub >>  7) & 0x1));

    VLOG(ERR,"\t- debug_status_nb                     :  0x%x\n", ((reg_val >>  8) & 0xff));
    reg_val_sub =(reg_val >>  8) & 0xff; 
    VLOG(ERR,"\t- debug_status_nb.fsm_save_cur        :  0x%x\n", ((reg_val_sub >>  0) & 0x7));
    VLOG(ERR,"\t- debug_status_nb.fsm_load_cur        :  0x%x\n", ((reg_val_sub >>  4) & 0x7));

    VLOG(ERR,"\t- debug_status_rec                    :  0x%x\n", ((reg_val >> 16) & 0xf));
    reg_val_sub = (reg_val >> 16) & 0xf; 
    VLOG(ERR,"\t- debug_status_rec.fsm_obuf_cur       :  0x%x\n", ((reg_val_sub >>  0) & 0x7));

    VLOG(ERR,"\t- debug_status_coe                    :  0x%x\n", ((reg_val >> 20) & 0xf));
    reg_val_sub = (reg_val >> 20) & 0xf; 
    VLOG(ERR,"\t- debug_status_coe.fsm_obuf_cur       :  0x%x\n", ((reg_val_sub >> 0) & 0x7));

    VLOG(ERR,"\t- debug_status_para                   :  0x%x\n", ((reg_val >> 24) & 0xff));
    reg_val_sub = (reg_val >> 24) & 0xff; 
    VLOG(ERR,"\t- debug_status_para.cur_sfu_rd_state  :  0x%x\n", ((reg_val_sub >> 0) & 0xf));
    VLOG(ERR,"\t- debug_status_para.cur_para_state    :  0x%x\n", ((reg_val_sub >> 4) & 0xf));

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugLF(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 2 : LF DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,2 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);

    VLOG(ERR,"\t- cur_enc_main_state   : 0x%x \n", (reg_val>>27)&0x1F);
    VLOG(ERR,"\t- i_sao_para_valie     : 0x%x \n", (reg_val>>26)&0x1);
    VLOG(ERR,"\t- i_sao_fetch_done     : 0x%x \n", (reg_val>>25)&0x1);
    VLOG(ERR,"\t- i_global_encode_en   : 0x%x \n", (reg_val>>24)&0x1);
    VLOG(ERR,"\t- i_bs_valid           : 0x%x \n", (reg_val>>23)&0x1);
    VLOG(ERR,"\t- i_rec_buf_rdo_ready  : 0x%x \n", (reg_val>>22)&0x1);
    VLOG(ERR,"\t- o_rec_buf_dbk_hold   : 0x%x \n", (reg_val>>21)&0x1);
    VLOG(ERR,"\t- cur_main_state       : 0x%x \n", (reg_val>>16)&0x1F);
    VLOG(ERR,"\t- r_lf_pic_dbk_disable : 0x%x \n", (reg_val>>15)&0x1);
    VLOG(ERR,"\t- r_lf_pic_sao_disable : 0x%x \n", (reg_val>>14)&0x1);
    VLOG(ERR,"\t- para_load_done       : 0x%x \n", (reg_val>>13)&0x1);
    VLOG(ERR,"\t- i_rdma_ack_wait      : 0x%x \n", (reg_val>>12)&0x1);
    VLOG(ERR,"\t- i_sao_intl_col_done  : 0x%x \n", (reg_val>>11)&0x1);
    VLOG(ERR,"\t- i_sao_outbuf_full    : 0x%x \n", (reg_val>>10)&0x1);
    VLOG(ERR,"\t- lf_sub_done          : 0x%x \n", (reg_val>>9)&0x1);
    VLOG(ERR,"\t- i_wdma_ack_wait      : 0x%x \n", (reg_val>>8)&0x1);
    VLOG(ERR,"\t- lf_all_sub_done      : 0x%x \n", (reg_val>>6)&0x1);
    VLOG(ERR,"\t- cur_ycbcr            : 0x%x \n", (reg_val>>5)&0x3);
    VLOG(ERR,"\t- sub8x8_done          : 0x%x \n", (reg_val>>0)&0xF);

    VLOG(ERR,"----------- MODE 2 : SYNC_Y_POS DEBUG INFO----------\n");
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);

    VLOG(ERR,"\t- fbc_y_pos            : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- bwb_y_pos            : 0x%x \n", (reg_val>>16)&0xff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- trace_frame		   :  0x%x\n", ((reg_val >> 0) & 0xffff));

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugSFU(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 3 : SFU DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,3 );

    reg_val = ReadRegVCE(0, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_sub_ctu_pos_y         : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_sub_ctu_pos_x         : 0x%x \n", (reg_val>>8)&0xff);
    VLOG(ERR,"\t- i_cu_fifo_wvalid        : 0x%x \n", (reg_val>>16)&0x1);
    VLOG(ERR,"\t- i_ctu_busy              : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- i_cs_sctu               : 0x%x \n", (reg_val>>24)&0x7);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_ctu_pos_y             : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_ctu_pos_x             : 0x%x \n", (reg_val>>8)&0xff);
    VLOG(ERR,"\t- i_sao_rdo_valid         : 0x%x \n", (reg_val>>16)&0x1);
    VLOG(ERR,"\t- i_sao_en_r              : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- i_ctu_fifo_wvalid       : 0x%x \n", (reg_val>>24)&0x1);
    VLOG(ERR,"\t- cs_sfu_ctu              : 0x%x \n", (reg_val>>28)&0x3);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cu_fifo_wvalid        : 0x%x \n", (reg_val>>0)&0x1);
    VLOG(ERR,"\t- i_rdo_cu_rd_valid       : 0x%x \n", (reg_val>>4)&0x1);
    VLOG(ERR,"\t- i_cu_size_r             : 0x%x \n", (reg_val>>8)&0x3);
    VLOG(ERR,"\t- i_cu_idx_r              : 0x%x \n", (reg_val>>12)&0xf);
    VLOG(ERR,"\t- cs_cu                   : 0x%x \n", (reg_val>>16)&0x7);
    VLOG(ERR,"\t- cs_fifo                 : 0x%x \n", (reg_val>>20)&0x7);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- w_dbg_tu_fifo_fsm       : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_coff_fifo_wvalid      : 0x%x \n", (reg_val>>8)&0x1);
    VLOG(ERR,"\t- i_tuh_fifo_wvalid       : 0x%x \n", (reg_val>>12)&0x1);
    VLOG(ERR,"\t- w_dbg_tu_ctrl_fsm       : 0x%x \n", (reg_val>>16)&0xf);
    VLOG(ERR,"\t- i_rdo_tc_ready          : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- w_dbg_coef_st_in_pic    : 0x%x \n", (reg_val>>24)&0x7);
    VLOG(ERR,"\t- i_rdo_tu_rd-valid       : 0x%x \n", (reg_val>>28)&0x1);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugDCI2(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 4 : DCI2 DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,4 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_rd_tuh2       : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_tuh2       : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_rd_cu2        : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_cu2        : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_rd_ctu2       : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_ctu2       : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_rd_coef2      : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_coef2      : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_dci_full_empty_flag   : 0x%x \n", reg_val);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugDCILast(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 5 : DCI LAST DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,5 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[143:112]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[111: 96]    : 0x%x \n", reg_val & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 95: 64]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 63: 32]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 31:  0]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_wr_read_point                  : 0x%x \n", (reg_val >> 16) & 0x7ff );
    VLOG(ERR,"\t- i_wr_read_point_limit            : 0x%x \n", (reg_val >>  0) & 0x7ff );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_sbuf_raddr_store               : 0x%x \n", (reg_val >>  0) & 0x3f );
    VLOG(ERR,"\t- i_read_point                     : 0x%x \n", (reg_val >>  8) & 0x1f );
    VLOG(ERR,"\t- i_dci_write_addr_b               : 0x%x \n", (reg_val >> 16) & 0x3f );
    VLOG(ERR,"\t- i_dci_write_addr_c               : 0x%x \n", (reg_val >> 24) & 0x1f );
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugBigBufferCnt(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 6 : BIG BUFFER CNT DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,6 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_tuh    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_tuh   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_cu     : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_cu    : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_coef   : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_coef  : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_tuh    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_tuh   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_cu     : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_cu    : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_ctu    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_tcu   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_coef   : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_coef  : 0x%x \n", (reg_val >>  0) & 0xffff);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugBigBufferAddr(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 6 : BIG BUFFER ADDR DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,7 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_tuh    : 0x%x \n", (reg_val >> 16) & 0x7ff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_tuh   : 0x%x \n", (reg_val >>  0) & 0x7ff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_cu     : 0x%x \n", (reg_val >> 16) & 0x1ff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_cu    : 0x%x \n", (reg_val >>  0) & 0x1ff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_coef   : 0x%x \n", (reg_val >> 16) & 0xfff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_coef  : 0x%x \n", (reg_val >>  0) & 0xfff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_tuh    : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_tuh   : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_cu     : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_cu    : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_ctu    : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_tcu   : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_coef   : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_coef  : 0x%x \n", (reg_val >>  0) & 0x1f);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugSubWb(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 7 : SUB_WB DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,8 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- subwb_debug_0              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- subwb_debug_1              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- subwb_debug_2              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- subwb_debug_3              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- subwb_debug_4              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- int_sync_ypos              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- pic_run_cnt                : 0x%x \n", ((reg_val) >> 0) & 0xffff);
    VLOG(ERR,"\t- pic_init_ct                : 0x%x \n", ((reg_val) >> 16) & 0xffff);

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void	DisplayVceEncDebugFBC(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 8 : FBC DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,9 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- ofs_request_count            : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- ofs_bvalid_count             : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- dat_request_count            : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- dat_bvalid_count             : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- fbc_debug                    : 0x%x \n", ((reg_val) >> 0) &  0x3FFFFFFF);
    VLOG(ERR,"\t- fbc_cr_idle_3d               : 0x%x \n", ((reg_val) >> 30) & 0x1);
    VLOG(ERR,"\t- fbc_cr_busy_3d               : 0x%x \n", ((reg_val) >> 31) & 0x1);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- outbuf_debug                 : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- fbcif_debug                  : 0x%x \n", reg_val);

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308
Uint32 ReadRegVCE(
    Uint32 core_idx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    )
{
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000*vce_core_idx;

    SetClockGate(core_idx, 1);
    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),vcpu_reg_addr + vce_core_base);

    if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) == 1)
        udata= vdi_fio_read_register(0, VCORE_DBG_DATA(vce_core_idx));
    else {
        VLOG(ERR, "failed to read VCE register: %d, 0x%04x\n", vce_core_idx, vce_addr);
        udata = -2;//-1 can be a valid value
    }

    SetClockGate(core_idx, 0);
    return udata;
}

void WriteRegVCE(
    Uint32   core_idx,
    Uint32   vce_core_idx,
    Uint32   vce_addr,
    Uint32   udata
    )
{
    int vcpu_reg_addr;

    SetClockGate(core_idx, 1);

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx),0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_DATA(vce_core_idx),udata);
    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),(vcpu_reg_addr) & 0x00007FFF);

    if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0) 
        VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);
    SetClockGate(core_idx, 0);
}

void PrintVpuStatus(
    Uint32 coreIdx, 
    Uint32 productId
    )
{
    SetClockGate(coreIdx, 1);
    if (PRODUCT_ID_W_SERIES(productId))
    {
        int      rd, wr;
        Uint32    tq, ip, mc, lf;
        Uint32    avail_cu, avail_tu, avail_tc, avail_lf, avail_ip;
        Uint32	 ctu_fsm, nb_fsm, cabac_fsm, cu_info, mvp_fsm, tc_busy, lf_fsm, bs_data, bbusy, fv;
        Uint32    reg_val;
        Uint32    index;
        Uint32    vcpu_reg[31]= {0,};

        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        VLOG(ERR,"------                            VCPU STATUS(DEC)                        -----\n");
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(ERR,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n", 
            rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

        // --------- VCPU register Dump 
        VLOG(ERR,"[+] VCPU REG Dump\n");
        for (index = 0; index < 25; index++) {
            VpuWriteReg (coreIdx, 0x14, (1<<9) | (index & 0xff));
            vcpu_reg[index] = VpuReadReg (coreIdx, 0x1c);

            if (index < 16) {
                VLOG(ERR,"0x%08x\t",  vcpu_reg[index]);
                if ((index % 4) == 3) VLOG(ERR,"\n");
            }
            else {
                switch (index) {
                case 16: VLOG(ERR,"CR0: 0x%08x\t", vcpu_reg[index]); break;
                case 17: VLOG(ERR,"CR1: 0x%08x\n", vcpu_reg[index]); break;
                case 18: VLOG(ERR,"ML:  0x%08x\t", vcpu_reg[index]); break;
                case 19: VLOG(ERR,"MH:  0x%08x\n", vcpu_reg[index]); break;
                case 21: VLOG(ERR,"LR:  0x%08x\n", vcpu_reg[index]); break;
                case 22: VLOG(ERR,"PC:  0x%08x\n", vcpu_reg[index]);break;
                case 23: VLOG(ERR,"SR:  0x%08x\n", vcpu_reg[index]);break;
                case 24: VLOG(ERR,"SSP: 0x%08x\n", vcpu_reg[index]);break;
                }
            }
        }

        VLOG(ERR,"[-] VCPU REG Dump\n");
        // --------- BIT register Dump 
        VLOG(ERR,"[+] BPU REG Dump\n");
        for (index=0;index < 30; index++)
        {
            int temp;
            temp = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18));
            VLOG(ERR,"BITPC = 0x%08x\n", temp);
            if ( temp == 0xffffffff)
                return;
        }
        VLOG(ERR,"BIT START=0x%08x, BIT END=0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x11c)), 
            vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x120)) );
        if (productId == PRODUCT_ID_410 )
            VLOG(ERR,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x100))); 
        if (productId == PRODUCT_ID_4102 || productId == PRODUCT_ID_510)
            VLOG(ERR,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x1FC))); 

        //DECODER SDMA INFO
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x120);
        while((vdi_fio_read_register(0,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(0,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_LOAD_CMD    = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x121);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_AURO_MODE  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x122);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_BASE_ADDR  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x123);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_ENC_ADDR   = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x124);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_ENDIAN     = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x125);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_IRQ_CLEAR  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x126);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_BUSY       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x127);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_LAST_ADDR  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x128);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_SC_BASE_ADDR = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x129);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_RD_SEL      = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x130);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_WR_SEL      = 0x%x\n",reg_val); 

        //DECODER SHU INFO
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x140);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_INIT         = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x141);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SEEK_NXT_NAL = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x142);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_RD_NAL_ADDR  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x143);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_STATUS       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x144);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE0       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x145);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE1       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x146);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE2       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x147);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE3       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x148);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE4       = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SBYTE_LOW   = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14D);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SBYTE_HIGH  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14E);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_ST_PAT_DIS  = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x150);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF0      = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x151);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF1      	= 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x152);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF2      	= 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x153);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF3      	= 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF_RPTR   = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15D);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF_WPTR   = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15E);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_REMAIN_BYTE = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15F);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_CONSUME_BYTE= 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x160);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_RD_SEL     = 0x%x\n",reg_val); 

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x161);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_WR_SEL     = 0x%x\n",reg_val); 

        // --------- BIT HEVC Status Dump 
        ctu_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x48));
        nb_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x4c));
        cabac_fsm	= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x50));
        cu_info		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x54));
        mvp_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x58));
        tc_busy		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x5c));
        lf_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x60));
        bs_data		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x64));
        bbusy		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x68));
        fv		    = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x6C));

        VLOG(ERR,"[DEBUG-BPUHEVC] CTU_X: %4d, CTU_Y: %4d\n",  vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x40)), vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x44))); 
        VLOG(ERR,"[DEBUG-BPUHEVC] CTU_FSM>   Main: 0x%02x, FIFO: 0x%1x, NB: 0x%02x, DBK: 0x%1x\n", ((ctu_fsm >> 24) & 0xff), ((ctu_fsm >> 16) & 0xff), ((ctu_fsm >> 8) & 0xff), (ctu_fsm & 0xff));
        VLOG(ERR,"[DEBUG-BPUHEVC] NB_FSM:	0x%02x\n", nb_fsm & 0xff);
        VLOG(ERR,"[DEBUG-BPUHEVC] CABAC_FSM> SAO: 0x%02x, CU: 0x%02x, PU: 0x%02x, TU: 0x%02x, EOS: 0x%02x\n", ((cabac_fsm>>25) & 0x3f), ((cabac_fsm>>19) & 0x3f), ((cabac_fsm>>13) & 0x3f), ((cabac_fsm>>6) & 0x7f), (cabac_fsm & 0x3f));
        VLOG(ERR,"[DEBUG-BPUHEVC] CU_INFO value = 0x%04x \n\t\t(l2cb: 0x%1x, cux: %1d, cuy; %1d, pred: %1d, pcm: %1d, wr_done: %1d, par_done: %1d, nbw_done: %1d, dec_run: %1d)\n", cu_info, 
            ((cu_info>> 16) & 0x3), ((cu_info>> 13) & 0x7), ((cu_info>> 10) & 0x7), ((cu_info>> 9) & 0x3), ((cu_info>> 8) & 0x1), ((cu_info>> 6) & 0x3), ((cu_info>> 4) & 0x3), ((cu_info>> 2) & 0x3), (cu_info & 0x3));
        VLOG(ERR,"[DEBUG-BPUHEVC] MVP_FSM> 0x%02x\n", mvp_fsm & 0xf);
        VLOG(ERR,"[DEBUG-BPUHEVC] TC_BUSY> tc_dec_busy: %1d, tc_fifo_busy: 0x%02x\n", ((tc_busy >> 3) & 0x1), (tc_busy & 0x7));
        VLOG(ERR,"[DEBUG-BPUHEVC] LF_FSM>  SAO: 0x%1x, LF: 0x%1x\n", ((lf_fsm >> 4) & 0xf), (lf_fsm  & 0xf));
        VLOG(ERR,"[DEBUG-BPUHEVC] BS_DATA> ExpEnd=%1d, bs_valid: 0x%03x, bs_data: 0x%03x\n", ((bs_data >> 31) & 0x1), ((bs_data >> 16) & 0xfff), (bs_data & 0xfff));
        VLOG(ERR,"[DEBUG-BPUHEVC] BUS_BUSY> mib_wreq_done: %1d, mib_busy: %1d, sdma_bus: %1d\n", ((bbusy >> 2) & 0x1), ((bbusy >> 1) & 0x1) , (bbusy & 0x1));
        VLOG(ERR,"[DEBUG-BPUHEVC] FIFO_VALID> cu: %1d, tu: %1d, iptu: %1d, lf: %1d, coff: %1d\n\n", ((fv >> 4) & 0x1), ((fv >> 3) & 0x1), ((fv >> 2) & 0x1), ((fv >> 1) & 0x1), (fv & 0x1));
        VLOG(ERR,"[-] BPU REG Dump\n");

        // --------- VCE register Dump 
        VLOG(ERR,"[+] VCE REG Dump\n");
        tq = ReadRegVCE(coreIdx, 0, 0xd0);
        ip = ReadRegVCE(coreIdx, 0, 0xd4); 
        mc = ReadRegVCE(coreIdx, 0, 0xd8);
        lf = ReadRegVCE(coreIdx, 0, 0xdc);
        avail_cu = (ReadRegVCE(coreIdx, 0, 0x11C)>>16) - (ReadRegVCE(coreIdx, 0, 0x110)>>16);
        avail_tu = (ReadRegVCE(coreIdx, 0, 0x11C)&0xFFFF) - (ReadRegVCE(coreIdx, 0, 0x110)&0xFFFF);
        avail_tc = (ReadRegVCE(coreIdx, 0, 0x120)>>16) - (ReadRegVCE(coreIdx, 0, 0x114)>>16);
        avail_lf = (ReadRegVCE(coreIdx, 0, 0x120)&0xFFFF) - (ReadRegVCE(coreIdx, 0, 0x114)&0xFFFF);
        avail_ip = (ReadRegVCE(coreIdx, 0, 0x124)>>16) - (ReadRegVCE(coreIdx, 0, 0x118)>>16);
        VLOG(ERR,"       TQ            IP              MC             LF      GDI_EMPTY          ROOM \n");
        VLOG(ERR,"------------------------------------------------------------------------------------------------------------\n");
        SetClockGate(coreIdx, 1);
        VLOG(ERR,"| %d %04d %04d | %d %04d %04d |  %d %04d %04d | %d %04d %04d | 0x%08x | CU(%d) TU(%d) TC(%d) LF(%d) IP(%d)\n",
            (tq>>22)&0x07, (tq>>11)&0x3ff, tq&0x3ff,
            (ip>>22)&0x07, (ip>>11)&0x3ff, ip&0x3ff,
            (mc>>22)&0x07, (mc>>11)&0x3ff, mc&0x3ff,
            (lf>>22)&0x07, (lf>>11)&0x3ff, lf&0x3ff,
            vdi_fio_read_register(coreIdx, 0x88f4),                      /* GDI empty */
            avail_cu, avail_tu, avail_tc, avail_lf, avail_ip);
        /* CU/TU Queue count */
        reg_val = ReadRegVCE(coreIdx, 0, 0x12C);
        VLOG(ERR,"[DCIDEBUG] QUEUE COUNT: CU(%5d) TU(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
        reg_val = ReadRegVCE(coreIdx, 0, 0x1A0);
        VLOG(ERR,"TC(%5d) IP(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
        reg_val = ReadRegVCE(coreIdx, 0, 0x1A4);
        VLOG(ERR,"LF(%5d)\n", (reg_val>>16)&0xffff);
        VLOG(ERR,"VALID SIGNAL : CU0(%d)  CU1(%d)  CU2(%d) TU(%d) TC(%d) IP(%5d) LF(%5d)\n" 
            "               DCI_FALSE_RUN(%d) VCE_RESET(%d) CORE_INIT(%d) SET_RUN_CTU(%d) \n",
            (reg_val>>6)&1, (reg_val>>5)&1, (reg_val>>4)&1, (reg_val>>3)&1, 
            (reg_val>>2)&1, (reg_val>>1)&1, (reg_val>>0)&1,
            (reg_val>>10)&1, (reg_val>>9)&1, (reg_val>>8)&1, (reg_val>>7)&1);

        VLOG(ERR,"State TQ: 0x%08x IP: 0x%08x MC: 0x%08x LF: 0x%08x\n", 
            ReadRegVCE(coreIdx, 0, 0xd0), ReadRegVCE(coreIdx, 0, 0xd4), ReadRegVCE(coreIdx, 0, 0xd8), ReadRegVCE(coreIdx, 0, 0xdc));
        VLOG(ERR,"BWB[1]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", ReadRegVCE(coreIdx, 0, 0x194), ReadRegVCE(coreIdx, 0, 0x198));
        VLOG(ERR,"BWB[2]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", ReadRegVCE(coreIdx, 0, 0x194), ReadRegVCE(coreIdx, 0, 0x198));
        VLOG(ERR,"DCI INFO\n");
        VLOG(ERR,"READ_CNT_0 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x110));
        VLOG(ERR,"READ_CNT_1 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x114));
        VLOG(ERR,"READ_CNT_2 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x118));
        VLOG(ERR,"WRITE_CNT_0: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x11c));
        VLOG(ERR,"WRITE_CNT_1: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x120));
        VLOG(ERR,"WRITE_CNT_2: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x124));
        reg_val = ReadRegVCE(coreIdx, 0, 0x128);
        VLOG(ERR,"LF_DEBUG_PT: 0x%08x\n", reg_val & 0xffffffff);
        VLOG(ERR,"cur_main_state %2d, r_lf_pic_deblock_disable %1d, r_lf_pic_sao_disable %1d\n",
            (reg_val >> 16) & 0x1f, 
            (reg_val >> 15) & 0x1, 
            (reg_val >> 14) & 0x1);
        VLOG(ERR,"para_load_done %1d, i_rdma_ack_wait %1d, i_sao_intl_col_done %1d, i_sao_outbuf_full %1d\n",
            (reg_val >> 13) & 0x1, 
            (reg_val >> 12) & 0x1, 
            (reg_val >> 11) & 0x1, 
            (reg_val >> 10) & 0x1);
        VLOG(ERR,"lf_sub_done %1d, i_wdma_ack_wait %1d, lf_all_sub_done %1d, cur_ycbcr %1d, sub8x8_done %2d\n", 
            (reg_val >> 9) & 0x1, 
            (reg_val >> 8) & 0x1, 
            (reg_val >> 6) & 0x1, 
            (reg_val >> 4) & 0x1, 
            reg_val & 0xf);
        VLOG(ERR,"[-] VCE REG Dump\n");
        VLOG(ERR,"[-] VCE REG Dump\n");

        VLOG(ERR,"-------------------------------------------------------------------------------\n");
    }
    if (productId == PRODUCT_ID_420 || productId == PRODUCT_ID_420L)
    {
        int       rd, wr;
        Uint32    reg_val, num;
        int       vce_enc_debug[12] = {0, };
        int		  set_mode;
        int       vcore_num, vcore_idx;
        Uint32    index;
        Uint32    vcpu_reg[31]= {0,};

        SetClockGate(coreIdx, 1);
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        VLOG(ERR,"------                            VCPU STATUS(ENC)                        -----\n");
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(ERR,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n", 
            rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

        // --------- VCPU register Dump 
        VLOG(ERR,"[+] VCPU REG Dump\n");
        for (index = 0; index < 25; index++) {
            VpuWriteReg (coreIdx, W4_VPU_PDBG_IDX_REG, (1<<9) | (index & 0xff));
            vcpu_reg[index] = VpuReadReg (coreIdx, W4_VPU_PDBG_RDATA_REG);

            if (index < 16) {
                VLOG(ERR,"0x%08x\t",  vcpu_reg[index]);
                if ((index % 4) == 3) VLOG(ERR,"\n");
            }
            else {
                switch (index) {
                case 16: VLOG(ERR,"CR0: 0x%08x\t", vcpu_reg[index]); break;
                case 17: VLOG(ERR,"CR1: 0x%08x\n", vcpu_reg[index]); break;
                case 18: VLOG(ERR,"ML:  0x%08x\t", vcpu_reg[index]); break;
                case 19: VLOG(ERR,"MH:  0x%08x\n", vcpu_reg[index]); break;
                case 21: VLOG(ERR,"LR:  0x%08x\n", vcpu_reg[index]); break;
                case 22: VLOG(ERR,"PC:  0x%08x\n", vcpu_reg[index]);break;
                case 23: VLOG(ERR,"SR:  0x%08x\n", vcpu_reg[index]);break;
                case 24: VLOG(ERR,"SSP: 0x%08x\n", vcpu_reg[index]);break;
                }
            }
        }
        VLOG(ERR,"[-] VCPU REG Dump\n");
        VLOG(ERR,"vce run flag = %d\n", VpuReadReg(coreIdx, 0x1E8));
        // --------- BIT register Dump 
        VLOG(ERR,"[+] BPU REG Dump\n");
        for (index=0;index < 30; index++)
        {
            int temp;
            temp = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18));
            VLOG(ERR,"BITPC = 0x%08x\n", temp);
            if ( temp == 0xffffffff)
                return;
        }

        // --------- BIT HEVC Status Dump 
        VLOG(ERR,"==================================\n");
        VLOG(ERR,"[-] BPU REG Dump\n");
        VLOG(ERR,"==================================\n");

        VLOG(ERR,"DBG_FIFO_VALID		[%08x]\n",vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x6C)));

        //SDMA debug information
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20) | (1<<16)| 0x13c);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_DBG_INFO		[%08x]\n", reg_val);
        VLOG(ERR,"\t- [   28] need_more_update  : 0x%x \n", (reg_val>>28)&0x1 );
        VLOG(ERR,"\t- [27:25] tr_init_fsm       : 0x%x \n", (reg_val>>25)&0x7 );
        VLOG(ERR,"\t- [24:18] remain_trans_size : 0x%x \n", (reg_val>>18)&0x7F);
        VLOG(ERR,"\t- [17:13] wdata_out_cnt     : 0x%x \n", (reg_val>>13)&0x1F);
        VLOG(ERR,"\t- [12:10] wdma_wd_fsm       : 0x%x \n", (reg_val>>10)&0x1F);
        VLOG(ERR,"\t- [ 9: 7] wdma_wa_fsm       : 0x%x ", (reg_val>> 7)&0x7 );
        if (((reg_val>>7) &0x7) == 3)
            VLOG(ERR,"-->WDMA_WAIT_ADDR  \n");
        else
            VLOG(ERR,"\n");
        VLOG(ERR,"\t- [ 6: 5] sdma_init_fsm     : 0x%x \n", (reg_val>> 5)&0x3 );
        VLOG(ERR,"\t- [ 4: 1] save_fsm          : 0x%x \n", (reg_val>> 1)&0xF );
        VLOG(ERR,"\t- [    0] unalign_written   : 0x%x \n", (reg_val>> 0)&0x1 );

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x13b);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_NAL_MEM_INF	[%08x]\n", reg_val);
        VLOG(ERR,"\t- [ 7: 1] nal_mem_empty_room : 0x%x \n", (reg_val>> 1)&0x3F);
        VLOG(ERR,"\t- [    0] ge_wnbuf_size      : 0x%x \n", (reg_val>> 0)&0x1 );

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x131);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_IRQ		[%08x]: [1]sdma_irq : 0x%x, [2]enable_sdma_irq : 0x%x\n",reg_val, (reg_val >> 1)&0x1,(reg_val &0x1));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x134);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_BS_BASE_ADDR [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x135);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_NAL_STR_ADDR [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x136);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_IRQ_ADDR     [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x137);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_BS_END_ADDR	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x13A);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_CUR_ADDR		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x139);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_STATUS			[%08x]\n",reg_val);
        VLOG(ERR,"\t- [2] all_wresp_done : 0x%x \n", (reg_val>> 2)&0x1);
        VLOG(ERR,"\t- [1] sdma_init_busy : 0x%x \n", (reg_val>> 1)&0x1 );
        VLOG(ERR,"\t- [0] save_busy      : 0x%x \n", (reg_val>> 0)&0x1 );

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x164);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SHU_DBG				[%08x] : shu_unaligned_num (0x%x)\n",reg_val, reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x169);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SHU_NBUF_WPTR		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x16A);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SHU_NBUF_RPTR		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x16C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SHU_NBUF_INFO		[%08x]\n",reg_val);
        VLOG(ERR,"\t- [5:1] nbuf_remain_byte : 0x%x \n", (reg_val>> 1)&0x1F);
        VLOG(ERR,"\t- [  0] nbuf_wptr_wrap   : 0x%x \n", (reg_val>> 0)&0x1 );

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x184);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"CTU_LAST_ENC_POS	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x187);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"CTU_POS_IN_PIC		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x110);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_EXTADDR			[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x111);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_INTADDR			[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x113);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_CMD				[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x114);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_BUSY			[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        VLOG(ERR,"DBG_BPU_ENC_NB0		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x40)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL0	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x44)));
        VLOG(ERR,"DBG_BPU_CAB_FSM0	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x48)));
        VLOG(ERR,"DBG_BPU_BIN_GEN0	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x4C)));
        VLOG(ERR,"DBG_BPU_CAB_MBAE0	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x50)));
        VLOG(ERR,"DBG_BPU_BUS_BUSY	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x68)));
        VLOG(ERR,"DBG_FIFO_VALID		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x6C)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL1	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x54)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL2	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x58)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL3	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x5C)));

        for (index=0x80; index<0xA0; index+=4) {
            reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + index));
            num     = (index - 0x80)/2;
            VLOG(ERR,"DBG_BIT_STACK		[%08x] : Stack%02d (0x%04x), Stack%02d(0x%04x) \n",reg_val, num, reg_val>>16, num+1, reg_val & 0xffff);
        }

        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA0));
        VLOG(ERR,"DGB_BIT_CORE_INFO	[%08x] : pc_ctrl_id (0x%04x), pfu_reg_pc(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA4));
        VLOG(ERR,"DGB_BIT_CORE_INFO	[%08x] : ACC0 (0x%08x)\n",reg_val, reg_val);
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA8));
        VLOG(ERR,"DGB_BIT_CORE_INFO	[%08x] : ACC1 (0x%08x)\n",reg_val, reg_val);

        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xAC));
        VLOG(ERR,"DGB_BIT_CORE_INFO	[%08x] : pfu_ibuff_id(0x%04x), pfu_ibuff_op(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);

        for (num=0; num<5; num+=1) {
            reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB0));
            VLOG(ERR,"DGB_BIT_CORE_INFO	[%08x] : core_pram_rd_en(0x%04x), core_pram_rd_addr(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);
        }

        VLOG(ERR,"SAO_LUMA_OFFSET	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB4)));
        VLOG(ERR,"SAO_CB_OFFSET	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB8)));
        VLOG(ERR,"SAO_CR_OFFSET	[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xBC)));

        VLOG(ERR,"GDI_NO_MORE_REQ		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x8f0)));
        VLOG(ERR,"GDI_EMPTY_FLAG		[%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x8f4)));

        if ( productId == PRODUCT_ID_420) {
            VLOG(ERR,"WAVE420_CODE VCE DUMP\n");
            vce_enc_debug[0] = 0x0bc8;//MODE SEL
            vce_enc_debug[1] = 0x0be0;
            vce_enc_debug[2] = 0x0bcc;
            vce_enc_debug[3] = 0x0be4;
            vce_enc_debug[4] = 0x0be8;
            vce_enc_debug[5] = 0x0bec;
            vce_enc_debug[6] = 0x0bc0;
            vce_enc_debug[7] = 0x0bc4;
            vce_enc_debug[8] = 0x0bf0;
            vce_enc_debug[9] = 0x0bf4;
            set_mode         = 0x0bc8;
            vcore_num        = 1;
        } 
        else if (productId == PRODUCT_ID_420L) {
            VLOG(ERR,"WAVE420L_CODE VCE DUMP\n");
            vce_enc_debug[0] = 0x0bd0;//MODE SEL
            vce_enc_debug[1] = 0x0bd4;
            vce_enc_debug[2] = 0x0bd8;
            vce_enc_debug[3] = 0x0bdc;
            vce_enc_debug[4] = 0x0be0;
            vce_enc_debug[5] = 0x0be4;
            vce_enc_debug[6] = 0x0be8;
            vce_enc_debug[7] = 0x0bc4;
            vce_enc_debug[8] = 0x0bf0;
            vce_enc_debug[9] = 0x0bf4;
            set_mode         = 0x0bd0;
            vcore_num        = 1;
        } 

        for (vcore_idx = 0; vcore_idx < vcore_num ; vcore_idx++)
        {
            VLOG(ERR,"==========================================\n");
            VLOG(ERR,"[+] VCE REG Dump VCORE_IDX : %d\n",vcore_idx);
            VLOG(ERR,"==========================================\n");
            DisplayVceEncDebugCommon         (coreIdx, vcore_idx, set_mode, vce_enc_debug[0], vce_enc_debug[1], vce_enc_debug[2]);
            DisplayVceEncDebugDCI            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugRDO            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugLF             (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugSFU            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugDCI2           (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugDCILast        (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugBigBufferCnt   (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugBigBufferAddr  (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugSubWb			 (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugFBC			 (coreIdx, vcore_idx, set_mode, vce_enc_debug);
        }

    }
    if ( productId == PRODUCT_ID_520 )
    {
        int       rd, wr;
        SetClockGate(coreIdx, 1);
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        VLOG(ERR,"------                            VCPU STATUS(ENC)                        -----\n");
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(ERR,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n", 
            rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

        VLOG(ERR,"=========Need to add debug code here =========\n");
    }
    else
    {
    }
    SetClockGate(coreIdx, 0);
}


void HandleEncoderError(
    EncHandle       handle,
    Uint32          encPicCnt,
    EncOutputInfo*  outputInfo
    )
{
    UNREFERENCED_PARAMETER(handle);
}

void HandleDecoderError(
    DecHandle       handle, 
    Uint32          frameIdx,
    TestDecConfig*  param,
    vpu_buffer_t*   fbMem,
    DecOutputInfo*  outputInfo
    )
{
    UNREFERENCED_PARAMETER(handle);
    UNREFERENCED_PARAMETER(outputInfo);
}

void PrintMemoryAccessViolationReason(
    Uint32          core_idx, 
    void            *outp
    )
{
    UNREFERENCED_PARAMETER(core_idx);
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
        if (seqInfo->seqInitErrReason == WAVE4_SPSERR_NOT_FOUND) {
            return -2;
        } else {
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PICTURE_WIDTH_SIZE) {
                VLOG(ERR, "Not supported picture width: MAX_SIZE(8192): %d\n", seqInfo->picWidth);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PICTURE_HEIGHT_SIZE) {
                VLOG(ERR, "Not supported picture height: MAX_SIZE(8192): %d\n", seqInfo->picHeight);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_CHROMA_FORMAT) {
                VLOG(ERR, "Not supported chroma idc: %d\n", seqInfo->chromaFormatIDC);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_BIT_DEPTH) {
                VLOG(INFO, "Not supported Luma or Chroma bitdepth: L(%d), C(%d)\n", seqInfo->lumaBitdepth, seqInfo->chromaBitdepth);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PROFILE) {
                VLOG(INFO, "Not supported profile: %d\n", seqInfo->profile);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPSERR_NOT_FOUND) {
                VLOG(INFO, "Not found SPS: RD_PTR(0x%08x), WR_PTR(0x%08x)\n", seqInfo->rdPtr, seqInfo->wrPtr);
            }
            return -1;
        }
    } 
    else {
        if (openParam->bitstreamMode == BS_MODE_PIC_END && (seqInfo->seqInitErrReason&(1<<31))) {
            VLOG(ERR, "SEQUENCE HEADER NOT FOUND\n");
            return -1;
        }
        else {
            return -1;
        }
    }
}

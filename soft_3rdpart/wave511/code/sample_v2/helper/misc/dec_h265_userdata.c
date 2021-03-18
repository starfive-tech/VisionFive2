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

#include "main_helper.h"

typedef struct H265UserDataContext {
    PhysicalAddress userDataBuf;
} H265UserDataContext;


static void UserDataH265Construct(
    Listener* observer,
    void*     data
    ) 
{
    H265UserDataContext* context;
    PhysicalAddress*     pAddr;

    context = (H265UserDataContext*)osal_malloc(sizeof(H265UserDataContext));
    osal_memset((void*)context, 0x00, sizeof(H265UserDataContext));

    pAddr = (PhysicalAddress*)data;
    context->userDataBuf = *pAddr;

    observer->ctx   = context;
}

static void UserDataH265Destruct(
    Listener* o
    )
{
    if (o != NULL) {
        osal_free(o->ctx);
        o->ctx = NULL;
    }
}

static BOOL UserDataH265Update(
    Listener* o,
    void*     data
    )
{
    DecOutputInfo*          info;

    if (info == NULL) return TRUE;
    info = (DecOutputInfo*)data;

    if (info->decOutputExtData.userDataNum > 0) {
        user_data_entry_t* pEntry = (user_data_entry_t*)pBase;

        VpuReadMem(coreIdx, vbUserData.phys_addr, pBase, vbUserData.size, VPU_USER_DATA_ENDIAN);

        if (info->decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
            h265_mastering_display_colour_volume_t* mastering;
            int i;

            mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
            VLOG(INFO, " MASTERING DISPLAY COLOR VOLUME\n");
            for (i=0; i<3; i++) {
                VLOG(INFO, " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", i, mastering->display_primaries_x[i], i, mastering->display_primaries_y[i]);
            }
            VLOG(INFO, " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
            VLOG(INFO, " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
        }

        if(info->decOutputExtData.userDataHeader&(1<<H265_USERDATA_FLAG_VUI)) {
            h265_vui_param_t* vui;

            vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
            VLOG(INFO, " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
            VLOG(INFO, "     VIDEO FORMAT(%d)\n", vui->video_format);
            VLOG(INFO, "     COLOUR PRIMARIES(%d)\n", vui->colour_primaries);
            VLOG(INFO, "log2_max_mv_length_horizontal: %d\n", vui->log2_max_mv_length_horizontal);
            VLOG(INFO, "log2_max_mv_length_vertical  : %d\n", vui->log2_max_mv_length_vertical);
        }
        if (info->decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
            h265_chroma_resampling_filter_hint_t* c_resampleing_filter_hint;
            Uint32 i,j;

            c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
            VLOG(INFO, " CHROMA_RESAMPLING_FILTER_HINT\n");
            VLOG(INFO, " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
            VLOG(INFO, " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);
            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                VLOG(INFO, " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);
                if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);
                    for (i=0; i<c_resampleing_filter_hint->num_vertical_filters; i++) {
                        VLOG(INFO, " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                        }
                    }
                }
                if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);
                    for (i=0; i<c_resampleing_filter_hint->num_horizontal_filters; i++) {
                        VLOG(INFO, " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                        }
                    }
                }
            }
        }

        if (info->decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
            h265_knee_function_info_t* knee_function;

            knee_function = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
            VLOG(INFO, " FLAG_KNEE_FUNCTION_INFO\n");
            VLOG(INFO, " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
            VLOG(INFO, " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);
            if (knee_function->knee_function_cancel_flag) {
                int idx;
                VLOG(INFO, " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                VLOG(INFO, " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                VLOG(INFO, " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                VLOG(INFO, " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                VLOG(INFO, " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                VLOG(INFO, " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);
                for (idx=0; idx<knee_function->num_knee_points_minus1; idx++) {
                    VLOG(INFO, " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[idx], knee_function->output_knee_point[idx]);
                }
            }
        }
    }
    return TRUE;
}

Listener decOutputInformation = {
    NULL,
    UserDataH265Construct,
    UserDataH265Update,
    UserDataH265Destruct
};


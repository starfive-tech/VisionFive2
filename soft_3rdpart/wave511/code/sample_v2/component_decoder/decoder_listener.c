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

#include "cnm_app.h"
#include "decoder_listener.h"
#include "misc/debug.h"
#include "misc/bw_monitor.h"

static BOOL IsDifferentResolution(DecGetFramebufInfo* fbInfo, FrameBuffer* srcFb)
{
    FrameBuffer* dstFb = &fbInfo->framebufPool[srcFb->myIndex];

    return (dstFb->width != srcFb->width || dstFb->height != srcFb->height);
}

static void PrintH265UserData(Uint8* pBase, Uint32 iFlags, vpu_buffer_t* vb)
{
    user_data_entry_t*  pEntry = NULL;
    user_data_entry_t*  item   = NULL;

    pEntry = (user_data_entry_t*)pBase;
    if (iFlags & (1<<H265_USERDATA_FLAG_VUI)) {
        h265_vui_param_t*  vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
        VLOG(INFO, "[VUI]\n");
        VLOG(INFO, "vui::aspect_ratio_idc              > %d\n", vui->aspect_ratio_idc);
        VLOG(INFO, "vui::video_signal_type_present_flag> %d\n", vui->video_signal_type_present_flag);
        VLOG(INFO, "vui::video_format                  > %d\n", vui->video_format);
        VLOG(INFO, "vui::vui_timing_info_present_flag  > %d\n", vui->vui_timing_info_present_flag);
        VLOG(INFO, "vui::vui_num_units_in_tick         > %d\n", vui->vui_num_units_in_tick);
        VLOG(INFO, "vui::vui_time_scale                > %d\n", vui->vui_time_scale);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_PIC_TIMING)) {
        h265_sei_pic_timing_t* picTiming = (h265_sei_pic_timing_t*)(pBase + pEntry[H265_USERDATA_FLAG_PIC_TIMING].offset);
        VLOG(INFO, "[SEI::PIC_TIMING]\n");
        VLOG(INFO, "sei::pic_timing::pic_struct        > %d\n", picTiming->pic_struct);
        VLOG(INFO, "sei::pic_timing::source_scan_type  > %d\n", picTiming->source_scan_type);
        VLOG(INFO, "sei::pic_timing::duplicate_flag    > %d\n", picTiming->duplicate_flag);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
        h265_mastering_display_colour_volume_t* mastering;
        mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
        VLOG(INFO, "[SEI::MASTERING DISPLAY COLOR VOLUME]\n");
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_x[0]          > %d\n", mastering->display_primaries_x[0]);
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_y[0]          > %d\n", mastering->display_primaries_y[0]);
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_x[1]          > %d\n", mastering->display_primaries_x[1]);
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_y[1]          > %d\n", mastering->display_primaries_y[1]);
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_x[2]          > %d\n", mastering->display_primaries_x[2]);
        VLOG(INFO, "sei::mastering_display_colour_volume::display_primaries_y[2]          > %d\n", mastering->display_primaries_y[2]);
        VLOG(INFO, "sei::mastering_display_colour_volume::white_point_x                   > %d\n", mastering->white_point_x);
        VLOG(INFO, "sei::mastering_display_colour_volume::white_point_y                   > %d\n", mastering->white_point_y);
        VLOG(INFO, "sei::mastering_display_colour_volume::max_display_mastering_luminance > %d%s\n", mastering->max_display_mastering_luminance);
        VLOG(INFO, "sei::mastering_display_colour_volume::min_display_mastering_luminance > %d %s\n", mastering->min_display_mastering_luminance);
    }
    if (iFlags & (1<<H265_USERDATA_FLAG_ITU_T_T35_PRE)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_PRE];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (PREFIX)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_ITU_T_T35_PRE_1)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_PRE_1];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (PREFIX1)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_ITU_T_T35_PRE_2)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_PRE_2];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (PREFIX2)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_ITU_T_T35_SUF)) {
        Uint8* pData        = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_SUF];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (SUFFIX)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_ITU_T_T35_SUF_1)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_SUF_1];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (SUFFIX 1)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_ITU_T_T35_SUF_2)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_ITU_T_T35_SUF_2];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_REGISTERED_ITU_T_T35 (SUFFIX 2)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_UNREGISTERED_PRE)) {
        Uint8* pData = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_UNREGISTERED_PRE];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H265::SEI::USER_DATA_UNREGISTERED (PREFIX)]\n");
        DisplayHex(pData, item->size, "H265 ITU-T-T35");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
        h265_chroma_resampling_filter_hint_t* pData = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);

        VLOG(INFO, "[H265::SEI::CHROMA_RESAMPLING_FILTER_HINT]\n");
        VLOG(INFO, "sei::chroma_resampling_filter_hint::ver_chroma_filter_idc               > %d\n", pData->ver_chroma_filter_idc);
        VLOG(INFO, "sei::chroma_resampling_filter_hint::hor_chroma_filter_idc               > %d\n", pData->hor_chroma_filter_idc);
        VLOG(INFO, "sei::chroma_resampling_filter_hint::ver_filtering_filed_processing_flag > %d\n", pData->ver_filtering_field_processing_flag);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
        h265_knee_function_info_t* pData = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
        VLOG(INFO, "[H265::SEI::KNEE_FUNCTION_INFO]\n");
        VLOG(INFO, "sei::knee_function_info::knee_function_id          > %d\n", pData->knee_function_id);
        VLOG(INFO, "sei::knee_function_info::knee_function_cancel_flag > %d\n", pData->knee_function_cancel_flag);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_UNREGISTERED_SUF)) {
        Uint8* pData = NULL;
        item  = &pEntry[H265_USERDATA_FLAG_UNREGISTERED_SUF];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[SEI::USER_DATA_UNREGISTERED (PREFIX OR SUFFIX)]\n");
        DisplayHex(pData, item->size, "H265 USERDATA_UNREGISTERED");
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_CONTENT_LIGHT_LEVEL_INFO)) {
        h265_content_light_level_info_t* pData = (h265_content_light_level_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_CONTENT_LIGHT_LEVEL_INFO].offset);
        VLOG(INFO, "[SEI::CONTENT_LIGHT_LEVEL_INFO(PREFIX)]\n");
        VLOG(INFO, "sei::content_light_level_info::max_content_light_level     > %04x\n", pData->max_content_light_level);
        VLOG(INFO, "sei::content_light_level_info::max_pic_average_light_level > %04x\n", pData->max_pic_average_light_level);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO)) {
        h265_film_grain_characteristics_t* pData = (h265_film_grain_characteristics_t*)(pBase + pEntry[H265_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO].offset);
        VLOG(INFO, "[SEI::FILM_GRAIN_CHARACTERISTICS_INFO(REFIX)]\n");
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_characteristics_cancel_flag   > %04x\n", pData->film_grain_characteristics_cancel_flag);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_model_id                      > %04x\n", pData->film_grain_model_id);
        VLOG(INFO, "sei::film_grain_characteristics_info::separate_colour_description_present_flag > %04x\n", pData->separate_colour_description_present_flag);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_bit_depth_luma_minus8         > %04x\n", pData->film_grain_bit_depth_luma_minus8);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_bit_depth_chroma_minus8       > %04x\n", pData->film_grain_bit_depth_chroma_minus8);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_full_range_flag               > %04x\n", pData->film_grain_full_range_flag);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_colour_primaries              > %04x\n", pData->film_grain_colour_primaries);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_transfer_characteristics      > %04x\n", pData->film_grain_transfer_characteristics);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_matrix_coeffs                 > %04x\n", pData->film_grain_matrix_coeffs);
        VLOG(INFO, "sei::film_grain_characteristics_info::blending_model_id                        > %04x\n", pData->blending_mode_id);
        VLOG(INFO, "sei::film_grain_characteristics_info::log2_scale_factor                        > %04x\n", pData->log2_scale_factor);
        VLOG(INFO, "sei::film_grain_characteristics_info::comp_model_present_flag[0]               > %04x\n", pData->comp_model_present_flag[0]);
        VLOG(INFO, "sei::film_grain_characteristics_info::comp_model_present_flag[1]               > %04x\n", pData->comp_model_present_flag[1]);
        VLOG(INFO, "sei::film_grain_characteristics_info::comp_model_present_flag[2]               > %04x\n", pData->comp_model_present_flag[2]);
        VLOG(INFO, "sei::film_grain_characteristics_info::film_grain_characterics_persistence_flag > %04x\n", pData->film_grain_characteristics_persistence_flag);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_TONE_MAPPING_INFO)) {
        h265_tone_mapping_info_t* pData = (h265_tone_mapping_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_TONE_MAPPING_INFO].offset);
        VLOG(INFO, "sei::tone_mapping_info::tone_map_id          > %04x\n", pData->tone_map_id);
        VLOG(INFO, "sei::tone_mapping_info::tone_map_cancel_flag > %04x\n", pData->tone_map_cancel_flag);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_COLOUR_REMAPPING_INFO)) {
        h265_colour_remapping_info_t* pData = (h265_colour_remapping_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_COLOUR_REMAPPING_INFO].offset);
        VLOG(INFO, "sei::tone_mapping_info::colour_remap_id          > %04x\n", pData->colour_remap_id);
        VLOG(INFO, "sei::tone_mapping_info::colour_rempa_cancel_flag > %04x\n", pData->colour_remap_cancel_flag);
    }

    return;
}

static void PrintH264UserData(Uint8* pBase, Uint32 iFlags, vpu_buffer_t* vb)
{
    user_data_entry_t*  pEntry = NULL;
    user_data_entry_t*  item   = NULL;

    pEntry = (user_data_entry_t*)pBase;
    if (iFlags & (1UL<<H264_USERDATA_FLAG_VUI)) {
        avc_vui_info_t*  vui = (avc_vui_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
        VLOG(INFO, "[VUI]\n");
        VLOG(INFO, "vui::aspect_ration_info_present_flag > %d\n", vui->aspect_ratio_info_present_flag);
        VLOG(INFO, "vui::aspect_ratio_idc                > %d\n", vui->aspect_ratio_idc);
        VLOG(INFO, "vui::overscan_info_present_flag      > %d\n", vui->overscan_info_present_flag);
        VLOG(INFO, "vui::video_signal_type_present_flag  > %d\n", vui->video_signal_type_present_flag);
        VLOG(INFO, "vui::vui_timing_info_present_flag    > %d\n", vui->vui_timing_info_present_flag);
        VLOG(INFO, "vui::vui_num_units_in_tick           > %d\n", vui->vui_num_units_in_tick);
        VLOG(INFO, "vui::vui_time_scale                  > %d\n", vui->vui_time_scale);
        VLOG(INFO, "vui::nal_hrd_parmeters_present_flag  > %d\n", vui->nal_hrd_parameters_present_flag);
        VLOG(INFO, "vui::cpb_cnt                         > %d\n", vui->nal_hrd.cpb_cnt);
        VLOG(INFO, "vui::bit_rate_value                  > %d\n", vui->nal_hrd.bit_rate_value[0]);
        VLOG(INFO, "vui::cpb_size_value                  > %d\n", vui->nal_hrd.cpb_size_value[0]);
        VLOG(INFO, "vui::initial_cpb_removal_delay_length> %d\n", vui->nal_hrd.initial_cpb_removal_delay_length);
        VLOG(INFO, "vui::cpb_removal_delay_length        > %d\n", vui->nal_hrd.cpb_removal_delay_length);
        VLOG(INFO, "vui::dpb_output_delay_length         > %d\n", vui->nal_hrd.dpb_output_delay_length);
        VLOG(INFO, "vui::vcl_hrd_parmeters_present_flag  > %d\n", vui->vcl_hrd_parameters_present_flag);
        VLOG(INFO, "vui::cpb_cnt                         > %d\n", vui->vcl_hrd.cpb_cnt);
        VLOG(INFO, "vui::bit_rate_value                  > %d\n", vui->vcl_hrd.bit_rate_value[0]);
        VLOG(INFO, "vui::cpb_size_value                  > %d\n", vui->vcl_hrd.cpb_size_value[0]);
        VLOG(INFO, "vui::initial_cpb_removal_delay_length> %d\n", vui->vcl_hrd.initial_cpb_removal_delay_length);
        VLOG(INFO, "vui::cpb_removal_delay_length        > %d\n", vui->vcl_hrd.cpb_removal_delay_length);
        VLOG(INFO, "vui::dpb_output_delay_length         > %d\n", vui->vcl_hrd.dpb_output_delay_length);
    }
    if (iFlags & (1UL<<H264_USERDATA_FLAG_PIC_TIMING)) {
        avc_sei_pic_timing_t* picTiming = (avc_sei_pic_timing_t*)(pBase + pEntry[H264_USERDATA_FLAG_PIC_TIMING].offset);
        VLOG(INFO, "[SEI::PIC_TIMING]\n");
        VLOG(INFO, "sei::pic_timing::cpb_removal_delay   > %d\n", picTiming->cpb_removal_delay);
        VLOG(INFO, "sei::pic_timing::dpb_output_delay    > %d\n", picTiming->dpb_output_delay);
        VLOG(INFO, "sei::pic_timing::pic_struct          > %d\n", picTiming->pic_struct);
        VLOG(INFO, "sei::pic_timing::num_clock_ts        > %d\n", picTiming->num_clock_ts);
    }
    if (iFlags & (1<<H264_USERDATA_FLAG_ITU_T_T35)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H264_USERDATA_FLAG_ITU_T_T35];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H264::SEI::USER_DATA_REGISTERED_ITU_T_T35]\n");
        DisplayHex(pData, item->size, "ITU-T-T35");
    }
    if (iFlags & (1<<H264_USERDATA_FLAG_ITU_T_T35_1)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H264_USERDATA_FLAG_ITU_T_T35_1];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H264::SEI::USER_DATA_REGISTERED_ITU_T_T35_1]\n");
        DisplayHex(pData, item->size, "ITU-T-T35_1");
    }
    if (iFlags & (1<<H264_USERDATA_FLAG_ITU_T_T35_2)) {
        Uint8* pData    = NULL;
        item  = &pEntry[H264_USERDATA_FLAG_ITU_T_T35_2];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[H264::SEI::USER_DATA_REGISTERED_ITU_T_T35_2]\n");
        DisplayHex(pData, item->size, "ITU-T-T35_2");
    }
    if (iFlags & (1UL<<H264_USERDATA_FLAG_UNREGISTERED)) {
        Uint8* pData = NULL;
        item  = &pEntry[H264_USERDATA_FLAG_UNREGISTERED];
        pData = (Uint8*)(pBase + item->offset);
        VLOG(INFO, "[AVC::SEI::USER_DATA_UNREGISTERED]\n");
        DisplayHex(pData, item->size, "UNREGISTERED");
    }
    if (iFlags & (1UL<<H264_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO)) {
        avc_sei_film_grain_t* pData = (avc_sei_film_grain_t*)(pBase + pEntry[H264_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO].offset);
        VLOG(INFO, "[SEI::FILM_GRAIN_CHARACTERISTICS]\n");
        VLOG(INFO, "sei::film_grain_characteristics::film_grain_characteristics_cancel_flag   > %04x\n", pData->film_grain_characteristics_cancel_flag);
        VLOG(INFO, "sei::film_grain_characteristics::film_grain_model_id                      > %04x\n", pData->film_grain_model_id);
        VLOG(INFO, "sei::film_grain_characteristics::separate_colour_description_present_flag > %04x\n", pData->separate_colour_description_present_flag);
        VLOG(INFO, "sei::film_grain_characteristics::blending_model_id                        > %04x\n", pData->blending_mode_id);
        VLOG(INFO, "sei::film_grain_characteristics::log2_scale_factor                        > %04x\n", pData->log2_scale_factor);
        VLOG(INFO, "sei::film_grain_characteristics::comp_model_present_flag[0]               > %04x\n", pData->comp_model_present_flag[0]);
        VLOG(INFO, "sei::film_grain_characteristics::comp_model_present_flag[1]               > %04x\n", pData->comp_model_present_flag[1]);
        VLOG(INFO, "sei::film_grain_characteristics::comp_model_present_flag[2]               > %04x\n", pData->comp_model_present_flag[2]);
    }
    if (iFlags & (1UL<<H264_USERDATA_FLAG_TONE_MAPPING_INFO)) {
        avc_sei_tone_mapping_info_t* pData = (avc_sei_tone_mapping_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_TONE_MAPPING_INFO].offset);
        VLOG(INFO, "[SEI::TONE_MAPPING_INFO]\n");
        VLOG(INFO, "sei::tone_mapping_info::tone_map_id                > %04x\n", pData->tone_map_id);
        VLOG(INFO, "sei::tone_mapping_info::tone_map_cancel_flag       > %04x\n", pData->tone_map_cancel_flag);
        VLOG(INFO, "sei::tone_mapping_info::tone_map_repetition_period > %04x\n", pData->tone_map_repetition_period);
        VLOG(INFO, "sei::tone_mapping_info::coded_data_bit_depth       > %04x\n", pData->coded_data_bit_depth);
        VLOG(INFO, "sei::tone_mapping_info::target_bit_depth           > %04x\n", pData->target_bit_depth);
        VLOG(INFO, "sei::tone_mapping_info::tone_map_model_id          > %04x\n", pData->tone_map_model_id);
        VLOG(INFO, "sei::tone_mapping_info::min_value                  > %08x\n", pData->min_value);
        VLOG(INFO, "sei::tone_mapping_info::max_value                  > %08x\n", pData->max_value);
    }
    if (iFlags & (1UL<<H265_USERDATA_FLAG_COLOUR_REMAPPING_INFO)) {
        avc_sei_colour_remap_info_t* pData = (avc_sei_colour_remap_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_COLOUR_REMAPPING_INFO].offset);
        VLOG(INFO, "[SEI::COLOR_MAPPING_INFO]\n");
        VLOG(INFO, "sei::tone_mapping_info::colour_remap_id          > %04x\n", pData->colour_remap_id);
        VLOG(INFO, "sei::tone_mapping_info::colour_rempa_cancel_flag > %04x\n", pData->colour_cancel_flag);
    }

    return;
}

static void PrintUserData(DecListenerContext* ctx, CNMComListenerDecDone*  lsnpDecDone)
{
    vpu_buffer_t*       vbUser    = NULL;
    DecOutputExtData*   ext       = NULL;
    Uint32              numFlags  = 0;
    Uint32              flags     = 0;
    Uint8*              pBase     = NULL;

    ext      = &lsnpDecDone->output->decOutputExtData;
    flags    = ext->userDataHeader;
    numFlags = ext->userDataNum;
    vbUser   = &lsnpDecDone->vbUser;

    if (0 == numFlags) {
        return;
    }
    VLOG(INFO, "----------------------------------------------------------------\n");
    VLOG(INFO, "- %d USERDATA(VUI, SEI)\n", numFlags);
    VLOG(INFO, "----------------------------------------------------------------\n");
    pBase  = (Uint8*)osal_malloc(vbUser->size);
    if (pBase == NULL) {
        VLOG(ERR, "%s failed to allocate memory\n", __FUNCTION__);
        return;
    }
    VpuReadMem(lsnpDecDone->handle->coreIdx, vbUser->phys_addr, pBase, vbUser->size, VPU_USER_DATA_ENDIAN);

    switch (lsnpDecDone->bitstreamFormat) {
    case STD_HEVC:
        PrintH265UserData(pBase, flags, vbUser);
        break;
    case STD_AVC:
        PrintH264UserData(pBase, flags, vbUser);
        break;
    default:
        VLOG(ERR, "Not supported yet!!!\n");
        break;
    }

    osal_free(pBase);

    return;
}


void HandleDecCompleteSeqEvent(Component com, CNMComListenerDecCompleteSeq* param, DecListenerContext* ctx)
{
    if (ctx->compareType == YUV_COMPARE) {
        Uint32 width  = param->initialInfo->picWidth;
        Uint32 height = param->initialInfo->picHeight;

        if (ctx->enableScaler == TRUE) {
            width  = VPU_CEIL(width, 16);
            height = VPU_CEIL(height, 4);
        }
        if ((ctx->comparator = Comparator_Create(YUV_COMPARE, param->refYuvPath, width, height, param->wtlFormat,
                                                 param->cbcrInterleave, param->bitstreamFormat==STD_VP9)) == NULL) {
            VLOG(ERR, "%s:%d Failed to Comparator_Create(%s)\n", __FUNCTION__, __LINE__, param->refYuvPath);
            CNMErrorSet(CNM_ERROR_FAILURE);
            CNMAppStop();
            return ;
        }
    }

}

void HandleDecRegisterFbEvent(Component com, CNMComListenerDecRegisterFb* param, DecListenerContext* ctx)
{
    Uint32 fps = (ctx->fps == 0) ? 30 : ctx->fps;
    //SimpleRenderer_SetFrameRate(ctx->renderer, 0);
    if ( ctx->bandwidth ) {
        ctx->bwCtx = BWMonitorSetup(param->handle, TRUE, GetBasename((const char *)ctx->inputPath));
    }
    if ( ctx->performance ) {
        ctx->pfCtx = PFMonitorSetup(param->handle->coreIdx, param->handle->instIndex, ctx->pfClock, fps, GetBasename((const char *)ctx->inputPath), 0);
    }
}

static void HandleDecInterruptEvent(Component com, CNMComListenerDecInt* param, DecListenerContext* ctx)
{
    if ((1<<INT_WAVE5_DEC_PIC) == param->flag) {
        if (ctx->bwCtx != NULL) {
            BWMonitorUpdate(ctx->bwCtx, ctx->numVCores);
            BWMonitorReset(ctx->bwCtx);
        }
    }
}

void HandleDecGetOutputEvent(Component com, CNMComListenerDecDone* param, DecListenerContext* ctx)
{
    DecOutputInfo*          output          = param->output;
    FrameBuffer*            pFb             = &output->dispFrame;
    void*                   decodedData     = NULL;
    Uint8*                  pYuv            = NULL;
    Uint32                  decodedDataSize = 0;
    VpuRect                 rcDisplay       = {0,};
    Uint32                  width=0, height = 0, Bpp;
    size_t                  frameSizeInByte = 0;
    if (param->ret != RETCODE_SUCCESS) return;

    if (ctx->pfCtx != NULL) {
        if (output->indexFrameDecoded >= 0) {
            if (ctx->productId == PRODUCT_ID_517) {
                PFMonitorUpdate(param->handle->coreIdx, ctx->pfCtx, output->frameCycle, output->seekCycle, output->parseCycle, output->DecodedCycle);
            } else {
                PFMonitorUpdate(param->handle->coreIdx, ctx->pfCtx, output->frameCycle);
            }
        }
    }
    if (ctx->bwCtx != NULL) {
        BWMonitorUpdatePrint(ctx->bwCtx, output->picType);
    }



    // Finished decoding a frame
    if (output->indexFrameDecoded >= 0) {
        PrintUserData(ctx, param);
        ctx->notDecodedCount = 0;
    }
    else {
        ctx->notDecodedCount++;
        if (ctx->notDecodedCount == MAX_NOT_DEC_COUNT) {
            VLOG(ERR, "Continuous not-decoded-count is %d\nThere is something problem in DPB control.\n", ctx->notDecodedCount);
            CNMAppStop();
        }
    }

    if (output->indexFrameDisplay < 0) {
        return;
    }

    if (ctx->lastSeqNo < pFb->sequenceNo || IsDifferentResolution(&ctx->fbInfo, pFb)) {
        /* When the video sequence or the resolution of picture was changed. */
        ctx->lastSeqNo  = pFb->sequenceNo;
        VPU_DecGiveCommand(param->handle, DEC_GET_FRAMEBUF_INFO, (void*)&ctx->fbInfo);
    }

    if (ctx->compareType == YUV_COMPARE) {
        rcDisplay.right  = output->dispPicWidth;
        rcDisplay.bottom = output->dispPicHeight;
        if (ctx->enableScaler == TRUE) {
           rcDisplay.right  = VPU_CEIL(rcDisplay.right, 16);
           rcDisplay.bottom = VPU_CEIL(rcDisplay.bottom, 2);
        }
        if (TRUE == PRODUCT_ID_W_SERIES(ctx->productId)) {
            pYuv = GetYUVFromFrameBuffer(param->handle, &output->dispFrame, rcDisplay, &width, &height, &Bpp, &frameSizeInByte);
        }
        decodedData     = (void*)pYuv;
        decodedDataSize = frameSizeInByte;
    }

    if (ctx->comparator) {
        if ((ctx->match=Comparator_Act(ctx->comparator, decodedData, decodedDataSize)) == FALSE) {
        }
    }

    if (ctx->compareType == YUV_COMPARE) {
        osal_free(decodedData);
    }


    if (ctx->match == FALSE) CNMAppStop();
}

void HandleDecCloseEvent(Component com, CNMComListenerDecClose* param, DecListenerContext* ctx)
{
    if (ctx->bwCtx != NULL)
        BWMonitorRelease(ctx->bwCtx);
    if (ctx->pfCtx != NULL)
        PFMonitorRelease(ctx->pfCtx);
}

void DecoderListener(Component com, Uint64 event, void* data, void* context)
{
#if defined(SUPPORT_MULTI_INSTANCE_TEST) || defined(SUPPORT_LOOK_AHEAD_RC)
#else
    int key=0;
    if (osal_kbhit()) {
        key = osal_getch();
        osal_flush_ch();

        switch (key) {
        case 'q':
        case 'Q':
            CNMAppStop();
            return;
        case 'f':
        case 'F':
            ComponentSetParameter(NULL, com, SET_PARAM_DEC_FLUSH, NULL);
            break;
        default:
            break;
        }
    }
#endif
    switch (event) {
    case COMPONENT_EVENT_DEC_OPEN:
        break;
    case COMPONENT_EVENT_DEC_ISSUE_SEQ:
        break;
    case COMPONENT_EVENT_DEC_COMPLETE_SEQ:
        HandleDecCompleteSeqEvent(com, (CNMComListenerDecCompleteSeq*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_REGISTER_FB:
        HandleDecRegisterFbEvent(com, (CNMComListenerDecRegisterFb*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_READY_ONE_FRAME:
        break;
    case COMPONENT_EVENT_DEC_START_ONE_FRAME:
        break;
    case COMPONENT_EVENT_DEC_INTERRUPT:
        HandleDecInterruptEvent(com, (CNMComListenerDecInt*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_GET_OUTPUT_INFO:
        HandleDecGetOutputEvent(com, (CNMComListenerDecDone*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_DECODED_ALL:
        break;
    case COMPONENT_EVENT_DEC_CLOSE:
        HandleDecCloseEvent(com, (CNMComListenerDecClose*)data, (DecListenerContext*)context);
        break;
    default:
        break;
    }
}

BOOL SetupDecListenerContext(DecListenerContext* ctx, CNMComponentConfig* config, Component renderer)
{
    TestDecConfig* decConfig = &config->testDecConfig;
    Int32 md5Size = 0;

    osal_memset((void*)ctx, 0x00, sizeof(DecListenerContext));

    md5Size = PRODUCT_ID_W_SERIES(decConfig->productId) ? 12 : 8 ;

    if (decConfig->compareType == MD5_COMPARE) {
        if ((ctx->comparator=Comparator_Create(MD5_COMPARE, decConfig->md5Path, md5Size)) == NULL) {
            VLOG(ERR, "%s:%d Failed to Comparator_Create(%s)\n", __FUNCTION__, __LINE__, decConfig->md5Path);
            return FALSE;
        }
    }


    ctx->renderer      = renderer;
    ctx->lastSeqNo     = -1;
    ctx->compareType   = decConfig->compareType;
    ctx->match         = TRUE;

    ctx->performance   = decConfig->performance;
    ctx->bandwidth     = decConfig->bandwidth;
    ctx->fps           = decConfig->fps;
    ctx->pfClock       = decConfig->pfClock;
    ctx->numVCores     = decConfig->wave.numVCores;
    ctx->bitFormat     = decConfig->bitFormat;
    ctx->productId     = decConfig->productId;
    ctx->enableScaler  = (decConfig->scaleDownWidth > 0 || decConfig->scaleDownHeight > 0);
    osal_memcpy(ctx->inputPath, decConfig->inputPath, sizeof(ctx->inputPath));

    return TRUE;
}

void ClearDecListenerContext(DecListenerContext* ctx)
{
    if (ctx->comparator)    Comparator_Destroy(ctx->comparator);
}



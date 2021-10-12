#ifndef __HEADER_STRUCT_H__
#define __HEADER_STRUCT_H__

#define EXTENDED_SAR 255
#define MAX_VPS_MAX_SUB_LAYERS 16
#define MAX_CPB_COUNT 16

typedef struct sei_buffering_period_tag {
	Uint32 sub_pic_hrd_params_present_flag;	//derived from syntax elements found in the hrd_parameters( )
	Uint32 au_cpb_removal_delay_length_minus1; //derived from syntax elements found in the hrd_parameters( )
	Uint32 initial_cpb_removal_delay_length_minus1; //derived from syntax elements found in the hrd_parameters( )
	Uint32 CpbCnt;	//CpbCnt are derived from syntax elements found in the sub_layer_hrd_parameters( ) syntax structure
	Uint32 NalHrdBpPresentFlag;		// NalHrdBpPresentFlag and VclHrdBpPresentFlag are found in or derived from syntax elements found in the hrd_parameters( )
	Uint32 VclHrdBpPresentFlag;		//NalHrdBpPresentFlag and VclHrdBpPresentFlag are found in or derived from syntax elements found in the hrd_parameters( )

	Uint32 bp_seq_parameter_set_id; 	
	Uint32 irap_cpb_params_present_flag;	
	Uint32 cpb_delay_offset;					//The syntax element has a length in bits given by au_cpb_removal_delay_length_minus1 + 1
	Uint32 dpb_delay_offset;
	Uint32 concatenation_flag; 
	Uint32 au_cpb_removal_delay_delta_minus1;	
	Uint32 nal_initial_cpb_removal_delay[MAX_CPB_COUNT];	// 
	Uint32 nal_initial_cpb_removal_offset[MAX_CPB_COUNT];
	Uint32 nal_initial_alt_cpb_removal_delay[MAX_CPB_COUNT];
	Uint32 nal_initial_alt_cpb_removal_offset[MAX_CPB_COUNT];

	Uint32 vcl_initial_cpb_removal_delay[MAX_CPB_COUNT];
	Uint32 vcl_initial_cpb_removal_offset[MAX_CPB_COUNT];
	Uint32 vcl_initial_alt_cpb_removal_delay[MAX_CPB_COUNT];
	Uint32 vcl_initial_alt_cpb_removal_offset[MAX_CPB_COUNT];	
} sei_buffering_period_t;

typedef struct sei_pic_timing_tag {
	Uint32 duplicate_flag; 
	Uint32 source_scan_type;
	Uint32 pic_struct;

	Uint32 CpbDpbDelaysPresentFlag;	// if nal_hrd_parameters_present_flag or vcl_hrd_parameters_present_flag  is 1,  
	Uint32 au_cpb_removal_delay_minus1;
	Uint32 pic_dpb_output_delay;
	Uint32 pic_dpb_output_du_delay;
	
	Uint32 num_decoding_units_minus1;
	Uint32 du_common_cpb_removal_delay_flag;
	Uint32 du_common_cpb_removal_delay_increment_minus1;
#define MAX_NUM_DECODING_UNIT 20
	Uint32 num_nalus_in_du_minus1[MAX_NUM_DECODING_UNIT];
	Uint32 du_cpb_removal_delay_increment_minus1[MAX_NUM_DECODING_UNIT];

} sei_pic_timing_t;

typedef struct sei_active_parameter_tag {
	Uint32 active_video_parameter_set_id; 
	Uint32 self_contained_cvs_flag;
	Uint32 no_parameter_set_update_flag;

	Uint32 num_sps_ids_minus1;
#define MAX_NUM_SEQ_ID 20
	Uint32 active_seq_parameter_set_id[MAX_NUM_SEQ_ID];
} sei_active_parameter_t;

typedef enum 
{
	BUFFERING_PERIOD = 0,
	PICTURE_TIMING,
	FILLER_PAYLOAD = 3,                      
	USER_DATA_REGISTERED_ITU_T_T35,      
	USER_DATA_UNREGISTERED,              
	RECOVERY_POINT,                      
	SCENE_INFO = 9,                          
	FULL_FRAME_SNAPSHOT = 15,                 
	PROGRESSIVE_REFINEMENT_SEGMENT_START,
	PROGRESSIVE_REFINEMENT_SEGMENT_END,  
	FILM_GRAIN_CHARACTERISTICS = 19,          
	POST_FILTER_HINT = 22,                    
	TONE_MAPPING_INFO,                   
	KNEE_FUNCTION_INFO,                  
	FRAME_PACKING = 45,                       
	DISPLAY_ORIENTATION = 47,                 
	SOP_DESCRIPTION,                     
	ACTIVE_PARAMETER_SETS = 129,               
	DECODING_UNIT_INFO,                  
	TEMPORAL_LEVEL0_INDEX,               
	DECODED_PICTURE_HASH,                
	SCALABLE_NESTING = 133,                    
	REGION_REFRESH_INFO,                 
	NO_DISPLAY,                          
	TIME_CODE,                           
	MASTERING_DISPLAY_COLOUR_VOLUME,     
	SEGM_RECT_FRAME_PACKING,             
	TEMP_MOTION_CONSTRAINED_TILE_SETS,   
	CHROMA_SAMPLING_FILTER_HINT
} SeiPayloadType;

enum 
{
	SNT_TRAIL_N         = 0,
	SNT_TRAIL_R         = 1,
	SNT_TSA_N           = 2,
	SNT_TSA_R           = 3,
	SNT_STSA_N          = 4,
	SNT_STSA_R          = 5,
	SNT_RADL_N          = 6,
	SNT_RADL_R          = 7,
	SNT_RASL_N          = 8,
	SNT_RASL_R          = 9,
	SNT_RSV_N10         = 10,
	SNT_RSV_R11         = 11,
	SNT_RSV_N12         = 12,
	SNT_RSV_R13         = 13,
	SNT_RSV_N14         = 14,
	SNT_RSV_R15         = 15,
	SNT_BLA_W_LP        = 16,
	SNT_BLA_W_RADL      = 17,
	SNT_BLA_N_LP        = 18,
	SNT_IDR_W_RADL      = 19,
	SNT_IDR_N_LP        = 20,
	SNT_CRA             = 21,
	SNT_RSV_IRAP22      = 22,
	SNT_RSV_IRAP23      = 23,
	SNT_VPS             = 32,
	SNT_SPS             = 33,
	SNT_PPS             = 34,
	SNT_AUD             = 35,
	SNT_EOS             = 36,
	SNT_EOB             = 37,
	SNT_FD              = 38,
	SNT_PREFIX_SEI      = 39,
	SNT_SUFFIX_SEI      = 40,
};

typedef struct 
{
	Uint32 vui_parameters_presesent_flag;

	// vui_present_flags
	Uint32 neutral_chroma_indication_flag;
	Uint32 field_seq_flag;		// this syntax shall be 1 when pic_struct in sei_pic_timing is 1 or 2
	Uint32 frame_field_info_present_flag;	//this syntax shall be 1 when pic_struct in sei_pic_timing is not 0
	Uint32 aspect_ratio_info_present_flag;
	Uint32 overscan_info_present_flag;
	Uint32 video_signal_type_present_flag;
	Uint32 colour_description_present_flag;
	Uint32 chroma_loc_info_present_flag;
	Uint32 default_display_window_flag;
	Uint32 vui_timing_info_present_flag;
	Uint32 bitstream_restriction_flag;

	// vui_aspect_ratio_idc
	Uint32 aspect_ratio_idc;

	// vui_sar_size
	Uint32 sar_width;
	Uint32 sar_height;

	// vui_overscan_appropriate
	Uint32 overscan_appropriate_flag;

	// vui_video_signal
	Uint32 video_format;
	Uint32 video_full_range_flag;
	Uint32 colour_primaries;
	Uint32 transfer_characteristics;
	Uint32 matrix_coeffs;

	// vui_chroma_sample_loc
	Uint32 chroma_sample_loc_type_top_field;
	Uint32 chroma_sample_loc_type_bottom_field;

	// vui_disp_win_left_right
	Uint32 def_disp_win_left_offset;
	Uint32 def_disp_win_right_offset;

	// vui_disp_win_top_bot
	Uint32 def_disp_win_top_offset;
	Uint32 def_disp_win_bottom_offset;

	Uint32 vui_hrd_parameters_present_flag;

	Uint32 vui_num_units_in_tick;     
	Uint32 vui_time_scale;     
	Uint32 vui_poc_proportional_to_timing_flag;     
	Uint32 vui_num_ticks_poc_diff_one_minus1; 

	Uint32 tiles_fixed_structure_flag;     
	Uint32 motion_vectors_over_pic_boundaries_flag;     
	Uint32 restricted_ref_pic_lists_flag;     
	Uint32 min_spatial_segmentation_idc; 
	Uint32 max_bytes_per_pic_denom; 
	Uint32 max_bits_per_min_cu_denom; 
	Uint32 log2_max_mv_length_horizontal; 
	Uint32 log2_max_mv_length_vertical; 

} vui_t;

typedef struct 
{
	Uint32 encodeRbspHrdInVui;
	Uint32 encodeRbspHrdInVps;

	Uint32 commonInfPresentFlag;
	Uint32 vps_max_sub_layers_minus1; // this parameter is from VPS

	Uint32 nal_hrd_parameters_present_flag;
	Uint32 vcl_hrd_parameters_present_flag;
	Uint32 sub_pic_hrd_params_present_flag;

	Uint32 tick_divisor_minus2;
	Uint32 du_cpb_removal_delay_increment_length_minus1;
	Uint32 sub_pic_cpb_params_in_pic_timing_sei_flag;
	Uint32 dpb_output_delay_du_length_minus1;

	Uint32 bit_rate_scale;
	Uint32 cpb_size_scale;

	Uint32 cpb_size_du_scale;
	Uint32 initial_cpb_removal_delay_length_minus1;
	Uint32 au_cpb_removal_delay_length_minus1;
	Uint32 dpb_output_delay_length_minus1;

	Uint32 fixed_pic_rate_general_flag[MAX_VPS_MAX_SUB_LAYERS];
	Uint32 fixed_pic_rate_within_cvs_flag[MAX_VPS_MAX_SUB_LAYERS];
	Uint32 elemental_duration_in_tc_minus1[MAX_VPS_MAX_SUB_LAYERS];
	Uint32 low_delay_hrd_flag[MAX_VPS_MAX_SUB_LAYERS];
	Uint32 cpb_cnt_minus1[MAX_VPS_MAX_SUB_LAYERS];

	//sub_layer_hrd_parameters(

	Uint32 bit_rate_value_minus1[MAX_CPB_COUNT][MAX_VPS_MAX_SUB_LAYERS];
	Uint32 cpb_size_value_minus1[MAX_CPB_COUNT][MAX_VPS_MAX_SUB_LAYERS];
	Uint32 cpb_size_du_value_minus1[MAX_CPB_COUNT][MAX_VPS_MAX_SUB_LAYERS];
	Uint32 bit_rate_du_value_minus1[MAX_CPB_COUNT][MAX_VPS_MAX_SUB_LAYERS];
	Uint32 cbr_flag[MAX_CPB_COUNT][MAX_VPS_MAX_SUB_LAYERS];	

} hrd_t;

#endif /* __HEADER_STRUCT_H__ */
 

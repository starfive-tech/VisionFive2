//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

/************************************************************************/
/* Error code definitions depending on product                          */
/************************************************************************/

#ifndef ERROR_CODE_H_INCLUDED
#define ERROR_CODE_H_INCLUDED

/*
 * CODA9(CODA960, CODA980, WAVE320)
 */

/*
 * WAVE410
 */

/************************************************************************/
/* WAVE4 SYSTEM ERROR                                                   */
/************************************************************************/
#define WAVE4_CODEC_ERROR                                               0x00000001
#define WAVE4_SYSERR_STREAM_BUF_FULL                                    0x00000100
#define WAVE4_SYSERR_ACCESS_VIOLATION_HW                                0x00001000
#define WAVE4_SYSERR_UNSPECIFIED                                        0x00002000
#define WAVE4_SYSERR_WRITEPROTECTION                                    0x00004000
#define WAVE4_SYSERR_CP0_EXCEPTION                                      0x00006000
#define WAVE4_SYSERR_WATCHDOG_TIMEOUT                                   0x00008000

/************************************************************************/
/*  WAVE4 CODEC ERROR ON DEC_PIC_HDR                                    */
/************************************************************************/
#define WAVE4_SPSERR_MAX_SUB_LAYERS_MINUS1                              0x00000001  // WARNING	sps_max_sub_layer_minus1 shall be 0 to 6                         
#define WAVE4_SPSERR_GENERAL_RESERVED_ZERO_44BITS                       0x00000002  // WARNING	general_reserved_zero_44bits shall be 0                          
#define WAVE4_SPSERR_RESERVED_ZERO_2BITS                                0x00000004  // WARNING	reserved_zero_2bits shall be 0                                   
#define WAVE4_SPSERR_SUB_LAYER_RESERVED_ZERO_44BITS                     0x00000008  // WARNING	sub_layer_reserved_zero_44bits shall be 0                        
#define WAVE4_SPSERR_GENERAL_LEVEL_IDC                                  0x00000010  // WARNING	general_level_idc shall have one of level of Table A.1                                  
#define WAVE4_SPSERR_SPS_MAX_DEC_PIC_BUFFERING_VALUE_OVER               0x00000020  // WARNING	sps_max_dec_pic_buffering[i] <= MaxDpbSize                                              
#define WAVE4_SPSERR_RBSP_TRAILING_BITS                                 0x00000040  // WARNING	trailing bits shall be 1000... pattern, 7.3.2.1      

#define WAVE4_SPSERR_BASE                                               0x00000100
#define WAVE4_SPSERR_SEQ_PARAMETER_SET_ID                               0x00000100  // ERROR	seq_parameter_set_id golomb decode error 
#define WAVE4_SPSERR_CHROMA_FORMAT_IDC                                  0x00000200  // ERROR	chroma_format_idc golomb decode error    
#define WAVE4_SPSERR_PIC_WIDTH_IN_LUMA_SAMPLES                          0x00000300  // ERROR	pic_width_in_luma_samples golomb decode error                      
#define WAVE4_SPSERR_PIC_HEIGHT_IN_LUMA_SAMPLES                         0x00000400  // ERROR	pic_height_in_luma_samples golomb decode error                     
#define WAVE4_SPSERR_CONF_WIN_LEFT_OFFSET                               0x00000500  // ERROR	conf_win_left_offset golomb decode error                           
#define WAVE4_SPSERR_CONF_WIN_RIGHT_OFFSET                              0x00000600  // ERROR	conf_win_right_offset golomb decode error                          
#define WAVE4_SPSERR_CONF_WIN_TOP_OFFSET                                0x00000700  // ERROR	conf_win_top_offset golomb decode error                            
#define WAVE4_SPSERR_CONF_WIN_BOTTOM_OFFSET                             0x00000800  // ERROR	conf_win_top_offset golomb decode error                            
#define WAVE4_SPSERR_BIT_DEPTH_LUMA_MINUS8                              0x00000900  // ERROR	bit_depth_luma_minus8 golomb decode error                          
#define WAVE4_SPSERR_BIT_DEPTH_CHROMA_MINUS8                            0x00000A00  // ERROR	bit_depth_chroma_minus8 golomb decode error                        
#define WAVE4_SPSERR_LOG2_MAX_PIC_ORDER_CNT_LSB_MINUS4                  0x00000B00  // ERROR	log2_max_pic_order_cnt_lsb_minus4 golomb decode error              
#define WAVE4_SPSERR_SPS_MAX_DEC_PIC_BUFFERING                          0x00000C00  // ERROR	sps_max_dec_pic_buffering[i] golomb decode error                                              
#define WAVE4_SPSERR_SPS_MAX_NUM_REORDER_PICS                           0x00000D00  // ERROR	sps_max_num_reorder_pics[i] golomb decode error                                               
#define WAVE4_SPSERR_SPS_MAX_LATENCY_INCREASE                           0x00000E00  // ERROR	sps_sps_max_latency_increase[i] golomb decode error                 
#define WAVE4_SPSERR_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS3             0x00000F00  // ERROR	log2_min_luma_coding_block_size_minus3 golomb decode error          
#define WAVE4_SPSERR_LOG2_DIFF_MAX_MIN_LUMA_CODING_BLOCK_SIZE           0x00001000  // ERROR	log2_diff_max_min_luma_coding_block_size golomb decode error        
#define WAVE4_SPSERR_LOG2_MIN_TRANSFORM_BLOCK_SIZE_MINUS2               0x00001100  // ERROR	log2_min_transform_block_size_minus2 golomb decode error            
#define WAVE4_SPSERR_LOG2_DIFF_MAX_MIN_TRANSFORM_BLOCK_SIZE             0x00001200  // ERROR	log2_diff_max_min_transform_block_size golomb decode error          
#define WAVE4_SPSERR_MAX_TRANSFORM_HIERARCHY_DEPTH_INTER                0x00001300  // ERROR	max_transform_hierarchy_depth_inter golomb decode error             
#define WAVE4_SPSERR_MAX_TRANSFORM_HIERARCHY_DEPTH_INTRA                0x00001400  // ERROR	max_transform_hierarchy_depth_intra golomb decode error             
#define WAVE4_SPSERR_SCALING_LIST                                       0x00001500  // ERROR	scaling list parsing error in scaling_list_data()                   
#define WAVE4_SPSERR_LOG2_DIFF_MIN_PCM_LUMA_CODING_BLOCK_SIZE_MINUS3    0x00001600  // ERROR	log2_diff_min_pcm_luma_coding_block_size_minus3 golomb decode error 
#define WAVE4_SPSERR_LOG2_DIFF_MAX_MIN_PCM_LUMA_CODING_BLOCK_SIZE       0x00001700  // ERROR	log2_diff_max_min_pcm_luma_coding_block_size golomb decode error    
#define WAVE4_SPSERR_NUM_SHORT_TERM_REF_PIC_SETS                        0x00001800  // ERROR	num_short_term_ref_pic_sets golomb decode error                     
#define WAVE4_SPSERR_NUM_LONG_TERM_REF_PICS_SPS                         0x00001900  // ERROR	num_long_term_ref_pics_sps golomb decode error                      

//  WAVE4 CODEC ERROR SPEC OVER SECTION : shared with DEC_PIC command 
#define WAVE4_SPEC_OVER_PICTURE_WIDTH_SIZE                              0x00010000  // Spec over	ERROR	decoded picture width size over 
#define WAVE4_SPEC_OVER_PICTURE_HEIGHT_SIZE                             0x00020000  // Spec over	ERROR	decoded picture height size over 
#define WAVE4_SPEC_OVER_CHROMA_FORMAT                                   0x00040000  // Spec over	ERROR	chroma format is not 4:2:0 
#define WAVE4_SPEC_OVER_BIT_DEPTH                                       0x00080000  // luma or chroma bit depth over.                                                 
                                                                                    // In WAVE410 v1.0 luma and chroma bit depth should be 8 bits.                   
                                                                                    // In WAVE410 v2.0 luma and chroma bit depth should be up to 10 bits repectively.
#define WAVE4_SPEC_OVER_PROFILE                                         0x00100000  // Spec over	WARNING	                                                                                                
                                                                                    // general_profile_idc and general_profile_compatibility_flag over.                                                  
                                                                                    // In WAVE410 v1.0. general_profile_idc and general_profile_compatibility_flag should specify Main profile.          
                                                                                    // In WAVE410 v2.0. general_profile_idc and general_profile_compatibility_flag should specify Main or Main10 profile.
#define WAVE4_SPEC_OVER_LEVEL                                           0x00200000  // Spec over	WARNING	general_level_idc over.

/************************************************************************/
/*  WAVE4 CODEC ERROR NO SEQUENCE INFORMATION SECTION                   */
/************************************************************************/
#define WAVE4_SPSERR_NOT_FOUND                                          0x01000000 
#define WAVE4_SPS_PARSING_ERROR                                         0x02000000

/************************************************************************/
/*  WAVE4 CODEC ERROR ON DEC_PIC                                        */
/************************************************************************/
/* Syntax error */
#define WAVE4_SHERR_FIRST_SLICE_SEGMENT_IN_PIC_FLAG         0x00000001      // WARNING first_slice_in_pic_flag shall be 1 at first slice
#define WAVE4_SHERR_NO_OUTPUT_OF_PRIOR_PICS_FLAG            0x00000002      // WARNING no_output_of_prior_pics_flag shall be equal to the first slice
#define WAVE4_SHERR_PIC_OUTPUT_FLAG                         0x00000004      // WARNING pic_output_flag shall be equal to the first slice
#define WAVE4_SHERR_SLICE_PIC_PARAMETER_SET_ID              0x00000100      // ERROR slice_pic_parameter_set_id decode error
#define WAVE4_SHERR_ACTIVATE_PPS                            0x00000200      // ERROR activate_pps decode error
#define WAVE4_SHERR_SLICE_TYPE                              0x00000300      // ERROR slice_type decode error
#define WAVE4_SHERR_FIRST_SLICE_IS_DEPENDENT_SLICE          0x00000400      // ERROR first_slice must be independent slice
#define WAVE4_SHERR_SHORT_TERM_REF_PIC_SET_SPS_FLAG         0x00000500      // ERROR short_term_ref_pic_set_sps_flag shall be equal to the first slice
#define WAVE4_SHERR_SHORT_TERM_REF_PIC_SET                  0x00000600      // ERROR short_term_ref_pic_set decode error
#define WAVE4_SHERR_SHORT_TERM_REF_PIC_SET_IDX              0x00000700      // ERROR short_term_ref_pic_set_idx shall be equal to the first slice
#define WAVE4_SHERR_NUM_LONG_TERM_SPS                       0x00000800      // ERROR num_long_term_sps decode error
#define WAVE4_SHERR_NUM_LONG_TERM_PICS                      0x00000900      // ERROR num_long_term_pics decode error
#define WAVE4_SHERR_LT_IDX_SPS_IS_OUT_OF_RANGE              0x00000A00      // ERROR lt_idx_sps is out of range
#define WAVE4_SHERR_DELTA_POC_MSB_CYCLE_LT                  0x00000B00      // ERROR delta_poc_msb_cycle_lt decode error
#define WAVE4_SHERR_NUM_REF_IDX_L0_ACTIVE_MINUS1            0x00000C00      // ERROR num_ref_idx_l0_active_minus1 decode error
#define WAVE4_SHERR_NUM_REF_IDX_L1_ACTIVE_MINUS1            0x00000D00      // ERROR num_ref_idx_l1_active_minus1 decode error
#define WAVE4_SHERR_COLLOCATED_REF_IDX                      0x00000E00      // ERROR collocated_ref_idx decode error
#define WAVE4_SHERR_PRED_WEIGHT_TABLE                       0x00000F00      // ERROR pred_weight_table decode error
#define WAVE4_SHERR_FIVE_MINUS_MAX_NUM_MERGE_CAND           0x00001000      // ERROR five_minus_max_num_merge_cand decode error
#define WAVE4_SHERR_SLICE_QP_DELTA                          0x00001100      // ERROR slice_qp_delta decode error
#define WAVE4_SHERR_SLICE_QP_DELTA_IS_OUT_OF_RANGE          0x00001200      // ERROR slice_qp_delta is out of range
#define WAVE4_SHERR_SLICE_CB_QP_OFFSET                      0x00001300      // ERROR slice_cb_qp_offset decode error
#define WAVE4_SHERR_SLICE_CR_QP_OFFSET                      0x00001400      // ERROR slice_cr_qp_offset decode error
#define WAVE4_SHERR_SLICE_BETA_OFFSET_DIV2                  0x00001500      // ERROR slice_beta_offset_div2 decode error
#define WAVE4_SHERR_SLICE_TC_OFFSET_DIV2                    0x00001600      // ERROR slice_tc_offset_div2 decode error
#define WAVE4_SHERR_NUM_ENTRY_POINT_OFFSETS                 0x00001700      // ERROR num_entry_point_offsets decode error
#define WAVE4_SHERR_OFFSET_LEN_MINUS1                       0x00001800      // ERROR offset_len_minus1 decode error
#define WAVE4_SHERR_SLICE_SEGMENT_HEADER_EXTENSION_LENGTH   0x00001900      // ERROR slice_segment_header_extension_length decode error
#define WAVE4_SHERR_SLICE_HEADER_OVER_CONSUMED              0x00001A00      // ERROR slice header over-consumed
#define WAVE4_SHERR_DPB_OVERFLOW                            0x00001B00      // ERROR DPB overflow

// ETC 
#define WAVE4_ETCERR_NEXT_AU_SLICE                          0x01000000      // WARNING  slice of next au found
#define WAVE4_ETCERR_SLICE_NOT_FOUND                        0x02000000      // WARNING  slice not found
#define WAVE4_ETCERR_SEQ_CHANGE_DETECT                      0x04000000      // ERROR    sequence change detected
#define WAVE4_ETCERR_PPS_NOT_FOUND                          0x08000000      // ERROR    PPS not found
#define WAVE4_ETCERR_SPS_NOT_FOUND                          0x10000000      // ERROR    SPS not found
#define WAVE4_ETCERR_MISSING_REFERENCE_PICTURE              0x20000000      // WARNING  Reference picture was missing
#define WAVE4_ETCERR_LACK_OF_STREAM                         0x40000000      // ERROR    notice lack_of_stream for support roll-back mode
#define WAVE4_ETCERR_SH_PARSING_ERROR                       0x80000000      // ERROR    syntax parsing error detected

/*
 * WAVE5
 */

/************************************************************************/
/* WAVE5 SYSTEM ERROR                                                   */
/************************************************************************/
#define WAVE5_CODEC_ERROR                                               0x00000001
#define WAVE5_SYSERR_ACCESS_VIOLATION_HW                                0x00000040
#define WAVE5_RESULT_NOT_READY                                          0x00000800
#define WAVE5_VPU_STILL_RUNNING                                         0x00001000
#define WAVE5_INSTANCE_DESTROYED                                        0x00004000
#define WAVE5_SYSERR_WATCHDOG_TIMEOUT                                   0x00008000

#endif /* ERROR_CODE_H_INCLUDED */


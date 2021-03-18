#!/bin/sh
#./multi_instance_dec_test --codec 0,0,0,0 --input bd_8b_01.cfg_0.264-0,bd_8b_01.cfg_0.264-1,bd_8b_01.cfg_0.264-2,bd_8b_01.cfg_0.264-3 --output bd_8b_01.cfg_0.yuv-0,bd_8b_01.cfg_0.yuv-1,bd_8b_01.cfg_0.yuv-2,bd_8b_01.cfg_0.yuv-3 --instance-num 4 --enable-wtl 1,1,1,1

./multi_instance_dec_test --codec 12,0,12 --input stream/hevc_fhd_inter_8b_11.cfg_0.265,stream/avc_uhd_cavlc_8b_04.cfg_0.264,stream/inter_8b_11.cfg.265 --output output/multi_hevc_fhd_inter_8b_11.cfg_0.265.yuv,output/multi_avc_uhd_cavlc_8b_04.cfg_0.264.yuv,output/multi_inter_8b_11.cfg.265.yuv --instance-num 3 --enable-wtl 1,1,1

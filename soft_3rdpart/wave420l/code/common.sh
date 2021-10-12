###############################################################################
# COMMON DATA
###############################################################################
ON_OFF=(
    "OFF"
    "ON"
)

readonly C_YUV_8B_P420=0
readonly C_YUV_8B_P422=1
readonly C_YUV_10B_MSB_P420=5
readonly C_YUV_10B_LSB_P420=6
readonly C_YUV_3P4B_MSB_P420=7          # each pixel 10bit
readonly C_YUV_3P4B_LSB_P420=8          # each pixel 10bit
readonly C_YUV_10B_MSB_P422=9
readonly C_YUV_10B_LSB_P422=10
readonly C_YUV_3P4B_MSB_P422=11         # each pixel 10bit
readonly C_YUV_3P4B_LSB_P422=12         # each pixel 10bit

readonly C_YUV_8B_YUYV=13         
readonly C_YUV_10B_MSB_YUYV=14
readonly C_YUV_10B_LSB_YUYV=15
readonly C_YUV_3P4B_MSB_YUYV=16
readonly C_YUV_3P4B_LSB_YUYV=17

readonly C_YUV_8B_YVYU=18         
readonly C_YUV_10B_MSB_YVYU=19
readonly C_YUV_10B_LSB_YVYU=20
readonly C_YUV_3P4B_MSB_YVYU=21
readonly C_YUV_3P4B_LSB_YVYU=22

readonly C_YUV_8B_UYVY=23 
readonly C_YUV_10B_MSB_UYVY=24
readonly C_YUV_10B_LSB_UYVY=25
readonly C_YUV_3P4B_MSB_UYVY=26
readonly C_YUV_3P4B_LSB_UYVY=27

readonly C_YUV_8B_VYUY=28 
readonly C_YUV_10B_MSB_VYUY=29
readonly C_YUV_10B_LSB_VYUY=30
readonly C_YUV_3P4B_MSB_VYUY=31
readonly C_YUV_3P4B_LSB_VYUY=32

readonly CODA_MIN_ENDIAN=0
readonly CODA_MAX_ENDIAN=3
readonly WAVE_MIN_ENDIAN=16
readonly WAVE_MAX_ENDIAN=31

readonly CODA_SEC_AXI_MAX=63
readonly SCALER_SEC_AXI_MAX=15
readonly BWB_SECONDARY_AXI_MAX=7

YUV_FORMAT_LIST=(
    "FORMAT_420"
    "RESERVED"
    "RESERVED"
    "RESERVED"
    "RESERVED"
    "FORMAT_420_P10_16BIT_MSB"
    "FORMAT_420_P10_16BIT_LSB"
    "FORMAT_420_P10_32BIT_MSB"
    "FORMAT_420_P10_32BIT_LSB"
)

YUV_FORMAT_LIST_8BIT=(
    "FORMAT_420"
)


AFBCE_FORMAT_NAME=(
    "NONE"
    "AFBCE_FORMAT_8BIT"
    "AFBCE_FORMAT_10BIT"
)

C9_ENDIAN_NAME=(
    "VDI_64BIT_LE"
    "VDI_64BIT_BE"
    "VDI_32BIT_LE"
    "VDI_32BIT_BE"
)

W4_ENDIAN_NAME=(
    "VDI_128BIT_BIG_ENDIAN"
    "VDI_128BIT_BE_BYTE_SWAP"
    "VDI_128BIT_BE_WORD_SWAP"
    "VDI_128BIT_BE_WORD_BYTE_SWAP"
    "VDI_128BIT_BE_DWORD_SWAP"
    "VDI_128BIT_BE_DWORD_BYTE_SWAP"
    "VDI_128BIT_BE_DWORD_WORD_SWAP"
    "VDI_128BIT_BE_DWORD_WORD_BYTE_SWAP"
    "VDI_128BIT_LE_DWORD_WORD_BYTE_SWAP"
    "VDI_128BIT_LE_DWORD_WORD_SWAP"
    "VDI_128BIT_LE_DWORD_BYTE_SWAP"
    "VDI_128BIT_LE_DWORD_SWAP"
    "VDI_128BIT_LE_WORD_BYTE_SWAP"
    "VDI_128BIT_LE_WORD_SWAP"
    "VDI_128BIT_LE_BYTE_SWAP"
    "VDI_128BIT_LITTLE_ENDIAN"
)

DEC_BSMODE_NAME=(
    "Interrupt"
    "Reserved"
    "PicEnd"
)

ENC_BSMODE_NAME=(
    "Ringbuffer"         
    "Linebuffer"
)

ENC_WAVE_BSMODE_NAME=(
    "Linebuffer"
    "Ringbuffer, Wrap On"
    "Ringbuffer, Wrap Off"
)

CODA980_MAPTYPE_NAME=(
    "LINEAR_FRAME"
    "TILED_FRAME_V"        
    "TILED_FRAME_H"        
    "TILED_FIELD_V"        
    "TILED_MIXED_V"        
    "TILED_FRAME_MB_RASTER"
    "TILED_FIELD_MB_RASTER"
    "TILED_FRAME_NO_BANK"  
    "TILED_FIELD_NO_BANK"  
)

CODA960_MAPTYPE_NAME=(
    "LINEAR_FRAME"
    "TILED_FRAME_V"        
    "TILED_FRAME_H"        
    "TILED_FIELD_V"        
    "TILED_MIXED_V"        
    "TILED_FRAME_MB_RASTER"
    "TILED_FIELD_MB_RASTER"
)

MIRROR_NAME=(
    "NONE"
    "VERTICAL"
    "HORIZONTAL"
    "BOTH"
)

CODEC_NAME=(
    "H.264"
    "VC-1"
    "MPEG-2"
    "MPEG-4"
    "H.263"
    "DIVX3"
    "RV"
    "AVS"
    "NONE"
    "THEORA"
    "NONE"
    "VP8"
    "HEVC"
    "VP9"
    "AVS2"
)


PASS="PASS"
FAIL="FAIL"

###############################################################################
# GLOBAL VARIABLES
###############################################################################
g_fpga_reset=0                       # 1 : reset fpga every time

######################## COMMON VARAIABLES FOR DECODER ########################
g_yuv_fmt_list=("${YUV_FORMAT_LIST[@]}")
g_match_mode=1                        # 0 - nomatch, 1 - yuv, 2 - md5
g_aclk_freq=20
g_bclk_freq=20
g_cclk_freq=20
g_mclk_freq=20
g_vclk_freq=20
g_read_latency=0
g_write_latency=0
g_secondary_axi=0
g_stream_endian=0
g_source_endian=0
g_frame_endian=0
g_bsmode=0
g_enable_thumbnail=0
g_cbcr_interleave=0
g_enable_nv21=0
g_enable_wtl=1
g_wtl_format=0
g_multi_vcore=0
g_num_cores=1
g_bs_size=0
g_codec_index=0
#ifdef SUPPORT_CONF_TESET
g_fake_test=0
g_skip_test=0
g_support_minipippen=0
#endif /* SUPPORT_CONF_TEST */
g_scaler=0
g_force_bsmode=0
g_revision=""
########################## WAVE4xx DECODER VARAIABLES #########################
g_tid_test=0
g_bw_opt=1
g_afbce=0               # WAVE412 Feature
g_force_num_cores=0

####################### CODA9&CODA7 DECODER VARAIABLES ########################
g_maptype_index=0           # CODA9xx feature
g_enable_tiled2linear=0
g_enable_deblock=0
g_enable_dering=0
g_enable_mvc=0              # H.264 MVC
g_mp4class=0
g_maptype_num=1
g_enable_ppu=1

######################## VARAIABLES FOR WAVE4 ENCODER #########################
g_subframe_sync=0
g_packedFormat=0
g_srcFormat3p4b=0
######################## VARAIABLES FOR CODA9 ENCODER #########################
g_enable_linear2tiled=0 
g_linebuf_int=0

########################## VARAIABLES FOR PPU #################################
g_rotAngle=0
g_mirDir=0


###############################################################################
# GLOBAL FUNCTIONS
###############################################################################
# ceiling function
function ceiling {
    local value=$1
    local align=$2

    echo "$(((($value+$align-1)/$align)*$align))"
}

# check_and_create_dir
function check_and_create_dir {
    CHECK_PATH=$1
    if [ ! -d "$CHECK_PATH" ]; then
        mkdir -p $CHECK_PATH
        chmod 777 $CHECK_PATH
        echo "create $CHECK_PATH"
    else
        echo "Already $CHECK_PATH Exist"
    fi
}

# get_random start end
# generate constraint random value with @start end @end.
#
function get_random {
    start=$1
    end=$2
    size=$(($end - $start + 1))
    val2=$RANDOM
    val1=$(($val2 % size))
    val=$(($start + $val1%$size))
    echo "$val"
}

function get_random_min_max {
    start=$1
    end=$2

    use_min_max_value=$(($RANDOM % 5))
    size=$(($end - $start - 1))

    if [ $use_min_max_value = 1 ] || [ $size -le 0 ]; then
        val2=$RANDOM
        val1=$(($val2 % 2))
        if [ $val1 = 1 ]; then
            val=$end
        else
            val=$start
        fi
    else
        val2=$RANDOM
        val1=$(($val2 % size))
        val=$(($start + $val1%$size + 1))
    fi

    echo "$val"
}

# log_conf log_message log_file
# write @log_message to @log_file
#
function log_conf {
    echo "$1" | tee -a "$2"
}

# parse_streamset_file streamset_path *array
# - parse streamset file and store data into @g_stream_file_array
# 
function parse_streamset_file {
    local f="$1"
    local index=0
    local line=""
    while read -r line || [ -n "$line" ]; do
        line="${line#*( )}"         # remove front whitespace
        line="${line%%[ $'\t']*}"   # remove start of whitespace
        line="${line//\\//}"        # replace backslash to slash
        line="${line/$'\r'/}"       # remove carriage return
        firstchar=${line:0:1}
        case "$firstchar" in
            "@") break;;        # exit flag
            "#") continue;;     # comment
            ";") continue;;     # comment
            "")  continue;;     # comment
            *)
        esac
        eval $2[$index]="$line"
        index=$(($index + 1))
    done < $f
}

#ifdef SUPPORT_CONF_TESET
function replace_byte {
    printf "$(printf '\\x%02X' $3)" | dd of="$1" bs=1 seek=$2 count=1 conv=notrunc &> /dev/null
}

function get_byte_data {
    ori_value=$(xxd -s $2 -l 1 $1 | cut -d' ' -f2) #get original value
    ori_value=$((16#$ori_value))                     #hex to dec
    echo "$ori_value"
}

function bit_toggle {
    local ori_byte=$1
    local bit_pos=$2

    (($ori_byte&((1<<$bit_pos)))) && temp=1 || temp=0 #bit exist check

    out=$(($ori_byte - ((1<<$bit_pos))))
    if [ "$temp" == "1" ];then  #bit exist -> clear bit
        out=$((1 << $bit_pos))
        out=$(($ori_byte - $out))
    else                        #bit not exit -> set bit
        out=$((1 << $bit_pos))
        out=$(($ori_byte + $out))
    fi
    echo "$out"
}

#make error_stream
function make_error_stream {
    local filename=$1
    local byte_cnt=$2

    if [ $codec == "hevc" ]; then
        # 128KB. 
        start_ofs_ori=131072
    elif [ $codec == "vp9" ]; then
        start_ofs_ori=4096
    else
        start_ofs_ori=4096
    fi
    start_ofs=$start_ofs_ori
    four_MB_ofs=4194304 #4MB
    size=$(stat -c%s "$filename") #get file size
    echo "Size of $filename = $size bytes."
    #get mininum between four_MB_ofs & file size
    end_ofs=$([ $four_MB_ofs -le $size ] && echo "$four_MB_ofs" || echo "$size")

    div=$(($end_ofs-$start_ofs))

    echo "whole div=$div"
    echo "end_ofs=$end_ofs"

    cnt=$(($div / $byte_cnt))
    echo "length div=$div"

    echo "tot_change_byte_cnt=$cnt"

#generating the random value array
    for ((i=0;i<$cnt;i++)); do
        start_ofs=$(($start_ofs_ori + $(($byte_cnt * $i))))
        end_ofs=$(($start_ofs_ori + $(( $(($byte_cnt * $i)) + $byte_cnt )) ))

        ofs[$i]=$(get_random $start_ofs $end_ofs)
        ori_value[$i]=$(get_byte_data $filename $ofs[$i])
        ch_val[$i]=$(get_random 0 255)

        en_random_one_bit=$(get_random 0 1)
        change_one_bit_pos=$(get_random 0 7)

        if [ "$en_random_one_bit" == "1" ]; then
            ch_val[$i]=$(bit_toggle ${ori_value[$i]} $change_one_bit_pos)
        fi

        echo "------------------------"
        #echo "between=$start_ofs ~ $end_ofs"
        #echo "en_random_one_bit=$en_random_one_bit"
        #echo "change_one_bit_pos=$change_one_bit_pos"
        printf "ofs=%d(0x%x) bitchange=%d\n" ${ofs[$i]} ${ofs[$i]} $en_random_one_bit
        printf "val= %d(0x%x) -> %d(0x%x)\n" ${ori_value[$i]} ${ori_value[$i]} ${ch_val[$i]} ${ch_val[$i]}
        echo "------------------------"
    done

#######################################
# copy and paste array
# copy the array log and paste it here
#######################################

#printf for regenerating array
    printf "=============================================================\n"
    printf "make random err array log start\n"
    printf "copy below log data and paste it to the 'copy and paste area'\n"
    printf "=============================================================\n"
    printf "cnt=%d\n" $cnt
    printf "ofs=("
    for ((i=0;i<$cnt;i++)); do
        if [ $(($i % 10))  == 0 ]; then
            printf " %d %d %d %d %d %d %d %d %d %d\n" ${ofs[$i]} ${ofs[(($i+1))]} ${ofs[(($i+2))]} ${ofs[(($i+3))]} ${ofs[(($i+4))]}  ${ofs[(($i+5))]}  ${ofs[(($i+6))]}  ${ofs[(($i+7))]}  ${ofs[(($i+8))]}  ${ofs[(($i+9))]}
        fi
    done
    printf ")\n"
    printf "ch_val=("
    for ((i=0;i<$cnt;i++)); do
        if [ $(($i % 10))  == 0 ]; then
            printf " %d %d %d %d %d %d %d %d %d %d\n" ${ch_val[$i]} ${ch_val[(($i+1))]} ${ch_val[(($i+2))]} ${ch_val[(($i+3))]} ${ch_val[(($i+4))]}  ${ch_val[(($i+5))]}  ${ch_val[(($i+6))]}  ${ch_val[(($i+7))]}  ${ch_val[(($i+8))]}  ${ch_val[(($i+9))]}
        fi
    done
    printf ")\n"
    printf "=============================================================\n"
    printf "make random err array log end\n"
    printf "=============================================================\n"

#change the byte.
    for ((i=0;i<$cnt;i++)); do
        replace_byte $filename ${ofs[$i]} ${ch_val[$i]}
    done
}
#endif /* SUPPORT_CONF_TEST */

function gen_val {
    local min_val=$1
    local max_val=$2
    local default_val=$3
    local val

    if [ -z "$default_val" ] || [ "$default_val" == "-99" ]; then
        # Generate random value
        val=$(get_random $min_val $max_val)
    else
        val=$default_val
    fi
    
    echo "$val"
}

function gen_min_max_val {
    local min_val=$1
    local max_val=$2
    local default_val=$3
    local val

    if [ -z "$default_val" ] || [ "$default_val" == "-99" ]; then
        # Generate random value
        val=$(get_random_min_max $min_val $max_val)
    else
        val=$default_val
    fi

    echo "$val"
}


# get_default_param(is_decoder)
function get_default_param {
    local is_decoder=$1
    local g_product_name=$2

    echo "default_opt=$default_opt"
    if [ "$default_opt" = "0" ]; then
        ##################################################
        # make argv parameter from TestRunnerParamWaveXXXDec.txt
        ##################################################
        g_stream_endian=$(gen_val 16 31 $stream_endian_temp)
        g_frame_endian=$(gen_val 16 31 $frame_endian_temp)

        if [ "$g_product_name" = "wave4" ]; then
            g_source_endian=$(gen_val 16 31 $source_endian_temp)
        fi
        if [ $g_force_bsmode -eq 0 ]; then
            g_bsmode=$(gen_val 0 1 $bsmode_temp)
        fi
        if [ "$codec" = "vp9" ]; then 
            g_bsmode=2;  #only support PIC_END MODE
        fi
        val=0
        g_cbcr_interleave=0
        g_enable_nv21=0
        
        
        if [ $is_decoder -eq 1 ]; then
            if [ $g_bsmode -eq 1 ]; then g_bsmode=2; fi
            val=$(gen_val 0 2 $cbcr_mode_temp)
            case $val in
                0) g_cbcr_interleave=0;;      # separate YUV
                1) g_cbcr_interleave=1        # NV12
                   g_enable_nv21=0;;
                2) g_cbcr_interleave=1        # NV21
                   g_enable_nv21=1;;
            esac
            if [ "$codec" = "mp2" ] || [ "$codec" = "mp4" ]; then
                g_enable_deblock=$(gen_val 0 1 $deblock_temp)
            fi
            if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
                g_enable_dering=$(gen_val 0 1 $dering_temp)
                val=$(gen_val 0 3 $rotAngle_temp)
                if [ "$val" -le "3" ]; then
                    g_rotAngle=$(expr $val \* 90)
                else
                    g_rotAngle=$val
                fi
                g_mirDir=$(gen_val 0 3 $mirDir_temp)
                g_enable_tiled2linear=$(gen_val 0 1 $tiled2linear_temp)
                g_maptype_index=$(gen_val 0 $(($g_maptype_num-1)) $maptype_temp)

                if [ $g_maptype_index -gt 0 ]; then
                    if [ $g_enable_wtl -eq 0 ] && [ $g_enable_tiled2linear -eq 0 ]; then
                        g_enable_wtl=1
                    fi
                else
                    # g_maptyp_index is LINEAR MAP.
                    g_enable_wtl=0
                    g_enable_tiled2linear=0
                fi
            fi
            if [ "$disable_wtl_temp" != -99 ] && [ $disable_wtl_temp ]; then 
                g_enable_wtl=1 
                [ $disable_wtl_temp -eq 0 ] || g_enable_wtl=0
            else 
                g_enable_wtl=$(get_random 0 1) 
            fi
            if [ "$g_product_name" = "coda7q" ]; then
                if [ "${codec}" != "hevc" ] && [ "${codec}" != "avc" ]; then
                    g_enable_wtl=0
                fi
            fi
            if [ "$codec" = "vp9" ]; then 
                # WAVE412 specific
                g_bsmode=2; 
                g_rotAngle=0
                g_mirDir=0
            fi
            if [ "$is_main10" = "" ]; then is_main10=0; fi
            if [ $is_main10 -eq 0 ]; then
                g_wtl_format=0
            fi
            if [ $g_enable_wtl -eq 1 ] && [ $is_main10 -eq 1 ]; then
                if [ "$wtl_format_temp" != -99 ] && [ $wtl_format_temp ]; then 
                    g_wtl_format=$wtl_format_temp; 
                    if [ "$wtl_format_temp" == "-1" ]; then
                        g_enable_wtl=0
                        g_wtl_format=""
                    fi
                else
                    while : ; do
                        g_wtl_format=$(get_random 0 ${#g_yuv_fmt_list[@]}-1)

                        if [ "${g_yuv_fmt_list[${g_wtl_format}]}" != "RESERVED" ]; then
                            break
                        fi
                    done
                fi
            fi
        else
            # ENCODER
            if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ] || [ "$g_product_name" = "coda7q" ]; then
                val=$(gen_val 0 2 $cbcr_mode_temp)
                case $val in
                    0) g_cbcr_interleave=0;;      # separate YUV
                    1) g_cbcr_interleave=1        # NV12
                       g_enable_nv21=0;;
                    2) g_cbcr_interleave=1        # NV21
                       g_enable_nv21=1;;
                esac
                if [ "$g_product_name" != "coda7q" ]; then
                    g_maptype_index=$(gen_val 0 $(($g_maptype_num -1)) $maptype_temp)
                    g_enable_linear2tiled=$(gen_val 0 1 $linear2tiled_temp)
                fi
                g_linebuf_int=$(gen_val 0 3 $linebuf_int_temp)
            else
                g_packedFormat=0
                g_srcFormat3p4b=$(gen_val 1 2 $srcFormat3p4b_temp)
                val=$(gen_val 0 6 $yuv_src_mode_temp)
                if [ "$val" == "1" ];   then #nv12
                    g_cbcr_interleave=1
                    g_enable_nv21=0
                elif [ "$val" == "2" ]; then #nv21
                    g_cbcr_interleave=1
                    g_enable_nv21=1
                elif [ $val -ge 3 ]; then #(packed) 3:YUYV, 4:YVYU, 5:UYVY, 6:VYUY
                    g_packedFormat=$(expr $val - 2)
                fi
            fi
            val=$(gen_val 0 3 $rotAngle_temp)
            if [ "$val" -le "3" ]; then
                g_rotAngle=$(expr $val \* 90)
            else
                g_rotAngle=$val
            fi

            g_mirDir=$(gen_val 0 3 $mirDir_temp)
            if [ "$MODE_COMP_ENCODED_temp" != -99 ] &&  [ $MODE_COMP_ENCODED_temp ];   then MODE_COMP_ENCODED=$MODE_COMP_ENCODED_temp;   else MODE_COMP_ENCODED=1; fi
        fi

    else    
        g_secondary_axi=0
        g_stream_endian=0
        g_source_endian=0
        g_frame_endian=0
        g_bsmode=0
        g_cbcr_interleave=0
        g_enable_nv21=0
        if [ $is_decoder -eq 1 ]; then                
            g_enable_thumbnail=0
            g_enable_wtl=1
            g_wtl_format=0
            # CODA7Q & CODA9 features
            g_enable_deblock=0
            g_enable_dering=0
            g_rotAngle=0
            g_mirDir=0
            g_maptype_index=0
            if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then g_enable_wtl=0; fi
            if [ "$g_product_name" = "coda7q" ]; then
                if [ "${codec}" != "hevc" ] && [ "${codec}" != "avc" ]; then
                    g_enable_wtl=0
                fi
            fi
            # END CODA7Q & CODA9
        else
            g_subframe_sync=0
            g_packedFormat=0
            g_subCtuMap=0
            g_rotAngle=0
            g_mirDir=0
        fi
    fi      
}           

function gen_wtl_format {
    local main10=$1
    local default_val=$2
    local val

    # g_yuv_fmt_list variable should be defined in TestRunnerXXX.sh

    if [ -z $default_val ] || [ "$default_val" == "-99" ]; then
        while : ; do
            val=$(get_random 0 ${#g_yuv_fmt_list[@]}-1)
            if [ "${g_yuv_fmt_list[$val]}" != "RESERVED" ]; then
                break
            fi
        done
    else
        val=$default_val
    fi
    echo "$val"
}

function get_default_param_wave512 {
    if [ "$default_opt" = "0" ]; then
        ##################################################
        # make argv parameter from TestRunnerParamWaveXXXDec.txt
        ##################################################
        g_stream_endian=$(gen_val 16 31 $stream_endian_temp)
        g_frame_endian=$(gen_val 16 31 $frame_endian_temp)

        if [ $g_force_bsmode -eq 0 ]; then
            g_bsmode=$(gen_val 0 1 $bsmode_temp)
        fi
        # NV12 or NV21 
        val=$(gen_val 0 2 $cbcr_mode_temp)
        case $val in
            0) g_cbcr_interleave=0;;      # separate YUV
            1) g_cbcr_interleave=1        # NV12
               g_enable_nv21=0;;
            2) g_cbcr_interleave=1        # NV21
               g_enable_nv21=1;;
        esac

        # WAVE512 has three output H/W modules.
        # 0 - BWB, 1 - AFBCE, 2 - DOWN-SCALER, 3 - DISABLE-WTL
        # BWB, AFBCE AND DOWNSCALER NEED WTL FUNCTION
        g_enable_wtl=1
        g_afbce=0
        if [ "$g_scaler" = "1" ]; then
            # DOWN SCALING TEST
            val=2
        else
            val=$(gen_val 0 3 $g_output_hw_temp)
        fi
        case $val in
        0) # BWB
            if [ "$is_main10" == "1" ]; then
                g_yuv_fmt_list=("${YUV_FORMAT_LIST[@]}")
            else
                g_yuv_fmt_list=("${YUV_FORMAT_LIST_8BIT[@]}")
            fi
            g_wtl_format=$(gen_wtl_format $wtl_format_temp)
            ;;
        1) 
            g_wtl_format=0
            ;;
        2) 
            g_wtl_format=0
            ;;
        3) # FBC
            # FBC IS A COMPRESSED FRAMEBUFFER FORMAT. DISABLE NV12/21
            g_enable_wtl=0
            g_cbcr_interleave=0
            g_enable_nv21=0
            ;;
        esac
    else    
        g_secondary_axi=0
        g_stream_endian=0
        g_frame_endian=0
        g_bsmode=0
        g_cbcr_interleave=0
        g_enable_nv21=0
        g_enable_thumbnail=0
        g_enable_wtl=1
        g_wtl_format=0
    fi      
}

function build_test_param {
    local is_decoder=$1
    local param=""
    local param2=""


    if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
        if [ $g_maptype_index -gt 0 ]; then
            param2="$param2 --maptype=$g_maptype_index"
        fi
    else
        if [ "$g_fbc_mode" == "" ]; then
            pram="$param --fbc-mode=0x0c"
        else
            param="$param --fbc-mode=$g_fbc_mode"
        fi
    fi

    param2="$param2 --stream-endian=$g_stream_endian"
    param2="$param2 --frame-endian=$g_frame_endian"
    if [ $is_decoder -eq 1 ]; then
        param="$param --render 0 -c $g_match_mode"
        param="$param --codec $g_codec_index"

        # CODA7Q & CODA9 features.
        if [ $g_codec_index -eq 3 ]; then
            param="$param --mp4class=$g_mp4class"
        fi

        if [ $g_enable_deblock -eq 1 ]; then
            param="$param --enable-deblock"
        fi
        if [ $g_enable_mvc -eq 1 ]; then
            param="$param --enable-mvc"
        fi
        # END and CODA7Q & CODA9
	
        param="$param $param2"
    
        if [ $g_bsmode -gt 0 ]; then
            param="$param --bsmode=$g_bsmode"
        fi
        if [ $g_enable_thumbnail -eq 1 ]; then
            param="$param --enable-thumbnail"
        fi

        if [ $g_cbcr_interleave -eq 1 ]; then
            param="$param --enable-cbcrinterleave"
            if [ $g_enable_nv21 -eq 1 ]; then
                param="$param --enable-nv21"
            fi
        fi
    
        if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
            if [ $g_enable_dering -eq 1 ]; then
                param="$param --enable-dering"
            fi	
            if [ $g_enable_wtl -eq 0 ] && [ $g_maptype_index -gt 0 ]; then
                param="$param --disable-wtl"
            fi
            if [ $g_enable_tiled2linear -eq 1 ]; then
                param="$param --enable-tiled2linear"
            fi
            if [ $g_rotAngle -gt 0 ]; then
                param="$param --rotate=$g_rotAngle"
            fi
            if [ $g_mirDir -gt 0 ]; then
                param="$param --mirror=$g_mirDir"
            fi
        else
            if [ "$g_product_name" = "coda7q" ]; then
                if [ "${codec}" != "hevc" ] && [ "${codec}" != "avc" ]; then
                    g_enable_wtl=0
                fi
            fi
            if [ $g_enable_wtl -eq 0 ]; then
                    param="${param} --disable-wtl"
            else
                param="${param} --wtl-format=$g_wtl_format"
            fi
        fi

    else
        # ENCODER
        if [ "$MODE_COMP_ENCODED" == "1" ];  then param="${param} -c"; fi
        if [ "$MODE_COMP_RECON" == "1" ];    then param="${param} -r"; fi

        param="$param $param2"
    
        if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ] ||  [ "$g_product_name" = "coda7q" ]; then
            if [ $g_enable_linear2tiled -eq 1 ] && [ "$g_product_name" != "coda7q" ]; then
                param="$param --enable-linear2tiled"
            fi
            if [ $g_bsmode -eq 0 ]; then
                param="$param --enable-ringBuffer"
            else
                if [ $g_linebuf_int -eq 1 ]; then
                    param="$param --enable-lineBufInt"
                fi
            fi
            if [ $g_cbcr_interleave -eq 1 ]; then
                param="$param --enable-cbcrInterleave"
                if [ $g_enable_nv21 -eq 1 ]; then
                    param="$param --enable-nv21"
                fi
            fi
            param="$param --rotate=$g_rotAngle"
            param="$param --mirror=$g_mirDir"
        else
            ### WAVE4xx ###
            param="$param --source-endian=$g_source_endian"
            param="$param --rotAngle=${g_rotAngle} --mirDir=${g_mirDir}"
            if [ $g_bsmode -eq 0 ]; then
                param="${param} --enable-ringBuffer"
            fi
            if  [ $g_packedFormat -ne 0 ]; then
                param="${param} --packedFormat=${g_packedFormat}"
            elif [ "$g_enable_nv21" == "0" ] && [ "$g_cbcr_interleave" == "1" ]; then
                param="${param} --enable-cbcrInterleave --nv21=${g_enable_nv21}"
            elif [ "$g_enable_nv21" == "1" ] && [ "$g_cbcr_interleave" == "1" ]; then
                param="${param} --enable-cbcrInterleave --nv21=${g_enable_nv21}"
            fi
            param="${param} --srcFormat3p4b=${g_srcFormat3p4b}"

        fi
    fi

    g_func_ret_str="$param"
}

function get_random_param {
    local is_decoder=$1
    g_secondary_axi=0
    if [ "$g_product_name" != "coda7q" ]; then
        g_maptype_index=$(get_random 0 $(($g_maptype_num-1)))
    fi
    if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
        g_stream_endian=$(get_random 0 3)
        g_frame_endian=$(get_random 0 3)
    elif [ "$g_product_name" = "wave4" ]; then
        g_stream_endian=$(get_random 16 31)
        g_frame_endian=$(get_random 16 31)
        g_source_endian=$(get_random 16 31)
    else
        g_stream_endian=$(get_random 16 31)
        g_frame_endian=$(get_random 16 31)
    fi


    if [ $is_decoder -eq 1 ]; then
        # TILED2LINEAR 
        g_enable_wtl=0
        g_enable_tiled2linear=0
        if [ $g_maptype_index -gt 0 ]; then 
            local val=$(get_random 0 1)
            if [ $val -eq 0 ]; then
                g_enable_tiled2linear=1
            else
                g_enable_wtl=1
            fi
        else
            g_enable_tiled2linear=0
            g_enable_wtl=0
        fi

        case $(get_random 0 2) in
            0) g_cbcr_interleave=0;;      # separate YUV
            1) g_cbcr_interleave=1        # NV12
               g_enable_nv21=0;;
            2) g_cbcr_interleave=1        # NV21
               g_enable_nv21=1;;
        esac
        if [ "$g_product_name" != "coda960" ] && [ "$g_product_name" != "coda980" ]; then
            g_enable_wtl=1
        fi
        g_wtl_format=0
        if [ "$is_main10" = "" ]; then is_main10=0; fi
        if [ $is_main10 -eq 1 ]; then
            g_wtl_format=$(get_random 0 ${#g_yuv_fmt_list[@]}-1)
            if [ 1 -le $g_wtl_format ] && [ $g_wtl_format -le 4 ]; then
                g_wtl_format=$(($g_wtl_format + 4))
            fi
            if [ "${g_yuv_fmt_list[${g_wtl_format}]}" == "RESERVED" ]; then
                g_wtl_format=5      # 16BIT_LSB_JUSTIFIED
            else
                g_enable_wtl=1
            fi
        fi
        # WAVE4 Bandwidth optimization
        if [[ $g_product_name =~ wave* ]] || [ "$g_product_name" = "coda7q" ]; then
            g_bw_opt=$(get_random 0 1)
            if [ $g_bw_opt -eq 1 ]; then
                g_enable_wtl=1
            fi
        fi
        # CODA7Q & CODA9 features
        if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
            g_rotAngle=$(($(get_random 0 3)*90))
            g_mirDir=$(get_random 0 3)
            g_enable_dering=$(get_random 0 1)
            g_enable_deblock=$(get_random 0 1)
        else
            g_rotAngle=0
            g_mirDir=0
            g_enable_dering=0
            g_enable_deblock=0
        fi

        if [ "$g_product_name" = "coda7q" ]; then
            g_enable_deblock=$(get_random 0 1)
            if [ "${codec}" != "hevc" ] && [ "${codec}" != "avc" ]; then
                g_enable_wtl=0
            fi
        fi
        # check combination of parameters
        if [ $g_enable_wtl -eq 1 ] || [ $g_enable_ppu -eq 0 ]; then
            g_rotAngle=0
            g_mirDir=0
            g_enable_dering=0
        fi

        if [ "${codec}" != "mp2" ] && [ "${codec}" != "mp4" ]; then
            g_enable_deblock=0
            g_enable_dering=0
        fi
        # END CODA7Q & CODA9
        # VP9 constraint(PIC_END Mode)
        if [ "$codec" = "vp9" ]; then 
            g_bsmode=2 
        fi
    else
        # encoder random options
        if [ "$g_produc_name" = "wave4" ]; then
            val=$(get_random 0 6)
            g_cbcr_interleave=0
            g_packedFormat=0
            if [ "$val" == "1" ];   then #frm_format 2
                g_cbcr_interleave=1
                g_enable_nv21=0
            elif [ "$val" == "2" ]; then #frm_format 3
                g_cbcr_interleave=1
                g_enable_nv21=1
            elif [ "$val" -ge "3" ]; then #(packed) 3:YUYV, 4:YVYU, 5:UYVY, 6:VYUY
                g_packedFormat=$(expr $val - 2)
            fi
            g_srcFormat3p4b=$(get_random 0 2)
            MODE_COMP_ENCODED=1
        else
            g_linebuf_int=$(get_random 0 1)
            if [ "$g_produc_name" != "coda7q" ]; then
                g_enable_linear2tiled=$(get_random 0 1)
            fi 
            # cbcr-interleave
            val=$(get_random 0 2)
            case $val in
                0) g_cbcr_interleave=0;;      # separate YUV
                1) g_cbcr_interleave=1        # NV12
                   g_enable_nv21=0;;
                2) g_cbcr_interleave=1        # NV21
                   g_enable_nv21=1;;
            esac
        fi
        g_bsmode=$(get_random 0 1)
        val=$(get_random 0 3 )
        g_rotAngle=$(expr $val \* 90)
        g_mirDir=$(get_random 0 3 )
    fi

    g_func_ret_str="$param"
}

function run_refc_dec_h265 {
    local stream_path="$1"
    local output_path="$2"
    local wtl_mode_index=0
    local skip_option=0
    local max_frames=""
    local color_format=""
    local output_param=""
    local file_ext=${stream_path##*.}
    if [ "$file_ext" = "mp4" ] || [ "$file_ext" = "mkv" ] || [ "$file_ext" = "avi" ]; then
        cp ${bin_dir}/../../../util/bin/Linux/vcp ${bin_dir} || echo ingore_err
        ${bin_dir}/vcp -i $stream_path output.bin
        stream_path="output.bin"
    fi

    if [ $g_enable_thumbnail -eq 1 ]; then
        skip_option=1
    fi

    if [ "$g_product_name" = "wave412" ] || [ "$g_product_name" = "wave512" ] || [ "$g_product_name" = "wave515" ]; then
        if [ $g_wtl_format -eq 0 ]; then 
            wtl_mode_index=0
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 5 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 6 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 7 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 8 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        fi
    else #if [ "$g_product_name" = "wave412" ];
        if [ $g_wtl_format -eq 0 ]; then 
            wtl_mode_index=0
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 5 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 6 ]; then
            wtl_mode_index=5
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 7 ]; then
            wtl_mode_index=2
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 8 ]; then
            wtl_mode_index=6
            output_param="-o $output_path"
        fi
    fi

    if [ $g_match_mode -eq 2 ]; then
        if [ $wtl_mode_index -ne 0 ]; then 
            wtl_mode_index=1
        fi
        str_opt="-i ${stream_path} -c -5 -y ${wtl_mode_index} ${color_format} ${output_param} --skip ${skip_option} ${max_frames} $g_refc_frame_num"
    else
            str_opt="-i ${stream_path} -c -y ${wtl_mode_index} ${color_format} ${output_param} --skip ${skip_option} ${max_frames} $g_refc_frame_num"
    fi


    str_cmd="${bin_dir}/hevc_dec ${str_opt} --sbs 46080"
    echo "${str_cmd}"
    ${str_cmd}

    if [ "$?" != "0" ]; then 
        echo "Failed to ref-c"
        return 1; 
    fi
    return 0
}

function run_refc_dec_vp9 {
    local stream_path="$1"
    local output_path="$2"
    local arg_cci=""
    local wtl_mode_index=0
    local max_frames=""

    if [ "$g_product_name" = "wave412" ] || [ "$g_product_name" = "wave512" ] || [ "$g_product_name" = "wave515" ]; then
        if [ $g_wtl_format -eq 0 ]; then 
            wtl_mode_index=0;
        elif [ $g_wtl_format -eq 1 ]; then 
            wtl_mode_index=0;
        elif [ $g_wtl_format -eq 13 ]; then
            wtl_mode_index=0;
        elif [ $g_wtl_format -eq 18 ]; then
            wtl_mode_index=0;
        elif [ $g_wtl_format -eq 23 ]; then
            wtl_mode_index=0;
        elif [ $g_wtl_format -eq 28 ]; then
            wtl_mode_index=0;
        else 
            wtl_mode_index=5;
        fi
    else #if [ "$g_product_name" = "wave412" ];
        if [ $g_wtl_format -eq 0 ]; then 
            wtl_mode_index=0
            output_param="-o $output_path"

        elif [ $g_wtl_format -eq 5 ]; then
            wtl_mode_index=1
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 6 ]; then
            wtl_mode_index=5
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 7 ]; then
            wtl_mode_index=2
            output_param="-o $output_path"
        elif [ $g_wtl_format -eq 8 ]; then
            wtl_mode_index=6
            output_param="-o $output_path"
    
        fi
    fi

    if [ $g_match_mode -eq 2 ]; then
        if [ $wtl_mode_index -ne 0 ]; then 
            wtl_mode_index=5    # 16BIT MSB
        fi
        param="--input=${stream_path} --outmd5=${output_path} --render 0 --codec $g_codec_index --stream-endian=$g_stream_endian --frame-endian=$g_frame_endian --bsmode=$g_bsmode"
    else
            param="--input=${stream_path} --output=${output_path} --render 0 --codec $g_codec_index --stream-endian=$g_stream_endian --frame-endian=$g_frame_endian --bsmode=$g_bsmode"
    fi


    param="$param $g_frame_num "

    if [ $g_bsmode -gt 0 ]; then
        param="$param --bsmode=$g_bsmode"
    fi
    if [ $g_enable_thumbnail -eq 1 ]; then
        param="$param --enable-thumbnail"
    fi


    if [ $g_enable_wtl -eq 0 ]; then
        param="${param} --disable-wtl"
    else
        if [ "$g_product_name" = "wave412" ] || [ "$g_product_name" = "wave512" ] || [ "$g_product_name" = "wave515" ]; then
            param="${param} --wtl-format=${wtl_mode_index}"
        else
            param="${param} --wtl-format=$g_wtl_format"
        fi
    fi

    if [ $g_cbcr_interleave -eq 1 ]; then
        param="${param} --enable-cbcrinterleave"
        if [ $g_enable_nv21 -eq 1 ]; then
            param="${param} --enable-nv21"
        fi

    fi

    if [ "$g_product_name" = "wave412" ]; then
        str_cmd="${bin_dir}/w412_refc_test ${param}"
    else
        str_cmd="${bin_dir}/w512_refc_test ${param}"
    fi
    echo "${str_cmd}"
    ${str_cmd}

    if [ "$?" != "0" ]; then 
        echo "Failed to ref-c"
        return 1; 
    fi
    return 0
}

# run_refc_dec_7l(stream_path, output_path, codec_name)
function run_refc_dec_7l {
    local stream_path="$1"
    local output_path="$2"
    local codec_name="$3"
    local wtl_mode_index=0
    local skip_option=0
    local output_param="-o ${output_path}"
    local str_opt=""
    local str_cmd=""
    local file_ext=${stream_path##*.}
    local cmodel=""

    if [ "$g_product_name" != "coda7q" ]; then 
        if [ "$file_ext" = "mp4" ] || [ "$file_ext" = "mkv" ] || [ "$file_ext" = "avi" ]; then
            ${bin_dir}/vcp -i $stream_path output.bin
            stream_path="output.bin"
        fi
    else
        if [ "$codec_name" = "mp4" ] && [ "$file_ext" = "avi" ]; then
            cp ${bin_dir}/../../../util/bin/Linux/vcp ${bin_dir} || echo ingore_err
            ${bin_dir}/vcp -i $stream_path output.bin
            stream_path="output.bin"
        fi
    fi

    if [ "$file_ext" = "rmvb" ]; then
        stream_path=${stream_path%.*}.rvf
    fi

    str_opt="-i ${stream_path} -c"

    shopt -s nocasematch
    case "$codec_name" in
        avc) if [ $g_enable_mvc -eq 1 ]; then
                cmodel="mvc_dec"
             else
                cmodel="avc_dec"
             fi
             ;;
        vc1) cmodel="vc1_dec";;
        mp2) cmodel="mp2_dec"
             if [ $g_enable_deblock -eq 1 ]; then
                 str_opt="$str_opt -d"
             fi	
             ;;
        h263) cmodel="mp4_dec";;
        mp4) cmodel="mp4_dec"
             str_opt="$str_opt -s $g_mp4class"
             if [ $g_enable_deblock -eq 1 ]; then
                 str_opt="$str_opt -d"
             fi
             ;;
        avs) cmodel="avs_dec";;
        dv3) cmodel="div_dec";;
        rvx) cmodel="rvx_dec";;
        vp8) cmodel="vp8_dec"
             str_opt="$str_opt --std 2";;
        *)    echo ""; return;;
    esac
    shopt -u nocasematch


    str_cmd="${bin_dir}/${cmodel} ${str_opt} ${output_param}"
    echo "${str_cmd}"
    ${str_cmd}

    if [ "$?" != "0" ]; then 
        echo "Failed to ref-c"
        return 1; 
    fi
    return 0
}

function run_refc_dec_coda9 {
    local stream_path=$1
    local output_yuv=$2
    local ext=${stream_path##*.}

    if [ "$ext" = "rmvb" ]; then
        stream_path=${stream_path%.*}.rvf
    fi

    cmodel_param="-i $stream_path -o $output_yuv"
    ext_param=""
    shopt -s nocasematch
    case "$codec" in
        avc) if [ $g_enable_mvc -eq 1 ]; then
                cmodel="RefMvcDec"
             else
                cmodel="RefAvcDec"
             fi
             ;;
        vc1) cmodel="RefVc1Dec";;
        mp2) cmodel="RefMp2Dec";;
        mp4) cmodel="RefMp4Dec"
             cmodel_param="$cmodel_param -s $g_mp4class";;
        avs) cmodel="RefAvsDec";;
        dv3) cmodel="RefDivDec"
             cmodel_param="$cmodel_param -E host_div3_rtl_cmd.org"
             if [ "$ext" = "ivf" ]; then
                ext_param="-v"
             fi
             ;;
        rvx) cmodel="RefRvxDec";;
        vp8) cmodel="RefVpxDec"
             cmodel_param="$cmodel_param --std 2";;
        tho) cmodel="RefThoDec"
             cmodel_param="$cmodel_param --make_stream --stream_filename temp/host_tho_rtl_cmd.org --stream_endian $g_stream_endian";;
        *)    echo ""; return;;
    esac
    shopt -u nocasematch


    ppu_option=0
    if [ $g_match_mode -eq 1 ]; then
        if [ $g_rotAngle -ne 0 ]; then
            local index=$(($g_rotAngle/90))
            ppu_option=$((0x10 | $index))
        fi
        if [ $g_mirDir -ne 0 ]; then
            ppu_option=$(($ppu_option | 0x10 | ($g_mirDir<<2)))
        fi
        if [ $ppu_option -gt 0 ]; then
            cmodel_param="$cmodel_param -g $ppu_option"
        fi
    fi
    if [ $g_enable_dering -eq 1 ]; then
        cmodel_param="$cmodel_param -p"
    fi
    if [ $g_enable_deblock -eq 1 ]; then
        cmodel_param="$cmodel_param -d"
    fi

    exec_cmd="${bin_dir}/$cmodel $cmodel_param -l $ext_param -c"
    echo "$exec_cmd"

    $exec_cmd

    return $?
}

function run_refc_enc_coda9 {
    local val=0
    local input=$1
    local output=$2
    local postfix=""

    if [ "$g_product_name" = "coda980" ]; then
        postfix="_980_rev"
    fi

    refc_exe="$bin_dir"
    case $codec in
    avc) 
        if [ $g_enable_mvc -eq 0 ]; then
            refc_exe="$refc_exe/RefAvcEnc$postfix"
        else
            refc_exe="$refc_exe/RefMvcEnc$postfix"
        fi
        ;;
    mp4)
        refc_exe="$refc_exe/RefMp4Enc$postfix" ;;
    *)  
        echo "run_refc_enc: Unknown codec $codec"
        return 1;;
    esac

    refc_param="-i $input -o $output -p $yuv_dir/"

    val=$(($g_rotAngle/90))
    val=$(($val | ($g_mirDir<<2)))
    if [ $val -ne 0 ]; then
        refc_param="$refc_param -g $val"
    fi

    refc_param=$(echo $refc_param | sed -e 's/\\/\//g')

    cmd="$refc_exe $refc_param"
    echo $cmd
    echo ""

    $cmd

    return $?
}

function run_refc_enc_coda7 {
    local val=0
    local input=$1
    local output=$2

    refc_exe="$bin_dir"
    case $codec in
    avc)
        refc_exe="$refc_exe/avc_enc" ;;
    mp4)
        refc_exe="$refc_exe/mp4_enc" ;;
    *)  
        echo "run_refc_enc: Unknown codec $codec"
        return 1;;
    esac

    refc_param="-i $input -o $output -p $yuv_dir/"

    val=$(($g_rotAngle/90))
    val=$(($val | ($g_mirDir<<2)))
    if [ $val -ne 0 ]; then
        refc_param="$refc_param -g $val"
    fi

    refc_param=$(echo $refc_param | sed -e 's/\\/\//g')

    cmd="$refc_exe $refc_param"
    echo $cmd
    echo ""

    $cmd

    return $?
}

function generate_golden_data_path {
    local codec=$1
    local streamset=$2
    local streamfile=$3

    codec_dir=""
    shopt -s nocasematch
    case "$codec" in
        vp9)  codec_dir="vp9dec";;
        hevc) codec_dir="hvcdec";;
        avc)  codec_dir="avcdec";;
        vc1)  codec_dir="vc1dec";;
        mp2)  codec_dir="mp2dec";;
        mp4)  codec_dir="mp4dec";;
        h263) codec_dir="mp4dec";;
        avs)  codec_dir="avsdec";;
        dv3)  codec_dir="dv3dec";;
        rvx)  codec_dir="rvxdec";;
        vp8)  codec_dir="vp8dec";;
        tho)  codec_dir="thodec";;
        *)    codec_dir="";;
    esac
    shopt -u nocasematch

    if [ "$is_main10" = "" ]; then is_main10=0; fi
    
    yuvext=""
    if [ "$g_enable_wtl" == "1" ]; then
        case "$codec" in
        hevc | vp9)
            if [ $is_main10 -eq 1 ]; then
                if [ $g_wtl_format -eq 0 ]; then
                    yuvext="_bwb_8"
                fi
            fi
            ;;
        *) 
            ;;
        esac
    elif [ $g_afbce -eq 1 ]; then
        case "$codec" in
        hevc | vp9)
            if [ $is_main10 -eq 1 ]; then
                yuvext="_bwb_8"
            fi
        esac
    fi
    
    # CODA9xx & CODA7x features
    local dering_str=""
    if [ $g_enable_dering -eq 1 ]; then
        dering_str="_dr"
    fi
    local deblk_str=""
    if [ $g_enable_deblock -eq 1 ]; then
        deblk_str="_dbk"
    fi
    # END CODA9xx & CODA7x

    streamset_name=$(basename $streamset)
    filename=$(basename $streamfile)

    local target_base="$yuv_dir/$codec_dir"
    local target_path=""
    thumbnail_ext=""
    if [ $g_enable_thumbnail -eq 1 ]; then
        thumbnail_ext="_thumb"
    fi

    local wtl_format_ext=""
    case $g_wtl_format in
        0) wtl_format_ext="";;
        5) wtl_format_ext="_16bit_msb";;
        6) wtl_format_ext="_16bit_lsb";;
        7) wtl_format_ext="_32bit_msb";;
        8) wtl_format_ext="_32bit_lsb";;
    esac

    local rot_index=0
    if [ $g_rotAngle -eq 0 ] && [ $g_mirDir -eq 0 ]; then
        rot_index=0 
    fi
    if [ $g_rotAngle -eq 180 ] && [ $g_mirDir -eq 3 ]; then
        rot_index=0
    fi
    if [ $g_rotAngle -eq 90 ] && [ $g_mirDir -eq 0 ]; then
        rot_index=1
    fi
    if [ $g_rotAngle -eq 270 ] && [ $g_mirDir -eq 3 ]; then 
        rot_index=1
    fi
    if [ $g_rotAngle -eq 180 ] && [ $g_mirDir -eq 0 ]; then
        rot_index=2
    fi
    if [ $g_rotAngle -eq 0 ] && [ $g_mirDir -eq 3 ]; then 
        rot_index=2
    fi
    if [ $g_rotAngle -eq 270 ] && [ $g_mirDir -eq 0 ]; then
        rot_index=3
    fi
    if [ $g_rotAngle -eq 90 ] && [ $g_mirDir -eq 3 ]; then
        rot_index=3
    fi
    if [ $g_mirDir -eq 1 ] && [ $g_rotAngle -eq 0 ]; then
        rot_index=4
    fi
    if [ $g_rotAngle -eq 180 ] && [ $g_mirDir -eq 2 ]; then 
        rot_index=4
    fi
    if [ $g_mirDir -eq 1 ] && [ $g_rotAngle -eq 90 ]; then
        rot_index=5
    fi
    if [ $g_mirDir -eq 2 ] && [ $g_rotAngle -eq 270 ]; then
        rot_index=5
    fi
    if [ $g_mirDir -eq 1 ] && [ $g_rotAngle -eq 180 ]; then
        rot_index=6
    fi
    if [ $g_mirDir -eq 2 ] && [ $g_rotAngle -eq 0 ]; then
        rot_index=6
    fi
    if [ $g_mirDir -eq 1 ] && [ $g_rotAngle -eq 270 ]; then
        rot_index=7
    fi
    if [ $g_mirDir -eq 2 ] && [ $g_rotAngle -eq 90 ]; then
        rot_index=7
    fi
 
    local target_ext="yuv"

    if [ "$streamset_name" = "" ]; then
        target_path="${target_base}/${filename}${wtl_format_ext}${deblk_str}${thumbnail_ext}${yuvext}_${rot_index}.$target_ext"
    else
        # ex) xxxx_01.cmd -> xxxx.cmd
        temp=`expr match "$streamset_name" '.*\(_[0-9]*\.cmd\)'`
        if [ "$temp" != "" ]; then
            streamset_name="${streamset_name%_[0-9]*\.cmd}.cmd"
        fi
        target_base="${target_base}/${streamset_name}"
        target_path="${target_base}/${filename}${wtl_format_ext}${deblk_str}${dering_str}${thumbnail_ext}${yuvext}_${rot_index}.$target_ext"
    fi

    if [ ! -d $target_base ]; then
        mkdir -p $target_base
    fi

    g_func_ret_str="$target_path"
}

# execute the ref-c program to get the decoded image or md5
# generate_yuv(stream_path, output_yuv_path)
function generate_yuv {
    local src=$1
    local output=$2

    case "$codec" in
        vp9)
            echo "run_refc_dec_vp9 $src $output"
            run_refc_dec_vp9 $src $output;;
        hevc)
            echo "run_refc_dec_h265 $src $output"
            run_refc_dec_h265 $src $output;;
        *)    
            if [ "$g_product_name" = "coda960" ] || [ "$g_product_name" = "coda980" ]; then
                echo "run_refc_dec_coda9 $src $output $codec"
                run_refc_dec_coda9 $src $output $codec
            else
                echo "run_refc_dec_7l $src $output $codec"
                run_refc_dec_7l $src $output $codec
            fi
            ;;
    esac
    if [ $? != 0 ]; then 
        return 1; 
    fi
}


# int get_product_id(test_prog)
# \test_prog    -   test program (ex, w4_dec_test)
# \return       -   It returns product id.
function get_product_id {
    local prog=$1
    local version=""
    local productId=0
    local count=0

    if [ ! -e $prog ] ; then
        echo " $prog doesn't exist"; 
        return 255
    fi

    str=`$prog -v ${fw_path}`
    count=0
    arr=()
    oldIFS=$IFS
    IFS=$'\n'
    for f in ${str}; do
        result=$(echo $f | grep 'VERSION\|REVISION\|PRODUCT_ID')
        echo "result -> $result"
        if [ "$result" != "" ]; then
            f="${f/$'\r'/}"     # remove carriage return
            arr[$count]=$f
            count=$(($count+1))
        fi
    done
    IFS=$oldIFS
    version=$(echo ${arr[0]} | cut -d'=' -f2)
    g_revision=$(echo ${arr[1]} | cut -d'=' -f2)
    productId=$(echo ${arr[2]} | cut -d'=' -f2)
    echo "F/W revision: ${g_revision} PRODUCT_ID: ${productId} "
    return $productId
}


###############################################################################
# ENCODER FUNCTIONS 
###############################################################################
# void c9_parse_cfg(cfg_path, &w, &h, &field_flag)
# desc: parse cfg file to get width, height and field_flag information
function c9_parse_cfg {
    local _cfg_path="$1"
    local _width=$2
    local _height=$3
    local _is_field=$4
    local _name=""
    local _firstchar=""

    shopt -s nocasematch 
    while read line; do
    _firstchar=${line:0:1}
    case "$_firstchar" in
        "#") continue;;     # comment
        ";") continue;;     # comment
        "")  continue;;
    esac
    name=$(echo $line | cut -d' ' -f1)
    if [ "$name" = "PICTURE_WIDTH" ]; then
        eval $_width=$(echo $line | cut -d' ' -f2)
    elif [ "$name" = "PICTURE_HEIGHT" ]; then
        eval $_height=$(echo $line | cut -d' ' -f2)
    elif [ "$name" = "INTERLACED_PIC" ]; then
        eval $_is_field=$(echo $line | cut -d' ' -f2)
    fi
    done < $_cfg_path
    shopt -u nocasematch 
}

# void c7_parse_cfg(cfg_path, &w, &h)
# desc: parse cfg file to get width, height information
function c7_parse_cfg {
    local _cfg_path="$1"
    local _width=$2
    local _height=$3
    local _name=""
    local _firstchar=""

    shopt -s nocasematch 
    while read line; do
    _firstchar=${line:0:1}
    case "$_firstchar" in
        "#") continue;;     # comment
        ";") continue;;     # commen
        "")  continue;;
    esac
    name=$(echo $line | cut -d' ' -f1)
    if [ "$name" = "PICTURE_WIDTH" ]; then
        eval $_width=$(echo $line | cut -d' ' -f2)
    elif [ "$name" = "PICTURE_HEIGHT" ]; then
        eval $_height=$(echo $line | cut -d' ' -f2)
    fi
    done < $_cfg_path
    shopt -u nocasematch 
}


 

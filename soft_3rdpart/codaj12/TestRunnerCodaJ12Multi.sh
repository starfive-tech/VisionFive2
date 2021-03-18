#!/bin/bash

source TestRunnerCommon.sh

# Global varaibles for encoder options

enable_sync=0
wiki_log_file="./log/confluence.log"
instance_num=0

DEFAULT_BS_SIZE=5242880    # 5MB
MIN_BS_SIZE=8192           # 8KB

simulation="false"

################################################################################
# Initialize parameter variables                                               #
################################################################################

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION --input=cfg1,cfg2,... --instance-num=n"
    echo "-h                        help"
    echo "--enable-random           It generates random parameters"
    echo "--bin-dir                 ref-c directory, default: ../../../design/ref_c/bin/Linux"
    echo "--cfg-dir                 base directory of encoder configuration files. default: ../../../fpga/board/jpgcfg_v20"
    echo "--yuv-dir                 base directory storing YUV. default: ./yuv"
    echo "--enable-tiledMode        enable tiled mode"
    echo "--stream-endian           ENDIAN  bitstream endianness. refer to datasheet Chapter 4."
    echo "--frame-endian            ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4."
    echo "--slice-height            the vertical height of a slice. multiple of 16 for 420, multiple of 8 for others"
    echo "--enable-slice-intr       enable get the interrupt at every slice encoded"
    echo "--pixelj                  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness"
    echo "--bs-size                 bitstream buffer size in byte"
    echo "--instance-num            The number of instances"
}

function generate_parameters {
    local input_file=$1
    local is_encoder=$2
    local ext="${input_file##%.}"

    if [ "$is_encoder" == "true" ]; then
        read_cfg $input_file
    else
        get_jpeg_info $input_file $jpeg_12bit
    fi

    if [ "$g_random_test" == "1" ] || [ "$txt_param_switch" == "1" ]; then
        g_stream_endian=$(gen_val 0 3 $stream_endian_temp)
        if [ "$jpeg_12bit" == "1" ]; then
            g_frame_endian=$(gen_val 0 7 $frame_endian_temp)
        else
            g_frame_endian=$(gen_val 0 3 $frame_endian_temp)
        fi
        slice_random=$(gen_val 0 1) # 0: disable slice done, 1: enable slice_done
        if [ "$slice_random" == "1" ] || [ "$slice_height_temp" != "0" ]; then
            if [ "$is_encoder" == "true" ]; then
                g_slice_height=$(gen_valid_slice_height $cfg_width $cfg_height $cfg_img_format $slice_height_temp)
            else
                pic_height_in_8=$(($jpg_height/8))
                if [ -z $slice_height_temp ]; then
                    g_slice_height=$(gen_val 0 $pic_height_in_8 $slice_height_temp)
                    g_slice_height=$(($g_slice_height * 8))
                else
                    g_slice_height=$slice_height_temp
                fi
            fi
            if [ "$g_slice_height" != "0" ]; then
                g_enable_slice_intr=$(gen_val 0 1 $g_enable_slice_intr_temp)
            else
                g_enable_slice_intr=0
            fi
        else
            g_slice_height=0
            g_enable_slice_intr=0
        fi

        g_pixel_just=$(gen_val 0 1 $pixel_just_temp)
        g_bs_size=$(gen_val $MIN_BS_SIZE $DEFAULT_BS_SIZE $bs_size_temp)
        g_bs_size=$((($g_bs_size + 4095) & ~4095))
        g_tiled_mode=$(gen_val 0 0 $g_tiled_mode_temp)
    else
        g_stream_endian=$(gen_val 0 0 $stream_endian_temp)
        g_frame_endian=$(gen_val 0 0 $frame_endian_temp)
        if [ "$is_encoder" == "true" ]; then
            g_slice_height=$(gen_valid_slice_height $cfg_width $cfg_height $cfg_img_format $slice_height_temp)
        else
            pic_height_in_8=$(($jpg_height/8))
            if [ -z $slice_height_temp ]; then
                g_slice_height=$(gen_val 0 $pic_height_in_8 $slice_height_temp)
                g_slice_height=$(($g_slice_height * 8))
            else
                g_slice_height=$slice_height_temp
            fi
        fi
        g_enable_slice_intr=$(gen_val 0 0 $g_enable_slice_intr_temp)
        g_pixel_just=$(gen_val 0 0 $pixel_just_temp)
        g_bs_size=$(gen_val $DEFAULT_BS_SIZE $DEFAULT_BS_SIZE $bs_size_temp)
        g_bs_size=$((($g_bs_size + 4095) & ~4095))
        g_tiled_mode=$(gen_val 0 0 $g_tiled_mode_temp)
    fi
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o e:hw: -l input:"
OPTSTRING="${OPTSTRING},enable-random,bin-dir:,cfg-dir:,yuv-dir:"
OPTSTRING="${OPTSTRING},instance-num:,12bit:"
OPTSTRING="${OPTSTRING},stream-endian:,frame-endian:,output:"
OPTSTRING="${OPTSTRING},slice-height:,enable-slice-intr:,pixelj:,bs-size:,enable-tiledMode:"

OPTS=`getopt $OPTSTRING -- "$@"`

if [ $? -ne 0 ]; then
    exit 1
fi

eval set -- "$OPTS"

while true; do
    case "$1" in
        -h)
            help
            exit 1
            shift;;
        -e)
            in_comp_param="$2"
            shift 2;;
        --bin-dir)
            bin_dir_user=$2
            shift 2;;
        --enable-random)
            g_random_test=1
            shift;;
        --cfg-dir)
            cfg_dir_user=$2
            shift 2;;
        --input)
            input_param=$2
            shift 2;;
        --yuv-dir)
            yuv_dir_user=$2
            shift 2;;
            #shift 2;;
        --instance-num)
            instance_num="$2"
            shift 2;;
        --12bit)
            jpeg_12bit_multi=$2
            shift 2;;
        --enable-tiledMode)
            g_tiled_mode_multi=$2
            shift 2;;
        --stream-endian)
            stream_endian_multi="$2"
            shift 2;;
        --frame-endian)
            frame_endian_multi="$2"
            shift 2;;
        --slice-height)
            slice_height_multi="$2"
            shift 2;;
        --enable-slice-intr)
            g_enable_slice_intr_multi="$2"
            shift 2;;
        --pixelj)
            pixel_just_multi="$2"
            shift 2;;
        --bs-size)
            bs_size_multi="$2"
            shift 2;;
        --output)
            output_multi="$2"
            shift 2;;
        --)
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))

yuv_dir="./yuv"
#bin_dir="../../../design/ref_c/bin/Linux"
bin_dir="./"
#cfg_dir="../../../fpga/board/jpgcfg_v20"
cfg_dir="./cfg"
if [ "$simulation" == "true" ]; then
    ACLK_MIN=160
    ACLK_MAX=300
    CCLK_MIN=160
    CCLK_MAX=300
    DEFAULT_ACLK=200
    DEFAULT_CCLK=200
    test_exec="./runit"
    g_random_test=0
else
    DEFAULT_ACLK=20
    DEFAULT_CCLK=20
    test_exec="./multi_instance_test"
fi
if [ -z $aclk_freq_temp ]; then
    g_aclk=$DEFAULT_ACLK
else
    g_aclk=$aclk_freq_temp
fi
if [ -z $cclk_freq_temp ]; then
    g_cclk=$DEFAULT_CCLK
else
    g_cclk=$cclk_freq_temp
fi
if [ ! -z $yuv_dir_user ]; then
    yuv_dir="$yuv_dir_user"
fi
if [ ! -z $cfg_dir_user ]; then
    cfg_dir="$cfg_dir_user"
fi
if [ ! -z $bin_dir_user ]; then
    bin_dir="$bin_dir_user"
fi
tiled_exec="./tiled_mode"

if [ ! -f ./$test_exec ] ; then
    echo " $test_exec file doesn't exist";
    exit 1
fi

################################################################################
# make log info
################################################################################
mkdir -p temp
mkdir -p output
mkdir -p log/multi_conformance
if [ $simulation == "true" ]; then
    conf_log_path="simv.log"
    conf_err_log_path="simv_error.log"
else
    conf_log_path="log/multi_conformance/${streamset_file}.log"
    conf_err_log_path="log/multi_conformance/${streamset_file}_error.log"
fi
temp_log_path="./temp/temp.log"
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

output=()
golden=()

################################################################################
# Add test param                                                               #
# Options of the each instance are seperated by ","                            #
################################################################################
test_param=""
comp_param=""
for ((i=0;i<$instance_num;i++)); do
    #######################
    # separate parameters #
    #######################
    g_tiled_mode_temp=0
    jpeg_12bit=0
    if [ -n "$input_param" ]; then
        filename=$(echo ${input_param} | cut -d',' -f$(($i+1)))
        ext=${filename##*.}
        ext=$(tr '[:upper:]' '[:lower:]' <<< $ext)
        if [ "$ext" == "cfg" ]; then
            is_encoder="true"
            input_path=${cfg_dir}/${filename}
            refc_exec="Nieuport_JpgEnc"
            output[$i]="instance_${i}_output.jpg"
            golden[$i]="output_refc_${i}.jpg"
        else
            is_encoder="false"
            input_path="$filename"
            refc_exec="Nieuport_JpgDec"
            output[$i]="instance_${i}_output.yuv"
            golden[$i]="output_refc_${i}.yuv"
        fi
        rm -f ${output[$i]}
        rm -f ${golden[$i]}
        stream[$i]=$filename

        if [ "$comp_param" != "" ]; then
            comp_param="${comp_param},"
        fi
        if [ "$is_encoder" == "true" ]; then
            comp_param="${comp_param}0"
        else
            comp_param="${comp_param}1"
        fi
    fi
    if [ -n "$output_multi" ]; then
        output[$i]=$(echo ${output_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$stream_endian_multi" ]; then
        stream_endian_temp=$(echo ${stream_endian_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$frame_endian_multi" ]; then
        frame_endian_temp=$(echo ${frame_endian_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$slice_height_multi" ]; then
        slice_height_temp=$(echo ${slice_height_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$g_enable_slice_intr_multi" ]; then
        g_enable_slice_intr_temp=$(echo ${g_enable_slice_intr_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$pixel_just_multi" ]; then
        pixel_just_temp=$(echo ${pixel_just_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$bs_size_multi" ]; then
        bs_size_temp=$(echo ${bs_size_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$g_tiled_mode_multi" ]; then
        g_tiled_mode_temp=$(echo ${g_tiled_mode_multi} | cut -d',' -f$(($i+1)))
    fi
    if [ -n "$jpeg_12bit_multi" ]; then
        jpeg_12bit=$(echo ${jpeg_12bit_multi} | cut -d',' -f$(($i+1)))
    fi

    ################################################################################
    # read from cfg file name                                                      #
    ################################################################################
    if [[ $filename == *"12b"* ]]; then
        echo "set jpeg_12bit from cfg file name"
        jpeg_12bit=1
    fi
    if [[ $filename == *"tiled"* ]]; then
        echo "set g_tiled_mode from cfg file name"
        g_tiled_mode_temp=1
    fi

    ################################################################################
    # make argc & argv parameter                                                   #
    ################################################################################
    generate_parameters $input_path $is_encoder

    if [ "$is_encoder" == "true" ]; then
        ################################################################################
        # Create Input YUV data for tiledMode                                          #
        ################################################################################
        if [ "$g_tiled_mode" == "1" ]; then
           generate_input_yuv ${input_path}
           result=$?

           if [ "$result" != "0" ]; then
               log_conf "failed to create yuv file $input_path" $conf_log_path
               log_conf "[RESULT] FAILURE" $conf_log_path
               echo "failed to create yuv file $input_path" >> $conf_err_log_path
               echo "[RESULT] FAILURE" >> $conf_err_log_path
               continue
           fi
        fi
    fi

    ################################################################################
    # Golden Data                                                                  #
    ################################################################################
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    if [ "$is_encoder" == "true" ]; then
        generate_golden_stream $filename ${golden[$i]}
    else
        if [ "$jpeg_12bit" == "1" ]; then
            g_jpeg_12bit="true"
        else
            g_jpeg_12bit="false"
        fi
        generate_golden_yuv $input_path ${golden[$i]}
    fi
    result=$?
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    if [ $result -ne 0 ]; then
        exit $result
    fi

    ################################################################################
    # make test param
    ################################################################################
    is_encoder[$i]=0 #encoder
    if [ "$i" = "0" ]; then
        codec_std_test_param="${is_encoder[$i]}"
        stream_test_param="${filename}"
        if [ -n ${output[$i]} ]; then
            output_test_param="${output[$i]}"
        fi
        stream_endian_test_param="${g_stream_endian}"
        frame_endian_test_param="${g_frame_endian}"
        bs_size_test_param="${g_bs_size}"
        slice_height_test_param="${g_slice_height}"
        slice_intr_test_param="${g_enable_slice_intr}"
        tiled_mode_test_param="${g_tiled_mode}"
        jpeg_12bit_test_param="${jpeg_12bit}"
        pixelj_test_param="${g_pixel_just}"
    else
        codec_std_test_param="${codec_std_test_param},${is_encoder[$i]}"
        stream_test_param="${stream_test_param},${filename}"
        if [ -n ${output[$i]} ]; then
            output_test_param="${output_test_param},${output[$i]}"
        fi
        stream_endian_test_param="${stream_endian_test_param},${g_stream_endian}"
        frame_endian_test_param="${frame_endian_test_param},${g_frame_endian}"
        bs_size_test_param="${bs_size_test_param},${g_bs_size}"
        slice_height_test_param="${slice_height_test_param},${g_slice_height}"
        slice_intr_test_param="${slice_intr_test_param},${g_enable_slice_intr}"
        tiled_mode_test_param="${tiled_mode_test_param},${g_tiled_mode}"
        jpeg_12bit_test_param="${jpeg_12bit_test_param},${jpeg_12bit}"
        pixelj_test_param="${pixelj_test_param},${g_pixel_just}"
    fi

################################################################################
# print information                                                            #
################################################################################
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "RANDOM TEST    : ${ON_OFF[$g_random_test]}" $temp_log_path
    log_conf "INSTANCE NUM   : $i" $temp_log_path
    log_conf "CFG            : $filename " $temp_log_path
    log_conf "ENDIAN         : STREAM($g_stream_endian) FRAME($g_frame_endian)" $temp_log_path
    log_conf "STANDARD       : JPEG" $temp_log_path
    log_conf "CBCR interleave: ${ON_OFF[$g_cbcr_interleave]}" $temp_log_path
    log_conf "PIXEL JUST.    : ${JUSTIFY_NAME[$g_pixel_just]}" $temp_log_path
    log_conf "TILED MODE     : ${ON_OFF[$g_tiled_mode]}" $temp_log_path
    log_conf "WIDTH          : $cfg_width" $temp_log_path
    log_conf "HEIGHT         : $cfg_height" $temp_log_path
    log_conf "SLICE_HEIGHT   : $g_slice_height" $temp_log_path
    log_conf "EN_SLICE_INTR  : $g_enable_slice_intr" $temp_log_path
    log_conf "BS SIZE        : $g_bs_size" $temp_log_path
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
done

if [ "$in_comp_param" != "" ]; then
    comp_param="$in_comp_param"
fi

test_param="-e ${comp_param} $g_w_param $g_fsdb_param $g_ius_param"
test_param="${test_param} --instance-num=${instance_num} --cfg-dir=$cfg_dir --yuv-dir=$yuv_dir"
test_param="${test_param} --stream-endian=${stream_endian_test_param}"
test_param="${test_param} --frame-endian=${frame_endian_test_param} --bs-size=${bs_size_test_param}"
test_param="${test_param} --pixelj=${pixelj_test_param}"
test_param="${test_param} --slice-height=${slice_height_test_param}"
test_param="${test_param} --enable-slice-intr=${slice_intr_test_param}"
test_param="${test_param} --enable-tiledMode=${tiled_mode_test_param}"
test_param="${test_param} --input=${stream_test_param} --output=${output_test_param}"
if [ "$simulation" == "true" ]; then
    test_param="${test_param} --TestRunner=1"
fi

success_count=0
failure_count=0
################################################################################
################################################################################
# Test Mode Thread                                                             #
################################################################################
################################################################################
result=0
log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
log_conf "MultiInstance test : Thread" ${temp_log_path}
log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
for ((i=0;i<$instance_num;i++)); do
    log_conf "[${i}/${instance_num}] ${stream[$i]}" ${temp_log_path}
    if [ -e ${output[$i]} ]; then
        rm -rf ${output[$i]}
    fi
done
log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
cat $temp_log_path >> $conf_log_path

echo "$test_exec $test_param"

if [ "$simulation" == "true" ]; then
    $test_exec $test_param 2>&1 | tee -a $conf_log_path
else
    echo "$test_exec $test_param"
	exit #samin
    #$test_exec $test_param
fi

if [ $result -eq 0 ]; then
    log_conf "---------------- COMPARE ------------------" $conf_log_path
    for ((i=0;i<$instance_num;i++)); do
        log_conf "cmp ${output[$i]} ${golden[$i]}"  $conf_log_path
        cmp ${output[$i]} ${golden[$i]} 2>&1 | tee -a $conf_log_path > /dev/null #save the log
        cmp ${output[$i]} ${golden[$i]} #get the result & show log
        if [ "$?" != "0" ]; then
            log_conf "[RESULT] COMPARE FAIL" $conf_log_path
            result=1
        fi
    done
    if [ $result -eq 0 ]; then
        log_conf "[RESULT] SUCCESS" $conf_log_path
        success_count=$(($success_count + 1))
    fi
fi
if [ $result -eq 1 ]; then
    log_conf "[RESULT] FAILURE" $conf_log_path
    failure_count=$(($failure_count + 1))
    cat $temp_log_path >> $conf_err_log_path
    cat ./ErrorLog.txt >> $conf_err_log_path
    echo "[RESULT] FAILURE" >> $conf_err_log_path
    if [ $result -eq 10 ]; then
        echo "Abnormal exit!!!"
        break
    fi
fi

echo "TEST_CONDITION: $0 $test_param"

# clear temp log
echo "" > $temp_log_path



################################################################################
# Create Test Log                                                              #
################################################################################
endTime=$(date +%s%N)
elapsed=$((($endTime - $beginTime) / 1000000000))
elapsedH=$(($elapsed / 3600))
elapsedS=$(($elapsed % 60))
elapsedM=$(((($elapsed - $elapsedS) / 60) % 60))
if [ "$((elapsedS / 10))" == "0" ]; then elapsedS="0${elapsedS}" ;fi
if [ "$((elapsedM / 10))" == "0" ]; then elapsedM="0${elapsedM}" ;fi
if [ "$((elapsedH / 10))" == "0" ]; then elapsedH="0${elapsedH}" ;fi

if [ $elapsed -le 60 ]; then
    time_hms="{color:red}*${elapsedH}:${elapsedM}:${elapsedS}*{color}"
else
time_hms="${elapsedH}:${elapsedM}:${elapsedS}"
fi

log_filename=$(basename $conf_log_path)
log_err_filename=$(basename $conf_err_log_path)
if [ "$failure_count" == "0" ] && [ "$instance_num" != "0" ]; then
    pass=${PASS}
    rm $conf_err_log_path
    log_err_filename=""
else
    pass=${FAIL}
fi

wiki_log="| $streamset_file | 1 | $success_count | $failure_count | 0 | $log_filename | ${log_err_filename} | $pass | $time_hms | | | Inst Num = $instance_num |"

echo $wiki_log
echo $wiki_log >> $wiki_log_file

if [ "$instance_num" == "0" ]; then
    echo "instance_num: $instance_num = exit 1"
    exit 1
fi

exit $failure_count


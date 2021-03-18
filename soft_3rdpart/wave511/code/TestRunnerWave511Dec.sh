#!/bin/bash

source common.sh

g_product_name="wave511"
# Global varaibles for decoder option
is_main10=0
build_yuv=0
fail_stop=0
fake_test=0
yuv_dir="./yuv"
stream_dir="./stream"
test_exec="./w5_dec_test"
bin_dir="."
wiki_log_file="./log/dec_confluence.log"
enable_random=0;
md5_path=""
codec=vp9
jenkins=""
txt_param_switch=0

simulation="false"

ACLK_MIN=13
BCLK_MIN=13
CCLK_MIN=13

ACLK_MAX=33
BCLK_MAX=33
CCLK_MAX=33

RET_SUCCESS=0
RET_HANGUP=2

function build_golden_yuvs {
    local src=$1
    local wtl_fmt_indexes=(0)

    if [ $is_main10 -eq 1 ]; then
        wtl_fmt_indexes=(0 5 6 7 8)
    fi

    for i in ${wtl_fmt_indexes[@]}; do
        g_wtl_format=$i
        echo ""
        echo "Generating ${g_yuv_fmt_list[$g_wtl_format]}..."
        generate_golden_data_path $codec $streamset_path $src 
        golden_yuv_path="$g_func_ret_str"
        generate_yuv $stream $golden_yuv_path
    done
}

rotAngle_temp=0
mirDir_temp=0

################################################################################
# scaler test                                                                  #
################################################################################
function calc_scale_tot {
    local min_x=0
    local min_y=0

    g_oriw=$ori_width
    g_orih=$ori_height
    g_ori_align_x=$ori_width 
    g_ori_align_y=$ori_height

    min_x=$(($g_ori_align_x / 8))
    min_x=$(ceiling $min_x 8)
    min_y=$(($g_ori_align_y / 8))
    min_y=$(ceiling $min_y 8)

    num_x_steps=$(($g_ori_align_x - $min_x))
    num_x_steps=$(($num_x_steps/2 + 1))
    num_y_steps=$(($g_ori_align_y - $min_y))
    num_y_steps=$(($num_y_steps/2 + 1))

    echo "orign_x = $g_ori_align_x, orign_y = $g_ori_align_y"
    echo "min_x = $min_x, min_y = $min_y"
    echo "num_x_steps = $num_x_steps num_y_steps = $num_y_steps"

    tot_scaler_cnt=$(($num_x_steps * $num_y_steps))
    g_min_scale_w=$min_x
    g_min_scale_h=$min_y

    echo "calculated_tot_scaler_cnt=$tot_scaler_cnt"
}

function calc_scale_val {
    local x=0
    local y=0
    local min_x=$g_min_scale_w
    local min_y=$g_min_scale_h
    
    echo "scaler_step_cnt_2: $scaler_step_cnt_2"
    echo "min_x: $min_x"
    echo "min_y: $min_y"
    if [ $scaler_test_mode -eq 1 ]; then
        if [ $num_x_steps -eq 0 ]; then
            g_sclw=$ori_width
        else
            x=$(($scaler_step_cnt_2 % $num_x_steps))
            g_sclw=$(($g_ori_align_x - $x * 2))
        fi
        if [ $num_y_steps -eq 0 ]; then
            g_sclh=$ori_height
        else
            y=$(($scaler_step_cnt_2 % $num_y_steps))
            g_sclh=$(($g_ori_align_y - $y * 2))
        fi
    elif [ $scaler_test_mode -eq 2 ]; then
        if [ $num_x_steps -eq 0 ]; then
            g_sclw=$ori_width
        else
            x=$(($scaler_step_cnt_2 % $num_x_steps))
            g_sclw=$(($min_x + $x * 2))
        fi
        if [ $num_y_steps -eq 0 ]; then
            g_sclh=$ori_height
        else
            y=$(($scaler_step_cnt_2 % $num_y_steps))
            g_sclh=$(($min_y + $y * 2))
        fi
    elif [ $scaler_test_mode -eq 3 ]; then
        g_sclw=$(get_random $min_x  $g_ori_align_x)
        g_sclh=$(get_random $min_y  $g_ori_align_y)
    elif [ $scaler_test_mode -eq 4 ]; then
        g_sclw=$g_ori_align_x
        g_sclh=$(get_random $min_y  $g_ori_align_y)
    elif [ $scaler_test_mode -eq 5 ]; then
        g_sclw=$min_x
        g_sclh=$(get_random $min_y  $g_ori_align_y)
    elif [ $scaler_test_mode -eq 6 ]; then
        g_sclw=$(get_random $min_x  $g_ori_align_x)
        g_sclh=$g_ori_align_y
    elif [ $scaler_test_mode -eq 7 ]; then
        g_sclw=$(get_random $min_x  $g_ori_align_x)
        g_sclh=$min_y
    fi
    g_sclw=$(ceiling $g_sclw 2)
    g_sclh=$(ceiling $g_sclh 2)
    echo "x=$x y=$y"
    echo "g_sclw=$g_sclw, g_sclh=$g_sclh"
}

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h                    help"
    echo "-c                    codec. default: hevc"
    echo "                      valid codec list: avc, hevc"
    echo "--bsmode              [optional] interrupt or picend, default: interrupt mode"
    echo "--bin-dir             ref-c directory"
    echo "--enable-random       generate random option."
    echo "--enable-thumbnail    enable thumbnail mode, default: off"
    echo "--main10              true or false. Just use in conformance env."
    echo "--stream-dir          stream directory"
    echo "--yuv-dir             Golden yuv directory"
    echo "--fail-stop           Stop testing if current test is failure"
    echo "--build-yuv           Just generate golden YUV with Ref-C."
    echo "--fbc-mode            Compressed mode: 0 ~ 0x3e"
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o hwc:n:f -l bsmode:,enable-random,stream-dir:,bin-dir:,match-mode:,test_bin_name:"
OPTSTRING="${OPTSTRING},enable-thumbnail,main10,build-yuv,yuv-dir:,fbc-mode:,md5-dir:"
OPTSTRING="${OPTSTRING},scaler-test-mode:,scaler-step-cnt:,ori-width:,enable-scaler,ori-height:"

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
        -n) 
            g_frame_num="-n $2"
            g_refc_frame_num="-m $2"
            shift 2;;
        -c)
            codec=$(echo $2 | tr '[:upper:]' '[:lower:]')
            shift 2;;
        -f) 
            g_fpga_reset=1
            shift;;
        --bsmode)
            g_bsmode=$2
            g_bsmode_force=$2
            shift 2;;
        --bin-dir)
            bin_dir=$2
            shift 2;;
        --enable-random)
            enable_random=1
            shift;;
        --enable-thumbnail)
            g_enable_thumbnail=1
            shift;;
        --md5-dir)
            md5_dir=$2
            shift 2;;
        --match-mode)
            g_match_mode=$2
            shift 2;;
        --stream-dir)
            stream_dir=$2
            shift 2;;
        --main10)
            is_main10=1
            shift;;
        --build-yuv)
            build_yuv=1
            shift;;
        --yuv-dir)
            yuv_dir=$2
            shift 2;;
        --fail-stop)
            fail_stop=1
            shift;;
        --fbc-mode)
            g_fbc_mode_temp=$2
            g_force_fbc_mode=1
            shift 2;;
        --enable-scaler)
            g_scaler="true"
            shift;;
        --scaler-test-mode)
            g_scaler="true"
            scaler_test_mode=$2
            shift 2;;
        --scaler-step-cnt)
            g_scaler="true"
            scaler_step_cnt=$2
            shift 2;;
        --ori-width)
            ori_width=$2
            shift 2;;
        --ori-height)
            ori_height=$2
            shift 2;;
        --test_bin_name)
            test_exec="$2"
            shift 2;;
        --) 
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))

case "${codec}" in
    "hevc")  g_codec_index=12;;
    "avc")   g_codec_index=0;;
   *)
        echo "unsupported codec: ${codec}"
        help
        exit 1
        ;;
esac


################################################################################
# Get param from target text file                                              #
################################################################################
name_value=()
input_param_name=TestRunnerParamWave511Dec.txt
if [ -f $input_param_name ] && [ $enable_random -eq 0 ]; then
    while read line; do
        # remove right comment
        line="${line%%\#*}"

        attr=$(echo $line | cut -d'=' -f1)
        attr=$(echo $attr | tr -d ' ')

        if [ "$attr" == "debug" ]; then
            value=$(echo $line | sed 's/debug=//')
        else
            value=$(echo $line | cut -d '=' -f2)
        fi

        case "$attr" in
            default)              default_opt="$value";;
            n)                    g_frame_num="-n $value"
                                  g_refc_frame_num="-m $value";;
            secondary-axi)        secondary_axi_temp="$value";;
            stream-endian)        stream_endian_temp="$value";;
            frame-endian)         frame_endian_temp="$value";;                  
            bsmode)               bsmode_temp="$value";;
            match-mode)           g_match_mode="$value";;
            cbcr_interleave_mode) g_yuv_mode_temp="$value";;
            cbcr)                 g_yuv_mode_temp="$value";;
            disable_wtl)          disable_wtl_temp="$value";;
            wtl-format)           wtl_format_temp="$value";;
            enable-thumbnail)     g_enable_thumbnail="$value";;
            fpga-reset)           g_fpga_reset="$value";;
            fbc-mode)             g_fbc_mode_temp="$value" 
                                  g_force_fbc_mode=1;;
            txt-param)            txt_param_switch=1;;
            output_hw)            g_output_hw_temp="$value";;
            bwopt)                g_bw_opt_temp="$value";;
            *) ;;
        esac
    done < $input_param_name
else
    if [ ! -f $input_param_name ]; then
        echo "$input_param_name file doesn't exist"; 
    fi
fi

streamset_path=$1
streamset_file=`basename ${streamset_path}`

if [ ! -e "${streamset_path}" ]; then
    echo "No such file: ${streamset_path}"
    exit 1
fi

if [ "$wtl_format_temp" != "-99" ] && [ -z "$wtl_format_temp" ]; then
    g_enable_wtl=1
    g_wtl_format="$wtl_format_temp"
fi

################################################################################
# scaler test                                                                  #
################################################################################
if [ ! -z "$scaler_test_mode" ]; then
    g_enable_wtl=1
    g_match_mode=1
    calc_scale_tot
    scaler_step_cnt_2=1
    temp_path=scaler.cmd
    if [ $scaler_step_cnt ] && [ "$scaler_step_cnt" != "ALL" ] ; then
        tot_scaler_cnt=$scaler_step_cnt
    fi
    max=$(($tot_scaler_cnt + 1))
    rm -rf $temp_path
    for (( c=1; c< $max ; c++ )) do
        echo `cat ${streamset_path}` >> ${temp_path}
    done
    streamset_path=$temp_path
fi

################################################################################
# count stream number                                                          #
################################################################################
stream_file_array=()
parse_streamset_file $streamset_path stream_file_array

num_of_streams=${#stream_file_array[@]}

#echo ${stream_file_array[@]}

count=1
success_count=0
failure_count=0
remain_count=${num_of_streams}
basedir=${basedir//\/}  # remove last delimeter

mkdir -p temp
mkdir -p output
mkdir -p log/decoder_conformance
conf_log_path="log/decoder_conformance/${streamset_file}_r${g_revision}.log"
conf_err_log_path="log/decoder_conformance/${streamset_file}_error_r${g_revision}.log"
if [ "$g_scaler" == "true" ]; then
conf_log_path="log/decoder_conformance/${stream_set_name}_r${g_revision}.log"
conf_err_log_path="log/decoder_conformance/${stream_set_name}_error_r${g_revision}.log"
fi
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

echo ""
echo ""
echo "##### PRESS 'q' for stop #####"
echo ""
echo ""

log_conf "STREAM BASE DIR: ${stream_dir}" ${conf_log_path}

################################################################################
# read cmd file.                                                               #
################################################################################
force_reset=0
for (( c=0; c< $num_of_streams ; c++ )) do
    line=${stream_file_array[$c]}
    # press 'q' for stop
    read -t 1 -n 1 key
    if [[ $key == q ]]; then
        break;
    fi

    test_param=""
    stream="${stream_dir}/${line}"
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_streams}] ${stream}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    if [ ! -f $stream ]; then
        log_conf "Not found $stream" $temp_log_path
        log_conf "[RESULT] FAILURE" $temp_log_path
        failure_count=$(($failure_count + 1))
        remain_count=$(($remain_count - 1))
        count=$(($count + 1))
        continue
    fi

    result=0
    target_stream=$stream
################################################################################
# scaler test                                                                  #
################################################################################
    if [ ! -z "$scaler_test_mode" ]; then
        g_match_mode=1 #0 : 1:1(same size)
        calc_scale_val
        echo "scaler_step_cnt_2 = $scaler_step_cnt_2, tot_scaler_cnt = $scaler_step_cnt"
        g_frame_num="-n 2"
		g_refc_frame_num="-m 2" #for HEVC ref-c
        if [ $scaler_step_cnt_2 -gt $tot_scaler_cnt ]; then
            echo "same value"
            break;
        else
            scaler_step_cnt_2=$(($scaler_step_cnt_2 + 1))
        fi
    fi


################################################################################
# make argc & argv parameter                                                   #
################################################################################
    if [ "$g_match_mode" == "1" ]; then
        disable_wtl_temp=0
    fi
    if [ $result -eq 0 ]; then
        if [ $enable_random -eq 1 ] && [ $txt_param_switch -eq 0 ]; then
            get_random_param 1 wave511
        else
            get_default_param 1 wave511
        fi
        
        # FBC does not support the endianness.
        if [ "$g_enable_wtl" == "0" ]; then
            g_frame_endian=16
        fi

        if [ "$force_wtl" != "" ]; then
            g_enable_wtl=1
            g_wtl_format=$force_wtl
        fi

        # reset after hangup
        backup=$g_fpga_reset
        if [ $force_reset -eq 1 ]; then
            g_fpga_reset=1
            force_reset=0
        fi
        
        #make argc & argv parameter
        build_test_param 1
        g_fpga_reset=$backup

        test_param="$test_param $g_func_ret_str"
        test_param="${test_param} $g_frame_num"

################################################################################
# make prebuilt stream                                                         #
################################################################################
        if [ $build_yuv -eq 1 ]; then
            build_golden_yuvs $stream
            remain_count=$(($remain_count - 1))
            count=$(($count + 1))
            continue
        fi

        output_filename="$(basename $line).yuv"

        case $g_match_mode in
            1)  generate_golden_data_path $codec $streamset_path $stream 
                golden_yuv_path="$g_func_ret_str"
                test_param="$test_param --ref-yuv=$golden_yuv_path";;
        esac


################################################################################
# print information                                                            #
################################################################################

        if [ $result -eq 0 ]; then
            log_conf "RANDOM TEST    : ${ON_OFF[$enable_random]}" $temp_log_path
            log_conf "BITSTREAM MODE : ${DEC_BSMODE_NAME[$g_bsmode]}" $temp_log_path
            log_conf "ENDIAN         : STREAM($g_stream_endian) FRAME($g_frame_endian)" $temp_log_path
            log_conf "STANDARD       : ${CODEC_NAME[$g_codec_index]}" $temp_log_path
            log_conf "REORDER        : ON" $temp_log_path
            log_conf "MAP TYPE       : Compressed" $temp_log_path
            log_conf "WTL MODE       : ${ON_OFF[$g_enable_wtl]}" $temp_log_path
            if [ "$g_enable_wtl" == "1" ]; then
                log_conf "WTL FORMAT     : ${g_yuv_fmt_list[$g_wtl_format]}" $temp_log_path
            else
                log_conf "WTL FORMAT     : NONE" $temp_log_path
            fi
            log_conf "CBCR interleave: ${ON_OFF[$g_cbcr_interleave]}" $temp_log_path
            log_conf "NV21           : ${ON_OFF[$g_enable_nv21]}" $temp_log_path
            log_conf "Secondary AXI  : ${g_secondary_axi}" $temp_log_path

            test_param_print=$test_param
            log_conf "Unix   : $test_exec $test_param_print --input=$stream" $temp_log_path
            winexec=$(echo "$test_exec $test_param_print --input=$stream" | sed -e 's/\//\\/g')
            log_conf "Windows: $winexec" $temp_log_path
            cat $temp_log_path >> $conf_log_path

            chmod 777 $test_exec
            test_exec_param="$test_exec $test_param --input=$target_stream"
            result=0
            if [ "$simulation" == "true" ]; then
                test_exec_param="$test_exec_param $g_w_param $g_fsdb_param $g_ius_param --TestRunner=1"
                set -o pipefail; $test_exec_param 2>&1 | tee -a $conf_log_path || result=?
            else
                $test_exec_param || result=$?
            fi
            if [ $result -eq $RET_HANGUP ]; then
                force_reset=1
            fi
        fi
    else
        cat $temp_log_path >> $conf_log_path
    fi

    if [ $result -eq 0 ]; then
        log_conf "[RESULT] SUCCESS" $conf_log_path
        success_count=$(($success_count + 1))
    else
        cat ./ErrorLog.txt >> $conf_log_path
        log_conf "[RESULT] FAILURE" $conf_log_path
        failure_count=$(($failure_count + 1))
        cat $temp_log_path >> $conf_err_log_path
        cat ./ErrorLog.txt >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        if [ $result -eq 10 ]; then
            echo "Abnormal exit!!!"
            break
        fi

        #############
        # Error Stop
        #############
        if [ "$error_stop" == "1" ]; then
            break;
        fi
    fi

    remain_count=$(($remain_count - 1))
    count=$(($count + 1))
    # clear temp log
    echo "" > $temp_log_path
    # delete temporal input file
    rm -f $g_copy_stream   
    if [ $fail_stop -eq 1 ] && [ $result -ne 0 ]; then
        break;
    fi
done 

if [ $build_yuv -eq 1 ]; then
    exit 0
fi

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
if [ $failure_count == 0 ] && [ $num_of_streams != 0 ]; then
    pass=${PASS}
    rm $conf_err_log_path
    log_err_filename=""
else
    pass=${FAIL}
fi

wiki_log="| $streamset_file | $num_of_streams | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | | | |"

echo "$wiki_log"
echo "$wiki_log" >> $wiki_log_file

if [ "$num_of_streams" == "0" ]; then
    echo "num_of_streams: $num_of_streams = exit 1"
    exit 1
fi

exit $failure_count
 

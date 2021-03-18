#!/bin/bash

source TestRunnerCommon.sh

# Global varaibles for decoder option
fail_stop=0
refc_exec="Nieuport_JpgDec"
bin_dir="./"
stream_dir="./stream"
wiki_log_file="./log/dec_confluence.log"
g_random_test="false";
jenkins=""
txt_param_switch=0
output_path="output_fpga.yuv"
default_yuv_fmt=""
default_subsample="none"
default_ordering="none"
simulation="false"
default_bssize=5242880      # 5MB
MAX_BS_SIZE=10485760        # 10MB, You can allocate more memory.
MIN_BS_SIZE=8192            # 8KB
g_random_test="false"
g_slice_height=0
g_enable_slice_intr=0
default_read_delay=
default_write_delay=

fmt_test_400_list=("none" "420p" "422p" "444p" "nv12" "nv21" "nv16" "nv61" "nv24" "nv42" "yuyv" "yvyu" "uyvy" "vyuy")
fmt_test_420_list=("none" "420p" "422p" "444p" "nv12" "nv21" "nv16" "nv61" "nv24" "nv42" "yuyv" "yvyu" "uyvy" "vyuy")
fmt_test_422_list=("none" "420p" "422p" "444p" "nv12" "nv21" "nv16" "nv61" "nv24" "nv42" "yuyv" "yvyu" "uyvy" "vyuy")
fmt_test_440_list=("none" "420p" "422p" "444p" "nv12" "nv21" "nv16" "nv61" "nv16v" "nv61v" "nv24" "nv42" "yuyv" "yvyu" "uyvy" "vyuy")
fmt_test_444_list=("none" "420p" "422p" "444p" "nv12" "nv21" "nv16" "nv61" "nv24" "nv42" "yuyv" "yvyu" "uyvy" "vyuy" "ayuv")

fmt_test_400_num=${#fmt_test_400_list[@]}
fmt_test_420_num=${#fmt_test_420_list[@]}
fmt_test_422_num=${#fmt_test_422_list[@]}
fmt_test_440_num=${#fmt_test_440_list[@]}
fmt_test_444_num=${#fmt_test_444_list[@]}

MAX_OUTPUT_FMT=$((${#SUPPORTED_YUV_FMT[@]} -1))

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h                        help"
    echo "-n num                    number of frames."
    echo "--enable-random           generate random option."
    echo "--bin-dir                 ref-c directory, default: ./"
    echo "--stream-dir              stream directory"
    echo "--12bit                   input stream is an extended sequential jpeg"
    echo "--stream-endian           ENDIAN  bitstream endianness. refer to datasheet Chapter 4."
    echo "--frame-endian            ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4."
    echo "--pixelj                  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness"
    echo "--subsample               conversion sub-sample(ignore case: NONE, 420, 422, 444";
    echo "--ordering                conversion ordering(ingore-case: NONE, NV12, NV21, YUYV, YVYU, UYVY, VYUY, AYUV";
    echo "                          NONE - planar format";
    echo "                          NV12, NV21 - semi-planar format for all the subsamples.";
    echo "                                       If subsample isn't defined or is none, the sub-sample depends on jpeg information";
    echo "                                       The subsample 440 can be converted to the semi-planar format. It means that the encoded sub-sample should be 440.";
    echo "                          YUVV..VYUY - packed format. subsample be ignored.";
    echo "                          AYUV       - 444 packed format. subsample be ignored.";
    echo "--scaleH                  Horizontal downscale: 0(none), 1(1/2), 2(1/4), 3(1/8)";
    echo "--scaleV                  Vertical downscale  : 0(none), 1(1/2), 2(1/4), 3(1/8)";
    echo "--rotation                0, 90, 180, 270 CCW"
    echo "--mirror                  0, 1(vertical), 2(horizontal), 3(both)"
}

function generate_parameters {
    local val=0
    local list

    g_stream_endian=$(gen_val 0 3 $default_stream_endian)
    if [ "$g_jpeg_12bit" == "true" ]; then
        g_frame_endian=$(gen_val 0 7 $default_frame_endian)
    else
        g_frame_endian=$(gen_val 0 3 $default_frame_endian)
    fi

    # PPU AND SCALER
    if [ "$g_random_test" == "true" ]; then
        ppu_random=$(gen_val 0 1) # 0 - rotation/mirror, 1 - scaler
        if [ "$ppu_random" == "0" ]; then
            g_rotation=$(gen_val_angle $default_rotation)
            g_mirror=$(gen_val 0 3 $default_mirror)
            g_scale_h=0
            g_scale_v=0
        elif [ "$ppu_random" == "1" ]; then
            g_rotation=0
            g_mirror=0
            g_scale_h=$(gen_val 0 3 $default_scaleH)
            g_scale_v=$(gen_val 0 3 $default_scaleV)
        fi
    else
        g_rotation=$(gen_val_angle $default_rotation)
        g_mirror=$(gen_val 0 3 $default_mirror)
        g_scale_h=$(gen_val 0 3 $default_scaleH)
        g_scale_v=$(gen_val 0 3 $default_scaleV)
        if [ "$g_rotation" != "0" ] || [ "$g_mirror" != "0" ]; then
            if [ "$g_scale_h" != "0" ] || [ "$g_scale_h" != "0" ]; then
                echo "The PPU(rotator and mirror) doesn't work with the down-scaler"
                echo "rotation: $g_rotation, mirror: $mirror, scale-H: $g_scale_h, scale-V: $g_scale_v"
                exit 1
            fi
        fi
    fi

    # SUBSAMPLE AND ORDERING
    if [ "$g_random_test" == "true" ]; then
        default_yuv_fmt="-99"
    fi
    if [ "$default_yuv_fmt" == "-99" ]; then
        case $g_orig_fmt in
        400) list=(${fmt_test_400_list[@]});;
        420) list=(${fmt_test_420_list[@]});;
        422) list=(${fmt_test_422_list[@]});;
        440) list=(${fmt_test_440_list[@]});;
        444) list=(${fmt_test_444_list[@]});;
        esac
        max_fmt_num=${#list[@]}
        val=$(gen_val 0 $((max_fmt_num-1)))
        output_format="${list[$val]}"
        # generate subsample and ordering parameter for ref-sw
        convert_fmt_val_for_refsw $output_format $g_orig_fmt
    else
        g_subsample="$default_subsample"
        g_ordering="$default_ordering"
    fi

    if [ "$g_rotation" != "0" ] || [ "$g_mirror" != "0" ]; then
        # color format converter and rotator cannot work together.
        if [ "$g_subsample" != "none" ]; then
            g_subsample="none"
        fi
        if [ "$g_ordering" != "nv12" ] && [ "$g_ordering" != "nv21" ]; then
            g_ordering="none"
        fi
    fi

    g_bs_size=$(gen_val $MIN_BS_SIZE $MAX_BS_SIZE $default_bssize)
    g_bs_size=$((($g_bs_size + 4095) & ~4095))

    g_pixel_just=$(gen_val 0 1 $default_pixelj)

    # generate -pn -pk and -v options for c-model
    convert_fmt_val_for_cmodel $g_subsample $g_orig_fmt $g_ordering
}

function build_test_param {
    local param=""

    param="$param --rotation=$g_rotation"
    param="$param --mirror=$g_mirror"
    param="$param --stream-endian=$g_stream_endian --frame-endian=$g_frame_endian"
    param="$param --subsample=$g_subsample --ordering=$g_ordering"
    param="$param --scaleH=$g_scale_h"
    param="$param --scaleV=$g_scale_v"
    param="$param --bs-size=$g_bs_size"
    param="$param --pixelj=$g_pixel_just"
    if [ "$simulation" == "true" ]; then
        param="$param $g_fsdb_param --TestRunner=1"
    fi

    g_func_ret_str="$param"
}

function get_yuv_format {
    local format=""
    local cmd="${bin_dir}/${refc_exec} -i $1 -m 1"

    if [ "$g_jpeg_12bit" == "true" ]; then
        cmd="$cmd -u"
    fi
    $cmd > result.txt
    while read line; do
        line=$(echo $line | tr -d '\n')
        line=$(echo $line | tr -d '\r')
        line="${line%%\#*}"
        attr=$(echo $line | cut -d':' -f1)
        attr=$(echo $attr | tr -d ' ')
        value=$(echo $line | cut -d':' -f2-)
        value=$(echo $value | tr -d ' ')
        value="${value//:/}"
        if [ "$attr" == "OutputYUVformat" ]; then
            format="$value"
            break;
        fi
    done < result.txt

    echo "$format"
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o hw: -l enable-random,stream-dir:,bin-dir:"
OPTSTRING="${OPTSTRING},stream-endian:,frame-endian:"
OPTSTRING="${OPTSTRING},slice-height:,enable-slice-intr:,subsample:,ordering:,output:,input:,bs-size:"
OPTSTRING="${OPTSTRING},scaleH:,scaleV:,rotation:,mirror:"
OPTSTRING="${OPTSTRING},12bit,pixelj:"

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
        --bin-dir)
            bin_dir=$2
            shift 2;;
        --enable-random)
            g_random_test="true"
            shift;;
        --stream-dir)
            echo "stream-dir"
            stream_dir=$2
            shift 2;;
        --stream-endian)
            default_stream_endian=$2
            shift 2;;
        --frame-endian)
            default_frame_endian="$2"
            shift 2;;
        --12bit)
            g_jpeg_12bit="true"
            shift;;
        --pixelj)
            default_pixelj="$2"
            shift 2;;
        --input)
            streamset_path="$2"
            shift 2;;
        --output)
            output_path="$2"
            shift 2;;
        --subsample)
            default_subsample="$2"
            shift 2;;
        --ordering)
            default_ordering="$2"
            shift 2;;
        --scaleH)
            default_scaleH="$2"
            shift 2;;
        --scaleV)
            default_scaleV="$2"
            shift 2;;
        --rotation)
            default_rotation="$2"
            shift 2;;
        --mirror)
            default_mirror="$2"
            shift 2;;
        --bs-size)
            default_bssize="$2"
            shift 2;;
        --)
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))

if [ "$simulation" == "true" ]; then
    ACLK_MIN=160
    ACLK_MAX=300
    CCLK_MIN=160
    CCLK_MAX=300
    DEFAULT_ACLK=200
    DEFAULT_CCLK=200
    test_exec="./runit"
else
    DEFAULT_ACLK=20
    DEFAULT_CCLK=20
    test_exec="./jpg_dec_test"
fi



################################################################################
# Get param from target text file                                              #
################################################################################
name_value=()
input_param_name="TestRunnerParamCodaJ12Dec.txt"
if [ -f $input_param_name ]; then
    while read line; do
        # remove right comment
        line="${line%%\#*}"
        if [ "$line" = "" ]; then continue; fi

        OIFS=$IFS
        IFS='='
        count=0
        for word in $line; do
            word=${word// /}
            name_value[$count]="$word"
            count=$(($count+1))
        done
        IFS=$OIFS
        attr="${name_value[0]}"
        value="${name_value[1]}"

        case "$attr" in
            default)            default_opt="$value";;
            aclk)               [ ! -z $default_aclk ]          || default_aclk="$value";;
            cclk)               [ ! -z $default_cclk ]          || default_cclk="$value";;
            n)                  g_frame_num="-n $value";;
            fake_test)          fake_test="$value";;
            stream-endian)      [ ! -z $default_stream_endian ] || default_stream_endian="$value";;
            frame-endian)       [ ! -z $default_frame_endian ]  || default_frame_endian="$value";;
            scaleH)             [ ! -z $default_scaleH ]        || default_scaleH="$value";;
            scaleV)             [ ! -z $default_scaleV ]        || default_scaleV="$value";;
            rotation)           [ ! -z $default_rotation ]      || default_rotation="$value";;
            mirror)             [ ! -z $default_mirror ]        || default_mirror="$value";;
            pixelj)             [ ! -z $default_pixelj ]        || default_pixelj="$value";;
            slice-height)       [ ! -z $default_slice_height ]  || default_slice_height="$value";;
            enable-slice-intr)  [ ! -z $default_slice_int ]     || default_slice_int="$value";;
            txt-param)      txt_param_switch=1;;
            *) ;;
        esac
    done < $input_param_name
else
    if [ ! -f $input_param_name ]; then
        echo "$input_param_name file doesn't exist";
    fi
fi

# SET DEFAULT VALUES
if [ "$g_random_test" != "true" ]; then
    [ ! -z $default_aclk ]          || default_aclk=$DEFAULT_ACLK
    [ ! -z $default_cclk ]          || default_cclk=$DEFAULT_CCLK
    [ ! -z $default_stream_endian ] || default_stream_endian=0
    [ ! -z $default_frame_endian ]  || default_frame_endian=0
    [ ! -z $default_scaleH ]        || default_scaleH=0
    [ ! -z $default_scaleV ]        || default_scaleV=0
    [ ! -z $default_rotation ]      || default_rotation=0
    [ ! -z $default_mirror ]        || default_mirror=0
    [ ! -z $default_slice_int ]     || default_slice_int=0
    [ ! -z $default_slice_height ]  || default_slice_height=0
    [ ! -z $default_pixelj ]        || default_pixelj=0
else
    default_slice_int=0
    default_slice_height=0
fi

if [ -z $streamset_path ]; then
    streamset_path=$1
fi
streamset_file=`basename ${streamset_path}`

################################################################################
# count stream number                                                          #
################################################################################
stream_file_array=()
ext="${streamset_file##*.}"
ext=$(tr '[:upper:]' '[:lower:]' <<< $ext)
single_file="false"
if [ "$ext" == "jpg" ] || [ "$ext" == "jpeg" ]; then
    stream_file_array[0]="$streamset_path"
    single_file="true"
else
    if [ ! -e "${streamset_path}" ]; then
        echo "No such file: ${streamset_path}"
        exit 1
    fi
    parse_streamset_file $streamset_path stream_file_array
fi

num_of_streams=${#stream_file_array[@]}

count=1
success_count=0
failure_count=0
remain_count=${num_of_streams}
basedir=${basedir//\/}  # remove last delimeter

mkdir -p temp
mkdir -p output
mkdir -p log/decoder_conformance
conf_log_path="log/decoder_conformance/${streamset_file}.log"
conf_err_log_path="log/decoder_conformance/${streamset_file}_error.log"
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

################################################################################
# read cmd file.                                                               #
################################################################################
for (( c=0; c< $num_of_streams ; c++ )) do
    line=${stream_file_array[$c]}
    if [ "$single_file" == "true" ]; then
        stream="${line}"
    else
        stream="${stream_dir}/${line}"
    fi
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_streams}] ${stream}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    if [ ! -f $stream ]; then
        log_conf "Not found $stream" $temp_log_path
        log_conf "[RESULT] FAILURE" $temp_log_path
        echo "Not found $stream" >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        cat $temp_log_path >> $conf_log_path
        failure_count=$(($failure_count + 1))
        remain_count=$(($remain_count - 1))
        count=$(($count + 1))
        continue
    fi

    result=0
    target_stream=$stream

################################################################################
# make argc & argv parameter                                                   #
################################################################################
    if [ $result -eq 0 ]; then
        g_orig_fmt=$(get_yuv_format $target_stream)
        generate_parameters
        build_test_param
        test_param="$g_func_ret_str"

################################################################################
# Golden YUV                                                                   #
################################################################################
        golden_yuv_path="output_refc.yuv"
        generate_golden_yuv $stream $golden_yuv_path
        result=$?

################################################################################
# print information                                                            #
################################################################################

        if [ $result -eq 0 ]; then
            log_conf "RANDOM TEST    : $g_random_test" $temp_log_path
            log_conf "ENDIAN         : STREAM($g_stream_endian) FRAME($g_frame_endian)" $temp_log_path
            log_conf "SUBSAMPLE      : $g_subsample" $temp_log_path
            log_conf "ORDERING       : $g_ordering" $temp_log_path
            log_conf "ROTATE         : $g_rotation" $temp_log_path
            log_conf "MIRROR         : ${MIRROR_NAME[$g_mirror]}" $temp_log_path
            log_conf "SCALE DOWN     : H: ${SCALEDOWN_NAME[$g_scale_h]} V: ${SCALEDOWN_NAME[$g_scale_v]}" $temp_log_path
            log_conf "PIXEL_JUST.    : ${JUSTIFY_NAME[$g_pixel_just]}" $temp_log_path

            rm -f output_fpga.yuv
            cmd="$test_exec $test_param --output=$output_path --input=$target_stream"
            cmd_log="$test_exec $test_param --output=$output_path --input=$stream"
            log_conf "$cmd_log" $temp_log_path
            cat $temp_log_path >> $conf_log_path

            if [ "$fake_test" == "1" ]; then
                echo "<< Testing just script >>"
            else
				exit
                $cmd
                result=$?
                if [ "$result" == "0" ]; then
                    cmp $output_path output_refc.yuv
                    if [ "$?" != "0" ]; then
                        result=1
                    fi
                fi
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
        echo "cmp $stream $target_stream"
        cmp $stream $target_stream
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        if [ $result -eq 10 ]; then
            echo "Abnormal exit!!!"
            break
        fi
    fi
    remain_count=$(($remain_count - 1))
    count=$(($count + 1))
    # clear temp log
    echo "" > $temp_log_path
    # delete temporal input file
    if [ $fail_stop -eq 1 ] && [ $result -ne 0 ]; then
        break;
    fi
done

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

echo $wiki_log
echo $wiki_log >> $wiki_log_file

if [ "$num_of_streams" == "0" ]; then
    echo "num_of_streams: $num_of_streams = exit 1"
    exit 1
fi

exit $failure_count

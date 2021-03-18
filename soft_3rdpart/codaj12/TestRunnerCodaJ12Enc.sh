#!/bin/bash

source TestRunnerCommon.sh

# Global varaibles for encoder options
wiki_log_file="./log/enc_confluence.log"
jenkins=""
txt_param_switch=0
output_path="output_fpga.jpg"
DEFAULT_BS_SIZE=5242880    # 5MB
MIN_BS_SIZE=8192           # 8KB
default_read_delay=
default_write_delay=

g_random_test=0
simulation="false"

yuv_dir="./yuv"
bin_dir="./"
cfg_dir="./cfg"
g_aclk=$DEFAULT_ACLK
g_cclk=$DEFAULT_CCLK
refc_exec="Nieuport_JpgEnc"
tiled_exec="./tiled_mode"


################################################################################
# Initialize parameter variables                                               #
################################################################################

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h                        help"
    echo "--enable-random           It generates random parameters"
    echo "--bin-dir                 ref-c directory, default: ./"
    echo "--cfg-dir                 base directory of encoder configuration files. default: ./cfg"
    echo "--yuv-dir                 base directory storing YUV. default: ./yuv"
    echo "--stream-endian           ENDIAN  bitstream endianness. refer to datasheet Chapter 4."
    echo "--frame-endian            ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4."
    echo "--slice-height            the vertical hieght of a slice. multiple of 16 for 420, multiple of 8 for others"
    echo "--enable-slice-intr=ONOFF enable get the interrupt at every slice encoded. ONOFF - 0 for disable, 1 for enable. default OFF"
    echo "--bs-size                 itstream buffer size in byte"
    echo "--output                  output path"
    echo "--input                   input path"
    echo "--rotation=angle          0, 90, 180, 270"
    echo "--mirror=value            0, 1(Vertical), 2(Horizontal), 3(both)"
}

function generate_parameters {
    local cfg=$1

    g_stream_endian=$(gen_val 0 3 $default_stream_endian)
    if [ "$jpeg_12bit" == "1" ]; then
        g_frame_endian=$(gen_val 0 7 $default_frame_endian)
    else
        g_frame_endian=$(gen_val 0 3 $default_frame_endian)
    fi

    g_pixel_just=$(gen_val 0 1 $default_pixelj)
    g_rotation=$(gen_val_angle $default_rotation)
    g_mirror=$(gen_val 0 3 $default_mirror)

    if [ "$g_random_test" == "1" ]; then
        slice_random=$(gen_val 0 1) # 0: disable slice done, 1: enable slice_done
    else
        slice_random=0
    fi
    if [ "$slice_random" == "1" ] || [ "$default_slice_height" != "0" ]; then
        read_cfg $cfg
        if [ "$g_rotation" == "90" ] || [ "$g_rotation" == "270" ]; then
            local temp=$cfg_width
            cfg_width=$cfg_height
            cfg_height=$temp
            if [ "$cfg_img_format" == "2" ]; then
                # 422 to 440
                cfg_img_format=3
            elif [ "$cfg_img_format" == "3" ]; then
                # 440 to 422
                cfg_img_format=2
            fi
        fi
        if [ "$g_rotation" != "0" ] || [ "$g_mirror" != "0" ]; then
            # disable the slice-encoding mode
            g_slice_height=0
            g_enable_slice_intr=0
        else
            g_slice_height=$(gen_valid_slice_height $cfg_width $cfg_height $cfg_img_format $default_slice_height)
            if [ "$g_slice_height" != "0" ]; then
                g_enable_slice_intr=$(gen_val 0 1 $default_slice_int)
            else
                g_enable_slice_intr=0
            fi
        fi
    else
        g_slice_height=0
        g_enable_slice_intr=0
    fi

    g_bs_size=$(gen_val $MIN_BS_SIZE $DEFAULT_BS_SIZE $default_bs_size)
    g_bs_size=$((($g_bs_size + 4095) & ~4095))

    g_tiled_mode=$default_tiled_mode
}

function build_test_param {
    local param=""

    if [ "$jpeg_12bit" == "1" ]; then
        param="$param --12bit"
    fi
    param="$param --stream-endian=$g_stream_endian --frame-endian=$g_frame_endian"
    param="$param --pixelj=$g_pixel_just"
    param="$param --slice-height=$g_slice_height"
    param="$param --enable-slice-intr=$g_enable_slice_intr"
    param="$param --enable-tiledMode=$g_tiled_mode"
    param="$param --bs-size=$g_bs_size"
    param="$param --rotation=$g_rotation"
    param="$param --mirror=$g_mirror"
    g_func_ret_str="$param"
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o hw: -l enable-random,bin-dir:,cfg-dir:,yuv-dir:"
OPTSTRING="${OPTSTRING},stream-endian:,frame-endian:"
OPTSTRING="${OPTSTRING},slice-height:,enable-slice-intr:,bs-size:,output:,input:"
OPTSTRING="${OPTSTRING},enable-tiledMode:"
OPTSTRING="${OPTSTRING},12bit,pixelj:"
OPTSTRING="${OPTSTRING},rotation:,mirror:"

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
            bin_dir_user=$2
            shift 2;;
        --enable-random)
            g_random_test=1
            shift;;
        --cfg-dir)
            cfg_dir_user=$2
            shift 2;;
        --yuv-dir)
            yuv_dir_user=$2
            shift 2;;
        --12bit)
            jpeg_12bit=1
            shift;;
        --pixelj)
            default_pixelj="$2"
            shift 2;;
        --enable-tiledMode)
            g_tiled_mode=$2
            shift 2;;
        --stream-endian)
            default_stream_endian="$2"
            shift 2;;
        --frame-endian)
            default_frame_endian="$2"
            shift 2;;
        --slice-height)
            default_slice_height="$2"
            shift 2;;
        --enable-slice-intr)
            default_slice_int="$2"
            shift 2;;
        --rotation)
            default_rotation="$2"
            shift 2;;
        --mirror)
            default_mirror="$2"
            shift 2;;
        --bs-size)
            default_bs_size="$2"
            shift 2;;
        --output)
            output_path="$2"
            shift 2;;
        --input)
            streamset_path="$2"
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
    test_exec="./jpg_enc_test"
fi

if [ ! -f ./$test_exec ] ; then
    echo " $test_exec file doesn't exist";
    exit 1
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



# SET DEFAULT VALUES
if [ "$g_random_test" != "1" ]; then
    [ ! -z $default_aclk ]          || default_aclk=$DEFAULT_ACLK
    [ ! -z $default_cclk ]          || default_cclk=$DEFAULT_CCLK
    [ ! -z $default_stream_endian ] || default_stream_endian=0
    [ ! -z $default_frame_endian ]  || default_frame_endian=0
    [ ! -z $default_pixelj ]        || default_pixelj=0
    [ ! -z $default_rotation ]      || default_rotation=0
    [ ! -z $default_mirror ]        || default_mirror=0
    [ ! -z $default_slice_int ]     || default_slice_int=0
    [ ! -z $default_slice_height ]  || default_slice_height=0
    [ ! -z $default_bs_size ]       || default_bs_size=$DEFAULT_BS_SIZE
    [ ! -z $default_tiled_mode ]    || default_tiled_mode=0
else
    # tiled option is not random value. It depends on a cfg name.
    default_tiled_mode=0
fi

if [ -z $streamset_path ]; then
    streamset_path=$1
fi
streamset_file=`basename ${streamset_path}`

################################################################################
# count stream number                                                          #
################################################################################
cfg_file_array=()
ext="${streamset_file##*.}"
if [ "$ext" == "cfg" ]; then
    cfg_file_array[0]="$streamset_path"
else
    if [ ! -e "${streamset_path}" ]; then
        echo "No such file: ${streamset_path}"
        exit 1
    fi
    parse_streamset_file $streamset_path cfg_file_array
fi

num_of_cfgs=${#cfg_file_array[@]}

count=1
success_count=0
failure_count=0
remain_count=${num_of_cfgs}
basedir=${basedir//\/}  # remove last delimeter

################################################################################
# make log info
################################################################################
mkdir -p temp
mkdir -p output
mkdir -p log/encoder_conformance
conf_log_path="log/encoder_conformance/${streamset_file}.log"
conf_err_log_path="log/encoder_conformance/${streamset_file}_error.log"
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

################################################################################
# read cmd file.                                                               #
################################################################################
for (( c=0; c< $num_of_cfgs ; c++ )) do
    line=${cfg_file_array[$c]}
    cfg="${line}"
    cfg_path="${cfg_dir}/${line}"
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_cfgs}] ${cfg_path}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    if [ ! -f $cfg_path ]; then
        log_conf "Not found $cfg_path" $conf_log_path
        log_conf "[RESULT] FAILURE" $conf_log_path
        echo "Not found $stream" >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        cat $temp_log_path >> $conf_log_path
        failure_count=$(($failure_count + 1))
        remain_count=$(($remain_count - 1))
        count=$(($count + 1))
        continue
    fi

    ################################################################################
    # read from cfg file name                                                      #
    ################################################################################
    if [[ $cfg == *"12b"* ]]; then
        echo "set jpeg_12bit from cfg file name"
        jpeg_12bit=1
    fi
    if [[ $cfg == *"tiled"* ]]; then
        echo "set g_tiled_mode from cfg file name"
        default_tiled_mode=1
    fi

    result=0
################################################################################
# make argc & argv parameter                                                   #
################################################################################
    if [ $result -eq 0 ]; then
        generate_parameters $cfg_path
        build_test_param
        test_param="$g_func_ret_str"

################################################################################
# Golden Data                                                                  #
################################################################################
        golden_path="output_refc.jpg"
        generate_golden_stream $line $golden_path
        result=$?

################################################################################
# print information                                                            #
################################################################################
        if [ $result -eq 0 ]; then
            log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
            log_conf "RANDOM TEST    : ${ON_OFF[$g_random_test]}" $temp_log_path
            log_conf "ENDIAN         : STREAM($g_stream_endian) FRAME($g_frame_endian)" $temp_log_path
            log_conf "STANDARD       : JPEG" $temp_log_path
            log_conf "PIXEL JUST.    : ${JUSTIFY_NAME[$g_pixel_just]}" $temp_log_path
            log_conf "TILED MODE     : ${ON_OFF[$g_tiled_mode]}" $temp_log_path
            log_conf "WIDTH          : $cfg_width" $temp_log_path
            log_conf "HEIGHT         : $cfg_height" $temp_log_path
            log_conf "SLICE_HEIGHT   : $g_slice_height" $temp_log_path
            log_conf "EN_SLICE_INTR  : $g_enable_slice_intr" $temp_log_path
            log_conf "BS SIZE        : $g_bs_size" $temp_log_path
            log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
            if [ -e $output_path ]; then
                rm -f $output_path
            fi
            cmd="$test_exec --output=$output_path --cfg-dir=$cfg_dir --yuv-dir=$yuv_dir $test_param --input=$cfg"
            if [ "$simulation" == "true" ]; then
                cmd="$cmd $g_w_param $g_fsdb_param --TestRunner=1"
            fi
            log_conf "$cmd" $temp_log_path
            cat $temp_log_path >> $conf_log_path

            if [ "$simulation" == "true" ]; then
                $cmd 2>&1 | tee -a $conf_log_path
            else
				exit
                $cmd
            fi
            log_conf "---------------- COMPARE ------------------" $conf_log_path
            log_conf "cmp $output_path output_refc.jpg" $conf_log_path
            if [ "$simulation" == "true" ]; then
                # For log messages.
                cmp $output_path output_refc.jpg 2>&1 | tee -a $conf_log_path > /dev/null #save the log
            fi
            cmp $output_path output_refc.jpg                                          #get the result & show log
            if [ "$?" != "0" ]; then
                log_conf "------- COMPARE FAIL--------------" $conf_log_path
                result=1
            else
                log_conf "------- COMPARE SUCCESS-----------" $conf_log_path
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
    fi
    remain_count=$(($remain_count - 1))
    count=$(($count + 1))
    # clear temp log
    echo "" > $temp_log_path
done

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
if [ "$failure_count" == "0" ] && [ $num_of_cfgs != 0 ]; then
    pass=${PASS}
    rm $conf_err_log_path
    log_err_filename=""
else
    pass=${FAIL}
fi

wiki_log="| $streamset_file | $num_of_cfgs | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | | | |"

echo $wiki_log
echo $wiki_log >> $wiki_log_file

if [ "$num_of_cfgs" == "0" ]; then
    echo "num_of_cfgs: $num_of_cfgs = exit 1"
    exit 1
fi

exit $failure_count

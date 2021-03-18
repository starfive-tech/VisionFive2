#!/bin/bash

source common.sh

# default values
g_product_name="wave521c"
test_exec="./w5_enc_test"
wiki_log_file="./log/enc_confluence.log"
enable_random=0;
txt_param_switch=0;
secondary_axi_def=3                     # bit0 : LF, bit1 : BIT
build_stream=0
cfg_dir=./cfg
ref_dir=.
yuv_base_path=./yuv
if [ "${OS}" = "Windows_NT" ]; then
    ref_c_exec=./hevc_enc.exe
else
    ref_c_exec=./hevc_enc
fi
cframe_c_exec=./cframe
codec=hevc


simulation="false"

ACLK_MIN_2000T=6
BCLK_MIN_2000T=6
CCLK_MIN_2000T=6
ACLK_AVC_MAX_2000T=15
BCLK_AVC_MAX_2000T=15
CCLK_AVC_MAX_2000T=15
ACLK_HEVC_MAX_2000T=8
BCLK_HEVC_MAX_2000T=8
CCLK_HEVC_MAX_2000T=8

ACLK_MIN_HAPS=1000 #min:300
BCLK_MIN_HAPS=1000
CCLK_MIN_HAPS=1000
ACLK_MAX_HAPS=5000
BCLK_MAX_HAPS=5000
CCLK_MAX_HAPS=5000

ACLK_MIN_VU440=5000 #min:300
BCLK_MIN_VU440=5000
CCLK_MIN_VU440=5000
ACLK_MAX_VU440=50000
BCLK_MAX_VU440=50000
CCLK_MAX_VU440=50000

RET_SUCCESS=0
RET_HANGUP=2


function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h              help"
    echo "-c              codec. default: hevc"
    echo "                valid codec list: avc, hevc"
    echo "--yuv-dir       input yuv directory"
    echo "--ref-dir       ref-c directory"
    echo "--bin-dir       ref-c directory"
    echo "--enable-random [optional] generate random opton"
    echo "--build-stream  Just generate golden stream with Ref-C."
}


OPTSTRING="-o hwn:c:"
OPTSTRING="${OPTSTRING} -l jump-stream:,bsmode:,enable-random,md5-dir:"
OPTSTRING="${OPTSTRING},yuv-dir:,build-stream,ref-dir:,wiki:,n:,cfg-dir:,bin-dir:,"
OPTS=`getopt $OPTSTRING -- "$@"`

if [ $? != 0 ]; then
    exit 1;
fi

eval set -- "$OPTS"

while true; do
    case "$1" in
        -h) 
            help
            exit 0
            shift;;
        -n) 
            g_frame_num="-n $2"
            if [ "$2" = "" ]; then
                frame_num_refc=""
            else
                frame_num_refc="-f $2"
            fi
            shift 2;;
        -c)
            codec=$(echo $2 | tr '[:upper:]' '[:lower:]')
            shift 2;;
        --bin-dir)
            bin_dir_user=$2
            shift 2;;
        --enable-random)
            enable_random=1
            shift;;
        --yuv-dir)
            yuv_dir_user=$2
            shift 2;;
        --cfg-dir)
            cfg_dir_user=$2
            shift 2;;
        --ref-dir)
            ref_dir=$2
            shift 2;;
        --build-stream)
            build_stream=1;
            shift;;
        --wiki)
            wiki_log_file=$2
            shift 2;;
        --) 
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))

case "${codec}" in
    "hevc")  g_codec_index=12;;
    "avc")   g_codec_index=0
             ref_c_exec=./avc_enc
             ;;
    *)
        echo "unsupported codec: ${codec}"
        help
        exit 1
        ;;
esac

if [ ! -z $yuv_dir_user ]; then
    yuv_dir="$yuv_dir_user"
    yuv_base_path="$yuv_dir_user"
fi
if [ ! -z $cfg_dir_user ]; then
    cfg_dir="$cfg_dir_user"
fi
if [ ! -z $bin_dir_user ]; then
    bin_dir="$bin_dir_user"
fi

################################################################################
# Get param from target text file                                              #
################################################################################
input_param_name=TestRunnerParamWave521CEnc.txt
if [ -f $input_param_name ] && [ $enable_random == 0 ]; then
    echo "read $input_param_name"
    while read line; do
        # remove right comment
        line=$(echo $line | tr -d '\n')
        line=$(echo $line | tr -d '\r')
        line="${line%%\#*}"

        attr=$(echo $line | cut -d'=' -f1)
        attr=$(echo $attr | tr -d ' ')
        if [ "$attr" == "enable-cbcrInterleave" ]; then
            value=$(echo $line | cut -d '=' -f3)
        else
            value=$(echo $line | cut -d'=' -f2)
        fi
        if [ "$attr" != "MODE_COMP" ]; then
            value=$(echo $value | tr -d ' ')
        fi

        case "$attr" in
            default)                default_opt="$value";;
            product)                g_product_name="$value";;
            core)                   g_num_cores_temp="$value"
                                    g_multi_vcore=1;;
            secondary-axi)          secondary_axi_temp="$value";;
            enable-cbcrInterleave)  nv21_temp="$value";;
            enable_nv21)            nv21_temp="$value";;
            yuv_src_mode)           g_yuv_mode_temp="$value";;
            srcFormat)              srcFormat_temp="$value";;
            stream-endian)          stream_endian_temp="$value";;
            source-endian)          source_endian_temp="$value";;
            frame-endian)           frame_endian_temp="$value";;                  
            bsmode)                 bsmode_temp="$value";;
            rotAngle)               rotAngle_temp="$value";;
            mirDir)                 mirDir_temp="$value";;
            lowLatencyMode)         g_lowLatencyMode_temp="$value";;
            cframe)                 g_cframe_temp="$value";;
            cframelossless)         g_cframelossless_temp="$value";;
            cframetx16y)            g_cframetx16y_temp="$value";;
            cframetx16c)            g_cframetx16c_temp="$value";;
            cframe_422)             g_cframe_422_temp="$value";;
            MODE_COMP_ENCODED)      MODE_COMP_ENCODED_temp="$value";;
            sfs)                    g_subFrameSync_temp="$value";;
            ring)                   g_ringBuffer_temp="$value";;
            n)                      g_frame_num="-n $value"
                                    if [ "$value" = "0" ]; then
                                        frame_num_refc=""
                                    else
                                        frame_num_refc="-f $value"
                                    fi;;
            txt-param)              txt_param_switch=1;;
            *) ;;
        esac
    done < $input_param_name
else
    if [ ! -f "$input_param_name" ]; then
        echo " $input_param_name file doesn't exist"; 
    fi
fi

streamset_path=$1
echo "streamset_path=$1"
streamset_file=`basename ${streamset_path}`

if [ ! -f "${streamset_path}" ]; then
    echo "No such file: ${streamset_path}"
    exit 1
fi

################################################################################
# calculate stream number                                                      #
################################################################################
stream_file_array=()
index=0
num_of_streams=0
while read line || [ -n "$line" ]; do
    line=$(echo $line | tr -d '\n')
    line=$(echo $line | tr -d '\r')
    line=${line#*( )}   # remove front whitespace
    line=$(echo $line | cut -d' ' -f1)
    firstchar=${line:0:1}
    case "$firstchar" in
        "@") break;;        # exit flag
        "#") continue;;     # comment
        ";") continue;;     # comment
        "")  continue;;     # comment
        *)
    esac
    stream_file_array[$index]="$line"
    index=$(($index + 1))
done < ${streamset_path}


num_of_streams=${#stream_file_array[@]}
#echo ${stream_file_array[@]}

count=1
success_count=0
failure_count=0
remain_count=${num_of_streams}
basedir=${basedir//\/}  # remove last delimeter

mkdir -p temp
mkdir -p output
mkdir -p log/encoder_conformance
conf_log_path="log/encoder_conformance/${streamset_file}_r${g_revision}.log"
conf_err_log_path="log/encoder_conformance/${streamset_file}_error_r${g_revision}.log"

temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

################################################################################
# read cfg file.                                                               #
################################################################################
force_reset=0
for line in ${stream_file_array[@]}; do
    cfg_path="${cfg_dir}/${line}"
    cfg_base_path=$(dirname $line)
    cfg_base_name=$(basename $line)
    if [ "$cfg_base_path" == "" ]; then
        cfg_base_path="."
    fi

    test_param=""
    if [ ! -f ${cfg_path} ]; then
        echo "cfg not exist, cfg_path=$cfg_path" >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        count=$(($count + 1))
        failure_count=$(($failure_count + 1))
        remain_count=$(($remain_count - 1))
        continue
    fi

    read_cfg $cfg_path

#default value
    MODE_COMP_ENCODED_temp=1            #MODE_COMP_ENCODED(bit_stream)
    if [ "$haps" = "1" ]; then
        ACLK_MIN=${ACLK_MIN_HAPS}
        BCLK_MIN=${BCLK_MIN_HAPS}
        CCLK_MIN=${CCLK_MIN_HAPS}
        ACLK_MAX=${ACLK_MAX_HAPS}
        BCLK_MAX=${BCLK_MAX_HAPS}
        CCLK_MAX=${CCLK_MAX_HAPS}
        KHz_clock=1
    elif [ "board" == "vu440" ] || [ "board" == "haps" ]; then
        ACLK_MIN=${ACLK_MIN_VU440}
        BCLK_MIN=${BCLK_MIN_VU440}
        CCLK_MIN=${CCLK_MIN_VU440}
        ACLK_MAX=${ACLK_MAX_VU440}
        BCLK_MAX=${BCLK_MAX_VU440}
        CCLK_MAX=${CCLK_MAX_VU440}
        KHz_clock=1
    else
        ACLK_MIN=${ACLK_MIN_2000T}
        BCLK_MIN=${BCLK_MIN_2000T}
        CCLK_MIN=${CCLK_MIN_2000T}
        if [ "$g_codec_index" = "12" ]; then #hevc
            ACLK_MAX=${ACLK_HEVC_MAX_2000T}
            BCLK_MAX=${BCLK_HEVC_MAX_2000T}
            CCLK_MAX=${CCLK_HEVC_MAX_2000T}
        else
            ACLK_MAX=${ACLK_AVC_MAX_2000T}
            BCLK_MAX=${BCLK_AVC_MAX_2000T}
            CCLK_MAX=${CCLK_AVC_MAX_2000T}
        fi
    fi

    if [ $enable_random == 1 ] && [ $txt_param_switch == 0 ]; then
        get_random_param 0 $g_product_name
    else
        get_default_param 0 $g_product_name
    fi

    g_secondary_axi=$(($g_secondary_axi >> 1))

################################################################################
# check minimum size with rotation                                             #
################################################################################
    if [ "$g_product_name" = "wave521c_dual" ]; then   
        g_rotAngle=0
        g_mirDir=0
    fi
    if [ "$g_rotAngle" == "90"  ] || [ "$g_rotAngle" == "270" ]; then
        if [ "$cfg_height" -lt "256" ] || [ "$cfg_width" -lt "128" ]; then 
            echo "width=$cfg_width, height=$cfg_height rot=$g_rotAngle"
            echo "[ERR] Not support src_width < 128 || src_height < 256 with 90, 270 degree rotation"
            echo "[ERR] change rotAngle value to 0"
            g_rotAngle=0
        fi
    fi

################################################################################
# check dual_core condition
################################################################################
    if [ "$g_ringBuffer" == "1" ] || [ "$g_ringBuffer" == "2" ]; then
        if [ $cfg_WaveFrontSynchro != 0 ]; then
            echo "========================================================"
            echo "disable ringBuffer(WaveFrontSynchro & ringbuffer enabled. not supported)"
            echo "========================================================"
            g_ringBuffer=0
        fi
    fi

################################################################################
# make argv parameter                                                          #
################################################################################

    #reset after hangup
    backup=$g_fpga_reset
    if [ $force_reset -eq 1 ]; then
        g_fpga_reset=1
        force_reset=0
    fi

    #make argc & argv parameter
    build_test_param 0 
    g_fpga_reset=$backup

    test_param="${test_param} $g_func_ret_str"

    if   [ "$g_rotAngle" == "0"   ] && [ "$g_mirDir" == "0" ]; then rot_arg=0
    elif [ "$g_rotAngle" == "180" ] && [ "$g_mirDir" == "3" ]; then rot_arg=0
    elif [ "$g_rotAngle" == "90"  ] && [ "$g_mirDir" == "0" ]; then rot_arg=1
    elif [ "$g_rotAngle" == "270" ] && [ "$g_mirDir" == "3" ]; then rot_arg=1
    elif [ "$g_rotAngle" == "180" ] && [ "$g_mirDir" == "0" ]; then rot_arg=2
    elif [ "$g_rotAngle" == "0"   ] && [ "$g_mirDir" == "3" ]; then rot_arg=2
    elif [ "$g_rotAngle" == "270" ] && [ "$g_mirDir" == "0" ]; then rot_arg=3
    elif [ "$g_rotAngle" == "90"  ] && [ "$g_mirDir" == "3" ]; then rot_arg=3
    elif [ "$g_rotAngle" == "0"   ] && [ "$g_mirDir" == "2" ]; then rot_arg=6
    elif [ "$g_rotAngle" == "180" ] && [ "$g_mirDir" == "1" ]; then rot_arg=6
    elif [ "$g_rotAngle" == "90"  ] && [ "$g_mirDir" == "2" ]; then rot_arg=7
    elif [ "$g_rotAngle" == "270" ] && [ "$g_mirDir" == "1" ]; then rot_arg=7
    elif [ "$g_rotAngle" == "180" ] && [ "$g_mirDir" == "2" ]; then rot_arg=4
    elif [ "$g_rotAngle" == "0"   ] && [ "$g_mirDir" == "1" ]; then rot_arg=4
    elif [ "$g_rotAngle" == "270" ] && [ "$g_mirDir" == "2" ]; then rot_arg=5
    elif [ "$g_rotAngle" == "90"  ] && [ "$g_mirDir" == "1" ]; then rot_arg=5
    fi

    output_filename="$(basename $line).bin"
    test_param="${test_param} --output=$output_filename --cfgFileName=$cfg_path"

#ifdef SUPPORT_LOOK_AHEAD_RC
    if [[ "$cfg_path" == *"larc"* ]]; then
        #set cfgFileName_larc_pass1
        cfg_path_pass1=`echo ${cfg_path} | sed 's/pass2/pass1/'`
        test_param="${test_param} --cfgFileName_larc_pass1=$cfg_path_pass1"

    fi
#endif
    test_param="${test_param} $g_frame_num"
    
    yuv_path="${yuv_dir}/${cfg_InputFile}"


    file_name=${cfg_BitstreamFile%.*}
    file_ext=${cfg_BitstreamFile##*.}
    
    ref_stream_path="${ref_dir}/${cfg_base_path}/${file_name}_${rot_arg}.${file_ext}"
    if [ "$simulation" == "true" ]; then
         test_param="${test_param} --input=${yuv_path}"
    fi
#ifdef SUPPORT_LOOK_AHEAD_RC
    if [[ "$cfg_path" == *"larc"* ]]; then
        test_param="${test_param} --input_larc_pass1=${larc_pass1_input_param}"
    fi
#endif
    test_param="${test_param} --ref_stream_path=${ref_stream_path}"

################################################################################
# print information                                                            #
################################################################################
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_streams}] ${cfg_path}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    log_conf "yuv_path          : $yuv_path" $temp_log_path
    log_conf "ref_stream_path   : $ref_stream_path" $temp_log_path
    log_conf "width X Height    : $cfg_width X $cfg_height" $temp_log_path
    log_conf "BitstreamFile     : $cfg_BitstreamFile" $temp_log_path
    log_conf "-------------------------------------------" $temp_log_path
    log_conf "RANDOM TEST       : ${ON_OFF[$enable_random]}" $temp_log_path
    log_conf "SEC AXI           : ${g_secondary_axi}, 0x01:RDO, 0x02:LF" $temp_log_path
    log_conf "BITSTREAM MODE    : ${ENC_WAVE_BSMODE_NAME[$g_ringBuffer]}" $temp_log_path
    log_conf "Endian            : ${g_stream_endian}, ${g_frame_endian}, ${g_source_endian}" $temp_log_path
    log_conf "Standard          : ${CODEC_NAME[$g_codec_index]}" $temp_log_path
    if  [ "$g_cframe" == "1" ]; then
        log_conf "Input Format      : CFRAME " $temp_log_path
        log_conf "      lossless    : $g_cframelossless " $temp_log_path
        log_conf "      tx16 y      : $g_cframetx16y" $temp_log_path
        log_conf "      tx16 c      : $g_cframetx16c" $temp_log_path
        log_conf "      422         : $g_cframe_422" $temp_log_path
    elif  [ "$g_packedFormat" == "1" ]; then
        log_conf "Input Format      : (${g_packedFormat})YUYV" $temp_log_path
    elif  [ "$g_packedFormat" == "2" ]; then
        log_conf "Input Format      : (${g_packedFormat})YVYU" $temp_log_path
    elif  [ "$g_packedFormat" == "3" ]; then
        log_conf "Input Format      : (${g_packedFormat})UYVY" $temp_log_path
    elif  [ "$g_packedFormat" == "4" ]; then
        log_conf "Input Format      : (${g_packedFormat})VYUY" $temp_log_path
    elif [ $g_cbcr_interleave -eq 1 ]; then
        if [ "$g_enable_nv21" == "0" ] || [ "$g_enable_nv21" == "1" ]; then
            log_conf "Input Format      : cbcrInterleave nv21=${g_enable_nv21}" $temp_log_path
        fi
    else
        log_conf "Input Format      : 420 Planar" $temp_log_path
    fi
    log_conf "srcFormat         : ${SRC_FORMAT_NAME[$g_srcFormat]}" $temp_log_path
    log_conf "Rotation          : ${g_rotAngle}" $temp_log_path
    if [ "$g_mirDir" == "0" ]; then
        log_conf "Mirror            : (${g_mirDir})NONE" $temp_log_path
    elif [ "$g_mirDir" == "1" ]; then
        log_conf "Mirror            : (${g_mirDir})Vertical" $temp_log_path
    elif [ "$g_mirDir" == "2" ]; then
        log_conf "Mirror            : (${g_mirDir})Horizontal" $temp_log_path
    else
        log_conf "Mirror            : (${g_mirDir})Vert-Horz" $temp_log_path
    fi
    log_conf "rot_arg           : $rot_arg" $temp_log_path
    log_conf "LOW LATENCY       : ${g_lowLatencyMode}, 0x01:interrupt, 0x02:fast bitstream-packing" $temp_log_path
    log_conf "--------------------------------------------------------------------------------" $temp_log_path

################################################################################
# make ref-prebuilt stream                                                     #
################################################################################
    if [ ! -f "$ref_stream_path" ] || [ $build_stream -eq 1 ]; then
        ref_stream_base_dir=$(dirname $ref_stream_path)
        if [ ! -d "$ref_stream_base_dir" ]; then
            mkdir -p "$ref_stream_base_dir"
	        chmod 755 "$ref_stream_base_dir"
            echo "CREATE REF-STREAM DIR: $ref_stream_base_dir"
        fi
        if [ "$g_num_cores" == "2" ]; then
            REF_C_EXEC_PARAM="-c $cfg_path -p ${yuv_base_path}/ -b temp.bin $extra_param $frame_num_refc --NumCores=2"
        else
            REF_C_EXEC_PARAM="-c $cfg_path -p ${yuv_base_path}/ -b temp.bin $extra_param $frame_num_refc"
        fi
        rm -rf temp.bin || echo ignore_err
        echo "$ref_c_exec $REF_C_EXEC_PARAM"
        result=0
        $ref_c_exec $REF_C_EXEC_PARAM || result=1
        if [ "$result" != "0" ]; then
            echo "ref-c error"
            remain_count=$(($remain_count - 1))
            count=$(($count + 1))
            continue
        fi
        if [ -s temp.bin ]; then
            echo "cp -v temp.bin $ref_stream_path"
            cp -v temp.bin $ref_stream_path
        fi
        if [ $build_stream -eq 1 ]; then
            remain_count=$(($remain_count - 1))
            count=$(($count + 1))
            continue
        fi
    fi

################################################################################
# run Test                                                                     #
################################################################################
    result=0
    test_param_print=$test_param
    log_conf "$test_exec $test_param_print" $temp_log_path
    cat $temp_log_path >> $conf_log_path

    test_exec_param="$test_exec $test_param"

    result=0
    if [ "$simulation" == "true" ]; then
        test_exec_param="$test_exec_param $g_w_param $g_fsdb_param $g_ius_param --TestRunner=1"
        set -o pipefail; $test_exec_param 2>&1 | tee -a $conf_log_path || result=1
    else
        $test_exec_param || result=$?
    fi
    if [ $result -eq $RET_HANGUP ]; then
        force_reset=1
    fi

    if [ "$result" == "0" ]; then
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
    echo ${cfg_path}
    echo "" > $temp_log_path
done 


endTime=$(date +%s%N)
elapsed=$((($endTime - $beginTime) / 1000000000))
elapsedH=$(($elapsed / 3600))
elapsedS=$(($elapsed % 60))
elapsedM=$(((($elapsed - $elapsedS) / 60) % 60))
if [ "$((elapsedS / 10))" == "0" ]; then elapsedS="0${elapsedS}" ;fi
if [ "$((elapsedM / 10))" == "0" ]; then elapsedM="0${elapsedM}" ;fi
if [ "$((elapsedH / 10))" == "0" ]; then elapsedH="0${elapsedH}" ;fi

if [ $elapsed -le 30 ]; then
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
echo "$wiki_log" >> $wiki_log_file


if [ "$num_of_streams" == "0" ]; then
    echo "num_of_streams: $num_of_streams = exit 1"
    exit 1
fi

exit $failure_count
 

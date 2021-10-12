#!/bin/bash

source common.sh

# default values
g_product_name="wave4"
test_exec="./w4_enc_test"
wiki_log_file="./log/enc_confluence.log"
enable_random=0;
txt_param_switch=0;
secondary_axi_def=0 
build_stream=0
cfg_dir=./cfg
yuv_base_path=./yuv
if [ "${OS}" = "Windows_NT" ]; then
    ref_c_exec=./hevc_enc.exe
else
    ref_c_exec=./hevc_enc
fi

ACLK_MAX=27
BCLK_MAX=27
CCLK_MAX=21
MCLK_MAX=29
VCLK_MAX=29
cframe50d_temp=0

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h              help"
    echo "--yuv-dir       input yuv directory"
    echo "--ref-dir       ref-c directory"
    echo "--bin-dir       ref-c directory"
    echo "--enable-random [optional] generate random opton"
    echo "--build-stream  Just generate golden stream with Ref-C."
}


OPTSTRING="-o hn:c:"
#ifdef SUPPORT_CONFTEST
OPTSTRING="${OPTSTRING}t"
#endif
OPTSTRING="${OPTSTRING} -l jump-stream:,bsmode:,enable-random,md5-dir:,yuv-dir:,build-stream,ref-dir:,wiki:,n:"
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
            frame_num="-n $2"
            if [ "$2" = "" ]; then
                frame_num_refc=""
            else
                frame_num_refc="-f $2"
            fi
            shift 2;;
        -c)
            codec=$(echo $2 | tr '[:upper:]' '[:lower:]')
            shift 2;;
        --enable-random)
            enable_random=1
            shift;;
        --yuv-dir)
            yuv_dir=$2
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

################################################################################
# Get param from target text file                                              #
################################################################################
input_param_name=TestRunnerParamWave420lEnc.txt
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
            secondary-axi)          secondary_axi_temp="$value";;
            enable-cbcrInterleave)  nv21_temp="$value";;
            enable_nv21)                   nv21_temp="$value";;
            yuv_src_mode)           yuv_src_mode_temp="$value";;
            srcFormat3p4b)          srcFormat3p4b_temp="$value";;
            stream-endian)          stream_endian_temp="$value";;
            source-endian)          source_endian_temp="$value";;
            frame-endian)           frame_endian_temp="$value";;                  
            bsmode)                 bsmode_temp="$value";;
            rotAngle)               rotAngle_temp="$value";;
            mirDir)                 mirDir_temp="$value";;
            MODE_COMP_ENCODED)      MODE_COMP_ENCODED_temp="$value";;
            n)                      frame_num="-n $value"
                                    if [ "$value" = "0" ]; then
                                        frame_num_refc=""
                                    else
                                        frame_num_refc="-f $value"
                                    fi;;
            subFrameSyncEn)         subframe_sync_temp="$value";;
            cframe50d)              cframe50d_temp=0;;
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
# Get version information                                                      #
################################################################################
if [ $build_stream -eq 0 ]; then
    get_product_id $test_exec
    product_id=$?
    if [ "${g_revision}" = "" ] || [ ${productId} != 6 ]; then
        echo "Invalid product ID($product_id), expected value(6)"
        exit 1
    fi
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
conf_log_path="log/encoder_conformance/${streamset_file}_r${revision}.log"
conf_err_log_path="log/encoder_conformance/${streamset_file}_error_r${revision}.log"
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

################################################################################
# read cfg file.                                                               #
################################################################################
for line in ${stream_file_array[@]}; do
    cfg_path="${cfg_dir}/${line}"
    cfg_base_path=$(dirname $line)
    if [ "$cfg_base_path" == "" ]; then
        cfg_base_path="."
    fi

    test_param=""

    while read line2; do
        #remove right comment
        line2=$(echo $line2 | tr -d '\n')
        line2=$(echo $line2 | tr -d '\r')
        line2="${line2%%\#*}"

        attr=$(echo $line2 | cut -d':' -f1)
        attr=$(echo $attr | tr -d ' ')
        value=$(echo $line2 | cut -d':' -f2)
        value=$(echo $value | tr -d ' ')

        value="${value//\"}"
        case "$attr" in
            BitstreamFile) 
                BitstreamFile="$value"
                ;;
            InputFile) 
                input_yuv="$value"
                ;;
            SourceWidth) 
                source_width="$value"
                ;;
            SourceHeight) 
                source_height="$value"
                ;;
            WaveFrontSynchro)
                WaveFrontSynchro="$value"
                ;;
            DeSliceMode)
                deslice_mode="$value"
                ;;
            *) ;;
        esac
        #echo "attr: $attr, value: $value"
    done < $cfg_path

#default value
    MODE_COMP_ENCODED=1            #MODE_COMP_ENCODED(bit_stream)
    if [ $enable_random == 1 ] && [ $txt_param_switch == 0 ]; then
        get_random_param 0 $g_product_name
    else
        get_default_param 0 $g_product_name
    fi

    g_secondary_axi=$(($g_secondary_axi >> 1))
    if [ "$MODE_COMP_CONV_YUV" == "1" ]; then
        g_subCtuMap=0
    fi

################################################################################
# WAVE420L CONSTRAINTS                                                         #
################################################################################
    g_rotAngle=0
    g_mirDir=0
    g_srcFormat3p4b=0
    g_bsmode=1
    g_afbcd=0
    g_enableWtl=0
    g_cframe50d=0
    g_minBurstLength=0
    g_yuv422=0

################################################################################
# make argv parameter                                                          #
################################################################################
    build_test_param 0 
    test_param="${test_param} $g_func_ret_str"
    rot_arg=0

    output_filename="$(basename $line).bin"
    test_param="${test_param} --output=$output_filename --cfgFileName=$cfg_path"
    
    yuv_path="${yuv_base_path}/${input_yuv}"


    file_name=${BitstreamFile%.*}
    file_ext=${BitstreamFile##*.}
    
    ref_stream_path="${ref_dir}/${cfg_base_path}/${file_name}_${rot_arg}.${file_ext}"

    test_param="${test_param} --ref_stream_path=${ref_stream_path}"

################################################################################
# print information                                                            #
################################################################################
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_streams}] ${cfg_path}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    log_conf "yuv_path          : $yuv_path" $temp_log_path
    log_conf "ref_stream_path   : $ref_stream_path" $temp_log_path
    log_conf "width X Height    : $source_width X $source_height" $temp_log_path
    log_conf "BitstreamFile     : $BitstreamFile" $temp_log_path
    log_conf "-------------------------------------------" $temp_log_path
    log_conf "RANDOM TEST       : ${ON_OFF[$enable_random]}" $temp_log_path
    log_conf "BITSTREAM MODE    : ${ENC_BSMODE_NAME[$g_bsmode]}" $temp_log_path
    log_conf "Endian            : ${g_stream_endian}, ${g_frame_endian}, ${g_source_endian}" $temp_log_path
    log_conf "Standard          : HEVC" $temp_log_path
    if  [ "$g_packedFormat" == "1" ]; then
        log_conf "YUV Format        : (${g_packedFormat})YUYV" $temp_log_path
    elif  [ "$g_packedFormat" == "2" ]; then
        log_conf "YUV Format        : (${g_packedFormat})YVYU" $temp_log_path
    elif  [ "$g_packedFormat" == "3" ]; then
        log_conf "YUV Format        : (${g_packedFormat})UYVY" $temp_log_path
    elif  [ "$g_packedFormat" == "4" ]; then
        log_conf "YUV Format        : (${g_packedFormat})VYUY" $temp_log_path
    elif [ "$g_enable_nv21" == "0" ] || [ "$g_enable_nv21" == "1" ]; then
        log_conf "YUV Format        : cbcrInterleave nv21=${g_enable_nv21}" $temp_log_path
    else
        log_conf "YUV Format        : 420 Planar" $temp_log_path
    fi
    log_conf "rot_arg           : $rot_arg" $temp_log_path
    log_conf "--------------------------------------------------------------------------------" $temp_log_path
    cat $temp_log_path >> $conf_log_path

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
        echo "$ref_c_exec -c $cfg_path -p $yuv_base_path -b temp.bin --PrpMode=$rot_arg $extra_param --enable_fbc=0 --TLevel=0 --DumpPicOut=1 --ApiScenario=0 -t ./ $frame_num_refc"
        rm -rf vcpu_pic_out_qc.txt
        $ref_c_exec -c $cfg_path -p $yuv_base_path -b temp.bin --PrpMode=$rot_arg $extra_param --enable_fbc=0 --TLevel=0 --DumpPicOut=1 --ApiScenario=0 -t ./ $frame_num_refc

        cp -v temp.bin $ref_stream_path
        cp -v vcpu_pic_out_qc.txt $recon_md5_base_path/${file_name}.265_0.txt

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
    $test_exec_param || result=1

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
 

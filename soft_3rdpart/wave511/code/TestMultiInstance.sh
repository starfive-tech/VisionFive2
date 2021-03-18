#!/bin/bash

source common.sh

g_product_name=""
enc_prebuilt=""
enable_wtl=0
enable_sync=0
enable_afbce=0
enable_afbcd=0
enable_scaler=0
enable_cframe50d=0
multi_random_cfg=0
haps=0
process_test="true"
test_exec="./multi_instance_test"
wiki_log_file="./log/multi_confluence.log"
yuv_base="yuv/"
bin_dir="./"
bitfile=0
first_sfs=0

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h                    Help"
    echo "--product             product name(coda960, coda980...)"
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o h -l product:"
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
        --product)
            g_product_name=$2
            shift 2;;
#ifdef CNM_FPGA_PLATFROM
        --bitfile)
            bitfile=$2
            shift 2;;
        --haps)
            haps=$2
            shift 2;;
        --vu440)
            vu440=$2
            shift 2;;
#endif /* CNM_FPGA_PLATFORM */
        --)
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))


streamset_path=$1
streamset_file=`basename ${streamset_path}`

if [ ! -f "${streamset_path}" ]; then
    echo "No such file: ${streamset_path}"
    exit 1
fi

################################################################################
# count stream number                                                          #
################################################################################
stream_command_array=()
linenum=0
while read -r line || [ -n "$line" ]; do
    line=$(echo $line | tr -d '\n')
    line=$(echo $line | tr -d '\r')
    line="${line//\\//}" # replace backslash to slash
    line=${line#*( )}    # remove front whitespace
    firstchar=${line:0:1}
    case "$firstchar" in
        "@") break;;        # exit flag
        "#") continue;;     # comment
        ";") continue;;     # comment
        "")  continue;;     # comment
        *)
    esac

    stream_command_array[$linenum]="$line"
    linenum=$(($linenum + 1))
done < ${streamset_path}

num_of_streams=${#stream_command_array[@]}

success_count=0
failure_count=0
remain_count=2          # process test + thread test = 2
if [ "$process_test" == "false" ]; then
    remain_count=1      # only thread test
fi
basedir=${basedir//\/}  # remove last delimeter

mkdir -p temp
mkdir -p output
mkdir -p log/multi_conformance
conf_log_path="log/multi_conformance/${streamset_file}_r${revision}.log"
conf_err_log_path="log/multi_conformance/${streamset_file}_error_r${revision}.log"
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)


################################################################################
# Read CMD File && Make test param                                             #
################################################################################
for ((i=0;i<$num_of_streams;i++)) do
    ################################################################################
    # SET DEFAULT VALUE                                                            #
    ################################################################################
    codec_std[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f1)
    stream[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f2)
    ref_file_path[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f3)
    isEncoder[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f4)
    main10[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f5)
    pic_width[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f6)
    pic_height[$i]=$(echo ${stream_command_array[$i]} | cut -d' ' -f7)
    afbce[$i]=0
    afbcd[$i]=0
    cframe50d[$i]=0
    cframe50d_mbl[$i]=0
    cframe50d_422[$i]=0
    scaler[$i]=0
    wtl[$i]=1
    bsmode[$i]=0
    match_mode[$i]=1
    enable_mvc[$i]=0
    core[$i]=0

    case "${codec_std[$i]}" in
        "0")  codec_name[$i]="avc";;
        "1")  codec_name[$i]="vc1";;
        "2")  codec_name[$i]="mp2";;
        "3")  codec_name[$i]="mp4";;
        "4")  codec_name[$i]="h263";;
        "5")  codec_name[$i]="dv3";;
        "6")  codec_name[$i]="rvx";;
        "7")  codec_name[$i]="avs";;
        "9")  codec_name[$i]="tho";;
        "10") codec_name[$i]="vp3";;
        "11") codec_name[$i]="vp8";;
        "12") codec_name[$i]="hevc";;
        "13") codec_name[$i]="vp9";;
        "14") codec_name[$i]="avs2";;
        "15") codec_name[$i]="svac";;
        "16") codec_name[$i]="av1";;
        *)
            echo "unsupported codec_std: ${codec_std[$i]}"
            help
            exit 1
            ;;
    esac

    if [ "${isEncoder[$i]}" = "1" ]; then
        output[$i]="instance_${i}_output.bin"
    else
        output[$i]=""
    fi

    ################################################################################
    # SET RANDOM VALUE                                                             #
    ################################################################################
    str=`echo ${stream[$i]} | grep "_MVC_\|MVCDS\|MVCICT\|MVCRP\|MVCSPS"` || echo ignore_error
    if [ "${str}" != "" ]; then
        enable_mvc[$i]=1
    fi

    if [ "${codec_name[$i]}" = "vp9" ] || [ "${codec_name[$i]}" = "dv3" ] || [ "${codec_name[$i]}" = "tho" ]; then
        bsmode[$i]=2
        ext=${stream[$i]##*.}
        if [ "$ext" = "ivf" ] && [ "${codec_name[$i]}" != "vp9" ]; then
            bsmode[$i]=0
        fi
    fi

    if [ "${isEncoder[$i]}" = "1" ]; then
        match_mode[$i]=3 # bitstream compare
    fi

    if [ "$enable_wtl" = "1" ]; then
        wtl[$i]=$(get_random 0 1)
    fi

    if [ "${g_product_name}" = "coda960" ] || [ "${g_product_name}" = "coda980" ]; then
        wtl[$i]=0
    fi

    if [ "${isEncoder[$i]}" = "1" ]; then
        wtl[$i]=0
    fi
    if [ "$enable_scaler" = "1" ]; then
        if [ "${i}" = "0" ]; then # at least one must use scaler.
            scaler[$i]=1
        else
            scaler[$i]=$(get_random 0 1)
        fi

        if [ "${scaler[$i]}" == "1" ]; then
            match_mode[$i]=1
        else
            match_mode[$i]=2
        fi
        wtl[$i]=1
        if [ "${scaler[$i]}" = "1" ]; then
            if [ "${pic_width[$i]}" = "" ] || [ "${pic_height[$i]}" = "" ]; then
                echo "stream size are not specified in cmd file"
                exit 1
            fi
            scaleWidth[$i]=$((${pic_width[$i]}/2))
            scaleWidth[$i]=$(ceiling ${scaleWidth[$i]} 2)
            scaleHeight[$i]=$((${pic_height[$i]}/2))
            scaleHeight[$i]=$(ceiling ${scaleHeight[$i]} 2)
        else
            scaleWidth[$i]=0
            scaleHeight[$i]=0
        fi
    fi
    if [ "${g_product_name}" = "wave521c_dual" ]; then
        core[$i]=$(get_random 1 2)
        ring[$i]=$(get_random 0 1)
        if [ ${ring[$i]} == 1 ]; then
            ring[$i]=2
        fi
    fi
done

################################################################################
# Generate compare file path                                                   #
################################################################################
for ((i=0;i<$num_of_streams;i++)) do
    if [ "${isEncoder[$i]}" = "1" ]; then
        echo "[A] path -> ${ref_file_path[$i]}"
    else
        codec=${codec_name[$i]}

        if [ "${scaler[$i]}" = "1" ]; then
            if [ "$g_product_name" == "wave512" ]; then
                if [ "${codec_name[$i]}" = "vp9" ]; then
                    bin_dir="../../../design/ref_c_vp9/bin/Linux"
                elif [ "${codec_name[$i]}" = "hevc" ]; then
                    bin_dir="../../../design/ref_c_hevc/bin/Linux"
                    g_support_minipippen=1
                fi
            else
                g_support_minipippen=1
            fi

            g_scaler="true"
            g_oriw=${pic_width[$i]}
            g_orih=${pic_height[$i]}
            g_sclw=${scaleWidth[$i]}
            g_sclh=${scaleHeight[$i]}
            g_codec_index=${codec_std[$i]}
            g_bsmode=${bsmode[$i]}
            ref_file_path[$i]="temp/output_${i}.yuv"
        fi
        if [ "${match_mode[$i]}" == "1" ]; then
            if [ "${main10[$i]}" == "1" ]; then
                is_main10=1
            fi
            generate_yuv ${stream[$i]} ${ref_file_path[$i]}
        fi
    fi
done

################################################################################
# Generate input file path                                                     #
################################################################################
for ((i=0;i<$num_of_streams;i++)) do
    cframe50sd_tx16y[$i]=0
    cframe50sd_tx16c[$i]=0
    cframe50sd_lossless[$i]=0
    g_cframe_422[$i]=0
    if [ "${isEncoder[$i]}" = "1" ]; then
        if [ "${cframe50d[$i]}" = "1" ]; then
            # create cframe50 file.
            yuv_name=$(grep 'InputFile' ${stream[$i]} | sed -e 's/\r//g' | cut -d' ' -f3)
            src_width=$(grep 'SourceWidth' ${stream[$i]} | sed -e 's/\r//g' | cut -d' ' -f3)
            src_height=$(grep 'SourceHeight' ${stream[$i]} | sed -e 's/\r//g' | cut -d' ' -f3)
            yuv_dir=${yuv_base}${yuv_name}

            echo ${yuv_dir}
            cp ${yuv_dir} instance_${i}_input.yuv

            bit_depth=8
            if [ "${main10[$i]}" = "1" ]; then
                bit_depth=10
            fi

            format_value=1
            if [ "${cframe50d_422[$i]}" = "1" ]; then
                format_value=2
            fi

            mbl_value=0
            case "${cframe50d_mbl[$i]}" in
            "0") mbl_value=16;;
            "1") mbl_value=32;;
            "2") mbl_value=64;;
            "3") mbl_value=128;;
            *) ;;
            esac

            ref_c_param="../../../design/cframe50_c/bin/cframe -i instance_${i}_input.yuv -b ${bit_depth} -w ${src_width} -h ${src_height} -f 1000 -min_burst_len ${mbl_value} -c refc_output_${i}.cf50 -format ${format_value} -convert422 ${cframe50d_422[$i]}"
            echo ${ref_c_param}
            ${ref_c_param}

            cp refc_output_${i}.cf50 ${stream[$i]}.cf50
        fi

        if [ "${cframe50sd[$i]}" = "1" ]; then
            # create cframe50s file.
            cframe_c_exec=../../../design/cframe50_c/bin/cframe
            if [ "$cfg_InputBitDepth" == "8" ]; then
                g_cframetx16y=$(get_random 2 7)
                g_cframetx16c=$(get_random 2 7)
            else
                g_cframetx16y=$(get_random 2 9)
                g_cframetx16c=$(get_random 2 9)
            fi
            g_cframetx16y=$(($g_cframetx16y * 16))
            g_cframetx16c=$(($g_cframetx16c * 16))
            cframe50sd_tx16y[$i]=$g_cframetx16y
            cframe50sd_tx16c[$i]=$g_cframetx16c

            if [ "$g_product_name" = "wave521" ] ||  [ "$g_product_name" = "wave521c" ]; then
                g_cframe_422[$i]=$(get_random 0 1)
            fi

            g_cframelossless=1 #test only lossless
            cframe50sd_lossless[$i]=$g_cframelossless #test only lossless
            read_cfg ${stream[$i]}
            yuv_path="/nstream/qctool/img/${cfg_InputFile}"
            if [ "${codec_name[$i]}" = "hevc" ]; then
                codec_path=hevc
            elif [ "${codec_name[$i]}" = "avc" ]; then
                codec_path=avc
            else
                codec_path=svac
            fi
            cframe_input_dir=/nstream/qctool/work/ENC_REF_STREAM/WAVE521_CFRAME/${codec_path}/${cfg_base_path} #cfg_base_path
            check_and_create_dir $cframe_input_dir
            input_file=$(basename $cfg_InputFile)
            input_dir=$(dirname $cfg_InputFile)
            check_and_create_dir $input_dir
            cframe_name=cframe_${input_file}_${cfg_width}_${cfg_height}_${g_cframelossless}_${g_cframetx16y}_${g_cframetx16c}.cf50s
            cframe_path=$cframe_input_dir/$cframe_name

            #run cframe cmodel if not exist
            if [ "$g_product_name" = "wave521" ] ||  [ "$g_product_name" = "wave521c" ]; then
                if [ ${g_cframe_422[$i]} == 1 ]; then
                    PARAM="-i $yuv_path -o ${cfg_InputFile}.422 -c temp.cf50s_${i} -b $cfg_InputBitDepth -w $cfg_width -h $cfg_height -format 2 -tx16 $g_cframetx16y -tx16_chroma $g_cframetx16c -lossless $g_cframelossless -f $cfg_FramesToBeEncoded -convert422 1 -oo ${cfg_InputFile} "
                else
                    PARAM="-i $yuv_path -o $cfg_InputFile -c temp.cf50s_${i} -b $cfg_InputBitDepth -w $cfg_width -h $cfg_height -format 1 -tx16 $g_cframetx16y -tx16_chroma $g_cframetx16c -lossless $g_cframelossless -f $cfg_FramesToBeEncoded"
                fi
            else
                PARAM="-i $yuv_path -o $cfg_InputFile -c temp.cf50s_${i} -b $cfg_InputBitDepth -w $cfg_width -h $cfg_height -format 1 -tx16 $g_cframetx16y -tx16_chroma $g_cframetx16c -lossless $g_cframelossless -f $cfg_FramesToBeEncoded"
            fi
            echo "$cframe_c_exec $PARAM"
            $cframe_c_exec $PARAM
            if [ -s temp.cf50s_${i} ]; then
                #cp -v temp.cf50s_${i} $cframe_path
                #chmod 666 $cframe_path
                echo "cframe success"
            else
                echo "cframe error"
                echo "cframe error"
                echo "cframe error"
                echo "cframe error"
                echo "cframe error"
                exit 1;
            fi
        fi
    fi
done


################################################################################
# Add test param                                                               #
# Options of the each instance are seperated by ","                            #
################################################################################
test_param=""
for ((i=0;i<$num_of_streams;i++)) do
    if [ "$i" = "0" ]; then
        codec_std_test_param="${codec_std[$i]}"
        stream_test_param="${stream[$i]}"
        if [ "${output[$i]}" != "" ]; then
            output_test_param="${output[$i]}"
        fi
        wtl_test_param="${wtl[$i]}"
        bsmode_test_param="${bsmode[$i]}"
        match_mode_test_param="${match_mode[$i]}"
        ref_file_path_test_param="${ref_file_path[$i]}"
        enc_dec_param="${isEncoder[$i]}"
        core_param="${core[$i]}"
        ring_param="${ring[$i]}"
        scaler_test_param="${scaler[$i]}"
        scaleWidth_test_param="${scaleWidth[$i]}"
        scaleHeight_test_param="${scaleHeight[$i]}"
    else
        codec_std_test_param="${codec_std_test_param},${codec_std[$i]}"
        stream_test_param="${stream_test_param},${stream[$i]}"
        if [ "${output[$i]}" != "" ]; then
            output_test_param="${output_test_param},${output[$i]}"
        fi
        wtl_test_param="${wtl_test_param},${wtl[$i]}"
        bsmode_test_param="${bsmode_test_param},${bsmode[$i]}"
        match_mode_test_param="${match_mode_test_param},${match_mode[$i]}"
        ref_file_path_test_param="${ref_file_path_test_param},${ref_file_path[$i]}"
        enc_dec_param="${enc_dec_param},${isEncoder[$i]}"
        core_param="${core_param},${core[$i]}"
        ring_param="${ring_param},${ring[$i]}"
        scaler_test_param="${scaler_test_param},${scaler[$i]}"
        scaleWidth_test_param="${scaleWidth_test_param},${scaleWidth[$i]}"
        scaleHeight_test_param="${scaleHeight_test_param},${scaleHeight[$i]}"
    fi
done

################################################################################
# make test param                                                               #
################################################################################
if [ "${output_test_param}" != "" ]; then
    output_param="--output=${output_test_param}"
fi

test_param="--instance-num=${num_of_streams} -c ${match_mode_test_param} -e ${enc_dec_param} --codec=${codec_std_test_param}"
test_param="$test_param --bsmode=${bsmode_test_param} --enable-wtl=${wtl_test_param}"
test_param="$test_param --input=${stream_test_param} ${output_param} --ref_file_path=${ref_file_path_test_param}"

if [ "$enable_scaler" = "1" ]; then
    test_param="${test_param} --scaler=${scaler_test_param} --sclw=${scaleWidth_test_param} --sclh=${scaleHeight_test_param}"
fi

if [ "$g_product_name" = "wave521c_dual" ]; then
    test_param="$test_param --cores=$core_param"
    test_param="$test_param --ring=$ring_param"
fi


################################################################################
# Test Mode Thread                                                             #
################################################################################
result=0
log_conf "MultiInstance test : Thread" ${temp_log_path}
log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
for ((i=0;i<$num_of_streams;i++)) do
    log_conf "[${i}/${num_of_streams}] ${stream[$i]}" ${temp_log_path}
done
log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
cat $temp_log_path >> $conf_log_path

echo "$test_exec $test_param"

nice -n -5 $test_exec $test_param || result=$?

if [ $result -eq 0 ]; then
    log_conf "[RESULT] SUCCESS" $conf_log_path
    success_count=$(($success_count + 1))
else
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

# clear temp log
echo "" > $temp_log_path

remain_count=$(($remain_count - 1))

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
time_hms="${elapsedH}:${elapsedM}:${elapsedS}"

log_filename=$(basename $conf_log_path)
log_err_filename=$(basename $conf_err_log_path)
if [ $failure_count == 0 ] && [ $num_of_streams != 0 ]; then
    pass=${PASS}
    rm $conf_err_log_path
    log_err_filename=""
else
    pass=${FAIL}
fi

wiki_log="| $streamset_file | 2 | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | | | Instacne Num = $num_of_streams |"
log_filename="[^$log_filename]"
if [ "$log_err_filename" != "" ]; then
    log_err_filename="[^$log_err_filename]"
fi
wiki_log="| $streamset_file | 2 | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | [Jenkins|$jenkins] | | Instacne Num = $num_of_streams |"
if [ "${haps}" = "1" ]; then
    wiki_log="| $streamset_file | 1 | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | [Jenkins|$jenkins] | | Instacne Num = $num_of_streams |"
fi

echo $wiki_log
echo $wiki_log >> $wiki_log_file

if [ "$num_of_streams" == "0" ]; then
    echo "num_of_streams: $num_of_streams = exit 1"
    exit 1
fi

exit $failure_count


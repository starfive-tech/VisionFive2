###############################################################################
# COMMON DATA
###############################################################################
ON_OFF=(
    "OFF"
    "ON"
)

YUV_FORMAT_NAME=(
    "AUTO"
    "YUV420"
    "YUV422"
    "YUV444"
    "YUV400"
)

ENDIAN_NAME=(
    "VDI_64BIT_LE"
    "VDI_64BIT_BE"
    "VDI_32BIT_LE"
    "VDI_32BIT_BE"
)

MIRROR_NAME=(
    "NONE"
    "VERTICAL"
    "HORIZONTAL"
    "BOTH"
)

JUSTIFY_NAME=(
    "MSB_JUSTFIED"
    "LSB_JUSTFIED"
)

YUV_PACKING_NAME=(
    "PLANAR"
    "SEMI-PLANAR(CbCr)"
    "SEMI-PLANAR(CrCb)"
    "YUYV"
    "UYVY"
    "YVYU"
    "VYUY"
    "444 PACK(8BIT)"
    "PPM(12BIT)"
)

SCALEDOWN_NAME=(
    "NONE"
    "1/2"
    "1/4"
    "1/8"
)


PASS="PASS"
FAIL="FAIL"
PASS="{color:green}*PASS*{color}"
FAIL="{color:red}*FAIL*{color}"

###############################################################################
# GLOBAL VARIABLES
###############################################################################
g_fpga_reset=0                       # 1 : reset fpga every time
g_jpeg_12bit="false"

######################## COMMON VARAIABLES FOR DECODER ########################
g_yuv_fmt_list=("${YUV_FORMAT_LIST[@]}")
g_match_mode=1                        # 0 - nomatch, 1 - yuv
g_aclk=20
g_cclk=20
g_secondary_axi=0
g_stream_endian=0
g_frame_endian=0
g_tiled_mode=0
g_cbcr_format=0                         # 0 - none, 1 - NV12, 2 - NV21
g_fake_test=0

########################## VARAIABLES FOR PPU #################################
g_rotation=0
g_mirror=0


###############################################################################
cmodel_pn_val="" # planar format
cmodel_pk_val="" # packed format
cmodel_nv_val="" # semi-planar

###############################################################################
# GLOBAL FUNCTIONS
###############################################################################
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

function gen_val {
    local min_val=$1
    local max_val=$2
    local default_val=$3
    local val

    if [ -z $default_val ] || [ "$default_val" == "-99" ]; then
        # Generate random value
        val=$(get_random $min_val $max_val)
    else
        val=$default_val
    fi
    
    echo "$val"
}

function gen_val_angle {
    local default_val=$1
    local val

    if [ -z $default_val ] || [ "$default_val" == "-99" ]; then
        val=$(($(get_random 0 3)*90))
    else
        val=$default_val
    fi

    echo "$val"
}

function gen_yuv_pack_12bit {
    local yuv_format=$1
    local default=$2
    local val

    case $yuv_format in
    0)  # No conversion
        val=0;;
    1)  # YUV420, 3 for ppm format
        val=$(gen_val 1 3 $default)
        if [ "$val" == "3" ]; then
            val=8 # PPM
        fi
        ;;
    2)  # YUV422
        val=$(gen_val 1 8 $default)
        if [ "$val" == "7" ]; then
            # 7 - YUV444 pack mode for 8bit yuv
            val=8
        fi
        ;;
    3)  # YUV444, 3 for ppm format
        val=$(gen_val 1 3 $default)
        if [ "$val" == "3" ]; then
            val=8
        fi
        ;;
    4)  # YUV400, 1 for ppm format
        val=$(gen_val 0 1 $default)
        if [ "$val" == "1" ]; then
            val=8
        fi
        ;;
    *)  echo "Unknown yuv_format: $yuv_format"
        exit;;
    esac

    echo "$val"
}

function gen_yuv_pack_8bit {
    local yuv_format=$1
    local default=$2
    local val

    case $yuv_format in
    0)  # No conversion
        val=0;;
    1)  # YUV420 
        val=$(gen_val 1 2 $default)
        ;;
    2)  # YUV422
        val=$(gen_val 1 6 $default)
        ;;
    3)  # YUV444, 3 for YUV444 pack
        val=$(gen_val 1 3 $default)
        if [ "$val" == "3" ]; then
            val="7"
        fi
        ;;
    4)  # YUV400
        val=0
        ;;
    *)  echo "Unknown yuv_format: $yuv_format"
        exit 1;;
    esac

    echo "$val"
}

function gen_yuv_pack {
    local yuv_12bit=$1
    local yuv_format=$2
    local default=$3
    local val

    if [ "$yuv_12bit" == "1" ]; then
        val=$(gen_yuv_pack_12bit $yuv_format $default)
    else
        val=$(gen_yuv_pack_8bit $yuv_format $default)
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

function convert_fmt_val_for_cmodel {
    local subsample=$1
    local orig_subsample=$2
    local ordering=$3

    ordering=$(tr '[:upper:]' '[:lower:]' <<< $ordering)
    cmodel_pn_val=0
    cmodel_pk_val=0
    cmodel_nv_val=0

    if [ "$ordering" == "none" ]; then
        case $subsample in
        420)   cmodel_pn_val=0;;
        422)   cmodel_pn_val=1;;
        444)   cmodel_pn_val=3;;
        *)     cmodel_pn_val=5;;
        esac
    else
        if [ "$ordering" == "nv12" ] || [ "$ordering" == "nv21" ]; then
            if [ "$subsample" == "none" ]; then
                cmodel_pn_val=5

                if [ "$ordering" == "nv12" ]; then
                    cmodel_pk_val=1
                else
                    cmodel_pk_val=2
                fi
            else
                case $subsample in
                400)   cmodel_pn_val=4;;
                420)   cmodel_pn_val=0;;
                422)   cmodel_pn_val=1;;
                440)   cmodel_pn_val=2;;
                444)   cmodel_pn_val=3;;
                esac

                if [ "$ordering" == "nv12" ]; then
                    cmodel_nv_val=1
                else
                    cmodel_nv_val=2
                fi
            fi
        else
            case $ordering in
            yuyv)   cmodel_pn_val=1
                    cmodel_pk_val=3;;
            uyvy)   cmodel_pn_val=1
                    cmodel_pk_val=4;;
            yvyu)   cmodel_pn_val=1
                    cmodel_pk_val=5;;
            vyuy)   cmodel_pn_val=1
                    cmodel_pk_val=6;;
            ayuv)   cmodel_pn_val=3
                    cmodel_pk_val=7;;
            esac
        fi
    fi

    if [ "$g_tiled_mode" == "1" ]; then
        if [ "$cmodel_pn_val" == "5" ]; then
            # no conversion
            cmodel_nv_val=0
            cmodel_pk_val=1
        fi
    fi
}

function convert_fmt_val_for_refsw {
    local fmt=$1
    local srcsample=$2
    g_subsample="none"
    g_ordering="none"
    case $fmt in
    420p)   g_subsample="420";;
    422p)   g_subsample="422";;
    444p)   g_subsample="444";;
    nv12)   if [ "$srcsample" != "420" ]; then
                g_subsample="420"
            fi
            g_ordering="nv12";;
    nv21)   if [ "$srcsample" != "420" ]; then
                g_subsample="420"
            fi
            g_ordering="nv21";;
    nv16)   if [ "$srcsample" != "422" ]; then
                g_subsample="422"
            fi
            g_ordering="nv12";;
    nv61)   if [ "$srcsample" != "422" ]; then
                g_subsample="422"
            fi
            g_ordering="nv21";;
    nv16v)  g_ordering="nv12";;
    nv61v)  g_ordering="nv21";;
    nv24)   if [ "$srcsample" != "444" ]; then
                g_subsample="444"
            fi
            g_ordering="nv12";;
    nv42)   if [ "$srcsample" != "444" ]; then
                g_subsample="444"
            fi
            g_ordering="nv21";;
    yuyv)   g_ordering="yuyv";;
    uyvy)   g_ordering="uyvy";;
    yvyu)   g_ordering="yvyu";;
    vyuy)   g_ordering="vyuy";;
    ayuv)   g_ordering="ayuv";;
    esac
}

function generate_golden_stream {
    local input=$1
    local output=$2
    local exec_path="${bin_dir}/${refc_exec}"
    local param=""
    local cmd=""

    if [ "$jpeg_12bit" == "1" ]; then
        param="-u"
    fi

    # ref_c do not support tiled mode.
    # In case of tiled mode, ref_c output data is same with linear mode.
    if [ "$g_tiled_mode" == "1" ]; then
        input=$(echo $input | sed 's/_tiled//g')
    fi

    param="$param -i $input -o ${output}"
    param="$param -g $(($g_mirror<<2 | $g_rotation/90))"
    param="$param -m 1 -ps $yuv_dir -pc $cfg_dir"
    if [ "$g_slice_height" ]; then
        param="$param -sh $g_slice_height"
    fi
    cmd="$exec_path $param"
    echo "$cmd"

    $cmd

    return $?
}

function generate_golden_yuv {
    local input=$1
    local output=$2
    local exec_path="${bin_dir}/${refc_exec}"
    local param=""
    local cmd=""

    if [ "$g_jpeg_12bit" == "true" ]; then
        param="-u"
    fi
    if [ "$cmodel_pn_val" != "" ]; then
        param="$param -pn $cmodel_pn_val" 
    fi
    if [ "$cmodel_pk_val" != "" ]; then
        param="$param -pk $cmodel_pk_val"
    fi
    if [ "$cmodel_nv_val" != "" ]; then
        param="$param -v $cmodel_nv_val"
    fi
    param="$param -i $input -o $output"
    param="$param -g $(($g_mirror<<2 | $g_rotation/90))"
    if [ "$g_scale_h" != "" ]; then
        param="$param -sx $g_scale_h"
    fi
    if [ "$g_scale_v" != "" ]; then
        param="$param -sy $g_scale_v"
    fi
    if [ "$g_pixel_just" == "1" ]; then
        param="$param -z"
    fi

    cmd="$exec_path $param -c"
    echo "$cmd"
    $cmd

    return $?
}

function generate_input_yuv {
    local input_cfg=$1
    local param=""
    local cmd=""
    local bit_depth=0
    local input_yuv=""
    local _srcyuv=""
    local _width=0
    local _height=0
    local _format=0
    local _name=""
    local _firstchar=""
    
    # Parse input cfg file.
    while read cfg_line; do
        _firstchar=${cfg_line:0:1}
        case "$_firstchar" in
            "#") continue;;     # comment
            ";") continue;;     # comment
            "")  continue;;
        esac
        _name=$(echo ${cfg_line} | cut -d' ' -f1)
        if [ "$_name" = "YUV_SRC_IMG" ]; then
            _srcyuv=$(echo ${cfg_line} | cut -d' ' -f2)
        elif [ "$_name" = "PICTURE_WIDTH" ]; then
            _width=$(echo ${cfg_line} | cut -d' ' -f2)
        elif [ "$_name" = "PICTURE_HEIGHT" ]; then
            _height=$(echo ${cfg_line} | cut -d' ' -f2)
        elif [ "$_name" = "IMG_FORMAT" ]; then
            _format=$(echo ${cfg_line} | cut -d' ' -f2)
        fi
    done < $input_cfg

    # Check the file exist
    echo "${yuv_dir}/${_srcyuv}"
    if [ -f "${yuv_dir}/${_srcyuv}" ]; then
        echo "File exist : ${yuv_dir}/${_srcyuv}"
        return 0
    fi

    # Copy lenear yuv
    input_yuv=$(echo $_srcyuv | sed 's/_tiled//g')
    cp "${yuv_dir}/${input_yuv}" ./

    if [ "$jpeg_12bit" == "1" ]; then
        bit_depth="16"
    else
        bit_depth="8"
    fi

    param="-i ${input_yuv} -o ${_srcyuv} --width=${_width} --height=${_height} --bit-depth=${bit_depth} --format=${_format} --map-type=1"

    # Execute tiled_mode program
    if [ "$simulation" == "true" ]; then
        gcc -o $tiled_exec ../../../fpga/board/ref-sw/tools/tiled_mode.c
    else
        gcc -o $tiled_exec tools/tiled_mode.c
    fi
    cmd="$tiled_exec $param"
    echo "$cmd"

    $cmd
    result=$?

    if [ "$result" != "0" ]; then
        return 1
    fi

    cp "${_srcyuv}" "${yuv_dir}/${_srcyuv}"
    return 0
}

# string get_stream_path(cfgfile)
function generate_golden_stream_path {
    local cfgfile=$1

    streamset_name=$(basename $streamset_path)
    temp=`expr match "$streamset_name" '.*\(_[0-9]*\.cmd\)'`
    if [ "$temp" != "" ]; then
        streamset_name="${streamset_name%_[0-9]*\.cmd}.cmd"
    fi
    echo "streamset_name: $streamset_name"
    path="${stream_dir}/$codec_dir/$streamset_name"
    cfgname=$(basename $cfgfile)
    if [ ! -d "$path" ]; then
        mkdir -p "$path"
    fi

    path="$path/${cfgname}_0.$ext"

    g_func_ret_str=$path
}

function read_cfg {
    local cfg=$1
    if [ -f ${cfg} ]; then
        while read line2; do
            # remove right comment
            line2=$(echo $line2 | tr -d '\n')
            line2=$(echo $line2 | tr -d '\r')
            line2="${line2%%\#*}"
    
            attr=$(echo $line2 | cut -d' ' -f1)
            attr=$(echo $attr | tr -d ' ')
    
            value=$(echo $line2 | cut -d' ' -f2)
            #echo "attr=$attr, value=$value"
    
            case "$attr" in
                PICTURE_WIDTH)  cfg_width="$value";;
                PICTURE_HEIGHT) cfg_height="$value";;
                IMG_FORMAT)     cfg_img_format="$value";;
                *) ;;
            esac
        done < $cfg
    else
        echo "File is not exist ($cfg)"
        exit 1
    fi
}

function get_jpeg_info {
    local format=""
    local cmd="${bin_dir}/${refc_exec} -i $1 -m 1" 

    if [ "$2" == "1" ]; then
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
            jpg_format="$value"
            break
        elif [ "$attr" == "Pictureconfig" ]; then
            value=${value%@*}    
            jpg_width=$(echo $value | cut -d'x' -f1)
            jpg_height=$(echo $value | cut -d'x' -f2)
        fi
    done < result.txt

    echo "$format"
}


function gen_valid_slice_height {
    local pic_width=$1
    local pic_height=$2
    local fmt=$3
    local default=$4
    local temp=0
    local sample_factor_x=1
    local sample_factor_y=1
    local mcu_height=8
    local slice_height=0
    local num_mcu_h=0

    if [ ! -z $default ] && [ "$default" != "-99" ]; then
        echo "$default"
        return
    fi

    case $fmt in
    0) # YUV420
       sample_factor_x=2
       sample_factor_y=2
       mcu_height=16;;
    1) # YUV422
       sample_factor_x=2
       mcu_height=8;;
    2) # YUV440
       sample_factor_y=2
       mcu_height=16;;
    4) # YUV400
       mcu_height=32;;
    *) ;;
    esac
    num_mcu_h=$(($pic_height/$mcu_height))

    mcu_num_x_fac=$(($sample_factor_x * 8))
    mcu_num_y_fac=$(($sample_factor_y * 8))
    mcu_num_w=$((($pic_width+($mcu_num_x_fac-1))/$mcu_num_x_fac))

    temp=$(gen_val 1 $(($pic_height / $mcu_height)) $default)
    while [ $temp -gt 0 ] ; do
        slice_height=$(($temp * $mcu_height))
        s_mcu_num_h=$((($slice_height+($mcu_num_y_fac-1))/$mcu_num_y_fac))
        total_mcu_in_slice=$(($mcu_num_w * $s_mcu_num_h))
        if [ $total_mcu_in_slice -lt 65536 ]; then
            # A value of restart interval should be less than 65536
            break
        fi
        temp=$(gen_val 1 $(($pic_height / $mcu_height)) $default)
    done

    if [ "$slice_height" == "$pic_height" ]; then
        slice_height=$(($slice_height - $mcu_height))
    fi

    echo "$slice_height"
}


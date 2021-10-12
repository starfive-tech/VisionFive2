//------------------------------------------------------------------------------
// File: Mixer.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include <stdio.h>
#include <errno.h>
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "main_helper.h"

#include <string.h>

const HevcCfgInfo hevcCfgInfo[MAX_CFG] = {
    //name                          min            max              default
    {"InputFile",                   0,              0,                      0},
    {"SourceWidth",                 0,              W4_MAX_ENC_PIC_WIDTH,   0},
    {"SourceHeight",                0,              W4_MAX_ENC_PIC_WIDTH,   0},
    {"InputBitDepth",               8,              10,                     8},
    {"FrameRate",                   0,              240,                    0}, // 5
    {"FrameSkip",                   0,              INT_MAX,                0},
    {"FramesToBeEncoded",           0,              INT_MAX,                0},
    {"IntraPeriod",                 0,              UI16_MAX,               0},
    {"DecodingRefreshType",         0,              2,                      1},
    {"DeriveLambdaWeight",          0,              1,                      0}, // 10
    {"GOPSize",                     1,              MAX_GOP_NUM,            1},
    {"EnIntraInInterSlice",         0,              1,                      1},
    {"IntraNxN",                    0,              1,                      1},
    {"EnCu8x8",                     0,              1,                      1},
    {"EnCu16x16",                   0,              1,                      1}, // 15
    {"EnCu32x32",                   0,              1,                      1},
    {"IntraTransSkip",              0,              2,                      1},
    {"ConstrainedIntraPred",        0,              1,                      0},
    {"IntraCtuRefreshMode",         0,              4,                      0},
    {"IntraCtuRefreshArg",          0,              UI16_MAX,               0}, // 20
    {"MaxNumMerge",                 0,              2,                      2},
    {"EnDynMerge",                  0,              1,                      1},
    {"EnTemporalMVP",               0,              1,                      1},
    {"ScalingList",                 0,              1,                      0}, 
    {"IndeSliceMode",               0,              1,                      0}, // 25
    {"IndeSliceArg",                0,              UI16_MAX,               0},
    {"DeSliceMode",                 0,              2,                      0},
    {"DeSliceArg",                  0,              UI16_MAX,               0},
    {"EnDBK",                       0,              1,                      1}, 
    {"EnSAO",                       0,              1,                      1}, // 30
    {"LFCrossSliceBoundaryFlag",    0,              1,                      1},
    {"BetaOffsetDiv2",             -6,              6,                      0},
    {"TcOffsetDiv2",               -6,              6,                      0},
    {"WaveFrontSynchro",            0,              1,                      0}, 
    {"LosslessCoding",              0,              1,                      0}, // 35
    {"UsePresetEncTools",           0,              3,                      0},
    {"NumTemporalLayers",           0,              7,                      1},
    {"GopPreset",                   0,             20,                      0},
    {"RateControl",                 0,              1,                      0}, 
    {"EncBitrate",                  0,              700000000,              0}, // 40
    {"TransBitrate",                0,              700000000,              0},
    {"InitialDelay",                10,             3000,                   3000},
    {"EnHvsQp",                     0,              1,                      1},
    {"CULevelRateControl",          0,              1,                      1}, 
    {"ConfWindSizeTop",             0,              W4_MAX_ENC_PIC_HEIGHT,  0}, // 45
    {"ConfWindSizeBot",             0,              W4_MAX_ENC_PIC_HEIGHT,  0},
    {"ConfWindSizeRight",           0,              W4_MAX_ENC_PIC_WIDTH,   0},
    {"ConfWindSizeLeft",            0,              W4_MAX_ENC_PIC_WIDTH,   0},
    {"HvsQpScaleDiv2",              0,              4,                      2},
    {"MinQp",                       0,              51,                     8}, // 50
    {"MaxQp",                       0,              51,                     51},
    {"MaxDeltaQp",                  0,              51,                     10},
    {"NumRoi",                      0,              MAX_ROI_NUMBER,         0},
    {"QP",                          0,              51,                     30},
    {"RoiDeltaQP",                  1,              51,                     3}, // 55
    {"IntraQpOffset",              -10,             10,                     0}, 
    {"InitBufLevelx8",              0,              8,                      1},
    {"BitAllocMode",                0,              2,                      0},
    {"FixedBitRatio%d",             1,              255,                    1},
    {"InternalBitDepth",            0,              10,                     0}, // 60
    {"EnUserDataSei",               0,              1,                      0},  
    {"UserDataEncTiming",           0,              1,                      0},
    {"UserDataSize",                0,              (1<<24) - 1,            1},
    {"UserDataPos",                 0,              1,                      0},
    {"EnRoi",                       0,              1,                      0}, // 65
    {"VuiParamFlags",               0,              INT_MAX,                0},  
    {"VuiAspectRatioIdc",           0,              255,                    0},  
    {"VuiSarSize",                  0,              INT_MAX,                0},  
    {"VuiOverScanAppropriate",      0,              1,                      0},  
    {"VideoSignal",                 0,              INT_MAX,                0}, // 70
    {"VuiChromaSampleLoc",          0,              INT_MAX,                0},  
    {"VuiDispWinLeftRight",         0,              INT_MAX,                0},  
    {"VuiDispWinTopBottom",         0,              INT_MAX,                0},  
    {"NumUnitsInTick",              0,              INT_MAX,                0},  
    {"TimeScale",                   0,              INT_MAX,                0}, // 75 
    {"NumTicksPocDiffOne",          0,              INT_MAX,                0},  
    {"EncAUD",                      0,              1,                      0},  
    {"EncEOS",                      0,              1,                      0},  
    {"EncEOB",                      0,              1,                      0},  
    {"CbQpOffset",                  -10,            10,                     0}, // 80
    {"CrQpOffset",                  -10,            10,                     0},  
    {"RcInitialQp",                 -1,              63,                     63},  
    {"EnNoiseReductionY",           0,              1,                      0},  
    {"EnNoiseReductionCb",          0,              1,                      0},  
    {"EnNoiseReductionCr",          0,              1,                      0}, // 85 
    {"EnNoiseEst",                  0,              1,                      1},  
    {"NoiseSigmaY",                 0,              255,                    0},  
    {"NoiseSigmaCb",                0,              255,                    0},  
    {"NoiseSigmaCr",                0,              255,                    0},  
    {"IntraNoiseWeightY",           0,              31,                     7}, // 90 
    {"IntraNoiseWeightCb",          0,              31,                     7},  
    {"IntraNoiseWeightCr",          0,              31,                     7},  
    {"InterNoiseWeightY",           0,              31,                     4},  
    {"InterNoiseWeightCb",          0,              31,                     4},  
    {"InterNoiseWeightCr",          0,              31,                     4}, // 95 
    {"IntraMinQp",                  0,              51,                     8},  
    {"IntraMaxQp",                  0,              51,                     51}, 
    {"MdFlag0",                     0,              1,                      0},  
    {"MdFlag1",                     0,              1,                      0}, 
    {"MdFlag2",                     0,              1,                      0}, // 100 
    {"EnSmartBackground",           0,              1,                      0},
    {"ThrPixelNumCnt",              0,              63,                     0}, 
    {"ThrMean0",                    0,              255,                    5},  
    {"ThrMean1",                    0,              255,                    5}, 
    {"ThrMean2",                    0,              255,                    5}, // 105 
    {"ThrMean3",                    0,              255,                    5}, 
    {"MdQpY",                       0,              51,                     30},  
    {"MdQpC",                       0,              51,                     30}, 
    {"ThrDcY0",                     0,              UI16_MAX,               2},  
    {"ThrDcC0",                     0,              UI16_MAX,               2}, // 110
    {"ThrDcY1",                     0,              UI16_MAX,               2}, 
    {"ThrDcC1",                     0,              UI16_MAX,               2}, 
    {"ThrDcY2",                     0,              UI16_MAX,               2},  
    {"ThrDcC2",                     0,              UI16_MAX,               2}, 
    {"ThrAcNumY0",                  0,              63,                     12},// 115  
    {"ThrAcNumC0",                  0,              15,                     3}, 
    {"ThrAcNumY1",                  0,              255,                    51},  
    {"ThrAcNumC1",                  0,              63,                     12}, 
    {"ThrAcNumY2",                  0,              1023,                   204},  
    {"ThrAcNumC2",                  0,              255,                    51},// 120 
    {"UseAsLongTermRefPeriod",      0,              INT_MAX,                0}, 
    {"RefLongTermPeriod",           0,              INT_MAX,                0}, 
    {"EnCtuMode",                   0,              1,                      0}, 
    {"EnCtuQp",                     0,              1,                      0}, 
    {"CropXPos",                    0,              W4_MAX_ENC_PIC_WIDTH,   0}, // 125
    {"CropYPos",                    0,              W4_MAX_ENC_PIC_HEIGHT,  0}, 
    {"CropXSize",                   0,              W4_MAX_ENC_PIC_WIDTH,   0}, 
    {"CropYSize",                   0,              W4_MAX_ENC_PIC_HEIGHT,  0}, 
    {"EncodeRbspVui",               0,              1,                      0}, 
    {"RbspVuiSize",                 0,              INT_MAX,                0}, // 130
    {"EncodeRbspHrdInVps",          0,              1,                      0}, 
    {"EncodeRbspHrdInVui",          0,              1,                      0}, 
    {"RbspHrdSize",                 0,              INT_MAX,                0}, 
    {"EnPrefixSeiData",             0,              1,                      0}, 
    {"PrefixSeiDataSize",           0,              UI16_MAX,               0}, // 135
    {"PrefixSeiTimingFlag",         0,              1,                      0}, 
    {"EnSuffixSeiData",             0,              1,                      0}, 
    {"SuffixSeiDataSize",           0,              UI16_MAX,               0}, 
    {"SuffixSeiTimingFlag",         0,              1,                      0}, 
    {"EnReportMvCol",               0,              1,                      0}, // 140
    {"EnReportDistMap",             0,              1,                      0}, 
    {"EnReportBitInfo",             0,              1,                      0}, 
    {"EnReportFrameDist",           0,              1,                      0}, 
    {"EnReportQpHisto",             0,              1,                      0}, 
    {"BitstreamFile",               0,              0,                      0}, // 145
    {"EnCustomVpsHeader",           0,              1,                      0},
    {"EnCustomSpsHeader",           0,              1,                      0},
    {"EnCustomPpsHeader",           0,              1,                      0},
    {"CustomVpsPsId",               0,              15,                     0},
    {"CustomSpsPsId",               0,              15,                     0}, // 150
    {"CustomSpsActiveVpsId",        0,              15,                     0},
    {"CustomPpsActiveSpsId",        0,              15,                     0},
    {"CustomVpsIntFlag",            0,              1,                      1},
    {"CustomVpsAvailFlag",          0,              1,                      1},
    {"CustomVpsMaxLayerMinus1",     0,              62,                     0}, // 155
    {"CustomVpsMaxSubLayerMinus1",  0,              6,                      0},
    {"CustomVpsTempIdNestFlag",     0,              1,                      0},
    {"CustomVpsMaxLayerId",         0,              31,                     0},
    {"CustomVpsNumLayerSetMinus1",  0,              2,                      0},
    {"CustomVpsExtFlag",            0,              1,                      0}, // 160
    {"CustomVpsExtDataFlag",        0,              1,                      0},
    {"CustomVpsSubOrderInfoFlag",   0,              1,                      0},
    {"CustomSpsSubOrderInfoFlag",   0,              1,                      0},
    {"CustomVpsLayerId0",           0,              0xFFFFFFFF,             0},
    {"CustomVpsLayerId1",           0,              0xFFFFFFFF,             0}, // 165
    {"CustomSpsLog2MaxPocMinus4",   0,              12,                     4}, 
    {"EnForcedIDRHeader",           0,              1,                      0}, 

// newly added for WAVE520
    {"EncMonochrome",               0,              1,                      0}, 
    {"StrongIntraSmoothing",        0,              1,                      1}, 
    {"RoiAvgQP",                    0,              51,                     0}, // 170
    {"WeightedPred",                0,              1,                      0}, 
    {"EnBgDetect",                  0,              1,                      0}, 
    {"BgThDiff",                    0,              255,                    8}, 
    {"BgThMeanDiff",                0,              255,                    1}, 
    {"BgLambdaQp",                  0,              51,                     32},// 175 
    {"BgDeltaQp",                   -16,            15,                     3}, 
    {"TileNumColumns",              1,              6,                      1}, 
    {"TileNumRows",                 1,              6,                      1}, 
    {"TileUniformSpace",            0,              1,                      1}, 
    {"EnLambdaMap",                 0,              1,                      0}, // 180
    {"EnCustomLambda",              0,              1,                      0}, 
    {"EnCustomMD",                  0,              1,                      0}, 
    {"PU04DeltaRate",               -128,           127,                    0}, 
    {"PU08DeltaRate",               -128,           127,                    0}, 
    {"PU16DeltaRate",               -128,           127,                    0}, // 185
    {"PU32DeltaRate",               -128,           127,                    0}, 
    {"PU04IntraPlanarDeltaRate",    -128,           127,                    0}, 
    {"PU04IntraDcDeltaRate",        -128,           127,                    0}, 
    {"PU04IntraAngleDeltaRate",     -128,           127,                    0}, 
    {"PU08IntraPlanarDeltaRate",    -128,           127,                    0}, // 190
    {"PU08IntraDcDeltaRate",        -128,           127,                    0}, 
    {"PU08IntraAngleDeltaRate",     -128,           127,                    0}, 
    {"PU16IntraPlanarDeltaRate",    -128,           127,                    0}, 
    {"PU16IntraDcDeltaRate",        -128,           127,                    0},
    {"PU16IntraAngleDeltaRate",     -128,           127,                    0}, // 195
    {"PU32IntraPlanarDeltaRate",    -128,           127,                    0},
    {"PU32IntraDcDeltaRate",        -128,           127,                    0},
    {"PU32IntraAngleDeltaRate",     -128,           127,                    0}, 
    {"CU08IntraDeltaRate",          -128,           127,                    0},
    {"CU08InterDeltaRate",          -128,           127,                    0}, // 200
    {"CU08MergeDeltaRate",          -128,           127,                    0},
    {"CU16IntraDeltaRate",          -128,           127,                    0},
    {"CU16InterDeltaRate",          -128,           127,                    0}, 
    {"CU16MergeDeltaRate",          -128,           127,                    0},
    {"CU32IntraDeltaRate",          -128,           127,                    0}, // 205
    {"CU32InterDeltaRate",          -128,           127,                    0},
    {"CU32MergeDeltaRate",          -128,           127,                    0}, 
    {"DisableCoefClear",            0,              1,                      0}, 
    {"EnModeMap",                   0,              3,                      0},
    {"EnTemporalLayerQp",           0,              1,                      0},
    {"TID_0_Qp",                    0,              51,                     30},
    {"TID_1_Qp",                    0,              51,                     33},
    {"TID_2_Qp",                    0,              51,                     36},
    {"TID_0_Period",                2,              128,                    60},
};

//------------------------------------------------------------------------------
// ENCODE PARAMETER PARSE FUNCSIONS
//------------------------------------------------------------------------------
// Parameter parsing helper
static int GetValue(osal_file_t fp, char *para, char *value)
{
	char lineStr[256];
	char paraStr[256];
	osal_fseek(fp, 0, SEEK_SET);
	
	while (1) {
		if (fgets(lineStr, 256, fp) == NULL)
			return 0;
		sscanf(lineStr, "%s %s", paraStr, value);
		if (paraStr[0] != ';') {
			if (strcmp(para, paraStr) == 0)
				return 1;
		}
	}
} 

// Parse "string number number ..." at most "num" numbers
// e.g. SKIP_PIC_NUM 1 3 4 5
static int GetValues(osal_file_t fp, char *para, int *values, int num)
{
    char line[1024];

    osal_fseek(fp, 0, SEEK_SET);

    while (1)
    {
        int  i;
        char *str;

        if (fgets(line, sizeof(line)-1, fp) == NULL)
            return 0;

        // empty line
        if ((str = strtok(line, " ")) == NULL)
            continue;

        if(strcmp(str, para) != 0)
            continue;

        for (i=0; i<num; i++)
        {
            if ((str = strtok(NULL, " ")) == NULL)
                return 1;
            if (!isdigit((Int32)str[0]))
                return 1;
            values[i] = atoi(str);
        }
        return 1;
    }
}

int parseMp4CfgFile(ENC_CFG *pEncCfg, char *FileName)
{
	osal_file_t fp;
    char sValue[1024];
    int  ret = 0;

    fp = osal_fopen(FileName, "rt");
    if (fp == NULL) {
        return ret;
    }

	if (GetValue(fp, "YUV_SRC_IMG", sValue) == 0)
        goto __end_parseMp4CfgFile;
    else
		strcpy(pEncCfg->SrcFileName, sValue);
	
	if (GetValue(fp, "FRAME_NUMBER_ENCODED", sValue) == 0)
        goto __end_parseMp4CfgFile;
    pEncCfg->NumFrame = atoi(sValue);
	if (GetValue(fp, "PICTURE_WIDTH", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->PicX = atoi(sValue);
	if (GetValue(fp, "PICTURE_HEIGHT", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->PicY = atoi(sValue);
	if (GetValue(fp, "FRAME_RATE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	{
		double frameRate;
        int  timeRes, timeInc;
#ifdef ANDROID
		frameRate = atoi(sValue);
#else
		frameRate = atof(sValue);
#endif
        timeInc = 1;
        while ((int)frameRate != frameRate) {
            timeInc *= 10;
            frameRate *= 10;
        }
        timeRes = (int) frameRate;
        // divide 2 or 5
        if (timeInc%2 == 0 && timeRes%2 == 0) {
            timeInc /= 2; 
            timeRes /= 2;
        }
        if (timeInc%5 == 0 && timeRes%5 == 0) {
            timeInc /= 5;
            timeRes /= 5;
        }
        if (timeRes == 2997 && timeInc == 100) {
            timeRes = 30000;
            timeInc = 1001;
        }
        pEncCfg->FrameRate = (timeInc - 1) << 16;
        pEncCfg->FrameRate |= timeRes;
    }
	if (GetValue(fp, "VERSION_ID", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->VerId = atoi(sValue);
	if (GetValue(fp, "DATA_PART_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->DataPartEn = atoi(sValue);
	if (GetValue(fp, "REV_VLC_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->RevVlcEn = atoi(sValue);

	if (GetValue(fp, "INTRA_DC_VLC_THRES", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->IntraDcVlcThr = atoi(sValue);
	if (GetValue(fp, "SHORT_VIDEO", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->ShortVideoHeader = atoi(sValue);
	if (GetValue(fp, "ANNEX_I_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->AnnexI = atoi(sValue);
	if (GetValue(fp, "ANNEX_J_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->AnnexJ = atoi(sValue);
	if (GetValue(fp, "ANNEX_K_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->AnnexK = atoi(sValue);
	if (GetValue(fp, "ANNEX_T_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->AnnexT = atoi(sValue);

	if (GetValue(fp, "VOP_QUANT_SCALE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->VopQuant = atoi(sValue);
	if (GetValue(fp, "GOP_PIC_NUMBER", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->GopPicNum = atoi(sValue);
	if (GetValue(fp, "SLICE_MODE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->SliceMode = atoi(sValue);
	if (GetValue(fp, "SLICE_SIZE_MODE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->SliceSizeMode = atoi(sValue);
	if (GetValue(fp, "SLICE_SIZE_NUMBER", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->SliceSizeNum = atoi(sValue);	
	
	if (GetValue(fp, "RATE_CONTROL_ENABLE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->RcEnable = atoi(sValue);
	if (GetValue(fp, "BIT_RATE_KBPS", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->RcBitRate = atoi(sValue);
	if (GetValue(fp, "DELAY_IN_MS", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->RcInitDelay = atoi(sValue);
	if (GetValue(fp, "VBV_BUFFER_SIZE", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->RcBufSize = atoi(sValue);
	if (GetValue(fp, "INTRA_MB_REFRESH", sValue) == 0)
        goto __end_parseMp4CfgFile;
	pEncCfg->IntraRefreshNum = atoi(sValue);

    pEncCfg->ConscIntraRefreshEnable = 0;
    if (pEncCfg->IntraRefreshNum > 0)
    {
        if (GetValue(fp, "CONSC_INTRA_REFRESH_EN", sValue) == 0)
            pEncCfg->ConscIntraRefreshEnable = 0;
        else
            pEncCfg->ConscIntraRefreshEnable = atoi(sValue);
    }
	if (GetValue(fp, "CONST_INTRA_QP_EN", sValue) == 0)
		pEncCfg->ConstantIntraQPEnable = 0;
	else
		pEncCfg->ConstantIntraQPEnable = atoi(sValue);
	if (GetValue(fp, "CONST_INTRA_QP", sValue) == 0)
        pEncCfg->RCIntraQP = 0;
	else
		pEncCfg->RCIntraQP = atoi(sValue);

	if (GetValue(fp, "HEC_ENABLE", sValue) == 0)
		pEncCfg->HecEnable = 0;
	else
		pEncCfg->HecEnable = atoi(sValue);

	if (GetValue(fp, "SEARCH_RANGE", sValue) == 0)
		pEncCfg->SearchRange = 0;
	else
		pEncCfg->SearchRange = atoi(sValue);
	if (GetValue(fp, "ME_USE_ZERO_PMV", sValue) == 0)
		pEncCfg->MeUseZeroPmv = 0;
	else
		pEncCfg->MeUseZeroPmv = atoi(sValue);
	if (GetValue(fp, "WEIGHT_INTRA_COST", sValue) == 0)
        pEncCfg->intraCostWeight = 0;
	else
		pEncCfg->intraCostWeight = atoi(sValue);

	if (GetValue(fp, "MAX_QP_SET_ENABLE", sValue) == 0)
		pEncCfg->MaxQpSetEnable= 0;
	else
		pEncCfg->MaxQpSetEnable = atoi(sValue);
	if (GetValue(fp, "MAX_QP", sValue) == 0)
        pEncCfg->MaxQp = 0;
	else
		pEncCfg->MaxQp = atoi(sValue);
	if (GetValue(fp, "GAMMA_SET_ENABLE", sValue) == 0)
        pEncCfg->GammaSetEnable = 0;
	else
		pEncCfg->GammaSetEnable = atoi(sValue);
	if (GetValue(fp, "GAMMA", sValue) == 0)
        pEncCfg->Gamma = 0;
	else
		pEncCfg->Gamma = atoi(sValue);

	if (GetValue(fp, "RC_INTERVAL_MODE", sValue) == 0)
        pEncCfg->rcIntervalMode = 0;
	else
		pEncCfg->rcIntervalMode = atoi(sValue);

	if (GetValue(fp, "RC_MB_INTERVAL", sValue) == 0)
        pEncCfg->RcMBInterval = 0;
	else
		pEncCfg->RcMBInterval = atoi(sValue);

    ret = 1; /* Success */

__end_parseMp4CfgFile:
	osal_fclose(fp);
	return ret;
}





int parseAvcCfgFile(ENC_CFG *pEncCfg, char *FileName)
{
    osal_file_t fp;
    char sValue[1024];
    int  ret = 0;

    fp = osal_fopen(FileName, "r");
    if (fp == NULL) {
        return 0;
    }

    if (GetValue(fp, "YUV_SRC_IMG", sValue) == 0)
        goto __end_parseAvcCfgFile;
    else
        strcpy(pEncCfg->SrcFileName, sValue);

    if (GetValue(fp, "FRAME_NUMBER_ENCODED", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->NumFrame = atoi(sValue);
    if (GetValue(fp, "PICTURE_WIDTH", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->PicX = atoi(sValue);
    if (GetValue(fp, "PICTURE_HEIGHT", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->PicY = atoi(sValue);
    if (GetValue(fp, "FRAME_RATE", sValue) == 0)
        goto __end_parseAvcCfgFile;
    {
        double frameRate;
        int  timeRes, timeInc;

#ifdef ANDROID
        frameRate = atoi(sValue);
#else
        frameRate = atof(sValue);
#endif

        timeInc = 1;
        while ((int)frameRate != frameRate) {
            timeInc *= 10;
            frameRate *= 10;
        }
        timeRes = (int) frameRate;
        // divide 2 or 5
        if (timeInc%2 == 0 && timeRes%2 == 0) {
            timeInc /= 2; 
            timeRes /= 2;
        }
        if (timeInc%5 == 0 && timeRes%5 == 0) {
            timeInc /= 5; 
            timeRes /= 5;
        }

        if (timeRes == 2997 && timeInc == 100) {
            timeRes = 30000; 
timeInc = 1001;
        }
        pEncCfg->FrameRate = (timeInc - 1) << 16;
        pEncCfg->FrameRate |= timeRes;
    }
    if (GetValue(fp, "CONSTRAINED_INTRA", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->ConstIntraPredFlag = atoi(sValue);
    if (GetValue(fp, "DISABLE_DEBLK", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->DisableDeblk = atoi(sValue);
    if (GetValue(fp, "DEBLK_ALPHA", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->DeblkOffsetA = atoi(sValue);
    if (GetValue(fp, "DEBLK_BETA", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->DeblkOffsetB = atoi(sValue);
    if (GetValue(fp, "CHROMA_QP_OFFSET", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->ChromaQpOffset = atoi(sValue);

    if (GetValue(fp, "LEVEL", sValue) == 0)
    {
        pEncCfg->level = 0;//note : 0 means auto calculation.
    }
    else
    {
        pEncCfg->level = atoi(sValue);
        if (pEncCfg->level<0 || pEncCfg->level>51)
            goto __end_parseAvcCfgFile;
    }

    if (GetValue(fp, "PIC_QP_Y", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->PicQpY = atoi(sValue);
    if (GetValue(fp, "GOP_PIC_NUMBER", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->GopPicNum = atoi(sValue);
    if (GetValue(fp, "IDR_INTERVAL", sValue) == 0)
        pEncCfg->IDRInterval = 0;
    else
        pEncCfg->IDRInterval = atoi(sValue);
    if (GetValue(fp, "SLICE_MODE", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->SliceMode = atoi(sValue);
    if (GetValue(fp, "SLICE_SIZE_MODE", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->SliceSizeMode = atoi(sValue);
    if (GetValue(fp, "SLICE_SIZE_NUMBER", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->SliceSizeNum = atoi(sValue);	
    if (GetValue(fp, "AUD_ENABLE", sValue) == 0)
        pEncCfg->aud_en = 0;
    else
        pEncCfg->aud_en = atoi(sValue);	

    /**
    * Error Resilience
    */
    // Intra Cost Weight : not mandatory. default zero
    if (GetValue(fp, "WEIGHT_INTRA_COST", sValue) == 0)
        pEncCfg->intraCostWeight = 0;
    else
        pEncCfg->intraCostWeight = atoi(sValue);

    /**
    * CROP information
    */
    if (GetValue(fp, "FRAME_CROP_LEFT", sValue) == 0)
        pEncCfg->frameCropLeft = 0;
    else
        pEncCfg->frameCropLeft = atoi(sValue);

    if (GetValue(fp, "FRAME_CROP_RIGHT", sValue) == 0)
        pEncCfg->frameCropRight = 0;
    else
        pEncCfg->frameCropRight = atoi(sValue);

    if (GetValue(fp, "FRAME_CROP_TOP", sValue) == 0)
        pEncCfg->frameCropTop = 0;
    else
        pEncCfg->frameCropTop = atoi(sValue);

    if (GetValue(fp, "FRAME_CROP_BOTTOM", sValue) == 0)
        pEncCfg->frameCropBottom = 0;
    else
        pEncCfg->frameCropBottom = atoi(sValue);

    /**
    * ME Option
    */

    if (GetValue(fp, "ME_USE_ZERO_PMV", sValue) == 0)
        pEncCfg->MeUseZeroPmv = 0;
    else
        pEncCfg->MeUseZeroPmv = atoi(sValue);

    if (GetValue(fp, "ME_BLK_MODE_ENABLE", sValue) == 0)
        pEncCfg->MeBlkModeEnable = 0;
    else
        pEncCfg->MeBlkModeEnable = atoi(sValue);

    if (GetValue(fp, "RATE_CONTROL_ENABLE", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->RcEnable = atoi(sValue);
    if (GetValue(fp, "BIT_RATE_KBPS", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->RcBitRate = atoi(sValue);
    if (GetValue(fp, "DELAY_IN_MS", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->RcInitDelay = atoi(sValue);

    if (GetValue(fp, "VBV_BUFFER_SIZE", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->RcBufSize = atoi(sValue);
    if (GetValue(fp, "INTRA_MB_REFRESH", sValue) == 0)
        goto __end_parseAvcCfgFile;
    pEncCfg->IntraRefreshNum = atoi(sValue);

    pEncCfg->ConscIntraRefreshEnable = 0;
    if (pEncCfg->IntraRefreshNum > 0)
    {
        if (GetValue(fp, "CONSC_INTRA_REFRESH_EN", sValue) == 0)
            pEncCfg->ConscIntraRefreshEnable = 0;
        else
            pEncCfg->ConscIntraRefreshEnable = atoi(sValue);
    }

    if (GetValue(fp, "FRAME_SKIP_DISABLE", sValue) == 0)
        pEncCfg->frameSkipDisable = 0;
    else
        pEncCfg->frameSkipDisable = atoi(sValue);

    if (GetValue(fp, "CONST_INTRAQP_ENABLE", sValue) == 0)
        pEncCfg->ConstantIntraQPEnable = 0;
    else
        pEncCfg->ConstantIntraQPEnable = atoi(sValue);

    if (GetValue(fp, "RC_INTRA_QP", sValue) == 0)
        pEncCfg->RCIntraQP = 0;
    else
        pEncCfg->RCIntraQP = atoi(sValue);
    if (GetValue(fp, "MAX_QP_SET_ENABLE", sValue) == 0)
        pEncCfg->MaxQpSetEnable= 0;
    else
        pEncCfg->MaxQpSetEnable = atoi(sValue);

    if (GetValue(fp, "MAX_QP", sValue) == 0)
        pEncCfg->MaxQp = 0;
    else
        pEncCfg->MaxQp = atoi(sValue);

    if (GetValue(fp, "MIN_QP_SET_ENABLE", sValue) == 0)
        pEncCfg->MinQpSetEnable= 0;
    else
        pEncCfg->MinQpSetEnable = atoi(sValue);
    if (GetValue(fp, "MIN_QP", sValue) == 0)
        pEncCfg->MinQp = 0;
    else
        pEncCfg->MinQp = atoi(sValue);

    if (GetValue(fp, "MAX_DELTA_QP_SET_ENABLE", sValue) == 0)
        pEncCfg->MaxDeltaQpSetEnable= 0;
    else
        pEncCfg->MaxDeltaQpSetEnable = atoi(sValue);
    if (GetValue(fp, "MAX_DELTA_QP", sValue) == 0)
        pEncCfg->MaxDeltaQp = 0;
    else
        pEncCfg->MaxDeltaQp = atoi(sValue);

    if (GetValue(fp, "MIN_DELTA_QP_SET_ENABLE", sValue) == 0)
        pEncCfg->MinDeltaQpSetEnable= 0;
    else
        pEncCfg->MinDeltaQpSetEnable = atoi(sValue);
    if (GetValue(fp, "MIN_DELTA_QP", sValue) == 0)
        pEncCfg->MinDeltaQp = 0;
    else
        pEncCfg->MinDeltaQp = atoi(sValue);

    if (GetValue(fp, "GAMMA_SET_ENABLE", sValue) == 0)
        pEncCfg->GammaSetEnable = 0;
    else
        pEncCfg->GammaSetEnable = atoi(sValue);
    if (GetValue(fp, "GAMMA", sValue) == 0)
        pEncCfg->Gamma = 0;
    else
        pEncCfg->Gamma = atoi(sValue);
    /* CODA960 features */
    if (GetValue(fp, "RC_INTERVAL_MODE", sValue) == 0)
        pEncCfg->rcIntervalMode = 0;
    else
        pEncCfg->rcIntervalMode = atoi(sValue);

    if (GetValue(fp, "RC_MB_INTERVAL", sValue) == 0)
        pEncCfg->RcMBInterval = 0;
    else
        pEncCfg->RcMBInterval = atoi(sValue);
    /***************************************/
    if (GetValue(fp, "RC_INTERVAL_MODE", sValue) == 0)
        pEncCfg->rcIntervalMode = 0;
    else
        pEncCfg->rcIntervalMode = atoi(sValue);

    if (GetValue(fp, "RC_MB_INTERVAL", sValue) == 0)
        pEncCfg->RcMBInterval = 0;
    else
        pEncCfg->RcMBInterval = atoi(sValue);
    if (GetValue(fp, "SEARCH_RANGE", sValue) == 0)
        pEncCfg->SearchRange = 0;
    else
        pEncCfg->SearchRange = atoi(sValue);


    osal_memset(pEncCfg->skipPicNums, 0, sizeof(pEncCfg->skipPicNums));
    GetValues(fp, "SKIP_PIC_NUMS", pEncCfg->skipPicNums, sizeof(pEncCfg->skipPicNums));    


    /**
    * VUI Parameter
    */
    if (GetValue(fp, "VUI_PARAMETERS_PRESENT_FLAG", sValue) == 0)
        pEncCfg->VuiPresFlag = 0;
    else 
        pEncCfg->VuiPresFlag = atoi(sValue);

    if (pEncCfg->VuiPresFlag == 1) {

        if (GetValue(fp, "VIDEO_SIGNAL_TYPE_PRESENT_FLAG", sValue) == 0)
            pEncCfg->VideoSignalTypePresFlag = 0;
        else
            pEncCfg->VideoSignalTypePresFlag = atoi(sValue);

        if (pEncCfg->VideoSignalTypePresFlag) {
            if (GetValue(fp, "VIDEO_FORMAT", sValue) == 0)
                pEncCfg->VideoFormat = 5;
            else
                pEncCfg->VideoFormat = atoi(sValue);

            if (GetValue(fp, "VIDEO_FULL_RANGE_FLAG", sValue) == 0)
                pEncCfg->VideoFullRangeFlag = 0;
            else 
                pEncCfg->VideoFullRangeFlag = atoi(sValue);

            if (GetValue(fp, "COLOUR_DESCRIPTION_PRESENT_FLAG", sValue) == 0)
                pEncCfg->ColourDescripPresFlag = 1;
            else
                pEncCfg->ColourDescripPresFlag = atoi(sValue);

            if (pEncCfg->ColourDescripPresFlag) {
                if (GetValue(fp, "COLOUR_PRIMARIES", sValue) == 0)
                    pEncCfg->ColourPrimaries = 1;
                else
                    pEncCfg->ColourPrimaries = atoi(sValue);

                if (GetValue(fp, "TRANSFER_CHARACTERISTICS", sValue) == 0)
                    pEncCfg->TransferCharacteristics = 2;
                else
                    pEncCfg->TransferCharacteristics = atoi(sValue);

                if (GetValue(fp, "MATRIX_COEFFICIENTS", sValue) == 0)
                    pEncCfg->MatrixCoeff = 2;
                else
                    pEncCfg->MatrixCoeff = atoi(sValue);
            }
        }
    }

    ret = 1; /* Success */
__end_parseAvcCfgFile:
    osal_fclose(fp);
    return ret;
}

static int HEVC_GetStringValue(
    osal_file_t fp,
    char* para,
    char* value
    )
{
    int pos = 0;
    char* token = NULL;
    char lineStr[256] = {0, };
    char valueStr[256] = {0, };
    osal_fseek(fp, 0, SEEK_SET);

    while (1) {
        if ( fgets(lineStr, 256, fp) == NULL ) {
            return 0;
        }

        if( (lineStr[0] == '#') || (lineStr[0] == ';') || (lineStr[0] == ':') ) { // check comment
            continue;
        }

        token = strtok(lineStr, ": "); // parameter name is separated by ' ' or ':'
        if( token != NULL ) {
            if ( strcasecmp(para, token) == 0) { // check parameter name
                token = strtok(NULL, ":\r\n");
                osal_memcpy( valueStr, token, strlen(token) );
                while( valueStr[pos] == ' ' ) { // check space
                    pos++;
                }

                strcpy(value, &valueStr[pos]);
                return 1;
            }
            else {
                continue;
            }
        }
        else {
            continue;
        }
    }
}

static int HEVC_GetValue(
    osal_file_t fp,
    HevcCfgName cfgName,
    int* value
    )
{
    int iValue;
    char sValue[256] = {0, };
    
    if(HEVC_GetStringValue(fp, hevcCfgInfo[cfgName].name, sValue) == 1) {
        iValue = atoi(sValue);
        if( (iValue >= hevcCfgInfo[cfgName].min) && (iValue <= hevcCfgInfo[cfgName].max) ) { // Check min, max
            *value = iValue;
            return 1;
        }
        else {
            VLOG(ERR, "CFG file error : %s value is not available. ( min = %d, max = %d)\n", hevcCfgInfo[cfgName].name, hevcCfgInfo[cfgName].min, hevcCfgInfo[cfgName].max);
            return 0;
        }
    }
    else {
        *value = hevcCfgInfo[cfgName].def;
        return 1;
    }
}


static int HEVC_SetGOPInfo(
    char* lineStr,
    CustomGopPicParam* gopPicParam,
    int* gopPicLambda,
    int useDeriveLambdaWeight,
    int intraQp
    )
{
    int numParsed;
    char sliceType;
    double lambda;

    osal_memset(gopPicParam, 0, sizeof(CustomGopPicParam));
    *gopPicLambda = 0;

    numParsed = sscanf(lineStr, "%c %d %d %lf %d %d %d", 
        &sliceType, &gopPicParam->pocOffset, &gopPicParam->picQp, &lambda,
        &gopPicParam->temporalId, &gopPicParam->refPocL0, &gopPicParam->refPocL1);

#if FLOATING_POINT_LAMBDA == 0
    lambda = (int)(lambda * 256); // * 2^10 
#endif

    if (sliceType=='I') {
        gopPicParam->picType = PIC_TYPE_I;
    }
    else if (sliceType=='P') {
        gopPicParam->picType = PIC_TYPE_P;
        if (numParsed == 6)
            gopPicParam->numRefPicL0 = 2;
        else
            gopPicParam->numRefPicL0 = 1;
    }
    else if (sliceType=='B') {
        gopPicParam->picType = PIC_TYPE_B;
    }
    else {
        return 0;
    }
    if (sliceType=='P' && numParsed != 6) {
        return 0;
    }
    if (sliceType=='B' && numParsed != 7) {
        return 0;
    }
    if (gopPicParam->temporalId < 0) {
        return 0;
    }

    gopPicParam->picQp = gopPicParam->picQp + intraQp;

    if (useDeriveLambdaWeight == 0) {
        *gopPicLambda = (int)(lambda * LAMBDA_SCALE_FACTOR);
    }
    else {
        *gopPicLambda = 0;
    }
    
    return 1;
}

int parseRoiCtuModeParam(
    char* lineStr,
    VpuRect* roiRegion,
    int* roiLevel,
    int picX,
    int picY
    )
{
    int numParsed;

    osal_memset(roiRegion, 0, sizeof(VpuRect));
    *roiLevel = 0;

    numParsed = sscanf(lineStr, "%d %d %d %d %d", 
        &roiRegion->left, &roiRegion->right, &roiRegion->top, &roiRegion->bottom,
        roiLevel);

    if (numParsed != 5) {
        return 0;
    }
    if (*roiLevel > MAX_ROI_LEVEL) {
        return 0;
    }
    if (roiRegion->left < 0 || roiRegion->top < 0) {
        return 0;
    }
    if (roiRegion->left > (Uint32)((picX + CTB_SIZE - 1) >> LOG2_CTB_SIZE) || \
        roiRegion->top > (Uint32)((picY + CTB_SIZE - 1) >> LOG2_CTB_SIZE)) {
        return 0;
    }
    if (roiRegion->right > (Uint32)((picX + CTB_SIZE - 1) >> LOG2_CTB_SIZE) || \
        roiRegion->bottom > (Uint32)((picY + CTB_SIZE - 1) >> LOG2_CTB_SIZE)) {
        return 0;
    }
    if (roiRegion->left > roiRegion->right) {
        return 0;
    }
    if (roiRegion->top > roiRegion->bottom) {
        return 0;
    }

    return 1;
}

int parseHevcCfgFile(
    ENC_CFG *pEncCfg, 
    char *FileName
    )

{
    osal_file_t fp;
    char sValue[256] = {0, };
    char tempStr[256] = {0, };
    int iValue = 0, ret = 0, i = 0;
    int intra8=0, intra16=0, intra32=0, frameSkip=0, dynamicMergeEnable; // temp value
    UNREFERENCED_PARAMETER(frameSkip);
    
    fp = osal_fopen(FileName, "r");
    if (fp == NULL) {
        VLOG(ERR, "file open err : %s, errno(%d)", FileName, errno);
        return ret;
    }

    if (HEVC_GetStringValue(fp, hevcCfgInfo[BITSTREAM_FILE].name, sValue) != 0)
        strcpy(pEncCfg->BitStreamFileName, sValue);

    if (HEVC_GetStringValue(fp, hevcCfgInfo[INPUT_FILE].name, sValue) == 0)
        goto __end_parse;
    else
        strcpy(pEncCfg->SrcFileName, sValue);

    if (HEVC_GetValue(fp, SOURCE_WIDTH, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.picX = iValue;
    if (HEVC_GetValue(fp, SOURCE_HEIGHT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.picY = iValue;
    if (HEVC_GetValue(fp, FRAMES_TO_BE_ENCODED, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->NumFrame = iValue;
    if (HEVC_GetValue(fp, INPUT_BIT_DEPTH, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->SrcBitDepth = iValue; // BitDepth == 8 ? HEVC_PROFILE_MAIN : HEVC_PROFILE_MAIN10

    if (HEVC_GetValue(fp, INTERNAL_BITDEPTH, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.internalBitDepth   = iValue;

    if (HEVC_GetValue(fp, LOSSLESS_CODING, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.losslessEnable = iValue;
    if (HEVC_GetValue(fp, CONSTRAINED_INTRA_PRED, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.constIntraPredFlag = iValue;
    if (HEVC_GetValue(fp, DECODING_REFRESH_TYPE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.decodingRefreshType = iValue;

    // RoiDetaQp
    if (HEVC_GetValue(fp, ROI_DELTA_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.ctuOptParam.roiDeltaQp = iValue;

    // IntraQpOffset
    if (HEVC_GetValue(fp, INTRA_QP_OFFSET, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraQpOffset = iValue;

    // Initial Buf level x 8
    if (HEVC_GetValue(fp, INIT_BUF_LEVELx8, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.initBufLevelx8 = iValue;

    // BitAllocMode
    if (HEVC_GetValue(fp, BIT_ALLOC_MODE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.bitAllocMode = iValue;

    // FixedBitRatio 0 ~ 7
    for (i=0; i<MAX_GOP_NUM; i++) {
        sprintf(tempStr, "FixedBitRatio%d", i);
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            iValue = atoi(sValue);
            if ( iValue >= hevcCfgInfo[FIXED_BIT_RATIO].min && iValue <= hevcCfgInfo[FIXED_BIT_RATIO].max )
                pEncCfg->hevcCfg.fixedBitRatio[i] = iValue;
            else
                pEncCfg->hevcCfg.fixedBitRatio[i] = hevcCfgInfo[FIXED_BIT_RATIO].def;
        }
		else
			pEncCfg->hevcCfg.fixedBitRatio[i] = hevcCfgInfo[FIXED_BIT_RATIO].def;

    }

    // IntraQp
    if (HEVC_GetValue(fp, INTRA_QP, &iValue) == 0)
        goto __end_parse;
    else 
        pEncCfg->hevcCfg.intraQP = iValue;

    if (pEncCfg->hevcCfg.losslessEnable)
        pEncCfg->hevcCfg.intraQP = 4;

    if (HEVC_GetValue(fp, INTRA_PERIOD, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraPeriod = iValue;

    if (HEVC_GetValue(fp, CONF_WIND_SIZE_TOP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.confWinTop = iValue;

    if (HEVC_GetValue(fp, CONF_WIND_SIZE_BOT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.confWinBot = iValue;

    if (HEVC_GetValue(fp, CONF_WIND_SIZE_RIGHT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.confWinRight = iValue;

    if (HEVC_GetValue(fp, CONF_WIND_SIZE_LEFT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.confWinLeft = iValue;

    if (HEVC_GetValue(fp, FRAME_RATE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.frameRate = iValue;

    if (HEVC_GetValue(fp, INDE_SLICE_MODE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.independSliceMode = iValue;

    if (HEVC_GetValue(fp, INDE_SLICE_ARG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.independSliceModeArg = iValue;

    if (HEVC_GetValue(fp, DE_SLICE_MODE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.dependSliceMode = iValue;

    if (HEVC_GetValue(fp, DE_SLICE_ARG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.dependSliceModeArg = iValue;

    if (HEVC_GetValue(fp, EN_INTRA_IN_INTER_SLICE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraInInterSliceEnable = iValue;

    if (HEVC_GetValue(fp, INTRA_CTU_REFRESH_MODE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraRefreshMode = iValue;

    if(pEncCfg->hevcCfg.intraInInterSliceEnable == 0) {
        pEncCfg->hevcCfg.intraRefreshMode = 0;
    }

    if (HEVC_GetValue(fp, INTRA_CTU_REFRESH_ARG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraRefreshArg = iValue;

    if (HEVC_GetValue(fp, USE_PRESENT_ENC_TOOLS, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.useRecommendEncParam = iValue;

    if (HEVC_GetValue(fp, SCALING_LIST, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.scalingListEnable = iValue;

    if (HEVC_GetValue(fp, EN_CU_8X8, &iValue) == 0)
        goto __end_parse;
    else
        intra8 = iValue;

    if (HEVC_GetValue(fp, EN_CU_16X16, &iValue) == 0)
        goto __end_parse;
    else
        intra16 = iValue;

    if (HEVC_GetValue(fp, EN_CU_32X32, &iValue) == 0)
        goto __end_parse;
    else
        intra32 = iValue;

    intra8 =  intra16 = intra32 = 1; // force enable all cu mode. [CEZ-1865]
    pEncCfg->hevcCfg.cuSizeMode = (intra8&0x01) | (intra16&0x01)<<1 | (intra32&0x01)<<2;

    if (HEVC_GetValue(fp, EN_TEMPORAL_MVP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.tmvpEnable = iValue;

    if (HEVC_GetValue(fp, WAVE_FRONT_SYNCHRO, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.wppenable = iValue;

    if (HEVC_GetValue(fp, MAX_NUM_MERGE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.maxNumMerge = iValue;

    if (HEVC_GetValue(fp, EN_DYN_MERGE, &iValue) == 0)
        goto __end_parse;
    else
        dynamicMergeEnable = iValue;

    pEncCfg->hevcCfg.dynamicMerge8x8Enable   = dynamicMergeEnable;  // [FIXME]
    pEncCfg->hevcCfg.dynamicMerge16x16Enable = dynamicMergeEnable;  // [FIXME]
    pEncCfg->hevcCfg.dynamicMerge32x32Enable = dynamicMergeEnable;  // [FIXME]

    if (HEVC_GetValue(fp, EN_DBK, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.disableDeblk = !(iValue);

    if (HEVC_GetValue(fp, LF_CROSS_SLICE_BOUNDARY_FLAG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.lfCrossSliceBoundaryEnable = iValue;

    if (HEVC_GetValue(fp, BETA_OFFSET_DIV2, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.betaOffsetDiv2 = iValue;

    if (HEVC_GetValue(fp, TC_OFFSET_DIV2, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.tcOffsetDiv2 = iValue;

    if (HEVC_GetValue(fp, INTRA_TRANS_SKIP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.skipIntraTrans = iValue;

    if (HEVC_GetValue(fp, EN_SAO, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.saoEnable = iValue;

    if (HEVC_GetValue(fp, INTRA_NXN, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraNxNEnable = iValue;

    if (HEVC_GetValue(fp, RATE_CONTROL, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->RcEnable = iValue;

    if (HEVC_GetValue(fp, ENC_BITRATE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->RcBitRate = iValue;

    if (HEVC_GetValue(fp, TRANS_BITRATE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.transRate = iValue;

    if (HEVC_GetValue(fp, CU_LEVEL_RATE_CONTROL, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.cuLevelRCEnable = iValue;

    if (HEVC_GetValue(fp, EN_HVS_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.hvsQPEnable = iValue;

    if (HEVC_GetValue(fp, HVS_QP_SCALE_DIV2, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.hvsQpScale = iValue;

    pEncCfg->hevcCfg.hvsQpScaleEnable = (iValue > 0) ? 1: 0;
    
    if (HEVC_GetValue(fp, INITIAL_DELAY, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->RcInitDelay = iValue;

    if (HEVC_GetValue(fp, MIN_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.minQp = iValue;

    if (HEVC_GetValue(fp, MAX_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.maxQp = iValue;

    if (HEVC_GetValue(fp, MAX_DELTA_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.maxDeltaQp = iValue;

    if (HEVC_GetValue(fp, GOP_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.customGopSize = iValue;

    if (HEVC_GetValue(fp, DERIVE_LAMBDA_WEIGHT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.useDeriveLambdaWeight = iValue;

    if (HEVC_GetValue(fp, ROI_ENABLE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.ctuOptParam.roiEnable = iValue;

    if (HEVC_GetValue(fp, GOP_PRESET, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopPresetIdx = iValue;
    if (HEVC_GetValue(fp, EN_TEMPORAL_LAYER_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.enTemporalLayerQp = iValue;
    if (HEVC_GetValue(fp, TID_0_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.tidQp0 = iValue;
    if (HEVC_GetValue(fp, TID_1_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.tidQp1 = iValue;
    if (HEVC_GetValue(fp, TID_2_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.tidQp2 = iValue;
    if (HEVC_GetValue(fp, TID_0_PERIOD, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.gopParam.tidPeriod0 = iValue;
    if (HEVC_GetValue(fp, FRAME_SKIP, &iValue) == 0)
        goto __end_parse;
    else
        frameSkip = iValue;

    // VUI encoding
    if (HEVC_GetValue(fp, VUI_PARAM_FLAG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiParamFlags = iValue;

    if (HEVC_GetValue(fp, VUI_ASPECT_RATIO_IDC, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiAspectRatioIdc = iValue;

    if (HEVC_GetValue(fp, VUI_SAR_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiSarSize = iValue;

    if (HEVC_GetValue(fp, VUI_OVERSCAN_APPROPRIATE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiOverScanAppropriate = iValue;

    if (HEVC_GetValue(fp, VIDEO_SIGNAL, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.videoSignal = iValue;

    if (HEVC_GetValue(fp, VUI_CHROMA_SAMPLE_LOC, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiChromaSampleLoc = iValue;

    if (HEVC_GetValue(fp, VUI_DISP_WIN_LEFT_RIGHT, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiDispWinLeftRight = iValue;

    if (HEVC_GetValue(fp, VUI_DISP_WIN_TOP_BOTTOM, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiParam.vuiDispWinTopBottom = iValue;

    if (HEVC_GetValue(fp, NUM_UNITS_IN_TICK, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.numUnitsInTick = iValue;

    if (HEVC_GetValue(fp, TIME_SCALE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.timeScale = iValue;

    if (HEVC_GetValue(fp, NUM_TICKS_POC_DIFF_ONE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.numTicksPocDiffOne = iValue;

    if (HEVC_GetValue(fp, ENC_AUD, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.encAUD = iValue;

    if (HEVC_GetValue(fp, ENC_EOS, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.encEOS = iValue;

    if (HEVC_GetValue(fp, ENC_EOB, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.encEOB = iValue;

    if (HEVC_GetValue(fp, CB_QP_OFFSET, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.chromaCbQpOffset = iValue;

    if (HEVC_GetValue(fp, CR_QP_OFFSET, &iValue) == 0)
            goto __end_parse;
    else
        pEncCfg->hevcCfg.chromaCrQpOffset = iValue;

    if (HEVC_GetValue(fp, RC_INIT_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.initialRcQp = iValue;

    if (HEVC_GetValue(fp, EN_NR_Y, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrYEnable = iValue;

    if (HEVC_GetValue(fp, EN_NR_CB, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrCbEnable = iValue;

    if (HEVC_GetValue(fp, EN_NR_CR, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrCrEnable = iValue;

    if (HEVC_GetValue(fp, EN_NOISE_EST, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrNoiseEstEnable = iValue;

    if (HEVC_GetValue(fp, NOISE_SIGMA_Y, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrNoiseSigmaY = iValue;

    if (HEVC_GetValue(fp, NOISE_SIGMA_CB, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrNoiseSigmaCb = iValue;

    if (HEVC_GetValue(fp, NOISE_SIGMA_CR, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrNoiseSigmaCr = iValue;

    if (HEVC_GetValue(fp, INTRA_NOISE_WEIGHT_Y, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrIntraWeightY = iValue;

    if (HEVC_GetValue(fp, INTRA_NOISE_WEIGHT_CB, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrIntraWeightCb= iValue;

    if (HEVC_GetValue(fp, INTRA_NOISE_WEIGHT_CR, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrIntraWeightCr = iValue;

    if (HEVC_GetValue(fp, INTER_NOISE_WEIGHT_Y, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrInterWeightY = iValue;

    if (HEVC_GetValue(fp, INTER_NOISE_WEIGHT_CB, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrInterWeightCb = iValue;

    if (HEVC_GetValue(fp, INTER_NOISE_WEIGHT_CR, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.nrInterWeightCr = iValue;

    if (HEVC_GetValue(fp, INTRA_MIN_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraMinQp = iValue;

    if (HEVC_GetValue(fp, INTRA_MAX_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.intraMaxQp = iValue;

    if (HEVC_GetValue(fp, USE_LONGTERM_PRRIOD, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.useAsLongtermPeriod = iValue;

    if (HEVC_GetValue(fp, REF_LONGTERM_PERIOD, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.refLongtermPeriod = iValue;


    if (HEVC_GetValue(fp, EN_CTU_QP, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.ctuOptParam.ctuQpEnable = iValue;



    if (HEVC_GetValue(fp, EN_VUI_DATA, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiDataEnable = iValue;

    if (HEVC_GetValue(fp, VUI_DATA_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.vuiDataSize = iValue;

    if (HEVC_GetValue(fp, EN_HRD_IN_VPS, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.hrdInVPS = iValue;

    if (HEVC_GetValue(fp, EN_HRD_IN_VUI, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.hrdInVUI = iValue;

    if (HEVC_GetValue(fp, HRD_DATA_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.hrdDataSize = iValue;

    if (HEVC_GetValue(fp, EN_PREFIX_SEI_DATA, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.prefixSeiEnable = iValue;

    if (HEVC_GetValue(fp, PREFIX_SEI_DATA_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.prefixSeiDataSize = iValue;

    if (HEVC_GetValue(fp, PREFIX_SEI_TIMING_FLAG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.prefixSeiTimingFlag = iValue;

    if (HEVC_GetValue(fp, EN_SUFFIX_SEI_DATA, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.suffixSeiEnable = iValue;

    if (HEVC_GetValue(fp, SUFFIX_SEI_DATA_SIZE, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.suffixSeiDataSize = iValue;

    if (HEVC_GetValue(fp, SUFFIX_SEI_TIMING_FLAG, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.suffixSeiTimingFlag = iValue;

    if (HEVC_GetValue(fp, EN_FORCED_IDR_HEADER, &iValue) == 0)
        goto __end_parse;
    else
        pEncCfg->hevcCfg.forcedIdrHeaderEnable = iValue;


    //GOP
    if (pEncCfg->hevcCfg.intraPeriod == 1) {
        pEncCfg->hevcCfg.gopParam.picParam[0].picType = PIC_TYPE_I;
        pEncCfg->hevcCfg.gopParam.picParam[0].picQp = pEncCfg->hevcCfg.intraQP;
        if (pEncCfg->hevcCfg.gopParam.customGopSize > 1) {
            VLOG(ERR, "CFG file error : gop size should be smaller than 2 for all intra case\n");
            goto __end_parse;
        }
    }
    else {
        for (i = 0; pEncCfg->hevcCfg.gopPresetIdx == PRESET_IDX_CUSTOM_GOP && i < pEncCfg->hevcCfg.gopParam.customGopSize; i++) {

            sprintf(tempStr, "Frame%d", i+1);
            if (HEVC_GetStringValue(fp, tempStr, sValue) == 0) {
                VLOG(ERR, "CFG file error : %s value is not available. \n", tempStr);
                goto __end_parse;
            }

            if ( HEVC_SetGOPInfo(sValue, &pEncCfg->hevcCfg.gopParam.picParam[i], 
                &pEncCfg->hevcCfg.gopParam.gopPicLambda[i], pEncCfg->hevcCfg.gopParam.useDeriveLambdaWeight, 
                pEncCfg->hevcCfg.intraQP) == 0) {
                    VLOG(ERR, "CFG file error : %s value is not available. \n", tempStr);
                    goto __end_parse;
            }

#if TEMP_SCALABLE_RC
            if ( (pEncCfg->hevcCfg.gopParam.picParam[i].temporalId + 1) > MAX_NUM_TEMPORAL_LAYER) {
                VLOG(ERR, "CFG file error : %s MaxTempLayer %d exceeds MAX_TEMP_LAYER(7). \n", tempStr, pEncCfg->hevcCfg.gopParam.picParam[i].temporalId + 1);
                goto __end_parse;
            }
#endif
        }
    }

    //ROI
    if (pEncCfg->hevcCfg.ctuOptParam.roiEnable) {
        sprintf(tempStr, "RoiFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.roiFileName);
        }
    }



    if (pEncCfg->hevcCfg.ctuOptParam.ctuQpEnable) {
        sprintf(tempStr, "CtuQpFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.ctuQpFileName);
        }
    }

    if (pEncCfg->hevcCfg.prefixSeiEnable) {
        sprintf(tempStr, "PrefixSeiDataFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.prefixSeiDataFileName);
        }
    }

    if (pEncCfg->hevcCfg.suffixSeiEnable) {
        sprintf(tempStr, "SuffixSeiDataFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.suffixSeiDataFileName);
        }
    }

    if (pEncCfg->hevcCfg.hrdInVPS || pEncCfg->hevcCfg.hrdInVUI) {
        sprintf(tempStr, "RbspHrdFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.hrdDataFileName);
        }
    }

    if (pEncCfg->hevcCfg.vuiDataEnable) {
        sprintf(tempStr, "RbspVuiFile");
        if (HEVC_GetStringValue(fp, tempStr, sValue) != 0) {
            sscanf(sValue, "%s\n", pEncCfg->hevcCfg.vuiDataFileName);
        }
    }

    if (pEncCfg->hevcCfg.losslessEnable) {
        pEncCfg->hevcCfg.disableDeblk = 1;
        pEncCfg->hevcCfg.saoEnable = 0;
        pEncCfg->RcEnable = 0;
    }



    ret = 1; /* Success */

__end_parse:
    osal_fclose(fp);
    return ret;
}

 

//------------------------------------------------------------------------------
// File: $Id$
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "main_helper.h"

#ifdef PLATFORM_WIN32
#pragma warning(disable : 4996)     //!<< disable waring C4996: The POSIX name for this item is deprecated. 
#endif


#ifndef min
#define min(a,b)       (((a) < (b)) ? (a) : (b))
#endif
/*******************************************************************************
 * REPORT                                                                      *
 *******************************************************************************/
#define USER_DATA_INFO_OFFSET       (8*17)
#define FN_PIC_INFO             "dec_pic_disp_info.log"
#define FN_SEQ_INFO             "dec_seq_disp_info.log"
#define FN_PIC_TYPE             "dec_pic_type.log"
#define FN_USER_DATA            "dec_user_data.log"
#define FN_SEQ_USER_DATA        "dec_seq_user_data.log"

// VC1 specific
enum {
    BDU_SEQUENCE_END               = 0x0A,
    BDU_SLICE                      = 0x0B,
    BDU_FIELD                      = 0x0C,
    BDU_FRAME                      = 0x0D,
    BDU_ENTRYPOINT_HEADER          = 0x0E,
    BDU_SEQUENCE_HEADER            = 0x0F,
    BDU_SLICE_LEVEL_USER_DATA      = 0x1B,
    BDU_FIELD_LEVEL_USER_DATA      = 0x1C,
    BDU_FRAME_LEVEL_USER_DATA      = 0x1D,
    BDU_ENTRYPOINT_LEVEL_USER_DATA = 0x1E,
    BDU_SEQUENCE_LEVEL_USER_DATA   = 0x1F
};

// AVC specific - SEI
enum {
    SEI_REGISTERED_ITUTT35_USERDATA = 0x04,
    SEI_UNREGISTERED_USERDATA       = 0x05,
    SEI_MVC_SCALABLE_NESTING        = 0x25
};

static vpu_rpt_info_t s_rpt_info[MAX_VPU_CORE_NUM];

void OpenDecReport(
    Uint32              core_idx, 
    VpuReportConfig_t*  cfg
    )
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];
    rpt->fpPicDispInfoLogfile = NULL;
    rpt->fpPicTypeLogfile     = NULL;
    rpt->fpSeqDispInfoLogfile = NULL;
    rpt->fpUserDataLogfile    = NULL;
    rpt->fpSeqUserDataLogfile = NULL;

    rpt->decIndex           = 0;
    rpt->userDataEnable     = cfg->userDataEnable;
    rpt->userDataReportMode = cfg->userDataReportMode;

    rpt->reportOpened = TRUE;

    return;
}

void CloseDecReport(
    Uint32 core_idx
    )
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];

    if (rpt->reportOpened == FALSE) {
        return;
    }

    if (rpt->fpPicDispInfoLogfile) {
        osal_fclose(rpt->fpPicDispInfoLogfile);
        rpt->fpPicDispInfoLogfile = NULL;
    }
    if (rpt->fpPicTypeLogfile) {
        osal_fclose(rpt->fpPicTypeLogfile);
        rpt->fpPicTypeLogfile = NULL;
    }
    if (rpt->fpSeqDispInfoLogfile) {
        osal_fclose(rpt->fpSeqDispInfoLogfile);
        rpt->fpSeqDispInfoLogfile = NULL;
    }

    if (rpt->fpUserDataLogfile) {
        osal_fclose(rpt->fpUserDataLogfile);
        rpt->fpUserDataLogfile= NULL;
    }

    if (rpt->fpSeqUserDataLogfile) {
        osal_fclose(rpt->fpSeqUserDataLogfile);
        rpt->fpSeqUserDataLogfile = NULL;
    }

    if (rpt->vb_rpt.base) {
        vdi_free_dma_memory(core_idx, &rpt->vb_rpt);
    }
    rpt->decIndex = 0;

    return;
}

static void SaveUserData(
    Uint32  core_idx, 
    BYTE*   userDataBuf
    ) 
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];
    Uint32          i;
    Uint32          UserDataType;
    Uint32          UserDataSize;
    Uint32          userDataNum;
    Uint32          TotalSize;
    BYTE*           tmpBuf;

    if (rpt->reportOpened == FALSE) {
        return;
    }

    if(rpt->fpUserDataLogfile == 0) {
        rpt->fpUserDataLogfile = osal_fopen(FN_USER_DATA, "w+");
    }

    tmpBuf      = userDataBuf;
    userDataNum = (short)((tmpBuf[0]<<8) | (tmpBuf[1]<<0));
    TotalSize   = (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));
    tmpBuf      = userDataBuf + 8;

    for(i=0; i<userDataNum; i++) {
        UserDataType = (short)((tmpBuf[0]<<8) | (tmpBuf[1]<<0));
        UserDataSize = (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));

        osal_fprintf(rpt->fpUserDataLogfile, "\n[Idx Type Size] : [%4d %4d %4d]",i, UserDataType, UserDataSize);

        tmpBuf += 8;
    }
    osal_fprintf(rpt->fpUserDataLogfile, "\n");

    tmpBuf = userDataBuf + USER_DATA_INFO_OFFSET;

    for(i=0; i<TotalSize; i++) {
        osal_fprintf(rpt->fpUserDataLogfile, "%02x", tmpBuf[i]);
        if ((i&7) == 7) {
            osal_fprintf(rpt->fpUserDataLogfile, "\n");
        }
    }
    osal_fprintf(rpt->fpUserDataLogfile, "\n");

    osal_fflush(rpt->fpUserDataLogfile);
}


static void SaveUserDataINT(
    Uint32  core_idx, 
    BYTE*   userDataBuf, 
    Int32   size, 
    Int32   intIssued, 
    Int32   decIdx, 
    CodStd  bitstreamFormat
    ) 
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];
    Int32           i;
    Int32           UserDataType = 0;
    Int32           UserDataSize = 0;
    Int32           userDataNum = 0;
    Int32           TotalSize;
    BYTE*           tmpBuf;
    BYTE*           backupBufTmp;
    static Int32    backupSize = 0;
    static BYTE*    backupBuf  = NULL;

    if (rpt->reportOpened == FALSE) {
        return;
    }

    if(rpt->fpUserDataLogfile == NULL) {
        rpt->fpUserDataLogfile = osal_fopen(FN_USER_DATA, "w+");
    }

    backupBufTmp = (BYTE *)osal_malloc(backupSize + size);

    if (backupBufTmp == 0) {
        VLOG( ERR, "Can't mem allock\n");
        return;
    }

    for (i=0; i<backupSize; i++) {
        backupBufTmp[i] = backupBuf[i];
    }
    if (backupBuf != NULL) {
        osal_free(backupBuf);
    }
    backupBuf = backupBufTmp;

    tmpBuf = userDataBuf + USER_DATA_INFO_OFFSET;
    size -= USER_DATA_INFO_OFFSET;

    for(i=0; i<size; i++) {
        backupBuf[backupSize + i] = tmpBuf[i];
    }

    backupSize += size;

    if (intIssued) {
        return;
    }

    tmpBuf = userDataBuf;
    userDataNum = (short)((tmpBuf[0]<<8) | (tmpBuf[1]<<0));
    if(userDataNum == 0) {
        return; 
    }

    tmpBuf = userDataBuf + 8;
    UserDataSize = (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));

    UserDataSize = (UserDataSize+7)/8*8;
    osal_fprintf(rpt->fpUserDataLogfile, "FRAME [%1d]\n", decIdx);

    for(i=0; i<backupSize; i++) {
        osal_fprintf(rpt->fpUserDataLogfile, "%02x", backupBuf[i]);
        if ((i&7) == 7) {
            osal_fprintf(rpt->fpUserDataLogfile, "\n");
        }

        if( (i%8==7) && (i==UserDataSize-1) && (UserDataSize != backupSize)) {
            osal_fprintf(rpt->fpUserDataLogfile, "\n");
            tmpBuf+=8;
            UserDataSize += (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));
            UserDataSize = (UserDataSize+7)/8*8;
        }
    }
    if (backupSize > 0) {
        osal_fprintf(rpt->fpUserDataLogfile, "\n");
    }

    tmpBuf = userDataBuf;
    userDataNum = (short)((tmpBuf[0]<<8) | (tmpBuf[1]<<0));
    TotalSize = (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));

    osal_fprintf(rpt->fpUserDataLogfile, "User Data Num: [%d]\n", userDataNum);
    osal_fprintf(rpt->fpUserDataLogfile, "User Data Total Size: [%d]\n", TotalSize);

    tmpBuf = userDataBuf + 8;
    for(i=0; i<userDataNum; i++) {
        UserDataType = (short)((tmpBuf[0]<<8) | (tmpBuf[1]<<0));
        UserDataSize = (short)((tmpBuf[2]<<8) | (tmpBuf[3]<<0));

        if(bitstreamFormat == STD_VC1) {
            switch (UserDataType) {

            case BDU_SLICE_LEVEL_USER_DATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "BDU_SLICE_LEVEL_USER_DATA");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            case BDU_FIELD_LEVEL_USER_DATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "BDU_FIELD_LEVEL_USER_DATA");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            case BDU_FRAME_LEVEL_USER_DATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "BDU_FRAME_LEVEL_USER_DATA");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            case BDU_ENTRYPOINT_LEVEL_USER_DATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "BDU_ENTRYPOINT_LEVEL_USER_DATA");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            case BDU_SEQUENCE_LEVEL_USER_DATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "BDU_SEQUENCE_LEVEL_USER_DATA");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            }
        }
        else if(bitstreamFormat == STD_AVC) {
            switch (UserDataType) {
            case SEI_REGISTERED_ITUTT35_USERDATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "registered_itu_t_t35");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            case SEI_UNREGISTERED_USERDATA:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "unregistered");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;

            case SEI_MVC_SCALABLE_NESTING:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "mvc_scalable_nesting");
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
                break;
            }
        }
        else if(bitstreamFormat == STD_MPEG2) {

            switch (UserDataType) {
            case 0:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Seq]\n", i);
                break;
            case 1:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Gop]\n", i);
                break;
            case 2:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Pic]\n", i);
                break;
            default:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Error]\n", i);
                break;
            }
            osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
        } 
        else if(bitstreamFormat == STD_AVS) {
            osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:%s]\n", i, "User Data");
            osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
        }
        else {
            switch (UserDataType) {
            case 0:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Vos]\n", i);
                break;
            case 1:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Vis]\n", i);
                break;
            case 2:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Vol]\n", i);
                break;
            case 3:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Gov]\n", i);
                break;
            default:
                osal_fprintf(rpt->fpUserDataLogfile, "User Data Type: [%d:Error]\n", i);
                break;
            }
            osal_fprintf(rpt->fpUserDataLogfile, "User Data Size: [%d]\n", UserDataSize);
        }

        tmpBuf += 8;
    }
    osal_fprintf(rpt->fpUserDataLogfile, "\n");
    osal_fflush(rpt->fpUserDataLogfile);

    backupSize = 0;
    if (backupBuf != NULL) {
        osal_free(backupBuf);
    }

    backupBuf = 0;
}

void CheckUserDataInterrupt(
    Uint32      core_idx, 
    DecHandle   handle, 
    Int32       decodeIdx, 
    CodStd      bitstreamFormat, 
    Int32       int_reason
    )
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];

    if (int_reason & (1<<INT_BIT_USERDATA)) {
        // USER DATA INTERRUPT Issued
        // User Data save
        if (rpt->userDataEnable == TRUE) {
            int size;
            BYTE *userDataBuf;
            size = rpt->vb_rpt.size + USER_DATA_INFO_OFFSET;
            userDataBuf = osal_malloc(size);
            osal_memset(userDataBuf, 0, size);

            vdi_read_memory(core_idx, rpt->vb_rpt.phys_addr, userDataBuf, size, VDI_BIG_ENDIAN);
            if (decodeIdx >= 0)
                SaveUserDataINT(core_idx, userDataBuf, size, 1, rpt->decIndex, bitstreamFormat);
            osal_free(userDataBuf);
        } else {
            VLOG(ERR, "Unexpected Interrupt issued");
        }
    }
}

void ConfigDecReport(
    Uint32      core_idx, 
    DecHandle   handle, 
    CodStd      bitstreamFormat
    )
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];

    if (rpt->reportOpened == FALSE) {
        return;
    }

    // Report Information
    if (!rpt->vb_rpt.base) {
        rpt->vb_rpt.size     = SIZE_REPORT_BUF;
        if (vdi_allocate_dma_memory(core_idx, &rpt->vb_rpt) < 0) {
            VLOG(ERR, "fail to allocate report  buffer\n" );
            return;
        }
    }

    VPU_DecGiveCommand(handle, SET_ADDR_REP_USERDATA,    &rpt->vb_rpt.phys_addr );
    VPU_DecGiveCommand(handle, SET_SIZE_REP_USERDATA,    &rpt->vb_rpt.size );
    VPU_DecGiveCommand(handle, SET_USERDATA_REPORT_MODE, &rpt->userDataReportMode );

    if (rpt->userDataEnable == TRUE) {
        VPU_DecGiveCommand( handle, ENABLE_REP_USERDATA, 0 );
    } 
    else {
        VPU_DecGiveCommand( handle, DISABLE_REP_USERDATA, 0 );
    }
}

void SaveDecReport(
    Uint32          core_idx, 
    DecHandle       handle, 
    DecOutputInfo*  pDecInfo, 
    CodStd          bitstreamFormat, 
    Uint32          mbNumX, 
    Uint32          mbNumY
    )
{
    vpu_rpt_info_t *rpt = &s_rpt_info[core_idx];

    if (rpt->reportOpened == FALSE) {
        return ;
    }

    // Report Information	

    // User Data
    if ((pDecInfo->indexFrameDecoded >= 0 || (bitstreamFormat == STD_VC1))  &&
        rpt->userDataEnable == TRUE && 
        pDecInfo->decOutputExtData.userDataSize > 0) {
        // Vc1 Frame user data follow picture. After last frame decoding, user data should be reported.
        Uint32 size        = 0;
        BYTE*  userDataBuf = NULL;

        if (pDecInfo->decOutputExtData.userDataBufFull == TRUE) {
            VLOG(ERR, "User Data Buffer is Full\n");
        }

        size = (pDecInfo->decOutputExtData.userDataSize+7)/8*8 + USER_DATA_INFO_OFFSET;
        userDataBuf = (BYTE*)osal_malloc(size);
        osal_memset(userDataBuf, 0, size);

        vdi_read_memory(core_idx, rpt->vb_rpt.phys_addr, userDataBuf, size, HOST_ENDIAN);
        if (pDecInfo->indexFrameDecoded >= 0) {
            SaveUserData(core_idx, userDataBuf);
        }
        osal_free(userDataBuf);
    }

    if (((pDecInfo->indexFrameDecoded >= 0 || (bitstreamFormat == STD_VC1 )) && rpt->userDataEnable) || // Vc1 Frame user data follow picture. After last frame decoding, user data should be reported.
        (pDecInfo->indexFrameDisplay >= 0 && rpt->userDataEnable) ) {
        Uint32 size        = 0;
        Uint32 dataSize    = 0;
        BYTE*  userDataBuf = NULL;

        if (pDecInfo->decOutputExtData.userDataBufFull) {
            VLOG(ERR, "User Data Buffer is Full\n");
        }

        dataSize = pDecInfo->decOutputExtData.userDataSize % rpt->vb_rpt.size;
        if (dataSize == 0 && pDecInfo->decOutputExtData.userDataSize != 0)	{
            dataSize = rpt->vb_rpt.size;
        }

        size = (dataSize+7)/8*8 + USER_DATA_INFO_OFFSET;
        userDataBuf = (BYTE*)osal_malloc(size);
        osal_memset(userDataBuf, 0, size);
        vdi_read_memory(core_idx, rpt->vb_rpt.phys_addr, userDataBuf, size, HOST_ENDIAN);
        if (pDecInfo->indexFrameDecoded >= 0 || (bitstreamFormat == STD_VC1)) {
            SaveUserDataINT(core_idx, userDataBuf, size, 0, rpt->decIndex, bitstreamFormat);		
        }
        osal_free(userDataBuf);
    }

    if (pDecInfo->indexFrameDecoded >= 0) {
        if (rpt->fpPicTypeLogfile == NULL) {
            rpt->fpPicTypeLogfile = osal_fopen(FN_PIC_TYPE, "w+");
        }
        osal_fprintf(rpt->fpPicTypeLogfile, "FRAME [%1d]\n", rpt->decIndex);

        switch (bitstreamFormat) {
        case STD_AVC:
            if(pDecInfo->pictureStructure == 3) {	// FIELD_INTERLACED
                osal_fprintf(rpt->fpPicTypeLogfile, "Top Field Type: [%s]\n", pDecInfo->picTypeFirst == 0 ? "I_TYPE" : 
                    (pDecInfo->picTypeFirst) == 1 ? "P_TYPE" :
                    (pDecInfo->picTypeFirst) == 2 ? "BI_TYPE" :
                    (pDecInfo->picTypeFirst) == 3 ? "B_TYPE" :
                    (pDecInfo->picTypeFirst) == 4 ? "SKIP_TYPE" :
                    (pDecInfo->picTypeFirst) == 5 ? "IDR_TYPE" :
                    "FORBIDDEN"); 

                osal_fprintf(rpt->fpPicTypeLogfile, "Bottom Field Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                    (pDecInfo->picType) == 1 ? "P_TYPE" :
                    (pDecInfo->picType) == 2 ? "BI_TYPE" :
                    (pDecInfo->picType) == 3 ? "B_TYPE" :
                    (pDecInfo->picType) == 4 ? "SKIP_TYPE" :
                    (pDecInfo->picType) == 5 ? "IDR_TYPE" :
                    "FORBIDDEN"); 
            }
            else {
                osal_fprintf(rpt->fpPicTypeLogfile, "Picture Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                    (pDecInfo->picType) == 1 ? "P_TYPE" :
                    (pDecInfo->picType) == 2 ? "BI_TYPE" :
                    (pDecInfo->picType) == 3 ? "B_TYPE" :
                    (pDecInfo->picType) == 4 ? "SKIP_TYPE" :
                    (pDecInfo->picType) == 5 ? "IDR_TYPE" :
                    "FORBIDDEN"); 
            }
            break;
        case STD_MPEG2 :
            osal_fprintf(rpt->fpPicTypeLogfile, "Picture Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                pDecInfo->picType == 1 ? "P_TYPE" :
                pDecInfo->picType == 2 ? "B_TYPE" :
                "D_TYPE"); 
            break;
        case STD_MPEG4 :
            osal_fprintf(rpt->fpPicTypeLogfile, "Picture Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                pDecInfo->picType == 1 ? "P_TYPE" :
                pDecInfo->picType == 2 ? "B_TYPE" :
                "S_TYPE"); 
            break;
        case STD_VC1:
            if(pDecInfo->pictureStructure == 3) {	// FIELD_INTERLACED
                osal_fprintf(rpt->fpPicTypeLogfile, "Top Field Type: [%s]\n", pDecInfo->picTypeFirst == 0 ? "I_TYPE" : 
                    (pDecInfo->picTypeFirst) == 1 ? "P_TYPE" :
                    (pDecInfo->picTypeFirst) == 2 ? "BI_TYPE" :
                    (pDecInfo->picTypeFirst) == 3 ? "B_TYPE" :
                    (pDecInfo->picTypeFirst) == 4 ? "SKIP_TYPE" :
                    "FORBIDDEN"); 

            osal_fprintf(rpt->fpPicTypeLogfile, "Bottom Field Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                (pDecInfo->picType) == 1 ? "P_TYPE" :
                (pDecInfo->picType) == 2 ? "BI_TYPE" :
                (pDecInfo->picType) == 3 ? "B_TYPE" :
                (pDecInfo->picType) == 4 ? "SKIP_TYPE" :
                "FORBIDDEN"); 
            }
            else {
                osal_fprintf(rpt->fpPicTypeLogfile, "Picture Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                    (pDecInfo->picTypeFirst) == 1 ? "P_TYPE" :
                    (pDecInfo->picTypeFirst) == 2 ? "BI_TYPE" :
                    (pDecInfo->picTypeFirst) == 3 ? "B_TYPE" :
                    (pDecInfo->picTypeFirst) == 4 ? "SKIP_TYPE" :
                    "FORBIDDEN"); 
            }
            break;
        default:
            osal_fprintf(rpt->fpPicTypeLogfile, "Picture Type: [%s]\n", pDecInfo->picType == 0 ? "I_TYPE" : 
                pDecInfo->picType == 1 ? "P_TYPE" :
                "B_TYPE");
            break;
        }
    }

    if (pDecInfo->indexFrameDecoded >= 0) {
        if (rpt->fpPicDispInfoLogfile == NULL) {
            rpt->fpPicDispInfoLogfile = osal_fopen(FN_PIC_INFO, "w+");
        }
        osal_fprintf(rpt->fpPicDispInfoLogfile, "FRAME [%1d]\n", rpt->decIndex);

        switch (bitstreamFormat) {
        case STD_MPEG2:
            osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", 
                pDecInfo->picType == 0 ? "I_TYPE" : 
                pDecInfo->picType == 1 ? "P_TYPE" :
                pDecInfo->picType == 2 ? "B_TYPE" :
                "D_TYPE"); 
            break;
        case STD_MPEG4:
            osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", 
                pDecInfo->picType == 0 ? "I_TYPE" : 
                pDecInfo->picType == 1 ? "P_TYPE" :
                pDecInfo->picType == 2 ? "B_TYPE" :
                "S_TYPE"); 
            break;
        case STD_VC1 :
            if(pDecInfo->pictureStructure == 3) {	// FIELD_INTERLACED
                osal_fprintf(rpt->fpPicDispInfoLogfile, "Top : %s\n", (pDecInfo->picType>>3) == 0 ? "I_TYPE" : 
                    (pDecInfo->picType>>3) == 1 ? "P_TYPE" :
                    (pDecInfo->picType>>3) == 2 ? "BI_TYPE" :
                    (pDecInfo->picType>>3) == 3 ? "B_TYPE" :
                    (pDecInfo->picType>>3) == 4 ? "SKIP_TYPE" :
                    "FORBIDDEN"); 

                osal_fprintf(rpt->fpPicDispInfoLogfile, "Bottom : %s\n", (pDecInfo->picType&0x7) == 0 ? "I_TYPE" : 
                    (pDecInfo->picType&0x7) == 1 ? "P_TYPE" :
                    (pDecInfo->picType&0x7) == 2 ? "BI_TYPE" :
                    (pDecInfo->picType&0x7) == 3 ? "B_TYPE" :
                    (pDecInfo->picType&0x7) == 4 ? "SKIP_TYPE" :
                    "FORBIDDEN"); 

                osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Interlaced Picture");
            }
            else {
                osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", (pDecInfo->picType>>3) == 0 ? "I_TYPE" : 
                    (pDecInfo->picType>>3) == 1 ? "P_TYPE" :
                    (pDecInfo->picType>>3) == 2 ? "BI_TYPE" :
                    (pDecInfo->picType>>3) == 3 ? "B_TYPE" :
                    (pDecInfo->picType>>3) == 4 ? "SKIP_TYPE" :
                    "FORBIDDEN"); 

                osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Frame Picture");
            }
            break;
        default:
            osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", 
                         pDecInfo->picType == 0 ? "I_TYPE" : 
                         pDecInfo->picType == 1 ? "P_TYPE" :
                         "B_TYPE"); 
            break;
        }

        if(bitstreamFormat != STD_VC1) {
            if (pDecInfo->interlacedFrame) {
                if(bitstreamFormat == STD_AVS) {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Frame Picture");
                }
                else {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Interlaced Picture");
                }
            }
            else {
                if(bitstreamFormat == STD_AVS) {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Interlaced Picture");
                }
                else {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Frame Picture");
                }
            }
        }

        if (bitstreamFormat != STD_RV) {
            if(bitstreamFormat == STD_VC1) {
                switch(pDecInfo->pictureStructure) {
                case 0:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "PROGRESSIVE");	break;
                case 2:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FRAME_INTERLACE"); break;
                case 3:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FIELD_INTERLACE");	break;
                default: osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FORBIDDEN"); break;				
                }
            }
            else if(bitstreamFormat == STD_AVC) {
                if(!pDecInfo->interlacedFrame) {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FRAME_PICTURE");
                }
                else {
                    if(pDecInfo->topFieldFirst) {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Top Field First");
                    }
                    else {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Bottom Field First");
                    }
                }
            }
            else if (bitstreamFormat != STD_MPEG4 && bitstreamFormat != STD_AVS) {
                switch (pDecInfo->pictureStructure) {
                case 1:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "TOP_FIELD");	break;
                case 2:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "BOTTOM_FIELD"); break;
                case 3:  osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FRAME_PICTURE");	break;
                default: osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "FORBIDDEN"); break;				
                }
            }

            if(bitstreamFormat != STD_AVC) {
                if (pDecInfo->topFieldFirst) {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Top Field First");
                }
                else {
                    osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Bottom Field First");
                }

                if (bitstreamFormat != STD_MPEG4) {
                    if (pDecInfo->repeatFirstField) {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Repeat First Field");
                    }
                    else {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Not Repeat First Field");
                    }

                    if (bitstreamFormat == STD_VC1) {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "VC1 RPTFRM [%1d]\n", pDecInfo->progressiveFrame);
                    }
                    else if (pDecInfo->progressiveFrame) {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Progressive Frame");
                    }
                    else {
                        osal_fprintf(rpt->fpPicDispInfoLogfile, "%s\n", "Interlaced Frame");
                    }
                }
            }
        }

        if (bitstreamFormat == STD_MPEG2) {
            osal_fprintf(rpt->fpPicDispInfoLogfile, "Field Sequence [%d]\n\n", pDecInfo->fieldSequence);
        }
        else {
            osal_fprintf(rpt->fpPicDispInfoLogfile, "\n");
        }

        osal_fflush(rpt->fpPicDispInfoLogfile);
    }

    if(pDecInfo->indexFrameDecoded >= 0) {
        rpt->decIndex ++;
    }

    return;
}

#define DEFAULT_ENC_OUTPUT_NUM      30

/*******************************************************************************
 * FUNCTIONS RELATED TO CPB                                                    *
 *******************************************************************************/
int FillBsResetBufHelper(Uint32 core_idx,
    BYTE *buf,
    PhysicalAddress paBsBufAddr,
    int bsBufsize,
    int endian)
{	
    if( !bsBufsize )
        return -1;
    VpuReadMem(core_idx, paBsBufAddr, buf, bsBufsize, endian);
    return bsBufsize;
}

RetCode ReadBsRingBufHelper(Uint32 core_idx,
    EncHandle handle,
    osal_file_t bsFp,
    PhysicalAddress bitstreamBuffer,
    Uint32 bitstreamBufferSize,
    int defaultsize,
    int endian)
{
    RetCode ret = RETCODE_SUCCESS;
    int loadSize = 0;
    PhysicalAddress paRdPtr, paWrPtr;
    int size = 0;
    PhysicalAddress paBsBufStart = bitstreamBuffer;
    PhysicalAddress paBsBufEnd   = bitstreamBuffer+bitstreamBufferSize;

    ret = VPU_EncGetBitstreamBuffer(handle, &paRdPtr, &paWrPtr, &size);
    if( ret != RETCODE_SUCCESS )
    {
        VLOG(ERR, "VPU_EncGetBitstreamBuffer failed Error code is 0x%x \n", ret );
        goto LOAD_BS_ERROR;
    }

    if( size > 0 )
    {
        if( defaultsize > 0 )
        {
            if( size < defaultsize )
                loadSize = ( ( size >> 9 ) << 9 ); 
            else
                loadSize = defaultsize;
        }
        else
        {
            loadSize = size;
        }

        if( loadSize > 0 )
        {
            ProcessEncodedBitstreamBurst(core_idx, bsFp, paRdPtr, paBsBufStart, paBsBufEnd, loadSize, endian, NULL);
            ret = VPU_EncUpdateBitstreamBuffer(handle, loadSize);
            if( ret != RETCODE_SUCCESS )
            {
                VLOG(ERR, "VPU_EncUpdateBitstreamBuffer failed Error code is 0x%x \n", ret );
                goto LOAD_BS_ERROR;
            }
        }

    }

LOAD_BS_ERROR:

    return ret;
}

/******************************************************************************
    DPB Image Data Control
******************************************************************************/
/* THIS FUNCTION WILL BE REMOVED */
/*lint -save -e438 */
int LoadYuvImageHelperFormat(Uint32 core_idx,
    osal_file_t yuvFp,
    Uint8 *pYuv,
    FrameBuffer *fb,
    TiledMapConfig mapCfg,
    int picWidth,
    int picHeight,
    int stride,
    int interleave,
    int format,
    int endian,
    BOOL converter)
{
    int frameSize;

    switch (format)
    {
    case FORMAT_420:
        frameSize = picWidth * picHeight * 3 / 2;
        break;
    case FORMAT_224:
        frameSize = picWidth * picHeight * 4 / 2;
        break;
    case FORMAT_422:
        frameSize = picWidth * picHeight * 4 / 2;
        break;
    case FORMAT_444:
        frameSize = picWidth * picHeight * 6 / 2;
        break;
    case FORMAT_400:
        frameSize = picWidth * picHeight;
        break;
    case FORMAT_YUYV:
        frameSize = picWidth * picHeight * 4 / 2;
        picWidth *= 2;
        stride*= 2;
        break;
    default:
        frameSize = picWidth * picHeight * 3 / 2;
        break;
    }

    // Load source one picture image to encode to SDRAM frame buffer.
    if( !osal_fread(pYuv, 1, frameSize, yuvFp) ) 
    {
        if( !osal_feof( yuvFp ) )
            VLOG(ERR, "Yuv Data osal_fread failed file handle is 0x%x \n", yuvFp );			
        return 0;
    }

    if (fb->mapType)
        LoadTiledImageYuvBurst(core_idx, pYuv, picWidth, picHeight, fb, mapCfg);
    else
        LoadYuvImageBurstFormat(core_idx, pYuv, picWidth, picHeight, fb, TRUE);

	return 1;
}
/*lint -restore */

#if 0
int SaveYuvImageHelperFormat(
    Uint32          core_idx,
    osal_file_t     yuvFp,
    FrameBuffer*    fbSrc,
    TiledMapConfig  mapCfg,
    Uint8*          pYuv, 
    VpuRect         rect,
    BOOL            enableCrop,
    int             interLeave,
    int             format,
    int             endian)
{
    int frameSize;
    int picWidth;
    int picHeight;

    picWidth  = enableCrop == TRUE ? (int)(rect.right - rect.left) : fbSrc->stride;
    picHeight = enableCrop == TRUE ? (int)(rect.bottom - rect.top) : fbSrc->height;

    switch (format)
    {
    case FORMAT_420:
        frameSize = picWidth * ((picHeight+1)/2*2) * 3/2;
        break;
    case FORMAT_224:
        frameSize = picWidth * ((picHeight+1)/2*2) * 4/2;
        break;
    case FORMAT_422:
        frameSize = picWidth * picHeight * 4/2;
        break;
    case FORMAT_444:
        frameSize = picWidth * picHeight * 6/2; 
        break;
    case FORMAT_400:
        frameSize = picWidth * picHeight;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        frameSize = picWidth*2 * ((picHeight+1)/2*2) * 3/2;
        break;
    default:
        frameSize = picWidth * ((picHeight+1)/2*2) * 3/2;
        break;
    }

    StoreYuvImageBurstFormat(core_idx, fbSrc, mapCfg, pYuv, rect, enableCrop, format, endian);
    if (yuvFp)
    {
        if( !osal_fwrite(pYuv, sizeof(Uint8), frameSize , yuvFp) )
        {
            VLOG(ERR, "Frame Data osal_fwrite failed file handle is 0x%x \n", yuvFp );
            return 0;
        }
    }

    return 1;
} 
#endif

int setWaveEncOpenParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig, ENC_CFG *pCfg)
{
    Int32   i = 0;
    Int32   srcWidth;
    Int32   srcHeight;
    Int32   outputNum;
    Int32   bitrate;

    EncHevcParam *param = &pEncOP->EncStdParam.hevcParam;

    srcWidth  = (pEncConfig->picWidth > 0)  ? pEncConfig->picWidth  : pCfg->hevcCfg.picX;
    srcHeight = (pEncConfig->picHeight > 0) ? pEncConfig->picHeight : pCfg->hevcCfg.picY;
    outputNum = (pEncConfig->outNum > 0)    ? min(pEncConfig->outNum,pCfg->NumFrame) : pCfg->NumFrame;
    bitrate   = (pEncConfig->kbps > 0)      ? pEncConfig->kbps*1024 : pCfg->RcBitRate;

    pEncConfig->outNum      = outputNum;
    pEncOP->picWidth        = srcWidth;
    pEncOP->picHeight       = srcHeight;
    pEncOP->frameRateInfo   = pCfg->hevcCfg.frameRate;

    param->level            = 0;
    param->tier             = 0;
    pEncOP->srcBitDepth     = pCfg->SrcBitDepth;

    if (pCfg->hevcCfg.internalBitDepth == 0)
        param->internalBitDepth = pCfg->SrcBitDepth;
    else
        param->internalBitDepth = pCfg->hevcCfg.internalBitDepth;

    if (param->internalBitDepth > 8) 
        param->profile   = HEVC_PROFILE_MAIN10;
    else
        param->profile   = HEVC_PROFILE_MAIN;

    param->chromaFormatIdc  = 0;
    param->losslessEnable   = pCfg->hevcCfg.losslessEnable;
    param->constIntraPredFlag = pCfg->hevcCfg.constIntraPredFlag;



    if (pCfg->hevcCfg.useAsLongtermPeriod > 0 || pCfg->hevcCfg.refLongtermPeriod > 0)
        param->useLongTerm = 1;
    else
        param->useLongTerm = 0;

    /* for CMD_ENC_SEQ_GOP_PARAM */
    param->gopPresetIdx     = pCfg->hevcCfg.gopPresetIdx;

    /* for CMD_ENC_SEQ_INTRA_PARAM */
    param->decodingRefreshType = pCfg->hevcCfg.decodingRefreshType;
    param->intraPeriod      = pCfg->hevcCfg.intraPeriod;
    param->intraQP          = pCfg->hevcCfg.intraQP;
    param->forcedIdrHeaderEnable    = pCfg->hevcCfg.forcedIdrHeaderEnable;

    /* for CMD_ENC_SEQ_CONF_WIN_TOP_BOT/LEFT_RIGHT */
    param->confWinTop    = pCfg->hevcCfg.confWinTop;
    param->confWinBot    = pCfg->hevcCfg.confWinBot;
    param->confWinLeft   = pCfg->hevcCfg.confWinLeft;
    param->confWinRight  = pCfg->hevcCfg.confWinRight;

    /* for CMD_ENC_SEQ_INDEPENDENT_SLICE */
    param->independSliceMode     = pCfg->hevcCfg.independSliceMode;
    param->independSliceModeArg  = pCfg->hevcCfg.independSliceModeArg;

    /* for CMD_ENC_SEQ_DEPENDENT_SLICE */
    param->dependSliceMode     = pCfg->hevcCfg.dependSliceMode;
    param->dependSliceModeArg  = pCfg->hevcCfg.dependSliceModeArg;

    /* for CMD_ENC_SEQ_INTRA_REFRESH_PARAM */
    param->intraRefreshMode     = pCfg->hevcCfg.intraRefreshMode;
    param->intraRefreshArg      = pCfg->hevcCfg.intraRefreshArg;
    param->useRecommendEncParam = pCfg->hevcCfg.useRecommendEncParam;

    /* for CMD_ENC_PARAM */
    param->scalingListEnable        = pCfg->hevcCfg.scalingListEnable;
    param->cuSizeMode               = pCfg->hevcCfg.cuSizeMode;
    param->tmvpEnable               = pCfg->hevcCfg.tmvpEnable;
    param->wppEnable                = pCfg->hevcCfg.wppenable;
    param->maxNumMerge              = pCfg->hevcCfg.maxNumMerge;
    param->dynamicMerge8x8Enable    = pCfg->hevcCfg.dynamicMerge8x8Enable;
    param->dynamicMerge16x16Enable  = pCfg->hevcCfg.dynamicMerge16x16Enable;
    param->dynamicMerge32x32Enable  = pCfg->hevcCfg.dynamicMerge32x32Enable;
    param->disableDeblk             = pCfg->hevcCfg.disableDeblk;
    param->lfCrossSliceBoundaryEnable   = pCfg->hevcCfg.lfCrossSliceBoundaryEnable;
    param->betaOffsetDiv2           = pCfg->hevcCfg.betaOffsetDiv2;
    param->tcOffsetDiv2             = pCfg->hevcCfg.tcOffsetDiv2;
    param->skipIntraTrans           = pCfg->hevcCfg.skipIntraTrans;
    param->saoEnable                = pCfg->hevcCfg.saoEnable;
    param->intraInInterSliceEnable  = pCfg->hevcCfg.intraInInterSliceEnable;
    param->intraNxNEnable           = pCfg->hevcCfg.intraNxNEnable;

    /* for CMD_ENC_RC_PARAM */
    pEncOP->rcEnable             = pCfg->RcEnable;
    pEncOP->initialDelay         = pCfg->RcInitDelay;
    param->cuLevelRCEnable       = pCfg->hevcCfg.cuLevelRCEnable;
    param->hvsQPEnable           = pCfg->hevcCfg.hvsQPEnable;
    param->hvsQpScaleEnable      = pCfg->hevcCfg.hvsQpScaleEnable;
    param->hvsQpScale            = pCfg->hevcCfg.hvsQpScale;

    param->ctuOptParam.roiDeltaQp= pCfg->hevcCfg.ctuOptParam.roiDeltaQp;
    param->intraQpOffset         = pCfg->hevcCfg.intraQpOffset;
    param->initBufLevelx8        = pCfg->hevcCfg.initBufLevelx8;
    param->bitAllocMode          = pCfg->hevcCfg.bitAllocMode;
    for (i = 0; i < MAX_GOP_NUM; i++) {
        param->fixedBitRatio[i] = pCfg->hevcCfg.fixedBitRatio[i];
    }

    /* for CMD_ENC_RC_MIN_MAX_QP */
    param->minQp             = pCfg->hevcCfg.minQp;
    param->maxQp             = pCfg->hevcCfg.maxQp;
    param->maxDeltaQp        = pCfg->hevcCfg.maxDeltaQp;
    param->transRate         = pCfg->hevcCfg.transRate;
    pEncOP->bitRate          = bitrate;
    param->gopParam.enTemporalLayerQp = pCfg->hevcCfg.gopParam.enTemporalLayerQp;
    param->gopParam.tidQp0            = pCfg->hevcCfg.gopParam.tidQp0;
    param->gopParam.tidQp1            = pCfg->hevcCfg.gopParam.tidQp1;
    param->gopParam.tidQp2            = pCfg->hevcCfg.gopParam.tidQp2;
    param->gopParam.tidPeriod0        = pCfg->hevcCfg.gopParam.tidPeriod0;

    /* for CMD_ENC_CUSTOM_GOP_PARAM */
    param->gopParam.customGopSize     = pCfg->hevcCfg.gopParam.customGopSize;
    param->gopParam.useDeriveLambdaWeight = pCfg->hevcCfg.gopParam.useDeriveLambdaWeight;

    for (i= 0; i<param->gopParam.customGopSize; i++) {
        param->gopParam.picParam[i].picType      = pCfg->hevcCfg.gopParam.picParam[i].picType;
        param->gopParam.picParam[i].pocOffset    = pCfg->hevcCfg.gopParam.picParam[i].pocOffset;
        param->gopParam.picParam[i].picQp        = pCfg->hevcCfg.gopParam.picParam[i].picQp;
        param->gopParam.picParam[i].refPocL0     = pCfg->hevcCfg.gopParam.picParam[i].refPocL0;
        param->gopParam.picParam[i].refPocL1     = pCfg->hevcCfg.gopParam.picParam[i].refPocL1;
        param->gopParam.picParam[i].temporalId   = pCfg->hevcCfg.gopParam.picParam[i].temporalId;
        param->gopParam.gopPicLambda[i]           = pCfg->hevcCfg.gopParam.gopPicLambda[i];              
    }

    param->ctuOptParam.roiEnable = pCfg->hevcCfg.ctuOptParam.roiEnable;
    param->ctuOptParam.ctuQpEnable = pCfg->hevcCfg.ctuOptParam.ctuQpEnable;
    // VPS & VUI

    param->numUnitsInTick       = pCfg->hevcCfg.numUnitsInTick;
    param->timeScale            = pCfg->hevcCfg.timeScale;
    param->numTicksPocDiffOne   = pCfg->hevcCfg.numTicksPocDiffOne;

    param->vuiParam.vuiParamFlags       = pCfg->hevcCfg.vuiParam.vuiParamFlags;
    param->vuiParam.vuiAspectRatioIdc   = pCfg->hevcCfg.vuiParam.vuiAspectRatioIdc;
    param->vuiParam.vuiSarSize          = pCfg->hevcCfg.vuiParam.vuiSarSize;
    param->vuiParam.vuiOverScanAppropriate  = pCfg->hevcCfg.vuiParam.vuiOverScanAppropriate;
    param->vuiParam.videoSignal         = pCfg->hevcCfg.vuiParam.videoSignal;
    param->vuiParam.vuiChromaSampleLoc  = pCfg->hevcCfg.vuiParam.vuiChromaSampleLoc;
    param->vuiParam.vuiDispWinLeftRight = pCfg->hevcCfg.vuiParam.vuiDispWinLeftRight;
    param->vuiParam.vuiDispWinTopBottom = pCfg->hevcCfg.vuiParam.vuiDispWinTopBottom;

    pEncOP->encodeVuiRbsp        = pCfg->hevcCfg.vuiDataEnable;
    pEncOP->vuiRbspDataSize      = pCfg->hevcCfg.vuiDataSize;
    pEncOP->encodeHrdRbspInVPS   = pCfg->hevcCfg.hrdInVPS;
    pEncOP->hrdRbspDataSize      = pCfg->hevcCfg.hrdDataSize;
    pEncOP->encodeHrdRbspInVUI   = pCfg->hevcCfg.hrdInVUI;

    param->chromaCbQpOffset = pCfg->hevcCfg.chromaCbQpOffset;
    param->chromaCrQpOffset = pCfg->hevcCfg.chromaCrQpOffset;
    param->initialRcQp      = pCfg->hevcCfg.initialRcQp;

    param->nrYEnable        = pCfg->hevcCfg.nrYEnable;
    param->nrCbEnable       = pCfg->hevcCfg.nrCbEnable;
    param->nrCrEnable       = pCfg->hevcCfg.nrCrEnable;
    param->nrNoiseEstEnable = pCfg->hevcCfg.nrNoiseEstEnable;
    param->nrNoiseSigmaY    = pCfg->hevcCfg.nrNoiseSigmaY;
    param->nrNoiseSigmaCb   = pCfg->hevcCfg.nrNoiseSigmaCb;
    param->nrNoiseSigmaCr   = pCfg->hevcCfg.nrNoiseSigmaCr;
    param->nrIntraWeightY   = pCfg->hevcCfg.nrIntraWeightY;
    param->nrIntraWeightCb  = pCfg->hevcCfg.nrIntraWeightCb;
    param->nrIntraWeightCr  = pCfg->hevcCfg.nrIntraWeightCr;
    param->nrInterWeightY   = pCfg->hevcCfg.nrInterWeightY;
    param->nrInterWeightCb  = pCfg->hevcCfg.nrInterWeightCb;
    param->nrInterWeightCr  = pCfg->hevcCfg.nrInterWeightCr;

    param->intraMinQp       = pCfg->hevcCfg.intraMinQp;
    param->intraMaxQp       = pCfg->hevcCfg.intraMaxQp;


    return 1;
}

int setCoda9EncOpenParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig, ENC_CFG *pCfg)
{
    Int32   bitFormat;
    Int32   srcWidth;
    Int32   srcHeight;
    Int32   outputNum;

    bitFormat = pEncOP->bitstreamFormat;

    srcWidth  = (pEncConfig->picWidth > 0)  ? pEncConfig->picWidth  : pCfg->PicX;
    srcHeight = (pEncConfig->picHeight > 0) ? pEncConfig->picHeight : pCfg->PicY;
    outputNum = (pEncConfig->outNum > 0)    ? pEncConfig->outNum    : pCfg->NumFrame;

    pEncConfig->outNum      = outputNum;
    osal_memcpy(pEncConfig->skipPicNums, pCfg->skipPicNums, sizeof(pCfg->skipPicNums));
    pEncOP->picWidth        = srcWidth;
    pEncOP->picHeight       = srcHeight;
    pEncOP->frameRateInfo   = pCfg->FrameRate;
    pEncOP->bitRate         = pCfg->RcBitRate;	
    pEncOP->rcInitDelay     = pCfg->RcInitDelay;
    pEncOP->vbvBufferSize           = pCfg->RcBufSize;
    pEncOP->frameSkipDisable        = pCfg->frameSkipDisable;   // for compare with C-model ( C-model = only 1 )
    pEncOP->meBlkMode               = pCfg->MeBlkModeEnable;    // for compare with C-model ( C-model = only 0 )
    pEncOP->gopSize                 = pCfg->GopPicNum;
    pEncOP->idrInterval             = pCfg->IDRInterval;
    pEncOP->sliceMode.sliceMode     = pCfg->SliceMode;
    pEncOP->sliceMode.sliceSizeMode = pCfg->SliceSizeMode;
    pEncOP->sliceMode.sliceSize     = pCfg->SliceSizeNum;
    pEncOP->intraRefreshNum         = pCfg->IntraRefreshNum;
    pEncOP->ConscIntraRefreshEnable = pCfg->ConscIntraRefreshEnable;
    pEncOP->rcIntraQp = pCfg->RCIntraQP;	
    pEncOP->intraCostWeight = pCfg->intraCostWeight;



    pEncOP->MESearchRange = pCfg->SearchRange;	
    pEncOP->rcEnable = pCfg->RcEnable;
    if (!pCfg->RcEnable)
        pEncOP->bitRate = 0;

    if (!pCfg->GammaSetEnable)
        pEncOP->userGamma = -1;
    else
        pEncOP->userGamma = pCfg->Gamma;
    pEncOP->MEUseZeroPmv = pCfg->MeUseZeroPmv;
    /* It was agreed that the statements below would be used. but Cmodel at r25518 is not changed yet according to the statements below   
    if (bitFormat == STD_MPEG4)
    pEncOP->MEUseZeroPmv = 1;			
    else
    pEncOP->MEUseZeroPmv = 0;		
    */
    // MP4 263 Only
    if (!pCfg->ConstantIntraQPEnable)
        pEncOP->rcIntraQp = -1;

    if (pCfg->MaxQpSetEnable)
        pEncOP->userQpMax       = pCfg->MaxQp;	
    else
        pEncOP->userQpMax       = -1;
    // H.264 Only
    if (bitFormat == STD_AVC)
    {
        if(pCfg->MinQpSetEnable)
            pEncOP->userQpMin            = pCfg->MinQp;
        else
            pEncOP->userQpMin = 12;

        if (pCfg->MaxQpSetEnable)
            pEncOP->userQpMax = pCfg->MaxQp;	
        else
            pEncOP->userQpMax = 51;
        if(pCfg->MaxDeltaQpSetEnable)
            pEncOP->userMaxDeltaQp = pCfg->MaxDeltaQp;
        else
            pEncOP->userMaxDeltaQp = -1;

        if(pCfg->MinDeltaQpSetEnable)
            pEncOP->userMinDeltaQp       = pCfg->MinDeltaQp;
        else
            pEncOP->userMinDeltaQp       = -1;


    }
    pEncOP->rcIntervalMode = pCfg->rcIntervalMode;		// 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    pEncOP->mbInterval = pCfg->RcMBInterval;			// FIXME

    // Standard specific
    if( bitFormat == STD_MPEG4 ) {
        pEncOP->EncStdParam.mp4Param.mp4DataPartitionEnable = pCfg->DataPartEn;
        pEncOP->EncStdParam.mp4Param.mp4ReversibleVlcEnable = pCfg->RevVlcEn;
        pEncOP->EncStdParam.mp4Param.mp4IntraDcVlcThr = pCfg->IntraDcVlcThr;
        pEncOP->EncStdParam.mp4Param.mp4HecEnable	= pCfg->HecEnable;
        pEncOP->EncStdParam.mp4Param.mp4Verid = pCfg->VerId;		
    }
    else if( bitFormat == STD_H263 ) {
        pEncOP->EncStdParam.h263Param.h263AnnexIEnable = pCfg->AnnexI;
        pEncOP->EncStdParam.h263Param.h263AnnexJEnable = pCfg->AnnexJ;
        pEncOP->EncStdParam.h263Param.h263AnnexKEnable = pCfg->AnnexK;
        pEncOP->EncStdParam.h263Param.h263AnnexTEnable = pCfg->AnnexT;		
    }
    else if( bitFormat == STD_AVC ) {
        pEncOP->EncStdParam.avcParam.constrainedIntraPredFlag = pCfg->ConstIntraPredFlag;
        pEncOP->EncStdParam.avcParam.disableDeblk = pCfg->DisableDeblk;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetAlpha = pCfg->DeblkOffsetA;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetBeta = pCfg->DeblkOffsetB;
        pEncOP->EncStdParam.avcParam.chromaQpOffset = pCfg->ChromaQpOffset;
        pEncOP->EncStdParam.avcParam.audEnable = pCfg->aud_en;
        pEncOP->EncStdParam.avcParam.frameCroppingFlag = pCfg->frameCroppingFlag;   
        pEncOP->EncStdParam.avcParam.frameCropLeft = pCfg->frameCropLeft;
        pEncOP->EncStdParam.avcParam.frameCropRight = pCfg->frameCropRight;
        pEncOP->EncStdParam.avcParam.frameCropTop = pCfg->frameCropTop;
        pEncOP->EncStdParam.avcParam.frameCropBottom = pCfg->frameCropBottom;
        pEncOP->EncStdParam.avcParam.level = pCfg->level;

        // Update cropping information : Usage example for H.264 frame_cropping_flag
        if (pEncOP->picHeight == 1080) 
        {
            // In case of AVC encoder, when we want to use unaligned display width(For example, 1080),
            // frameCroppingFlag parameters should be adjusted to displayable rectangle
            if (pEncConfig->rotAngle != 90 && pEncConfig->rotAngle != 270) // except rotation
            {        
                if (pEncOP->EncStdParam.avcParam.frameCroppingFlag == 0) 
                {
                    pEncOP->EncStdParam.avcParam.frameCroppingFlag = 1;
                    // frameCropBottomOffset = picHeight(MB-aligned) - displayable rectangle height
                    pEncOP->EncStdParam.avcParam.frameCropBottom = 8;
                }
            }
        }
    }
    else {
        VLOG(ERR, "Invalid codec standard mode \n" );
        return 0;
    }
    return 1;
}

#if 0
void changeRcParaTest(Uint32 core_idx, EncHandle handle, 
    osal_file_t bsFp, 
    EncParam *pEncParam, 
    EncHeaderParam *pEncHeaderParam,
    TestEncConfig *pEncConfig,
    EncOpenParam	*pEncOP)
{
    int optionNum;

    while(1)
    {
        printf("\n	0: GOP number change\n");
        printf("	1: Intra Qp change\n");
        printf("	2: Bit Rate change\n");
        printf("	3: Frame Rate change\n");
        printf("	4: Intra Refresh Number change\n");
        printf("	5: Slice Mode change\n");
        if(pEncOP->bitstreamFormat == STD_MPEG4)
        {
            printf("	6: HEC Mode change\n");
        }

        printf("	9: go encoding\n");
        scanf("%d", &optionNum);

        switch(optionNum){
        case 0:
            {
                int newGopNum=0;
                printf("\n	New Gop Number=");
                scanf("%d", &newGopNum);
                VPU_EncGiveCommand(handle, ENC_SET_GOP_NUMBER, &newGopNum);
            }
            break;
        case 1:
            {
                int newIntraQp=0;
                printf("\n	New Intra Qp value=");
                scanf("%d", &newIntraQp);
                VPU_EncGiveCommand(handle, ENC_SET_INTRA_QP, &newIntraQp);
            }
            break;
        case 2:
            {
                int newBitrate=0;
                printf("\n	New Bit Rate=");
                scanf("%d", &newBitrate);
                VPU_EncGiveCommand(handle, ENC_SET_BITRATE, &newBitrate);
            }
            break;
        case 3:
            {
                int newFramerate=0;
                printf("\n	New Frame Rate=");
                scanf("%d", &newFramerate);
                VPU_EncGiveCommand(handle, ENC_SET_FRAME_RATE, &newFramerate);
                if( pEncOP->bitstreamFormat == STD_MPEG4 ) 
                {
                    pEncHeaderParam->headerType = VOL_HEADER;
                    VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, pEncHeaderParam); 
#ifdef SUPPORT_FFMPEG_DEMUX
                    if( pEncOP->ringBufferEnable == 0 && !pEncConfig->en_container ) 
#else
                    if( pEncOP->ringBufferEnable == 0 ) 
#endif
                    {
#if 0
                        if (!ReadBsResetBufHelper(core_idx, bsFp, pEncHeaderParam->buf, pEncHeaderParam->size, pEncOP->streamEndian))
                            break;
#else
                        BitstreamReader_Act(bsReader, core_idx, pEncHeaderParam->buf, pEncHeaderParam->size, pEncOP->streamEndian, comparator);
#endif
                    }
                }
            }
            break;
        case 4:
            {
                int newIntraRefreshNum=0;
                printf("\n	New Intra Refresh Number=");
                scanf("%d", &newIntraRefreshNum);
                VPU_EncGiveCommand(handle, ENC_SET_INTRA_MB_REFRESH_NUMBER, &newIntraRefreshNum);
            }
            break;
        case 5:
            {
                EncSliceMode newSlice;
                printf("\n	New Slice Mode[0:one slice, 1:muliple slice]=");
                scanf("%d", &newSlice.sliceMode);
                if(!newSlice.sliceMode)
                {
                    newSlice.sliceSizeMode	= 0;
                    newSlice.sliceSize		= 0;
                }
                else
                {
                    printf("\n	New Slice Size Mode[0:bit number, 1:mb number]=");
                    scanf("%d", &newSlice.sliceSizeMode);
                        if(!newSlice.sliceSizeMode)
                        {
                            printf("\n	New Slice bit number=");
                        }
                        else
                        {
                            printf("\n	New Slice MB number=");
                        }
                        scanf("%d", &newSlice.sliceSize);
                }

                VPU_EncGiveCommand(handle, ENC_SET_SLICE_INFO, &newSlice);
            }
            break;
        case 6:
            {
                int newHecMode=0;
                printf("\n	New Hec Mode Enable[0:disable, 1:enable]=");
                scanf("%d", &newHecMode);
                if(newHecMode > 0)
                {
                    VPU_EncGiveCommand(handle, ENC_ENABLE_HEC, &newHecMode);
                }
                else
                {
                    VPU_EncGiveCommand(handle, ENC_DISABLE_HEC, &newHecMode);
                }

            }
            break;
        default:
            break;
        }
        if(optionNum == 9)
            break;
    }

}
#endif

/******************************************************************************
EncOpenParam Initialization
******************************************************************************/
/**
* To init EncOpenParam by runtime evaluation
* IN
*   EncConfigParam *pEncConfig
* OUT
*   EncOpenParam *pEncOP
*/
Int32 GetEncOpenParamDefault(EncOpenParam *pEncOP, TestEncConfig *pEncConfig)
{
    int bitFormat;

    pEncConfig->outNum              = pEncConfig->outNum == 0 ? DEFAULT_ENC_OUTPUT_NUM : pEncConfig->outNum;
    bitFormat                       = pEncOP->bitstreamFormat;

    pEncOP->picWidth                = pEncConfig->picWidth;
    pEncOP->picHeight               = pEncConfig->picHeight;
    pEncOP->frameRateInfo           = 30;
    pEncOP->MESearchRange           = 3;
    pEncOP->bitRate                 = pEncConfig->kbps;
    pEncOP->rcInitDelay             = 0;
    pEncOP->vbvBufferSize           = 0;        // 0 = ignore
    pEncOP->meBlkMode               = 0;        // for compare with C-model ( C-model = only 0 )
    pEncOP->frameSkipDisable        = 1;        // for compare with C-model ( C-model = only 1 )
    pEncOP->gopSize                 = 30;       // only first picture is I
    pEncOP->sliceMode.sliceMode     = 1;        // 1 slice per picture
    pEncOP->sliceMode.sliceSizeMode = 1;
    pEncOP->sliceMode.sliceSize     = 115;
    pEncOP->intraRefreshNum         = 0;
    pEncOP->rcIntraQp               = -1;       // disable == -1
    pEncOP->userQpMax               = -1;				// disable == -1
    pEncOP->userGamma               = (Uint32)(0.75*32768);   //  (0*32768 < gamma < 1*32768)
    pEncOP->rcIntervalMode          = 1;                        // 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    pEncOP->mbInterval              = 0;
    pEncConfig->picQpY              = 23;

    if (bitFormat == STD_MPEG4)
        pEncOP->MEUseZeroPmv = 1;            
    else
        pEncOP->MEUseZeroPmv = 0;            

    pEncOP->intraCostWeight = 400;

    // Standard specific
    if( bitFormat == STD_MPEG4 ) {
        pEncOP->EncStdParam.mp4Param.mp4DataPartitionEnable = 0;
        pEncOP->EncStdParam.mp4Param.mp4ReversibleVlcEnable = 0;
        pEncOP->EncStdParam.mp4Param.mp4IntraDcVlcThr = 0;
        pEncOP->EncStdParam.mp4Param.mp4HecEnable	= 0;
        pEncOP->EncStdParam.mp4Param.mp4Verid = 2;		
    }
    else if( bitFormat == STD_H263 ) {
        pEncOP->EncStdParam.h263Param.h263AnnexIEnable = 0;
        pEncOP->EncStdParam.h263Param.h263AnnexJEnable = 0;
        pEncOP->EncStdParam.h263Param.h263AnnexKEnable = 0;
        pEncOP->EncStdParam.h263Param.h263AnnexTEnable = 0;		
    }
    else if( bitFormat == STD_AVC ) {
        pEncOP->EncStdParam.avcParam.constrainedIntraPredFlag = 0;
        pEncOP->EncStdParam.avcParam.disableDeblk = 1;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetAlpha = 6;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetBeta = 0;
        pEncOP->EncStdParam.avcParam.chromaQpOffset = 10;
        pEncOP->EncStdParam.avcParam.audEnable = 0;
        pEncOP->EncStdParam.avcParam.frameCroppingFlag = 0;
        pEncOP->EncStdParam.avcParam.frameCropLeft = 0;
        pEncOP->EncStdParam.avcParam.frameCropRight = 0;
        pEncOP->EncStdParam.avcParam.frameCropTop = 0;
        pEncOP->EncStdParam.avcParam.frameCropBottom = 0;
        pEncOP->EncStdParam.avcParam.level = 0;

        // Update cropping information : Usage example for H.264 frame_cropping_flag
        if (pEncOP->picHeight == 1080) 
        {
            // In case of AVC encoder, when we want to use unaligned display width(For example, 1080),
            // frameCroppingFlag parameters should be adjusted to displayable rectangle
            if (pEncConfig->rotAngle != 90 && pEncConfig->rotAngle != 270) // except rotation
            {        
                if (pEncOP->EncStdParam.avcParam.frameCroppingFlag == 0) 
                {
                    pEncOP->EncStdParam.avcParam.frameCroppingFlag = 1;
                    // frameCropBottomOffset = picHeight(MB-aligned) - displayable rectangle height
                    pEncOP->EncStdParam.avcParam.frameCropBottom = 8;
                }
            }
        }
    }
    else if( bitFormat == STD_HEVC ) {
        EncHevcParam *param = &pEncOP->EncStdParam.hevcParam;
        Int32   rcBitrate   = pEncConfig->kbps * 1000;
        Int32   i=0;

        pEncOP->bitRate         = rcBitrate;
        param->profile          = HEVC_PROFILE_MAIN;
        param->level            = 0;
        param->tier             = 0;
        param->internalBitDepth = 8;
        pEncOP->srcBitDepth     = 8;
        param->chromaFormatIdc  = 0;
        param->losslessEnable   = 0;
        param->constIntraPredFlag = 0;
        param->enableAFBCD        = 0;
        param->useLongTerm  = 0;

        /* for CMD_ENC_SEQ_GOP_PARAM */
        param->gopPresetIdx     = PRESET_IDX_IBBBP;

        /* for CMD_ENC_SEQ_INTRA_PARAM */
        param->decodingRefreshType = 1;
        param->intraPeriod         = 28;
        param->intraQP             = 0;

        /* for CMD_ENC_SEQ_CONF_WIN_TOP_BOT/LEFT_RIGHT */
        param->confWinTop    = 0;
        param->confWinBot    = 0;
        param->confWinLeft   = 0;
        param->confWinRight  = 0;

        /* for CMD_ENC_SEQ_INDEPENDENT_SLICE */
        param->independSliceMode     = 0;
        param->independSliceModeArg  = 0;

        /* for CMD_ENC_SEQ_DEPENDENT_SLICE */
        param->dependSliceMode     = 0;
        param->dependSliceModeArg  = 0;

        /* for CMD_ENC_SEQ_INTRA_REFRESH_PARAM */
        param->intraRefreshMode     = 0;
        param->intraRefreshArg      = 0;
        param->useRecommendEncParam = 1;

        pEncConfig->seiDataEnc.prefixSeiNalEnable   = 0;
        pEncConfig->seiDataEnc.suffixSeiNalEnable   = 0;

        pEncOP->encodeHrdRbspInVPS  = 0;
        pEncOP->encodeHrdRbspInVUI  = 0;
        pEncOP->encodeVuiRbsp       = 0;

        pEncConfig->roi_enable = 0;
        pEncConfig->ctu_qp_enable   = 0;
        /* for CMD_ENC_PARAM */
        if (param->useRecommendEncParam != 1) {		// 0 : Custom,  2 : Boost mode (normal encoding speed, normal picture quality),  3 : Fast mode (high encoding speed, low picture quality)
			param->scalingListEnable        = 0;
            param->cuSizeMode               = 0x7;  
            param->tmvpEnable               = 1;
            param->wppEnable                = 0;
            param->maxNumMerge              = 2;
            param->dynamicMerge8x8Enable    = 1;
            param->dynamicMerge16x16Enable  = 1;
            param->dynamicMerge32x32Enable  = 1;
            param->disableDeblk             = 0;
            param->lfCrossSliceBoundaryEnable   = 1;
            param->betaOffsetDiv2           = 0;
            param->tcOffsetDiv2             = 0;
            param->skipIntraTrans           = 1;
            param->saoEnable                = 1;
            param->intraInInterSliceEnable  = 1;
            param->intraNxNEnable           = 1;
        }
		
        /* for CMD_ENC_RC_PARAM */
        pEncOP->rcEnable             = rcBitrate == 0 ? FALSE : TRUE;
        pEncOP->initialDelay         = 3000;
        param->ctuOptParam.roiEnable    = 0;
        param->ctuOptParam.roiDeltaQp   = 3;
        param->ctuOptParam.ctuQpEnable  = 0;
        param->intraQpOffset         = 0;
        param->initBufLevelx8        = 1;
        param->bitAllocMode          = 0;
        for (i = 0; i < MAX_GOP_NUM; i++) {
            param->fixedBitRatio[i] = 1;
        }
        param->cuLevelRCEnable       = 0;
        param->hvsQPEnable           = 1;
		param->hvsQpScale            = 2;
		param->hvsQpScaleEnable      = (param->hvsQpScale > 0) ? 1: 0;

        /* for CMD_ENC_RC_MIN_MAX_QP */
        param->minQp             = 8;
        param->maxQp             = 51;
        param->maxDeltaQp        = 10;


        param->minQpI            = 8;
        param->maxQpI            = 51;
        param->minQpP            = 8;
        param->maxQpP            = 51;
        param->minQpB            = 8;
        param->maxQpB            = 51;

        param->hvsMaxDeltaQp     = 10;

        param->gopParam.enTemporalLayerQp = 0;
        param->gopParam.tidQp0            = 30;
        param->gopParam.tidQp1            = 33;
        param->gopParam.tidQp2            = 36;
        param->gopParam.tidPeriod0        = 60;

        /* for CMD_ENC_CUSTOM_GOP_PARAM */
        param->gopParam.customGopSize     = 0;
        param->gopParam.useDeriveLambdaWeight = 0;

        for (i= 0; i<param->gopParam.customGopSize; i++) {
            param->gopParam.picParam[i].picType      = PIC_TYPE_I;
            param->gopParam.picParam[i].pocOffset    = 1;
            param->gopParam.picParam[i].picQp        = 30;
            param->gopParam.picParam[i].refPocL0     = 0;
            param->gopParam.picParam[i].refPocL1     = 0;
            param->gopParam.picParam[i].temporalId   = 0;
            param->gopParam.gopPicLambda[i]           = 0;              
        }

        param->transRate = 0;

        // for VUI / time information.
        param->numTicksPocDiffOne   = 0;
        param->timeScale            =  pEncOP->frameRateInfo * 1000;
        param->numUnitsInTick       =  1000;

        param->vuiParam.vuiParamFlags       = 0;                // when vuiParamFlags == 0, VPU doesn't encode VUI

        param->chromaCbQpOffset = 0;
        param->chromaCrQpOffset = 0;
        param->initialRcQp      = 63;       // 63 is meaningless.
        param->nrYEnable        = 0;
        param->nrCbEnable       = 0;
        param->nrCrEnable       = 0;
        param->nrNoiseEstEnable = 0;

        param->intraMinQp       = 8;
        param->intraMaxQp       = 51;

        


        param->rcWeightParam                = 2;
        param->rcWeightBuf                  = 128;
        // for H.264 encoder
        param->avcIdrPeriod                 = 0;
        param->rdoSkip                      = 1;
        param->lambdaScalingEnable          = 1;

        param->transform8x8Enable           = 1;
        param->avcSliceMode                 = 0;
        param->avcSliceArg                  = 0;
        param->intraMbRefreshMode           = 0;
        param->intraMbRefreshArg            = 1;
        param->mbLevelRcEnable              = 0;
        param->entropyCodingMode            = 1;
        if (bitFormat == STD_AVC)
            param->disableDeblk             = 2;

    }
    else {
        VLOG(ERR, "Invalid codec standard mode: codec index(%d) \n", bitFormat);
        return 0;
    }


    return 1;
}


/**
* To init EncOpenParam by CFG file
* IN
*   EncConfigParam *pEncConfig
* OUT
*   EncOpenParam *pEncOP
*   char *srcYuvFileName
*/
Int32 GetEncOpenParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig, ENC_CFG *pEncCfg)
{
    int bitFormat;
    ENC_CFG encCfgInst;
    ENC_CFG *pCfg;
    char yuvDir[256] = "yuv/";

    // Source YUV Image File to load
    if (pEncCfg) {
        pCfg = pEncCfg;
    }
    else {
        osal_memset( &encCfgInst, 0x00, sizeof(ENC_CFG));
        pCfg = &encCfgInst;
    }
    bitFormat = pEncOP->bitstreamFormat;
    switch(bitFormat) {
    case STD_AVC:
        if (parseAvcCfgFile(pCfg, pEncConfig->cfgFileName) == 0)
            return 0;
        pEncConfig->picQpY = pCfg->PicQpY;	
        if (pEncCfg)
            strcpy(pEncConfig->yuvFileName, pCfg->SrcFileName);			
        else
            sprintf(pEncConfig->yuvFileName,  "%s%s", yuvDir, pCfg->SrcFileName);		

#ifdef SUPPORT_980_RC_LIB
        if(pEncOP->EncStdParam.avcParam.svcExtension == TRUE)
        {
            if(pCfg->RcEnable != 0)
                pCfg->RcEnable = 4; // Other modes are not supported.

            if(pCfg->field_flag == 1) // Field is not supported
                return 0;
        }

        if (pCfg->coda9RoiEnable) {
            strcpy(pEncConfig->roi_file_name, pCfg->RoiFile);
        }
#endif
        break;
    case STD_MPEG4:
    case STD_H263:
        if (parseMp4CfgFile(pCfg, pEncConfig->cfgFileName) == 0)
            return 0;
        pEncConfig->picQpY = pCfg->VopQuant;
        if (pEncCfg)
            strcpy(pEncConfig->yuvFileName, pCfg->SrcFileName);			
        else
            sprintf(pEncConfig->yuvFileName,  "%s%s", yuvDir, pCfg->SrcFileName);
        if (pCfg->ShortVideoHeader == 1) {
            pEncOP->bitstreamFormat = STD_H263;
            bitFormat = STD_H263;
        }
        break;
    case STD_HEVC:
        if (parseHevcCfgFile(pCfg, pEncConfig->cfgFileName) == 0)
            return 0;
        if (pEncCfg)
            strcpy(pEncConfig->yuvFileName, pCfg->SrcFileName);			
        else
            sprintf(pEncConfig->yuvFileName,  "%s%s", yuvDir, pCfg->SrcFileName);
        if (pEncConfig->bitstreamFileName[0] == 0 && pCfg->BitStreamFileName[0] != 0)
            sprintf(pEncConfig->bitstreamFileName, "%s", pCfg->BitStreamFileName);

        if ( pEncConfig->bitstreamFileName[0] == 0 )
            sprintf(pEncConfig->bitstreamFileName, "%s", "output_stream.265");

        if (pCfg->hevcCfg.ctuOptParam.roiEnable)
            strcpy(pEncConfig->roi_file_name, pCfg->hevcCfg.roiFileName);

        pEncConfig->roi_enable  = pCfg->hevcCfg.ctuOptParam.roiEnable;
        pEncConfig->roi_delta_qp= pCfg->hevcCfg.ctuOptParam.roiDeltaQp;

        if (pCfg->hevcCfg.prefixSeiEnable)
            strcpy(pEncConfig->prefix_sei_nal_file_name, pCfg->hevcCfg.prefixSeiDataFileName);

        if (pCfg->hevcCfg.suffixSeiEnable)
            strcpy(pEncConfig->suffix_sei_nal_file_name, pCfg->hevcCfg.suffixSeiDataFileName);

        pEncConfig->seiDataEnc.prefixSeiNalEnable       = pCfg->hevcCfg.prefixSeiEnable;
        pEncConfig->seiDataEnc.prefixSeiDataSize        = pCfg->hevcCfg.prefixSeiDataSize;
        pEncConfig->seiDataEnc.prefixSeiDataEncOrder    = pCfg->hevcCfg.prefixSeiTimingFlag;

        pEncConfig->seiDataEnc.suffixSeiNalEnable       = pCfg->hevcCfg.suffixSeiEnable;
        pEncConfig->seiDataEnc.suffixSeiDataSize        = pCfg->hevcCfg.suffixSeiDataSize;
        pEncConfig->seiDataEnc.suffixSeiDataEncOrder    = pCfg->hevcCfg.suffixSeiTimingFlag;

        if (pCfg->hevcCfg.hrdInVPS || pCfg->hevcCfg.hrdInVUI)
            strcpy(pEncConfig->hrd_rbsp_file_name, pCfg->hevcCfg.hrdDataFileName);

        if (pCfg->hevcCfg.vuiDataEnable)
            strcpy(pEncConfig->vui_rbsp_file_name, pCfg->hevcCfg.vuiDataFileName);


        if (pCfg->hevcCfg.ctuOptParam.ctuQpEnable)
            strcpy(pEncConfig->ctuqp_file_name, pCfg->hevcCfg.ctuQpFileName);

        pEncConfig->ctu_qp_enable = pCfg->hevcCfg.ctuOptParam.ctuQpEnable;
        pEncConfig->encAUD  = pCfg->hevcCfg.encAUD;
        pEncConfig->encEOS  = pCfg->hevcCfg.encEOS;
        pEncConfig->encEOB  = pCfg->hevcCfg.encEOB;
        pEncConfig->useAsLongtermPeriod = pCfg->hevcCfg.useAsLongtermPeriod;
        pEncConfig->refLongtermPeriod   = pCfg->hevcCfg.refLongtermPeriod;


        break;
    default :
        break;
    }

    if (bitFormat == STD_HEVC) {
        if (setWaveEncOpenParam(pEncOP, pEncConfig, pCfg) == 0)
            return 0;
    }
    else {
        if (setCoda9EncOpenParam(pEncOP, pEncConfig, pCfg) == 0)
            return 0;
    }

    return 1;
}

int ReadBsResetBufHelper(Uint32 core_idx,
    osal_file_t streamFp,
    PhysicalAddress bitstream,
    int size,
    int endian)
{
    unsigned char *buf = osal_malloc(size);

    if (!buf)
    {
        VLOG(ERR, "fail to allocate bitstream buffer\n" );
        return 0;
    }

    vdi_read_memory(core_idx, bitstream, buf, size, endian);
    osal_fwrite((void *)buf, sizeof(Uint8), size, streamFp);
    osal_fflush(streamFp);
    osal_free(buf);
    return 1;
}

/*
 * To be compatible with Ref-SW 4.0
 */

frame_queue_item_t* frame_queue_init(
    Int32 count
    )
{
    frame_queue_item_t* queue = NULL; 

    queue = (frame_queue_item_t *)osal_malloc(sizeof(frame_queue_item_t));
    if (!queue) {
        return NULL;
    }

    queue->size   = count;
    queue->count  = 0;
    queue->front  = 0;
    queue->rear   = 0;
    queue->buffer = (DecOutputInfo*)osal_malloc(count*sizeof(DecOutputInfo));

    return queue;
}

void frame_queue_deinit(
    frame_queue_item_t* queue
    )
{
    if (queue == NULL) {
        return;
    }

    if (queue->buffer) {
        osal_free(queue->buffer);
    }

    osal_free(queue);
}

/*
* Return 0 on success.
*	   -1 on failure
*/
Int32 frame_queue_enqueue(
    frame_queue_item_t* queue, 
    DecOutputInfo       data
    )
{
    if (queue == NULL) {
        return -1;
    }

    /* Queue is full */
    if (queue->count == queue->size) {
        return -1;
    }

    queue->buffer[queue->rear++] = data;
    queue->rear %= queue->size;
    queue->count++;

    return 0;
}

/*
* Return 0 on success.
*	   -1 on failure
*/
Int32 frame_queue_dequeue(
    frame_queue_item_t* queue, 
    DecOutputInfo*      data
    )
{
    if (queue == NULL) {
        return -1;
    }

    /* Queue is empty */
    if (queue->count == 0) {
        return -1;
    }

    *data = queue->buffer[queue->front++];
    queue->front %= queue->size;
    queue->count--;

    return 0;
}

Int32 frame_queue_dequeue_all(
    frame_queue_item_t* queue
    )
{
    Int32           ret;
    DecOutputInfo   data;

    if (queue == NULL) {
        return -1;
    }

    do {
        ret = frame_queue_dequeue(queue, &data);
        if (ret >=0) {
            VLOG(INFO, "Empty display Queue for flush display_index=%d\n", data.indexFrameDisplay);
        }
    } while (ret >= 0);	
    return 0;
}

Int32 frame_queue_peekqueue(
    frame_queue_item_t* queue, 
    DecOutputInfo*      data
    )
{
    if (queue == NULL) {
        return -1;
    }
    /* Queue is empty */
    if (queue->count == 0) {
        return -1;
    }

    *data = queue->buffer[queue->front];
    return 0;
}

Int32 frame_queue_check_in_queue(
    frame_queue_item_t* queue, 
    Int32               index
    )
{
    DecOutputInfo   data;
    Int32           front;
    Int32           count;

    if (queue == NULL) {
        return -1;
    }

    front = queue->front;
    count = queue->count;
    while(count > 0) {
        data = queue->buffer[front++];
        if (data.indexFrameDisplay == index) {
            return 1;
        }

        count--;
        front %= queue->size;
    }

    return 0;
}

Int32 frame_queue_count(
    frame_queue_item_t* queue
    )
{
    if (queue == NULL) {
        return -1;
    }

    return queue->count;
}

#ifdef SUPPORT_980_RC_LIB
#define       AVC_ENC_NUT_SLICE     1
#define       AVC_ENC_NUT_IDR       5
static frm_t *get_current_frame(
    EncInfo *seq)
{
    int i;
    frm_t *curr_frm = NULL;

    if (seq->openParam.gopSize== 1) //all intra
    {
        curr_frm = &seq->frm[0];
        seq->prev_idx = 0;
    }
    else
    {
        for (i = 0; i < seq->num_total_frames; i++)
        {
            if (seq->frm[i].used_for_ref == 0)
            {
                curr_frm = &seq->frm[i];
                seq->prev_idx = i;
                break;
            }
        }
    }

    return curr_frm;
}

static void set_mmco(
    EncInfo *seq)
{
    int frmidx;
    int next_idx = (seq->enc_idx_modulo == seq->gop_size - 1) ? 0 : seq->enc_idx_modulo + 1;
    rps_t *next_rps = &seq->rps[next_idx];
    int curr_gop_poc = seq->curr_frm->poc - seq->gop_entry[seq->enc_idx_modulo].curr_poc;
    int next_gop_poc = curr_gop_poc + ((next_idx == 0) ? seq->gop_size : 0);

    //mmco1 : set short-term as non-ref
    seq->num_mmco1 = 0;

    for (frmidx = 0; frmidx < seq->num_total_frames; frmidx++)
    {
        frm_t *frm = &seq->frm[frmidx];

        if (frm->used_for_ref && (frm != seq->curr_frm) && (frmidx != seq->longterm_frmidx) )
        {
            int i;
            int found = 0;

            //check if the ref-frame is in next rps
            for (i = 0; i < next_rps->num_poc; i++)
                if (frm->poc == next_gop_poc + next_rps->poc[i])
                    found = 1;

            if (!found)
            {
                seq->mmco1_frmidx[seq->num_mmco1++] = frmidx;
            }
        }
    }
}

static int search_ref_frame(
    EncInfo *seq,
    int poc)
{
    int i;
    for (i = 0; i < 31; i++)
        if (seq->frm[i].used_for_ref && seq->frm[i].poc == poc)
            return i;
    return -1;
}

static void set_refpic_list(
    EncInfo *seq,
    int ref_long_term)
{
    int ref, list;
    int num_ref, num_list;
    int enc_idx;
    int gop_poc;

    enc_idx = seq->enc_idx_modulo;

    gop_poc = seq->curr_frm->poc - seq->gop_entry[enc_idx].curr_poc;


    seq->num_ref_idx = 0;
    seq->ref_pic_list =0;

    //fill ref_list
    num_ref = 1;//(seq->curr_frm->slice_type == AVC_ENC_SLICE_TYPE_B || seq->gop_entry[enc_idx].use_multi_ref_p) ? 2 : 1;
    for (ref = 0; ref < num_ref; ref++)
    {
        int frmidx;
        int poc = gop_poc + seq->gop_entry[enc_idx].ref_poc;

        if (ref_long_term && (ref == num_ref - 1) && (seq->longterm_frmidx >= 0)) //long-term
            frmidx = seq->longterm_frmidx;
        else
            frmidx = search_ref_frame(seq, poc);

        if (frmidx != -1)
        {
            seq->ref_pic_list = frmidx;
            seq->num_ref_idx++;
        }
    }

    //when num_ref_idx is 0, fill ref_list with rps
    num_list =  1;
    for (list = 0; list < num_list; list++)
    {
        if (seq->num_ref_idx == 0)
        {
            int i;
            rps_t *rps = &seq->rps[enc_idx];

            for (i = 0; i < rps->num_poc; i++)
            {
                int poc = gop_poc + rps->poc[i];
                int frmidx = search_ref_frame(seq, poc);
                if (frmidx != -1)
                {
                    seq->ref_pic_list = frmidx;
                    seq->num_ref_idx++;
                    break;
                }
            }
        }
    }
}

static int get_nal_ref_idc(
    EncInfo *seq,
    int enc_idx)
{
    if (seq->idr_picture)
        return 3;
    else if (seq->gop_entry[enc_idx].curr_poc == seq->gop_size) //anchor
        return 2;
    else if (seq->curr_frm->used_for_ref)
        return 1;
    else
        return 0;
}

#define       AVC_ENC_NUT_SLICE     1
#define       AVC_ENC_NUT_IDR       5

void dpb_pic_init( EncInfo *pAvcInfo )
{

    int encode_frame;
    int enc_idx;// = pAvcInfo->enc_idx_modulo;
    int i;
    int gop_num = pAvcInfo->openParam.gopSize;

    encode_frame = pAvcInfo->encoded_frames_in_gop;
    enc_idx = pAvcInfo->enc_idx_modulo;


    if (gop_num == 1)   // All I
        encode_frame        = 0;
    else
    {
        if (encode_frame >= gop_num && gop_num != 0)   // I frame
            encode_frame        = 0;
    }


    pAvcInfo->idr_picture = 0;
    if(pAvcInfo->frm_cnt == 0)
    {
        pAvcInfo->Idr_cnt = 0;
        pAvcInfo->idr_picture = 1;
    }
    else
    {
        if((pAvcInfo->openParam.idrInterval) > 0 && (pAvcInfo->openParam.gopSize > 0))
        {
            if(encode_frame == 0)
            {
                pAvcInfo->Idr_cnt++;
            }

            if(pAvcInfo->openParam.idrInterval == pAvcInfo->Idr_cnt)
            {
                pAvcInfo->frm_cnt = 0;
                pAvcInfo->Idr_cnt = 0;
                pAvcInfo->idr_picture = 1;
            }
        }
    }

    if (pAvcInfo->idr_picture)
        pAvcInfo->Idr_picId = (pAvcInfo->Idr_picId + 1) & 0xFFFF; // Idr_pic_id: 0 ~ 65535


    pAvcInfo->slice_type = (encode_frame == 0) ? 0 : 1;


    if(pAvcInfo->enc_long_term_cnt > 0)
    {
        if(pAvcInfo->enc_long_term_cnt == pAvcInfo->long_term_period)
            pAvcInfo->enc_long_term_cnt = 0;
    }


    if(pAvcInfo->enc_long_term_cnt == 0 && pAvcInfo->long_term_period != -1 )
        pAvcInfo->curr_long_term = 1;// enc_long_term_cnt % pAvcInfo->long_term_period;
    else
        pAvcInfo->curr_long_term = 0;

    pAvcInfo->ref_long_term = pAvcInfo->curr_long_term || pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].ref_long_term;


    if(pAvcInfo->enc_idx_modulo >=0)
        pAvcInfo->src_idx = pAvcInfo->enc_idx_gop + pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].curr_poc;
    else
        pAvcInfo->src_idx = pAvcInfo->enc_idx_gop;


    pAvcInfo->num_mmco1 = 0;
    pAvcInfo->curr_frm = get_current_frame(pAvcInfo);


    if(pAvcInfo->idr_picture)
        pAvcInfo->src_idx_last_idr = pAvcInfo->src_idx;

    if (pAvcInfo->idr_picture )
    {
        //pAvcInfo->Idr_picId ^= 1;

        //clear all refs
        for (i = 0; i < 31; i++)
            pAvcInfo->frm[i].used_for_ref = 0;

        pAvcInfo->curr_frm->used_for_ref = 1;
        pAvcInfo->curr_frm->slice_type = 0;//I_SLICE;
        pAvcInfo->curr_frm->poc = 0;
        pAvcInfo->curr_frm->frame_num = 0;
    }
    else
    {
        int enc_idx = pAvcInfo->enc_idx_modulo;


        pAvcInfo->curr_frm->used_for_ref = pAvcInfo->rps[enc_idx].used_for_ref || pAvcInfo->curr_long_term;
        pAvcInfo->curr_frm->slice_type = pAvcInfo->slice_type;
        pAvcInfo->curr_frm->poc = pAvcInfo->src_idx - pAvcInfo->src_idx_last_idr;
        pAvcInfo->curr_frm->frame_num = (pAvcInfo->prev_frame_num + 1) % (1 << 4);

        if (pAvcInfo->curr_frm->used_for_ref)
            set_mmco(pAvcInfo);

        if (pAvcInfo->curr_frm->slice_type != 0) // I_SLICE)
            set_refpic_list(pAvcInfo,pAvcInfo->ref_long_term);
    }


    pAvcInfo->nal_ref_idc =  get_nal_ref_idc(pAvcInfo, enc_idx);
    pAvcInfo->nal_unit_type = pAvcInfo->idr_picture ? AVC_ENC_NUT_IDR : AVC_ENC_NUT_SLICE;

    //update prev_frame_num
    if (pAvcInfo->curr_frm->used_for_ref)
        pAvcInfo->prev_frame_num = pAvcInfo->curr_frm->frame_num;

//    pAvcInfo->RefPicList[0] = pAvcInfo->ref_pic_list;

    pAvcInfo->temporal_id = (pAvcInfo->enc_idx_modulo == -1) ? 0 : pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].temporal_id;

    if(pAvcInfo->curr_long_term)
        pAvcInfo->temporal_id = 0;

    //Update cnt
    pAvcInfo->enc_long_term_cnt++;
    pAvcInfo->frm_cnt++;

}
#endif


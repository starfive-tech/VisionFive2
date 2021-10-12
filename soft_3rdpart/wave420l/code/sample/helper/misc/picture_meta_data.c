//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include <assert.h>
#include <string.h>
#include "main_helper.h"

FILE * CreatePicInfoXmlFile(
    const char *yuvPath
    )
{
    char strXmlFilePath[256];
    FILE *fp;

    if (yuvPath != NULL) {
        sprintf(strXmlFilePath, "%s.xml", yuvPath);
    }
    else {
        strcpy(strXmlFilePath, "out.xml");
    }
    if ((fp=osal_fopen(strXmlFilePath, "wb")) == NULL) {
        printf("%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strXmlFilePath);
        return NULL;
    }

    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n",fp);
    fputs("<ffprobe>\r\n", fp);
    fputs("<frames>\r\n", fp);

    return fp;
}

void ClosePicInfoXMLFile(
    FILE *fpXML
    )
{
    if (fpXML != NULL) {
        fputs("</frames>\r\n", fpXML);
        fputs("</ffprobe>\r\n", fpXML);

        osal_fclose(fpXML);
    }
}

void SavePicInfoToXml(
    FILE*           fpXml, 
    DecOutputInfo*  fbInfo
    )
{
    //BOOL bCrop;
    BOOL   bKeyFrame;
    Uint32 pkt_pts;
    Uint32 pkt_dts;
    Uint32 pkt_pos;
    double pkt_pts_sec;
    double pkt_dts_sec;
    Uint32 pkt_size;
    Uint32 width;
    Uint32 height;
    Uint32 bitdepthY;
    Uint32 bitdepthC;
    char   strFormat[256] = {"yuv420p"};
    char   strPicType[8];

    pkt_size = fbInfo->consumedByte;
    pkt_pos = 0;
    pkt_size = 0;
    pkt_pts = 0;
    pkt_dts = 0;
    pkt_pts_sec = pkt_pts/1000.0;
    pkt_dts_sec = pkt_dts/1000.0;
    bKeyFrame = FALSE;
    switch(fbInfo->picType) {
    case PIC_TYPE_I:
        strcpy(strPicType, "I");
        bKeyFrame = TRUE;
        break;
    case PIC_TYPE_P:
        strcpy(strPicType, "P");
        break;
    case PIC_TYPE_B:
        strcpy(strPicType, "B");
        break;
    default:
        strcpy(strPicType, "B");
        break;
    }

    if (fbInfo->dispFrame.format == FORMAT_420) {
        if (fbInfo->dispFrame.cbcrInterleave == TRUE) {
            strcpy(strFormat, "nv12");
        }
        else {
            strcpy(strFormat, "yuv420p");
        }

        if (fbInfo->dispFrame.nv21 == TRUE) {
            strcpy(strFormat, "nv21");
        }
    }

    width     = fbInfo->dispPicWidth;
    height    = fbInfo->dispPicHeight;
    bitdepthY = fbInfo->dispFrame.lumaBitDepth;
    bitdepthC = fbInfo->dispFrame.chromaBitDepth;

    fprintf(fpXml, "<frame media_type=\"video\" key_frame=\"%d\" pkt_pts=\"%d\" pkt_pts_time=\"%.1f\" "
                   "pkt_dts=\"%d\" pkt_dts_time=\"%.1f\" pkt_pos=\"%d\" pkt_size=\"%d\" width=\"%d\" "
                   "height=\"%d\" pix_fmt=\"%s\" pict_type=\"%s\" coded_picture_number=\"0\" display_picture_number=\"0\""
                   "interlaced_frame=\"0\" top_field_first=\"0\" repeat_pict=\"0\" bitdepthY=\"%d\" bitdepthC=\"%d\"/>\r\n", 
            bKeyFrame, pkt_pts, pkt_pts_sec, pkt_dts, pkt_dts_sec, pkt_pos, pkt_size, width, height, strFormat, strPicType, bitdepthY, bitdepthC);
    osal_fflush(fpXml);

    return;
}
 

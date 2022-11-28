/**
  ******************************************************************************
  * @file sensor_phy.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief Implementation phy control for VIC phy
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */


#include "stf_types.h"
#include "stf_errors.h"
#include "ci/ci_api.h"
#include <stdlib.h>
#include <stdio.h>
#include "sensorapi/sensor_phy.h"
#if !file_i2c
#include <sys/param.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define LOG_TAG "SensorPHY"
#include "stf_common/userlog.h"

#ifdef SIFIVE_ISP

/* Local define. Add by feng.qu@infotm.com */
#define CI_BANK_CTX 2
#define IMGR_CTRL 0x00D0
#define BIT_IMGR_BAYER_FORMAT_SHIFT 16
#define BIT_IMGR_BAYER_FORMAT_WIDTH 2
#endif //SIFIVE_ISP

enum IMGR_BAYER_FORMAT_ENUM {
    /**
     * @brief Start on a line of Blue-Green followed by a line of Green-Red
     */
    IMGR_BAYER_FORMAT_BGGR = 0x3,
    /**
     * @brief Start on a line of Green-Blue, followed by a line of Red-Green
     */
    IMGR_BAYER_FORMAT_GBRG = 0x2,
    /**
     * @brief Start on a line of Green-Red followed by a line of Blue-Green
     */
    IMGR_BAYER_FORMAT_GRBG = 0x1,
    /**
     * @brief Start on a line of Red-Green followed by a line of Green-Blue.
     */
    IMGR_BAYER_FORMAT_RGGB = 0x0,
};


SENSOR_PHY* SensorPhyInit(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx
#else
    STF_VOID
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    )
{
    SENSOR_PHY *phy = (SENSOR_PHY *)STF_CALLOC(1, sizeof(*phy));
    unsigned int phy_off = 0;

    if (!phy) {
        LOG_ERROR("Failed to allocate internal structure\n");
        return NULL;
    }

#ifdef SIFIVE_FAKE
    // when doing fake driver we can't access registers
    return phy;
#else
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    phy->u8IspIdx = u8IspIdx;
    if (pCIConnection) {
        phy->psConnection = pCIConnection;
        phy->bHasCall_CI_DriverInit = STF_FALSE;
    } else {
        if (STF_SUCCESS != CI_DriverInit(&(phy->psConnection))) {
            LOG_ERROR("Failed to open connection to Mantis Driver\n");
            goto err_connection;
        }
        phy->bHasCall_CI_DriverInit = STF_TRUE;
    }
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
  #ifdef UNUSED_CODE_AND_VARIABLE
    if (PHY_OFFSET_N < phy->psConnection->sHWInfo.rev_ui8Major
        || 1 > phy->psConnection->sHWInfo.rev_ui8Major) {
        LOG_ERROR("unsupported HW version %d.%d, supported version 1.x to " \
            "%d.x\n", phy->psConnection->sHWInfo.rev_ui8Major,
            phy->psConnection->sHWInfo.rev_ui8Minor,
            PHY_OFFSET_N
        );
        goto err_version;
    }
    phy_off = phy->psConnection->sHWInfo.rev_ui8Major - 1;
    if (2 == phy->psConnection->sHWInfo.rev_ui8Major
        && 3 <= phy->psConnection->sHWInfo.rev_ui8Minor) {
        // 2.3 and 2.4 have same offsets than 3.0
        phy_off = phy->psConnection->sHWInfo.rev_ui8Major;
    }
  #endif //UNUSED_CODE_AND_VARIABLE

    phy->psGasket = (CI_GASKET*)STF_CALLOC(1, sizeof(CI_GASKET));
    if (!phy->psGasket) {
        LOG_ERROR("Failed to allocate gasket struct\n");
        goto err_gasket;
    }
    CI_GasketInit(phy->psGasket);

    phy->psGasket->bParallel = STF_FALSE; // default

    return phy;

err_gasket:
    STF_FREE(phy->psGasket);
err_version:
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    if (phy->bHasCall_CI_DriverInit) {
        CI_DriverFinalise(phy->psConnection);
    }
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
err_connection:
    STF_FREE(phy);
    phy = NULL;
    return phy;
#endif /* SIFIVE_FAKE */
}

STF_VOID SensorPhyDeinit(
    SENSOR_PHY *psSensorPhy
    )
{
    STF_ASSERT(psSensorPhy);
#ifndef SIFIVE_FAKE
    STF_ASSERT(psSensorPhy->psGasket);
    STF_ASSERT(psSensorPhy->psConnection);

    STF_FREE(psSensorPhy->psGasket);
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    if (psSensorPhy->bHasCall_CI_DriverInit) {
        CI_DriverFinalise(psSensorPhy->psConnection);
        psSensorPhy->bHasCall_CI_DriverInit = STF_FALSE;
    }
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
#endif /* SIFIVE_FAKE */
    STF_FREE(psSensorPhy);
}

STF_RESULT SensorPhyCtrl(
    SENSOR_PHY *psSensorPhy,
    STF_BOOL8 bEnable,
    STF_U8 uiMipiLanes,
    STF_U8 ui8SensorNum
    )
{
    STF_RESULT ret = STF_SUCCESS;

#ifdef UNUSED_CODE_AND_VARIABLE
  #ifndef SIFIVE_FAKE
    if (NULL == psSensorPhy->psGasket) {
        return STF_ERROR_INVALID_PARAMETERS;
    }

    ret = bEnable
        ? CI_GasketAcquire(psSensorPhy->psGasket, psSensorPhy->psConnection)
        : CI_GasketRelease(psSensorPhy->psGasket, psSensorPhy->psConnection);

    if (STF_SUCCESS != ret) {
        LOG_ERROR("failed to %s the gasket %d!\n",
                bEnable ? "acquire" : "release", psSensorPhy->psGasket->uiGasket);
        return ret;
    }
    LOG_DEBUG("%s gasket %d, %s PHY ...\n",
            bEnable ? "Acquired" : "Released",
            psSensorPhy->psGasket->uiGasket,
            bEnable ? "enabling" : "disabling");
  #endif /* SIFIVE_FAKE */
#endif //UNUSED_CODE_AND_VARIABLE

    return ret;
}

/* Add by feng.qu@infotm.com */
STF_RESULT SensorPhyBayerFormat(
    SENSOR_PHY *psSensorPhy,
    enum MOSAICType bayerFmt
    )
{
    STF_RESULT ret;
    enum IMGR_BAYER_FORMAT_ENUM eBayerFormat =
        IMGR_BAYER_FORMAT_RGGB;
    STF_U32 value;

    if (NULL == psSensorPhy->psGasket) {
        return STF_ERROR_INVALID_PARAMETERS;
    }

    switch (bayerFmt) {
        case MOSAIC_RGGB:
            eBayerFormat = IMGR_BAYER_FORMAT_RGGB;
            break;
        case MOSAIC_GRBG:
            eBayerFormat = IMGR_BAYER_FORMAT_GRBG;
            break;
        case MOSAIC_GBRG:
            eBayerFormat = IMGR_BAYER_FORMAT_GBRG;
            break;
        case MOSAIC_BGGR:
            eBayerFormat = IMGR_BAYER_FORMAT_BGGR;
            break;
        default:
            LOG_ERROR(
                "user-side mosaic is unknown - using RGGB\n");
    }
#ifdef UNUSED_CODE_AND_VARIABLE

    ret = CI_DriverDebugRegRead(
        psSensorPhy->psConnection, CI_BANK_CTX + 0, IMGR_CTRL, &value);
    if (ret) {
        LOG_ERROR("CI_DriverDebugRegRead ebank=%d, offset=0x%x failed %d\n",
            CI_BANK_CTX+0, IMGR_CTRL, ret);
        return ret;
    }

    value &= ~(((1 << BIT_IMGR_BAYER_FORMAT_WIDTH) -1) << BIT_IMGR_BAYER_FORMAT_SHIFT);
    value |= (eBayerFormat << BIT_IMGR_BAYER_FORMAT_SHIFT);

    ret = CI_DriverDebugRegWrite(psSensorPhy->psConnection, CI_BANK_CTX+0, IMGR_CTRL, value);
    if (ret) {
        LOG_ERROR("CI_DriverDebugRegWrite ebank=%d, offset=0x%x failed %d\n", CI_BANK_CTX+0, IMGR_CTRL, ret);
        return ret;
    }
#endif //UNUSED_CODE_AND_VARIABLE

    return STF_SUCCESS;
}

STF_RESULT FindI2cDev(
#ifdef SIFIVE_ISP
    STF_CHAR *szI2cDevPath,
    STF_U32 u32Len,
    STF_U32 u32Addr,
    const STF_CHAR *i2c_adaptor
#else
    STF_CHAR *szI2cDevPath,
    STF_U32 u32Len
#endif //SIFIVE_ISP
    )
{
#if !file_i2c
    FILE *file;
    DIR *dir;
    struct dirent *dirEntry;
    char sysfs[NAME_MAX], fstype[NAME_MAX], line[NAME_MAX];
#ifdef SIFIVE_ISP
	char temp[NAME_MAX];
#endif //SIFIVE_ISP
    unsigned found = 0;

    if (NAME_MAX > u32Len) {
        fprintf(stderr, "%s: Wrong parameters, len = %u. Minimum length "
                "should be %u\n", __FILE__, u32Len, NAME_MAX);
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Try to find out where sysfs was mounted */
    if ((file = fopen("/proc/mounts", "r")) == NULL) {
        return STF_ERROR_DEVICE_NOT_FOUND;
    }
    while (fgets(line, NAME_MAX, file)) {
        sscanf(line, "%*[^ ] %[^ ] %[^ ] %*s\n", sysfs, fstype);
        if (strcasecmp(fstype, "sysfs") == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);
    if (!found)
        return STF_ERROR_DEVICE_NOT_FOUND;

    /* Get i2c-dev name */
    strcat(sysfs, "/class/i2c-dev");
    if(!(dir = opendir(sysfs)))
        return STF_ERROR_DEVICE_NOT_FOUND;
    /* go through the busses */
    while ((dirEntry = readdir(dir)) != NULL) {
        if (!strcmp(dirEntry->d_name, "."))
            continue;
        if (!strcmp(dirEntry->d_name, ".."))
            continue;
#ifdef SIFIVE_ISP
		sprintf(line, "%s/%s/device", sysfs, dirEntry->d_name);

		if (!strcmp(dirEntry->d_name, i2c_adaptor)) {
			//strcat(line, "/4-0010");
			sprintf(temp, "/%d-%04x",
			    i2c_adaptor[strlen(i2c_adaptor) - 1] - 48, u32Addr);
			strcat(line, temp);

			if(-1 == access(line, F_OK)) {
				//printf("i2c slave device driver find failed\n");
				closedir(dir);
				return STF_ERROR_DEVICE_NOT_FOUND;
			} else {
				strcpy(szI2cDevPath, "/dev/");
				strcat(szI2cDevPath, dirEntry->d_name);
				//printf("i2c_dev_path = %s\n", i2c_dev_path);
				closedir(dir);
				return STF_SUCCESS;
			}
		}
    }

#else
        sprintf(line, "%s/%s/name", sysfs, dirEntry->d_name);
        file = fopen(line, "r");
        if (file != NULL) {
            char *name;

            name = fgets(line, NAME_MAX, file);
            fclose(file);
            if (!name) {
                fprintf(stderr, "%s/%s: couldn't read i2c name.\n",
                        sysfs, dirEntry->d_name);
                continue;
            }
            if (strncmp(line, STF_I2C_NAME, strlen(STF_I2C_NAME)) == 0) {
                /* Found STF SCB device */
                strcpy(i2c_dev_path, "/dev/");
                strcat(i2c_dev_path, dirEntry->d_name);
                closedir(dir);
                return STF_SUCCESS;
            }
        }
    }	
#endif //SIFIVE_ISP
	closedir(dir);

	return STF_ERROR_DEVICE_NOT_FOUND;
#else

    return STF_SUCCESS;
#endif // file_i2c
}

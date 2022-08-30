/**
  ******************************************************************************
  * @file sensor_phy.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief Header describing the sensors name
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


#ifndef SENSOR_NAME_H_
#define SENSOR_NAME_H_


#ifdef __cplusplus
extern "C" {
#endif


#define OV4689MIPI_SENSOR_INFO_NAME     "ov4689mipi"
#define SC2235DVP_SENSOR_INFO_NAME      "sc2235dvp"
#define IMX219MIPI_SENSOR_INFO_NAME     "imx219mipi"


#define delete_space(pstr) do { \
    char *ptmp = pstr; \
    while(*pstr != '\0') { \
        if ((*pstr != ' ')) { \
            *ptmp++ = *pstr; \
        } \
        ++pstr; \
    } \
    *ptmp = '\0'; \
} while(0)



#ifdef __cplusplus
}
#endif 


#endif /* SENSOR_NAME_H_ */

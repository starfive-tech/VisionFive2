/**
  ******************************************************************************
  * @file  ISP_test.h
  * @author  SiFive Isp Team
  * @version  V1.0
  * @date  03/24/2020
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, SIFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai SiFive</center></h2>
  */


#ifndef __ISP_TEST_H__
#define __ISP_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <termios.h>


#define TEST_ISP_DEVICE             (0)


/**
 * @return seconds for the FPS calculation
 */
double currTime();
void printUsage();
/**
 * @brief Enable mode to allow proper key capture in the application
 */
void enableRawMode(struct termios *p_orig_term_attr);
/**
 * @brief Disable mode to allow proper key capture in the application
 */
void disableRawMode(struct termios *p_orig_term_attr);
/**
 * @brief read key
 *
 * Could use fgetc but does not work with some versions of uclibc
 */
char getch(void);
int run(int argc, char *argv[]);


#ifdef __cplusplus
}
#endif


#endif /* __ISP_TEST_H__ */

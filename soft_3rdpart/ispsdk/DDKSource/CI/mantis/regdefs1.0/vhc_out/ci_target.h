/**
*******************************************************************************
 @file ci_target.h

 @brief Configuration of the TAL light for the Mantis HW

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license Strictly Confidential.
   No part of this software, either material or conceptual may be copied or
   distributed, transmitted, transcribed, stored in a retrieval system or
   translated into any human or computer language in any form by any means,
   electronic, mechanical, manual or other-wise, or disclosed to third
   parties without the express written permission of
   Shanghai StarFive Technology Co., Ltd.

******************************************************************************/
#ifndef CI_TARGET_H_
#define CI_TARGET_H_

#define TAL_TARGET_NAME "MANTIS"

/** @brief number of memory spaces defined in this file */
#define TAL_MEM_SPACE_ARRAY_SIZE \
    (sizeof(gasMemSpaceInfo)/sizeof(gasMemSpaceInfo[0]))

#endif /* CI_TARGET_H_ */


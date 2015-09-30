#pragma once

/*
  Copyright (c) 2013, J.D. Koftinoff Software, Ltd.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   3. Neither the name of J.D. Koftinoff Software, Ltd. nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "jdksavdecc_world.h"
#include "jdksavdecc_aecp.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup aecp AECP - Clause 9 */
/*@{*/

/** \addtogroup aecpdu_hdcp_apm AECP HDCP APM
 *  @todo aecpdu_hdcp_apm
*/
/*@{*/

/** \addtogroup aecpdu_hdcp_apm_offsets AECP HDCP APM Offsets - Clause 9.2.1.6 */
/*@{*/

#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_CONTROLLER_ENTITY_ID ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 0 )
#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_SEQUENCE_ID ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 8 )
#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_HDCP_APM_LENGTH ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 10 )
#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_HDCP_APM_FLAGS ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 12 )
#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_RESERVED ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 13 )
#define JDKSAVDECC_AECP_HDCP_APM_OFFSET_HDCP_APM_FRAGMENT_OFFSET ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 14 )
#define JDKSAVDECC_AECP_HDCP_APM_LEN ( JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 16 )

/*@}*/

/** \addtogroup aecpdu_hdcp_apm_status AECP HDCP APM Status codes - Clause 9.2.1.6.6 */
/*@{*/

#define JDKSAVDECC_AECP_HDCP_APM_STATUS_SUCCESS ( 0 )
#define JDKSAVDECC_AECP_HDCP_APM_STATUS_NOT_IMPLEMENTED ( 1 )
#define JDKSAVDECC_AECP_HDCP_APM_STATUS_FRAGMENT_MISSING ( 2 )

/*@}*/

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

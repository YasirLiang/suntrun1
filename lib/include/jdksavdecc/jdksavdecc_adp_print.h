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
#include "jdksavdecc_adp.h"
#include "jdksavdecc_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup adp ADP - Clause 6 */
/*@{*/

/** \addtogroup adp_print ADP Print
 *  @todo adp_print
*/
/*@{*/

extern struct jdksavdecc_uint16_name jdksavdecc_adpdu_print_message_type[];
extern struct jdksavdecc_32bit_name jdksavdecc_adpdu_print_entity_capabilities[];
extern struct jdksavdecc_16bit_name jdksavdecc_adpdu_print_talker_capabilities[];
extern struct jdksavdecc_16bit_name jdksavdecc_adpdu_print_listener_capabilities[];
extern struct jdksavdecc_32bit_name jdksavdecc_adpdu_print_controller_capabilities[];

void jdksavdecc_adpdu_print_common_control_header( struct jdksavdecc_printer *self,
                                                   struct jdksavdecc_adpdu_common_control_header const *p );
void jdksavdecc_adpdu_print( struct jdksavdecc_printer *self, struct jdksavdecc_adpdu const *p );

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

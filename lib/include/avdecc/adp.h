#pragma once
/*
Copyright (c) 2014, J.D. Koftinoff Software, Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "jdksavdecc_adp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief adp_form_msg
 *
 * Create an ADPDU message ethernet frame with the specified message_type,
 * possibly directed to the specified target_entity (as ascii)
 *
 * @param frame Pointer to the ethernet frame that will be filled in except for SA
 * @param adpdu Pointer to adpdu high level structure that will be filled in
 * @param message_type ADP Message type code
 * @param target_entity target entity id value
 * @return 0 success
 */
int adp_form_msg( struct jdksavdecc_frame *frame,
                  struct jdksavdecc_adpdu *adpdu,
                  uint16_t message_type,
                  struct jdksavdecc_eui64 target_entity );

/**
 * @brief adp_check
 *
 * Validate an ethernet frame to see if it contains an ADP message, potentially from the target entity
 *
 * @param frame The ethernet frame to validate
 * @param adpdu The ADPDU structure that will be filled in if the frame is matching
 * @param target_entity_id The target entity_id to expect, or 0 for any
 * @return 0 on success
 */
int adp_check( const struct jdksavdecc_frame *frame,
               struct jdksavdecc_adpdu *adpdu,
               const struct jdksavdecc_eui64 *target_entity_id );

#ifdef __cplusplus
}
#endif

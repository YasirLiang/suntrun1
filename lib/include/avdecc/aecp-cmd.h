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

#include "avdecc-cmd.h"
#include "raw.h"
#include "jdksavdecc_aecp.h"
#include "jdksavdecc_aecp_aa.h"
#include "jdksavdecc_aecp_aem.h"
#include "jdksavdecc_aecp_vendor.h"
#include "jdksavdecc_aecp_hdcp_apm.h"
#include "jdksavdecc_aecp_print.h"
#include "jdksavdecc_aem_command.h"
#include "jdksavdecc_aem_descriptor.h"
#include "jdksavdecc_aem_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief aecp_aem_print
 *
 * Print the details of the aecpdu message in the ethernet frame
 *
 * @param s The output stream to print the ascii to
 * @param frame The ethernet frame which contains an aecpdu message
 * @param aecpdudu The parsed aecpdu message
 */
void aecp_aem_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_aecpdu_aem *aecpdudu );

/**
 * @brief aecpdu_process
 * @param request_
 * @param net
 * @param frame
 * @return
 */
int aecp_aem_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame );
int aecp_my_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame );

/**
 * @brief handle aecp command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv );

/**
 * @brief handle aecp_aem command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_aem( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

/**
 * @brief handle aecp_aa command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_aa( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

/**
 * @brief handle aecp_avc command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_avc( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

/**
 * @brief handle aecp_hdcp_apm command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_hdcp_apm( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

/**
 * @brief handle aecp_vendor command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_vendor( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

/**
 * @brief handle aecp_extended command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param aecp message_type code
 * @param argc count of arguments including "aecp"
 * @param argv array of arguments starting at "aecp"
 * @return 0 on success
 */
int aecp_extended( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv );

#ifdef __cplusplus
}
#endif

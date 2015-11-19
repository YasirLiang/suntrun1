
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
#include "test/jdksavdecc_test_descriptor_storage.h"

static uint8_t jdksavdecc_test_descriptor_storage_data[] = {
/* Offset 0x0000 */
    0x41, 0x45, 0x4d, 0x31, // Magic number "AEM1"
    0x00, 0x00, 0x00, 0x00, // toc_count
    0x00, 0x00, 0x00, 0x00, // toc_offset
    0x00, 0x00, 0x00, 0x00, // symbol_count
    0x00, 0x00, 0x00, 0x00, // symbol_offset
/* Offset 0x0014 */
};

bool jdksavdecc_test_descriptor_storage_create(struct jdksavdecc_descriptor_storage *self) {
    bool r=false;

    r=jdksavdecc_descriptor_storage_buffer_init(
                self,
                jdksavdecc_test_descriptor_storage_data,
                sizeof(jdksavdecc_test_descriptor_storage_data)
                );

    return r;
}

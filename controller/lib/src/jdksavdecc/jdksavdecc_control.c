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
#include "jdksavdecc_control.h"

#ifdef TODO

bool jdksavdecc_control_init( struct jdksavdecc_control_info *self, void const *descriptor_data, uint16_t descriptor_len )
{
    self->descriptor_data = descriptor_data;
    self->descriptor_len = descriptor_len;
    return true;
}

bool jdksavdecc_control_get_localized_description( struct jdksavdecc_control_info const *control_info,
                                                   struct jdksavdecc_entity_model *entity_model,
                                                   uint16_t configuration_index,
                                                   uint16_t locale_id,
                                                   struct jdksavdecc_string *result )
{
    bool r = false;

    if ( control_info->descriptor_data != 0 && control_info->descriptor_len >= JDKSAVDECC_DESCRIPTOR_CONTROL_LEN )
    {
        uint16_t localized_string_id
            = jdksavdecc_descriptor_control_get_localized_description( control_info->descriptor_data, 0 );

        jdksavdecc_string_init( result );
        entity_model->read_localized_string( entity_model, configuration_index, locale_id, localized_string_id, result );
        r = true;
    }

    return r;
}

uint16_t jdksavdecc_control_get_num_items( struct jdksavdecc_control_info const *control_info )
{
    uint16_t r = 0;
    if ( control_info->descriptor_data != 0 && control_info->descriptor_len >= JDKSAVDECC_DESCRIPTOR_CONTROL_LEN )
    {
        r = jdksavdecc_descriptor_control_get_number_of_values( control_info->descriptor_data, 0 );
    }
    return r;
}

bool jdksavdecc_control_get_item_localized_description( struct jdksavdecc_control_info const *control_info,
                                                        uint16_t item,
                                                        struct jdksavdecc_entity_model *entity_model,
                                                        uint16_t configuration_index,
                                                        uint16_t locale_id,
                                                        struct jdksavdecc_string *result )
{
    bool r = false;

    if ( control_info->descriptor_data != 0 && control_info->descriptor_len >= JDKSAVDECC_DESCRIPTOR_CONTROL_LEN )
    {

        uint16_t localized_string_id = 0xffff;

        // TODO
        jdksavdecc_string_init( result );

        entity_model->read_localized_string( entity_model, configuration_index, locale_id, localized_string_id, result );
        r = true;
    }

    return r;
}

bool jdksavdecc_control_is_numeric( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_integer( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_uses_multiplier( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_floating_point( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_selector( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_linear( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_array( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_is_vendor_blob( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_vendor_id( struct jdksavdecc_control_info const *control_info,
                                       struct jdksavdecc_eui64 *result_vendor_eui64 )
{
    // TODO
    return false;
}

uint16_t jdksavdecc_control_get_vendor_blob_length( struct jdksavdecc_control_info const *control_info )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_vendor_blob( struct jdksavdecc_control_info const *control_info,
                                         uint16_t item,
                                         uint8_t const *control_data,
                                         uint16_t control_data_len,
                                         uint8_t *blob_buf,
                                         size_t blob_buf_max_len )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_vendor_blob( struct jdksavdecc_control_info const *control_info,
                                         uint16_t item,
                                         uint8_t *control_data,
                                         uint16_t control_data_len,
                                         uint8_t const *blob_buf,
                                         size_t blob_buf_max_len )
{
    // TODO
    return false;
}

int jdksavdecc_control_get_item_multiplier_power( struct jdksavdecc_control_info const *control_info, uint16_t item )
{
    // TODO
    return 0;
}

double jdksavdecc_control_get_item_multiplier( struct jdksavdecc_control_info const *control_info, uint16_t item )
{
    // TODO
    return 1.0;
}

char const *jdksavdecc_control_get_item_units_as_string( struct jdksavdecc_control_info const *control_info, uint16_t item )
{
    // TODO
    return 0;
}

bool jdksavdecc_control_get_item_as_string( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            char *string_buf,
                                            size_t string_buf_max_len,
                                            struct jdksavdecc_entity_model *entity_model,
                                            uint16_t locale_id )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_string( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t *control_data,
                                              uint16_t control_data_len,
                                              char const *string_buf,
                                              size_t string_buf_max_len )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_string( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    char *string_buf,
                                                    size_t string_buf_max_len,
                                                    struct jdksavdecc_entity_model *entity_model,
                                                    uint16_t locale_id )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_string( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    char *string_buf,
                                                    size_t string_buf_max_len,
                                                    struct jdksavdecc_entity_model *entity_model,
                                                    uint16_t locale_id )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_string( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    char *string_buf,
                                                    size_t string_buf_max_len,
                                                    struct jdksavdecc_entity_model *entity_model,
                                                    uint16_t locale_id )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_string( struct jdksavdecc_control_info const *control_info,
                                                 uint16_t item,
                                                 char *string_buf,
                                                 size_t string_buf_max_len,
                                                 struct jdksavdecc_entity_model *entity_model,
                                                 uint16_t locale_id )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_string( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    char *string_buf,
                                                    size_t string_buf_max_len,
                                                    struct jdksavdecc_entity_model *entity_model,
                                                    uint16_t locale_id )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_int8( struct jdksavdecc_control_info const *control_info,
                                          uint16_t item,
                                          uint8_t const *control_data,
                                          uint16_t control_data_len,
                                          int8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_int8( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t *control_data,
                                            uint16_t control_data_len,
                                            int8_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_int8( struct jdksavdecc_control_info const *control_info,
                                                  uint16_t item,
                                                  int8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_int8( struct jdksavdecc_control_info const *control_info,
                                                  uint16_t item,
                                                  int8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_int8( struct jdksavdecc_control_info const *control_info,
                                                  uint16_t item,
                                                  int8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_int8( struct jdksavdecc_control_info const *control_info,
                                               uint16_t item,
                                               int8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_int8( struct jdksavdecc_control_info const *control_info,
                                                  uint16_t item,
                                                  int8_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_uint8( struct jdksavdecc_control_info const *control_info,
                                           uint16_t item,
                                           uint8_t const *control_data,
                                           uint16_t control_data_len,
                                           uint8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_uint8( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t *control_data,
                                             uint16_t control_data_len,
                                             uint8_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_uint8( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   uint8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_uint8( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   uint8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_uint8( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   uint8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_uint8( struct jdksavdecc_control_info const *control_info,
                                                uint16_t item,
                                                uint8_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_uint8( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   uint8_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_int16( struct jdksavdecc_control_info const *control_info,
                                           uint16_t item,
                                           uint8_t const *control_data,
                                           uint16_t control_data_len,
                                           int16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_int16( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t *control_data,
                                             uint16_t control_data_len,
                                             int16_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_int16( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_int16( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_int16( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_int16( struct jdksavdecc_control_info const *control_info,
                                                uint16_t item,
                                                int16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_int16( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int16_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_uint16( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            uint16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_uint16( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t *control_data,
                                              uint16_t control_data_len,
                                              uint16_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_uint16( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_uint16( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_uint16( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_uint16( struct jdksavdecc_control_info const *control_info,
                                                 uint16_t item,
                                                 uint16_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_uint16( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint16_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_int32( struct jdksavdecc_control_info const *control_info,
                                           uint16_t item,
                                           uint8_t const *control_data,
                                           uint16_t control_data_len,
                                           int32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_int32( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t *control_data,
                                             uint16_t control_data_len,
                                             int32_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_int32( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_int32( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_int32( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_int32( struct jdksavdecc_control_info const *control_info,
                                                uint16_t item,
                                                int32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_int32( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int32_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_uint32( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            uint32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_uint32( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t *control_data,
                                              uint16_t control_data_len,
                                              uint32_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_uint32( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_uint32( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_uint32( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_uint32( struct jdksavdecc_control_info const *control_info,
                                                 uint16_t item,
                                                 uint32_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_uint32( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint32_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_int64( struct jdksavdecc_control_info const *control_info,
                                           uint16_t item,
                                           uint8_t const *control_data,
                                           uint16_t control_data_len,
                                           int64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_int64( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t *control_data,
                                             uint16_t control_data_len,
                                             int64_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_int64( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_int64( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_int64( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_int64( struct jdksavdecc_control_info const *control_info,
                                                uint16_t item,
                                                int64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_int64( struct jdksavdecc_control_info const *control_info,
                                                   uint16_t item,
                                                   int64_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_item_as_uint64( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            uint64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_item_from_uint64( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t *control_data,
                                              uint16_t control_data_len,
                                              uint64_t new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_current_as_uint64( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_minimum_as_uint64( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_maximum_as_uint64( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_step_as_uint64( struct jdksavdecc_control_info const *control_info,
                                                 uint16_t item,
                                                 uint64_t *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_item_default_as_uint64( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    uint64_t *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_bode_plot_item( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            struct jdksavdecc_control_bode_plot_point *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_bode_plot_item( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t *control_data,
                                            uint16_t control_data_len,
                                            struct jdksavdecc_control_bode_plot_point const *new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_bode_plot_layout( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              struct jdksavdecc_values_bode_plot *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_current_bode_plot_item( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    struct jdksavdecc_control_bode_plot_point *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_smpte_time_item( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t const *control_data,
                                             uint16_t control_data_len,
                                             struct jdksavdecc_values_smpte_time *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_smpte_time_item( struct jdksavdecc_control_info const *control_info,
                                             uint16_t item,
                                             uint8_t *control_data,
                                             uint16_t control_data_len,
                                             struct jdksavdecc_values_smpte_time const *new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_current_smpte_time_item( struct jdksavdecc_control_info const *control_info,
                                                     uint16_t item,
                                                     struct jdksavdecc_values_smpte_time *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_sample_rate_item( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t const *control_data,
                                              uint16_t control_data_len,
                                              struct jdksavdecc_values_sample_rate *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_sample_rate_item( struct jdksavdecc_control_info const *control_info,
                                              uint16_t item,
                                              uint8_t *control_data,
                                              uint16_t control_data_len,
                                              struct jdksavdecc_values_sample_rate const *new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_current_sample_rate_item( struct jdksavdecc_control_info const *control_info,
                                                      uint16_t item,
                                                      struct jdksavdecc_values_sample_rate *result )
{
    // TODO
    return false;
}

///
bool jdksavdecc_control_get_gptp_time_item( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t const *control_data,
                                            uint16_t control_data_len,
                                            struct jdksavdecc_values_gptp_time *result )
{
    // TODO
    return false;
}

bool jdksavdecc_control_set_gptp_time_item( struct jdksavdecc_control_info const *control_info,
                                            uint16_t item,
                                            uint8_t *control_data,
                                            uint16_t control_data_len,
                                            struct jdksavdecc_values_gptp_time const *new_value )
{
    // TODO
    return false;
}

bool jdksavdecc_control_get_current_gptp_time_item( struct jdksavdecc_control_info const *control_info,
                                                    uint16_t item,
                                                    struct jdksavdecc_values_gptp_time *result )
{
    // TODO
    return false;
}

#else
const char *jdksavdecc_control_file = __FILE__;

#endif

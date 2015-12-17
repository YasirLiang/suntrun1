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

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup util Utilities */
/*@{*/

#ifndef JDKSAVDECC_TIMESTAMP_TYPE
#define JDKSAVDECC_TIMESTAMP_TYPE uint64_t
#endif

typedef JDKSAVDECC_TIMESTAMP_TYPE jdksavdecc_timestamp_in_microseconds;

typedef JDKSAVDECC_TIMESTAMP_TYPE jdksavdecc_timestamp_in_milliseconds;

int jdksavdecc_util_parse_nibble( uint8_t *result, char d1 );
int jdksavdecc_util_parse_byte( uint8_t *result, char d1, char d2 );

/**
 * Validate buffer position in a buffer len for an element of elem_size.
 *
 * @param bufpos position of element in buffer, in octets
 * @param buflen length of buffer, in octets
 * @param elem_size element size, in octets
 * @return -1 if element does not fit, bufpos+elem_size if it does.
 */
static inline ssize_t jdksavdecc_validate_range( ssize_t bufpos, size_t buflen, size_t elem_size )
{
    return ( ( size_t )( bufpos ) + (size_t)elem_size <= (size_t)buflen ) ? ( ssize_t )( bufpos + elem_size ) : ( ssize_t ) - 1;
}

/** \addtogroup endian Endian helpers / data access */
/*@{*/

/// Reverses endian of uint16_t value at *vin and returns it
static inline uint16_t jdksavdecc_endian_reverse_uint16( uint16_t const *vin )
{
    uint8_t const *p = (uint8_t const *)( vin );
    return ( ( uint16_t )( p[1] ) << 0 ) + ( ( uint16_t )( p[0] ) << 8 );
}

/// Reverses endian of uint32_t value at *vin and returns it
static inline uint32_t jdksavdecc_endian_reverse_uint32( uint32_t const *vin )
{
    uint8_t const *p = (uint8_t const *)vin;
    return ( ( uint32_t )( p[3] ) << 0 ) + ( ( uint32_t )( p[2] ) << 8 ) + ( ( uint32_t )( p[1] ) << 16 )
           + ( ( uint32_t )( p[0] ) << 24 );
}

/// Reverses endian of uint64_t value at *vin and returns it
static inline uint64_t jdksavdecc_endian_reverse_uint64( uint64_t const *vin )
{
    uint8_t const *p = (uint8_t const *)vin;
    return ( ( uint64_t )( p[7] ) << 0 ) + ( ( uint64_t )( p[6] ) << 8 ) + ( ( uint64_t )( p[5] ) << 16 )
           + ( ( uint64_t )( p[4] ) << 24 ) + ( ( uint64_t )( p[3] ) << 32 ) + ( ( uint64_t )( p[2] ) << 40 )
           + ( ( uint64_t )( p[1] ) << 48 ) + ( ( uint64_t )( p[0] ) << 56 );
}

/// Read the uint8_t value at base[pos] and store it in *host_value. Returns new
/// pos, or -1 if pos is out of bounds.
static inline ssize_t jdksavdecc_uint8_read( uint8_t *host_value, void const *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 1 );
    if ( r >= 0 )
    {
        uint8_t const *b = (uint8_t const *)base;
        *host_value = b[pos];
    }
    return r;
}

/// get the uint8_t value at base[pos] and return it without bounds checking
static inline uint8_t jdksavdecc_uint8_get( void const *base, ssize_t pos )
{
    uint8_t const *b = (uint8_t const *)base;
    return b[pos];
}

/// store the uint8_t value v into base[pos]. Returns -1 if pos is out of
/// bounds.
static inline ssize_t jdksavdecc_uint8_write( uint8_t v, void *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 1 );
    if ( r >= 0 )
    {
        uint8_t const *host_value = (uint8_t const *)&v;
        uint8_t *b = ( (uint8_t *)base ) + pos;

        b[0] = *host_value;
    }
    return r;
}

/// Set the uint8_t value at base[pos] to v without bounds checking
static inline void jdksavdecc_uint8_set( uint8_t v, void *base, ssize_t pos )
{
    uint8_t *b = (uint8_t *)base;
    b[pos] = v;
}

/// Read the network order Doublet value at base[pos] and store it in
/// *host_value. Returns new pos, or -1 if pos is out of bounds.
static inline ssize_t jdksavdecc_uint16_read( uint16_t *host_value, void const *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 2 );
    if ( r >= 0 )
    {
        uint8_t const *b = ( (uint8_t const *)base ) + pos;
        *host_value = ( ( (uint16_t)b[0] ) << ( 8 * 1 ) ) + b[1];
    }
    return r;
}

/// get the uint8_t value at base[pos] and return it without bounds checking
static inline uint16_t jdksavdecc_uint16_get( void const *base, ssize_t pos )
{
    uint8_t const *b = ( (uint8_t const *)base ) + pos;
    return ( ( (uint16_t)b[0] ) << 8 ) + b[1];
}

static inline ssize_t jdksavdecc_uint16_write( uint16_t v, void *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 2 );
    if ( r >= 0 )
    {
        uint16_t const *host_value = (uint16_t const *)&v;
        uint8_t *b = ( (uint8_t *)base ) + pos;

        b[0] = ( uint8_t )( ( *host_value ) >> 8 ) & 0xff;
        b[1] = ( uint8_t )( ( *host_value ) >> 0 ) & 0xff;
    }
    return r;
}

static inline void jdksavdecc_uint16_set( uint16_t v, void *base, ssize_t pos )
{
    uint8_t *b = ( (uint8_t *)base ) + pos;
    b[0] = ( uint8_t )( ( v ) >> 8 ) & 0xff;
    b[1] = ( uint8_t )( ( v ) >> 0 ) & 0xff;
}

static inline ssize_t jdksavdecc_uint32_read( uint32_t *host_value, void const *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 4 );
    if ( r >= 0 )
    {
        uint8_t const *b = ( (uint8_t const *)base ) + pos;
        *host_value = ( ( (uint32_t)b[0] ) << ( 8 * 3 ) ) + ( ( (uint32_t)b[1] ) << ( 8 * 2 ) )
                      + ( ( (uint32_t)b[2] ) << ( 8 * 1 ) ) + b[3];
    }
    return r;
}

static inline uint32_t jdksavdecc_uint32_get( void const *base, ssize_t pos )
{
    uint8_t const *b = ( (uint8_t const *)base ) + pos;
    return ( ( (uint32_t)b[0] ) << 24 ) + ( ( (uint32_t)b[1] ) << 16 ) + ( ( (uint32_t)b[2] ) << 8 ) + b[3];
}

static inline ssize_t jdksavdecc_uint32_write( uint32_t v, void *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 4 );
    if ( r >= 0 )
    {
        uint32_t const *host_value = (uint32_t const *)&v;
        uint8_t *b = ( (uint8_t *)base ) + pos;

        b[0] = ( uint8_t )( ( *host_value ) >> 24 ) & 0xff;
        b[1] = ( uint8_t )( ( *host_value ) >> 16 ) & 0xff;
        b[2] = ( uint8_t )( ( *host_value ) >> 8 ) & 0xff;
        b[3] = ( uint8_t )( ( *host_value ) >> 0 ) & 0xff;
    }
    return r;
}

static inline void jdksavdecc_uint32_set( uint32_t v, void *base, ssize_t pos )
{
    uint8_t *b = ( (uint8_t *)base ) + pos;
    b[0] = ( uint8_t )( ( v ) >> 24 ) & 0xff;
    b[1] = ( uint8_t )( ( v ) >> 16 ) & 0xff;
    b[2] = ( uint8_t )( ( v ) >> 8 ) & 0xff;
    b[3] = ( uint8_t )( ( v ) >> 0 ) & 0xff;
}

static inline ssize_t jdksavdecc_uint64_read( uint64_t *host_value, void const *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 8 );
    if ( r >= 0 )
    {
        uint8_t const *b = ( (uint8_t const *)base ) + pos;
        *host_value = ( ( (uint64_t)b[0] ) << ( 8 * 7 ) ) + ( ( (uint64_t)b[1] ) << ( 8 * 6 ) )
                      + ( ( (uint64_t)b[2] ) << ( 8 * 5 ) ) + ( ( (uint64_t)b[3] ) << ( 8 * 4 ) )
                      + ( ( (uint64_t)b[4] ) << ( 8 * 3 ) ) + ( ( (uint64_t)b[5] ) << ( 8 * 2 ) )
                      + ( ( (uint64_t)b[6] ) << ( 8 * 1 ) ) + b[7];
    }
    return r;
}

static inline uint64_t jdksavdecc_uint64_get( void const *base, ssize_t pos )
{
    uint8_t const *b = ( (uint8_t const *)base ) + pos;

    return ( ( (uint64_t)b[0] ) << ( 8 * 7 ) ) + ( ( (uint64_t)b[1] ) << ( 8 * 6 ) ) + ( ( (uint64_t)b[2] ) << ( 8 * 5 ) )
           + ( ( (uint64_t)b[3] ) << ( 8 * 4 ) ) + ( ( (uint64_t)b[4] ) << ( 8 * 3 ) ) + ( ( (uint64_t)b[5] ) << ( 8 * 2 ) )
           + ( ( (uint64_t)b[6] ) << ( 8 * 1 ) ) + b[7];
}

static inline ssize_t jdksavdecc_uint64_write( uint64_t v, void *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 8 );
    if ( r >= 0 )
    {
        uint64_t const *host_value = (uint64_t const *)&v;
        uint8_t *b = ( (uint8_t *)base ) + pos;

        b[0] = ( uint8_t )( ( *host_value ) >> ( 8 * 7 ) ) & 0xff;
        b[1] = ( uint8_t )( ( *host_value ) >> ( 8 * 6 ) ) & 0xff;
        b[2] = ( uint8_t )( ( *host_value ) >> ( 8 * 5 ) ) & 0xff;
        b[3] = ( uint8_t )( ( *host_value ) >> ( 8 * 4 ) ) & 0xff;
        b[4] = ( uint8_t )( ( *host_value ) >> ( 8 * 3 ) ) & 0xff;
        b[5] = ( uint8_t )( ( *host_value ) >> ( 8 * 2 ) ) & 0xff;
        b[6] = ( uint8_t )( ( *host_value ) >> ( 8 * 1 ) ) & 0xff;
        b[7] = ( uint8_t )( ( *host_value ) ) & 0xff;
    }
    return r;
}

static inline void jdksavdecc_uint64_set( uint64_t v, void *base, ssize_t pos )
{
    uint8_t *b = ( (uint8_t *)base ) + pos;
    b[0] = ( uint8_t )( ( v ) >> ( 8 * 7 ) ) & 0xff;
    b[1] = ( uint8_t )( ( v ) >> ( 8 * 6 ) ) & 0xff;
    b[2] = ( uint8_t )( ( v ) >> ( 8 * 5 ) ) & 0xff;
    b[3] = ( uint8_t )( ( v ) >> ( 8 * 4 ) ) & 0xff;
    b[4] = ( uint8_t )( ( v ) >> ( 8 * 3 ) ) & 0xff;
    b[5] = ( uint8_t )( ( v ) >> ( 8 * 2 ) ) & 0xff;
    b[6] = ( uint8_t )( ( v ) >> ( 8 * 1 ) ) & 0xff;
    b[7] = ( uint8_t )( ( v ) ) & 0xff;
}

static inline ssize_t jdksavdecc_float_read( float *host_value, void const *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( *host_value ) );

    if ( r >= 0 )
    {
        union
        {
            uint32_t host_value_int;
            float f;
        } hp;
        hp.host_value_int = jdksavdecc_uint32_get( base, pos );
        *host_value = hp.f;
    }
    return r;
}

static inline float jdksavdecc_float_get( void const *base, ssize_t pos )
{
    union
    {
        uint32_t host_value_int;
        float f;
    } hp;
    hp.host_value_int = jdksavdecc_uint32_get( base, pos );
    return hp.f;
}

static inline ssize_t jdksavdecc_float_write( float v, void *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 4 );

    if ( r >= 0 )
    {
        float *fp = &v;
        uint32_t *ip = (uint32_t *)fp;
        uint32_t host_value_int = *ip;
        jdksavdecc_uint32_set( host_value_int, base, pos );
    }
    return r;
}

static inline void jdksavdecc_float_set( float v, void *base, ssize_t pos )
{
    union
    {
        uint32_t *host_value_int;
        float *f;
    } hp;
    hp.f = &v;
    jdksavdecc_uint32_set( *( hp.host_value_int ), base, pos );
}

static inline ssize_t jdksavdecc_double_read( double *host_value, void const *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( *host_value ) );

    if ( r >= 0 )
    {
        union
        {
            uint64_t host_value_int;
            float f;
        } hp;
        hp.host_value_int = jdksavdecc_uint64_get( base, pos );
        *host_value = hp.f;
    }
    return r;
}

static inline double jdksavdecc_double_get( void const *base, ssize_t pos )
{
    union
    {
        uint64_t host_value_int;
        double d;
    } hp;
    hp.host_value_int = jdksavdecc_uint64_get( base, pos );
    return hp.d;
}

static inline ssize_t jdksavdecc_double_write( double v, void *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 8 );

    if ( r >= 0 )
    {
        double *dp = &v;
        uint64_t *ip = (uint64_t *)dp;
        uint64_t host_value_int = *ip;
        jdksavdecc_uint64_set( host_value_int, base, pos );
    }
    return r;
}

static inline void jdksavdecc_double_set( double v, void *base, ssize_t pos )
{
    union
    {
        uint64_t *host_value_int;
        double *d;
    } hp;
    hp.d = &v;
    jdksavdecc_uint64_set( *( hp.host_value_int ), base, pos );
}

/*@}*/

/** \addtogroup string String */
/*@{*/

struct jdksavdecc_string
{
    uint8_t value[64];
};

static inline void jdksavdecc_string_init( struct jdksavdecc_string *self )
{
    memset( &self->value[0], 0, sizeof( self->value ) );
}

static inline void jdksavdecc_string_clear( struct jdksavdecc_string *self )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = 0;
    }
}

static inline void jdksavdecc_string_assign( struct jdksavdecc_string *self, char const *s )
{
    size_t i;
    int ended = 0;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        if ( *s == 0 )
        {
            ended = 1;
            self->value[i] = 0;
        }

        if ( !ended )
        {
            self->value[i] = *s++;
        }
    }
}

static inline size_t jdksavdecc_string_length( struct jdksavdecc_string const *self )
{
    size_t i;
    size_t len = 0;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        len = i;
        if ( self->value[i] == 0 )
        {
            break;
        }
    }
    return len;
}

static inline void jdksavdecc_string_copy( struct jdksavdecc_string *self, struct jdksavdecc_string const *other )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = other->value[i];
    }
}

static inline int jdksavdecc_string_compare( struct jdksavdecc_string const *self, struct jdksavdecc_string const *other )
{
    size_t i;
    int r = 0;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        if ( self->value[i] < other->value[i] )
        {
            r = -1;
            break;
        }
        if ( self->value[i] > other->value[i] )
        {
            r = -1;
            break;
        }
    }
    return r;
}

static inline struct jdksavdecc_string jdksavdecc_string_get( void const *base, ssize_t pos )
{
    struct jdksavdecc_string v;
    memcpy( v.value, ( (uint8_t const *)base ) + pos, sizeof( v.value ) );
    return v;
}

static inline void jdksavdecc_string_set( struct jdksavdecc_string v, void *base, ssize_t pos )
{
    memcpy( ( (uint8_t *)base ) + pos, v.value, sizeof( v.value ) );
}

static inline void jdksavdecc_string_set_from_cstr( struct jdksavdecc_string *self, char const *s )
{
    size_t i;
    int ended = 0;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        if ( ended )
        {
            self->value[i] = '\0';
        }
        else
        {
            self->value[i] = s[i];
            if ( s[i] == '\0' )
            {
                ended = 1;
            }
        }
    }
}

static inline int jdksavdecc_string_get_cstr( struct jdksavdecc_string *self, char *s, size_t s_len )
{
    size_t i;
    int ended = 0;
    for ( i = 0; i < s_len; ++i )
    {
        if ( ended || i >= sizeof( self->value ) )
        {
            s[i] = '\0';
        }
        else
        {
            s[i] = self->value[i];
            if ( self->value[i] == '\0' )
            {
                ended = 1;
            }
        }
    }
    return ended;
}

static inline ssize_t jdksavdecc_string_read( struct jdksavdecc_string *host_value, void const *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( host_value->value, ( (uint8_t const *)base ) + pos, sizeof( host_value->value ) );
    }
    return r;
}

static inline ssize_t jdksavdecc_string_write( struct jdksavdecc_string const *host_value, void *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( ( (uint8_t *)base ) + pos, host_value->value, sizeof( host_value->value ) );
    }
    return r;
}

/*@}*/

/** \addtogroup eui48 eui48 */
/*@{*/

struct jdksavdecc_eui48
{
    uint8_t value[6];
};

static inline void jdksavdecc_eui48_init( struct jdksavdecc_eui48 *self )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = 0xff;
    }
}

static inline void jdksavdecc_eui48_init_from_uint64( struct jdksavdecc_eui48 *self, uint64_t other )
{
    self->value[0] = ( uint8_t )( ( other >> ( 5 * 8 ) ) & 0xff );
    self->value[1] = ( uint8_t )( ( other >> ( 4 * 8 ) ) & 0xff );
    self->value[2] = ( uint8_t )( ( other >> ( 3 * 8 ) ) & 0xff );
    self->value[3] = ( uint8_t )( ( other >> ( 2 * 8 ) ) & 0xff );
    self->value[4] = ( uint8_t )( ( other >> ( 1 * 8 ) ) & 0xff );
    self->value[5] = ( uint8_t )( ( other >> ( 0 * 8 ) ) & 0xff );
}

int jdksavdecc_eui48_init_from_cstr( struct jdksavdecc_eui48 *self, const char *str );

static inline uint64_t jdksavdecc_eui48_convert_to_uint64( struct jdksavdecc_eui48 const *self )
{
    uint64_t v = 0;
    v |= ( (uint64_t)self->value[0] ) << ( 5 * 8 );
    v |= ( (uint64_t)self->value[1] ) << ( 4 * 8 );
    v |= ( (uint64_t)self->value[2] ) << ( 3 * 8 );
    v |= ( (uint64_t)self->value[3] ) << ( 2 * 8 );
    v |= ( (uint64_t)self->value[4] ) << ( 1 * 8 );
    v |= ( (uint64_t)self->value[5] ) << ( 0 * 8 );
    return v;
}

static inline void jdksavdecc_eui48_copy( struct jdksavdecc_eui48 *self, struct jdksavdecc_eui48 const *other )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = other->value[i];
    }
}

static inline int jdksavdecc_eui48_compare( struct jdksavdecc_eui48 const *self, struct jdksavdecc_eui48 const *other )
{
    int r = memcmp( self->value, other->value, sizeof( self->value ) );
    return r;
}

static inline ssize_t jdksavdecc_eui48_read( struct jdksavdecc_eui48 *host_value, void const *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( host_value->value, ( (uint8_t const *)base ) + pos, sizeof( host_value->value ) );
    }
    return r;
}

static inline struct jdksavdecc_eui48 jdksavdecc_eui48_get( void const *base, ssize_t pos )
{
    struct jdksavdecc_eui48 v;
    memcpy( v.value, ( (uint8_t const *)base ) + pos, sizeof( v.value ) );
    return v;
}

static inline ssize_t jdksavdecc_eui48_write( struct jdksavdecc_eui48 const *host_value, void *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( ( (uint8_t *)base ) + pos, host_value->value, sizeof( host_value->value ) );
    }
    return r;
}

static inline void jdksavdecc_eui48_set( struct jdksavdecc_eui48 v, void *base, ssize_t pos )
{
    memcpy( ( (uint8_t *)base ) + pos, v.value, sizeof( v.value ) );
}

static inline int jdksavdecc_eui48_is_unset( struct jdksavdecc_eui48 v )
{
    int r = 0;
    if ( v.value[0] == 0xff && v.value[1] == 0xff && v.value[2] == 0xff && v.value[3] == 0xff && v.value[4] == 0xff
         && v.value[5] == 0xff )
    {
        r = 1;
    }
    return r;
}

static inline int jdksavdecc_eui48_is_set( struct jdksavdecc_eui48 v )
{
    int r = 0;
    if ( v.value[0] != 0xff || v.value[1] != 0xff || v.value[2] != 0xff || v.value[3] != 0xff || v.value[4] != 0xff
         || v.value[5] != 0xff )
    {
        r = 1;
    }
    return r;
}

/*@}*/

/** \addtogroup eui64 eui64 */
/*@{*/

///
struct jdksavdecc_eui64
{
    uint8_t value[8];
};

static inline void jdksavdecc_eui64_init( struct jdksavdecc_eui64 *self )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = 0xff;
    }
}

static inline void jdksavdecc_eui64_init_from_uint64( struct jdksavdecc_eui64 *self, uint64_t other )
{
    self->value[0] = ( uint8_t )( ( other >> ( 7 * 8 ) ) & 0xff );
    self->value[1] = ( uint8_t )( ( other >> ( 6 * 8 ) ) & 0xff );
    self->value[2] = ( uint8_t )( ( other >> ( 5 * 8 ) ) & 0xff );
    self->value[3] = ( uint8_t )( ( other >> ( 4 * 8 ) ) & 0xff );
    self->value[4] = ( uint8_t )( ( other >> ( 3 * 8 ) ) & 0xff );
    self->value[5] = ( uint8_t )( ( other >> ( 2 * 8 ) ) & 0xff );
    self->value[6] = ( uint8_t )( ( other >> ( 1 * 8 ) ) & 0xff );
    self->value[7] = ( uint8_t )( ( other >> ( 0 * 8 ) ) & 0xff );
}

int jdksavdecc_eui64_init_from_cstr( struct jdksavdecc_eui64 *self, const char *str );

static inline uint64_t jdksavdecc_eui64_convert_to_uint64( struct jdksavdecc_eui64 const *self )
{
    uint64_t v = 0;
    v |= ( (uint64_t)self->value[0] ) << ( 7 * 8 );
    v |= ( (uint64_t)self->value[1] ) << ( 6 * 8 );
    v |= ( (uint64_t)self->value[2] ) << ( 5 * 8 );
    v |= ( (uint64_t)self->value[3] ) << ( 4 * 8 );
    v |= ( (uint64_t)self->value[4] ) << ( 3 * 8 );
    v |= ( (uint64_t)self->value[5] ) << ( 2 * 8 );
    v |= ( (uint64_t)self->value[6] ) << ( 1 * 8 );
    v |= ( (uint64_t)self->value[7] ) << ( 0 * 8 );
    return v;
}

static inline void jdksavdecc_eui64_copy( struct jdksavdecc_eui64 *self, struct jdksavdecc_eui64 const *other )
{
    size_t i;
    for ( i = 0; i < sizeof( self->value ); ++i )
    {
        self->value[i] = other->value[i];
    }
}

static inline int jdksavdecc_eui64_compare( struct jdksavdecc_eui64 const *self, struct jdksavdecc_eui64 const *other )
{
    int r = memcmp( self->value, other->value, sizeof( self->value ) );
    return r;
}

static inline ssize_t jdksavdecc_eui64_read( struct jdksavdecc_eui64 *host_value, void const *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( host_value->value, ( (uint8_t const *)base ) + pos, sizeof( host_value->value ) );
        r = pos + sizeof( host_value->value );
    }
    return r;
}

static inline struct jdksavdecc_eui64 jdksavdecc_eui64_get( void const *base, ssize_t pos )
{
    struct jdksavdecc_eui64 v;
    memcpy( v.value, ( (uint8_t const *)base ) + pos, sizeof( v.value ) );
    return v;
}

static inline ssize_t jdksavdecc_eui64_write( struct jdksavdecc_eui64 const *host_value, void *base, ssize_t pos, size_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, sizeof( host_value->value ) );
    if ( r >= 0 )
    {
        memcpy( ( (uint8_t *)base ) + pos, host_value->value, sizeof( host_value->value ) );
        r = pos + sizeof( host_value->value );
    }
    return r;
}

static inline void jdksavdecc_eui64_set( struct jdksavdecc_eui64 v, void *base, ssize_t pos )
{
    memcpy( ( (uint8_t *)base ) + pos, v.value, sizeof( v.value ) );
}

static inline int jdksavdecc_eui64_is_unset( struct jdksavdecc_eui64 v )
{
    int r = 0;
    if ( v.value[0] == 0xff && v.value[1] == 0xff && v.value[2] == 0xff && v.value[3] == 0xff && v.value[4] == 0xff
         && v.value[5] == 0xff && v.value[6] == 0xff && v.value[7] == 0xff )
    {
        r = 1;
    }
    return r;
}

static inline int jdksavdecc_eui64_is_set( struct jdksavdecc_eui64 v )
{
    int r = 0;
    if ( v.value[0] != 0xff || v.value[1] != 0xff || v.value[2] != 0xff || v.value[3] != 0xff || v.value[4] != 0xff
         || v.value[5] != 0xff || v.value[6] != 0xff || v.value[7] != 0xff )
    {
        r = 1;
    }
    return r;
}
/*@}*/

/** \addtogroup gptp GPTP time */
/*@{*/

struct jdksavdecc_gptp_seconds
{
    uint64_t seconds : 48;
};

static inline void jdksavdecc_gptp_seconds_init( struct jdksavdecc_gptp_seconds *self )
{
    self->seconds = 0;
}

static inline void jdksavdecc_gptp_seconds_read( struct jdksavdecc_gptp_seconds *host_value, void const *base, ssize_t pos )
{
    uint8_t const *b = ( (uint8_t const *)base ) + pos;
    host_value->seconds = ( ( (uint64_t)b[0] ) << ( 8 * 5 ) ) + ( ( (uint64_t)b[1] ) << ( 8 * 4 ) )
                          + ( ( (uint64_t)b[2] ) << ( 8 * 3 ) ) + ( ( (uint64_t)b[3] ) << ( 8 * 2 ) )
                          + ( ( (uint64_t)b[4] ) << ( 8 * 1 ) ) + b[5];
}

static inline struct jdksavdecc_gptp_seconds jdksavdecc_gptp_seconds_get( void const *base, ssize_t pos )
{
    struct jdksavdecc_gptp_seconds v;
    uint8_t const *b = ( (uint8_t const *)base ) + pos;

    v.seconds = ( ( (uint64_t)b[0] ) << ( 8 * 5 ) ) + ( ( (uint64_t)b[1] ) << ( 8 * 4 ) ) + ( ( (uint64_t)b[2] ) << ( 8 * 3 ) )
                + ( ( (uint64_t)b[3] ) << ( 8 * 2 ) ) + ( ( (uint64_t)b[4] ) << ( 8 * 1 ) ) + b[5];

    return v;
}

static inline ssize_t
    jdksavdecc_gptp_seconds_write( struct jdksavdecc_gptp_seconds host_value, void *base, ssize_t pos, ssize_t len )
{
    ssize_t r = jdksavdecc_validate_range( pos, len, 6 );
    if ( r >= 0 )
    {
        uint8_t *b = ( (uint8_t *)base ) + pos;

        b[0] = ( uint8_t )( ( host_value.seconds ) >> ( 8 * 5 ) ) & 0xff;
        b[1] = ( uint8_t )( ( host_value.seconds ) >> ( 8 * 4 ) ) & 0xff;
        b[2] = ( uint8_t )( ( host_value.seconds ) >> ( 8 * 3 ) ) & 0xff;
        b[3] = ( uint8_t )( ( host_value.seconds ) >> ( 8 * 2 ) ) & 0xff;
        b[4] = ( uint8_t )( ( host_value.seconds ) >> ( 8 * 1 ) ) & 0xff;
        b[5] = ( uint8_t )( ( host_value.seconds ) ) & 0xff;
    }
    return r;
}

static inline void jdksavdecc_gptp_seconds_set( struct jdksavdecc_gptp_seconds v, void *base, ssize_t pos )
{
    uint8_t *b = ( (uint8_t *)base ) + pos;
    b[0] = ( uint8_t )( ( v.seconds ) >> ( 8 * 5 ) ) & 0xff;
    b[1] = ( uint8_t )( ( v.seconds ) >> ( 8 * 4 ) ) & 0xff;
    b[2] = ( uint8_t )( ( v.seconds ) >> ( 8 * 3 ) ) & 0xff;
    b[3] = ( uint8_t )( ( v.seconds ) >> ( 8 * 2 ) ) & 0xff;
    b[4] = ( uint8_t )( ( v.seconds ) >> ( 8 * 1 ) ) & 0xff;
    b[5] = ( uint8_t )( ( v.seconds ) ) & 0xff;
}

/*@}*/

/// Typedef for a pointer to a procedure that can be used to
/// lookup a localized_string from a localized string index, given a context/tag
/// which would refer to an entity model.
typedef struct jdksavdecc_string const
    *( *jdksavdecc_localized_string_lookup_proc )( void *context, const char *locale_name, uint16_t localized_string_id );

/*@}*/
#ifdef __cplusplus
}
#endif

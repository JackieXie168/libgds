/*
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  $Id$
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Changed to use uint32_t instead of uint 2004 by Anders Broman
 *	Original code found at http://www.cr0.net:8040/code/crypto/sha1/
 *  References: http://www.ietf.org/rfc/rfc3174.txt?number=3174
 */

#ifndef __CRYPT_SHA1_H__
#define __CRYPT_SHA1_H__

#include <libgds/types.h>

typedef struct _SHA1Context SSHA1Context;
struct _SHA1Context {
  uint32_t total[2];
  uint32_t state[5];
  uint8_t buffer[64];
};

void sha1_starts( SSHA1Context *ctx );
void sha1_update( SSHA1Context *ctx, 
		  const uint8_t *input, 
		  uint32_t length );
void sha1_finish( SSHA1Context *ctx, 
		  uint8_t digest[20] );
void sha1_hmac( const uint8_t *key, 
		uint32_t keylen, 
		const uint8_t *buf, 
		uint32_t buflen,
                uint8_t digest[20] );

#endif /* __CRYPT_SHA1_H__ */

/* o_init.c */
/* Written by Dr Stephen N Henson (steve@openssl.org) for the OpenSSL
 * project.
 */
/* ====================================================================
 * Copyright (c) 2011 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 */

#include <e_os.h>
#include <openssl/err.h>
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#include <openssl/rand.h>
#endif

#if defined(__GNUC__) && __GNUC__>=2
  void OPENSSL_init(void) __attribute__((constructor));
  /* Most commonly this results in pointer to OPENSSL_init to be dropped
   * to .ctors segment, which is traversed by GCC crtbegin.o upon
   * program startup. Except on a.out OpenBSD where it results in
   * _GLOBAL_$I$init() {init();} being auto-generated by
   * compiler... But one way or another this is believed to cover
   * *all* GCC targets. */
#elif defined(_MSC_VER)
# ifdef _WINDLL
  __declspec(dllexport)	/* this is essentially cosmetics... */
# endif
  void OPENSSL_init(void);
  static int init_wrapper(void) { OPENSSL_init(); return 0; }
# ifdef _WIN64
# pragma section(".CRT$XCU",read)
  __declspec(allocate(".CRT$XCU"))
# else
# pragma data_seg(".CRT$XCU")
# endif
  static int (*p)(void) = init_wrapper;
  /* This results in pointer to init to appear in .CRT segment,
   * which is traversed by Visual C run-time initialization code.
   * This applies to both Win32 and [all flavors of] Win64. */
# pragma data_seg()
#elif defined(__SUNPRO_C)
  void OPENSSL_init(void);
# pragma init(OPENSSL_init)
  /* This results in a call to init to appear in .init segment. */
#elif defined(__DECC) && (defined(__VMS) || defined(VMS))
  void OPENSSL_init(void);
# pragma __nostandard
  globaldef { "LIB$INITIALIZ" } readonly _align (LONGWORD)
	int spare[8] = {0};
  globaldef { "LIB$INITIALIZE" } readonly _align (LONGWORD)
	void (*x_OPENSSL_init)(void) = OPENSSL_init;
  /* Refer to LIB$INITIALIZE to ensure it exists in the image. */
  int lib$initialize();
  globaldef int (*lib_init_ref)() = lib$initialize;
# pragma __standard
#elif 0
  The rest has to be taken care of through command line:

	-Wl,-init,OPENSSL_init		on OSF1 and IRIX
	-Wl,+init,OPENSSL_init		on HP-UX
	-Wl,-binitfini:OPENSSL_init	on AIX

  On ELF platforms this results in a call to OPENSSL_init to appear in
  .init segment...
#endif

/* Perform any essential OpenSSL initialization operations.
 * Currently only sets FIPS callbacks
 */

void OPENSSL_init(void)
	{
	static int done = 0;
	if (done)
		return;
	done = 1;
#ifdef OPENSSL_FIPS
	FIPS_set_locking_callbacks(CRYPTO_lock, CRYPTO_add_lock);
	FIPS_set_error_callbacks(ERR_put_error, ERR_add_error_vdata);
	FIPS_set_malloc_callbacks(CRYPTO_malloc, CRYPTO_free);
	RAND_init_fips();
#endif
#if 0
	fprintf(stderr, "Called OPENSSL_init\n");
#endif
	}

/*	$OpenBSD: pthread_specific.c,v 1.1 2002/05/03 10:08:55 wcobb Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <pthread.h>
#include <pthread_np.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"

#define NTHREADS	128

pthread_key_t key;
int destroy_run = 0;

void *
run_thread(void *arg)
{
	int i;

	fprintf(stderr, ".");
	for (i = 0; i < 32767; i++) {
		void *p;

		p = pthread_getspecific(key);
		if (p == NULL) {
			CHECKr(pthread_setspecific(key, pthread_self()));
		} else {
			ASSERT(p == pthread_self());
		}
		fflush(stderr);
	}

	return (NULL);
}

void
destroy_key(void *keyp)
{
	destroy_run++;
}

int
main()
{
	pthread_t threads[NTHREADS];
	int i;

	CHECKr(pthread_key_create(&key, destroy_key));
	for (i = 0; i < NTHREADS; i++) {
		CHECKr(pthread_create(&threads[i], NULL, run_thread, NULL));
	}
	for (i = 0; i < NTHREADS; i++) {
		CHECKr(pthread_join(threads[i], NULL));
	}
	fprintf(stderr, "\n");
	
	CHECKr(pthread_key_delete(key));

	ASSERT(destroy_run > 0);

	SUCCEED;
}

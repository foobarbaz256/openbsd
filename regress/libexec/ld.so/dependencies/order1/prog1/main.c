/*
 * Copyright (c) 2014 Henri Kemppainen <duclare@guu.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <dlfcn.h>
#include <stdio.h>

int
main()
{
        void *libaa, *libbb;
	int flag = RTLD_NOW;

        if ((libaa = dlopen("libaa.so", flag)) == NULL) {
                printf("dlopen(\"libaa.so\", %d) FAILED\n", flag);
                return 1;
        }

	if ((libbb = dlopen("libbb.so", flag)) == NULL) {
		printf("dlopen(\"libbb.so\", %d) FAILED\n", flag);
		return 1;
	}

        if (dlclose(libbb)) {
                printf("dlclose(libbb) FAILED\n%s\n", dlerror());
		return 1;
        }

	if ((libbb = dlopen("libbb.so", flag)) == NULL) {
		printf("dlopen(\"libbb.so\", %d) FAILED\n", flag);
		return 1;
	}

        if (dlclose(libbb)) {
                printf("dlclose(libbb) FAILED\n%s\n", dlerror());
		return 1;
        }

        if (dlclose(libaa)) {
                printf("dlclose(libaa) FAILED\n%s\n", dlerror());
		return 1;
        }

	return 0;
}


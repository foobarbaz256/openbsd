/*	$OpenBSD: getentropy_osx.c,v 1.5 2014/07/13 08:24:20 beck Exp $	*/

/*
 * Copyright (c) 2014 Theo de Raadt <deraadt@openbsd.org>
 * Copyright (c) 2014 Bob Beck <beck@obtuse.com>
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <mach/mach_time.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <sys/socketvar.h>
#include <sys/vmmeter.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_var.h>
#include <netinet/udp_var.h>
#include <CommonCrypto/CommonDigest.h>
#define SHA512_Update(a, b, c)	(CC_SHA512_Update((a), (b), (c)))
#define SHA512_Init(xxx) (CC_SHA512_Init((xxx)))
#define SHA512_Final(xxx, yyy) (CC_SHA512_Final((xxx), (yyy)))
#define SHA512_CTX CC_SHA512_CTX
#define SHA512_DIGEST_LENGTH CC_SHA512_DIGEST_LENGTH

#define REPEAT 5
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define HX(a, b) \
	do { \
		if ((a)) \
			HD(errno); \
		else \
			HD(b); \
	} while (0)

#define HR(x, l) (SHA512_Update(&ctx, (char *)(x), (l)))
#define HD(x)	 (SHA512_Update(&ctx, (char *)&(x), sizeof (x)))
#define HF(x)    (SHA512_Update(&ctx, (char *)&(x), sizeof (void*)))

int	getentropy(void *buf, size_t len);

#if 0
extern int main(int, char *argv[]);
#endif
static int gotdata(char *buf, size_t len);
static int getentropy_urandom(void *buf, size_t len);
static int getentropy_fallback(void *buf, size_t len);

int
getentropy(void *buf, size_t len)
{
	int ret = -1;

	if (len > 256) {
		errno = EIO;
		return -1;
	}

	/*
	 * Try to get entropy with /dev/urandom
	 *
	 * This can fail if the process is inside a chroot or if file
	 * descriptors are exhausted.
	 */
	ret = getentropy_urandom(buf, len);
	if (ret != -1)
		return (ret);

	/*
	 * Entropy collection via /dev/urandom and sysctl have failed.
	 *
	 * No other API exists for collecting entropy, and we have
	 * no failsafe way to get it on OSX that is not sensitive
	 * to resource exhaustion.
	 *
	 * We have very few options:
	 *     - Even syslog_r is unsafe to call at this low level, so
	 *	 there is no way to alert the user or program.
	 *     - Cannot call abort() because some systems have unsafe
	 *	 corefiles.
	 *     - Could raise(SIGKILL) resulting in silent program termination.
	 *     - Return EIO, to hint that arc4random's stir function
	 *       should raise(SIGKILL)
	 *     - Do the best under the circumstances....
	 *
	 * This code path exists to bring light to the issue that OSX
	 * does not provide a failsafe API for entropy collection.
	 *
	 * We hope this demonstrates that OSX should consider
	 * providing a new failsafe API which works in a chroot or
	 * when file descriptors are exhausted.
	 */
#undef FAIL_INSTEAD_OF_TRYING_FALLBACK
#ifdef FAIL_INSTEAD_OF_TRYING_FALLBACK
	raise(SIGKILL);
#endif
	ret = getentropy_fallback(buf, len);
	if (ret != -1)
		return (ret);

	errno = EIO;
	return (ret);
}

/*
 * Basic sanity checking; wish we could do better.
 */
static int
gotdata(char *buf, size_t len)
{
	char	any_set = 0;
	size_t	i;

	for (i = 0; i < len; ++i)
		any_set |= buf[i];
	if (any_set == 0)
		return -1;
	return 0;
}

static int
getentropy_urandom(void *buf, size_t len)
{
	struct stat st;
	size_t i;
	int fd, flags;
	int save_errno = errno;

start:

	flags = O_RDONLY;
#ifdef O_NOFOLLOW
	flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif
	fd = open("/dev/urandom", flags, 0);
	if (fd == -1) {
		if (errno == EINTR)
			goto start;
		goto nodevrandom;
	}
#ifndef O_CLOEXEC
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif

	/* Lightly verify that the device node looks sane */
	if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)) {
		close(fd);
		goto nodevrandom;
	}
	for (i = 0; i < len; ) {
		size_t wanted = len - i;
		ssize_t ret = read(fd, (char *)buf + i, wanted);

		if (ret == -1) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			close(fd);
			goto nodevrandom;
		}
		i += ret;
	}
	close(fd);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return 0;		/* satisfied */
	}
nodevrandom:
	errno = EIO;
	return -1;
}

static int tcpmib[] = { CTL_NET, AF_INET, IPPROTO_TCP, TCPCTL_STATS };
static int udpmib[] = { CTL_NET, AF_INET, IPPROTO_UDP, UDPCTL_STATS };
static int ipmib[] = { CTL_NET, AF_INET, IPPROTO_IP, IPCTL_STATS };
static int kmib[] = { CTL_KERN, KERN_USRSTACK };
static int hwmib[] = { CTL_HW, HW_USERMEM };

static int
getentropy_fallback(void *buf, size_t len)
{
	uint8_t results[SHA512_DIGEST_LENGTH];
	int save_errno = errno, e, pgs = getpagesize(), faster = 0, repeat;
	static int cnt;
	struct timespec ts;
	struct timeval tv;
	struct rusage ru;
	sigset_t sigset;
	struct stat st;
	SHA512_CTX ctx;
	static pid_t lastpid;
	pid_t pid;
	size_t i, ii, m;
	char *p;
	struct tcpstat tcpstat;
	struct udpstat udpstat;
	struct ipstat ipstat;
	u_int64_t mach_time;
	unsigned int idata;
	void *addr;

	pid = getpid();
	if (lastpid == pid) {
		faster = 1;
		repeat = 2;
	} else {
		faster = 0;
		lastpid = pid;
		repeat = REPEAT;
	}
	for (i = 0; i < len; ) {
		int j;
		SHA512_Init(&ctx);
		for (j = 0; j < repeat; j++) {
			HX((e = gettimeofday(&tv, NULL)) == -1, tv);
			if (e != -1) {
				cnt += (int)tv.tv_sec;
				cnt += (int)tv.tv_usec;
			}

			mach_time = mach_absolute_time();
			HD(mach_time);

			ii = sizeof(addr);
			HX(sysctl(kmib, sizeof(kmib) / sizeof(kmib[0]),
			    &addr, &ii, NULL, 0) == -1, addr);

			ii = sizeof(idata);
			HX(sysctl(hwmib, sizeof(hwmib) / sizeof(hwmib[0]),
			    &idata, &ii, NULL, 0) == -1, idata);

			ii = sizeof(tcpstat);
			HX(sysctl(tcpmib, sizeof(tcpmib) / sizeof(tcpmib[0]),
			    &tcpstat, &ii, NULL, 0) == -1, tcpstat);

			ii = sizeof(udpstat);
			HX(sysctl(udpmib, sizeof(udpmib) / sizeof(udpmib[0]),
			    &udpstat, &ii, NULL, 0) == -1, udpstat);

			ii = sizeof(ipstat);
			HX(sysctl(ipmib, sizeof(ipmib) / sizeof(ipmib[0]),
			    &ipstat, &ii, NULL, 0) == -1, ipstat);

			HX((pid = getpid()) == -1, pid);
			HX((pid = getsid(pid)) == -1, pid);
			HX((pid = getppid()) == -1, pid);
			HX((pid = getpgid(0)) == -1, pid);
			HX((e = getpriority(0, 0)) == -1, e);

			if (!faster) {
				ts.tv_sec = 0;
				ts.tv_nsec = 1;
				(void) nanosleep(&ts, NULL);
			}

			HX(sigpending(&sigset) == -1, sigset);
			HX(sigprocmask(SIG_BLOCK, NULL, &sigset) == -1,
			    sigset);

#if 0
			HF(main);		/* an addr in program */
#endif
			HF(getentropy);	/* an addr in this library */
			HF(printf);		/* an addr in libc */
			p = (char *)&p;
			HD(p);		/* an addr on stack */
			p = (char *)&errno;
			HD(p);		/* the addr of errno */

			if (i == 0) {
				struct sockaddr_storage ss;
				struct statvfs stvfs;
				struct termios tios;
				struct statfs stfs;
				socklen_t ssl;
				off_t off;

				/*
				 * Prime-sized mappings encourage fragmentation;
				 * thus exposing some address entropy.
				 */
				struct mm {
					size_t	npg;
					void	*p;
				} mm[] =	 {
					{ 17, MAP_FAILED }, { 3, MAP_FAILED },
					{ 11, MAP_FAILED }, { 2, MAP_FAILED },
					{ 5, MAP_FAILED }, { 3, MAP_FAILED },
					{ 7, MAP_FAILED }, { 1, MAP_FAILED },
					{ 57, MAP_FAILED }, { 3, MAP_FAILED },
					{ 131, MAP_FAILED }, { 1, MAP_FAILED },
				};

				for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
					HX(mm[m].p = mmap(NULL,
					    mm[m].npg * pgs,
					    PROT_READ|PROT_WRITE,
					    MAP_PRIVATE|MAP_ANON, -1,
					    (off_t)0), mm[m].p);
					if (mm[m].p != MAP_FAILED) {
						size_t mo;

						/* Touch some memory... */
						p = mm[m].p;
						mo = cnt %
						    (mm[m].npg * pgs - 1);
						p[mo] = 1;
						cnt += (int)((long)(mm[m].p)
						    / pgs);
					}

					/* Check cnts and times... */
					mach_time = mach_absolute_time();
					HD(mach_time);
					cnt += (int)mach_time;

					HX((e = getrusage(RUSAGE_SELF,
					    &ru)) == -1, ru);
					if (e != -1) {
						cnt += (int)ru.ru_utime.tv_sec;
						cnt += (int)ru.ru_utime.tv_usec;
					}
				}

				for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
					if (mm[m].p != MAP_FAILED)
						munmap(mm[m].p, mm[m].npg * pgs);
					mm[m].p = MAP_FAILED;
				}

				HX(stat(".", &st) == -1, st);
				HX(statvfs(".", &stvfs) == -1, stvfs);
				HX(statfs(".", &stfs) == -1, stfs);

				HX(stat("/", &st) == -1, st);
				HX(statvfs("/", &stvfs) == -1, stvfs);
				HX(statfs("/", &stfs) == -1, stfs);

				HX((e = fstat(0, &st)) == -1, st);
				if (e == -1) {
					if (S_ISREG(st.st_mode) ||
					    S_ISFIFO(st.st_mode) ||
					    S_ISSOCK(st.st_mode)) {
						HX(fstatvfs(0, &stvfs) == -1,
						    stvfs);
						HX(fstatfs(0, &stfs) == -1,
						    stfs);
						HX((off = lseek(0, (off_t)0,
						    SEEK_CUR)) < 0, off);
					}
					if (S_ISCHR(st.st_mode)) {
						HX(tcgetattr(0, &tios) == -1,
						    tios);
					} else if (S_ISSOCK(st.st_mode)) {
						memset(&ss, 0, sizeof ss);
						ssl = sizeof(ss);
						HX(getpeername(0,
						    (void *)&ss, &ssl) == -1,
						    ss);
					}
				}

				HX((e = getrusage(RUSAGE_CHILDREN,
				    &ru)) == -1, ru);
				if (e != -1) {
					cnt += (int)ru.ru_utime.tv_sec;
					cnt += (int)ru.ru_utime.tv_usec;
				}
			} else {
				/* Subsequent hashes absorb previous result */
				HD(results);
			}

			HX((e = gettimeofday(&tv, NULL)) == -1, tv);
			if (e != -1) {
				cnt += (int)tv.tv_sec;
				cnt += (int)tv.tv_usec;
			}

			HD(cnt);
		}

		SHA512_Final(results, &ctx);
		memcpy((char *)buf + i, results, min(sizeof(results), len - i));
		i += min(sizeof(results), len - i);
	}
	memset(results, 0, sizeof results);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return 0;		/* satisfied */
	}
	errno = EIO;
	return -1;
}

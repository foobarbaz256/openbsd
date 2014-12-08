/*	$OpenBSD: syscalls.c,v 1.162 2014/12/08 20:56:34 guenther Exp $	*/

/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from;	OpenBSD: syscalls.master,v 1.149 2014/12/08 20:56:11 guenther Exp 
 */

char *syscallnames[] = {
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"open",			/* 5 = open */
	"close",			/* 6 = close */
	"getentropy",			/* 7 = getentropy */
	"__tfork",			/* 8 = __tfork */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"wait4",			/* 11 = wait4 */
	"chdir",			/* 12 = chdir */
	"fchdir",			/* 13 = fchdir */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"break",			/* 17 = break */
	"getdtablecount",			/* 18 = getdtablecount */
	"getrusage",			/* 19 = getrusage */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"unmount",			/* 22 = unmount */
	"setuid",			/* 23 = setuid */
	"getuid",			/* 24 = getuid */
	"geteuid",			/* 25 = geteuid */
#ifdef PTRACE
	"ptrace",			/* 26 = ptrace */
#else
	"#26 (unimplemented ptrace)",		/* 26 = unimplemented ptrace */
#endif
	"recvmsg",			/* 27 = recvmsg */
	"sendmsg",			/* 28 = sendmsg */
	"recvfrom",			/* 29 = recvfrom */
	"accept",			/* 30 = accept */
	"getpeername",			/* 31 = getpeername */
	"getsockname",			/* 32 = getsockname */
	"access",			/* 33 = access */
	"chflags",			/* 34 = chflags */
	"fchflags",			/* 35 = fchflags */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"stat",			/* 38 = stat */
	"getppid",			/* 39 = getppid */
	"lstat",			/* 40 = lstat */
	"dup",			/* 41 = dup */
	"fstatat",			/* 42 = fstatat */
	"getegid",			/* 43 = getegid */
	"profil",			/* 44 = profil */
#ifdef KTRACE
	"ktrace",			/* 45 = ktrace */
#else
	"#45 (unimplemented ktrace)",		/* 45 = unimplemented ktrace */
#endif
	"sigaction",			/* 46 = sigaction */
	"getgid",			/* 47 = getgid */
	"sigprocmask",			/* 48 = sigprocmask */
	"getlogin",			/* 49 = getlogin */
	"setlogin",			/* 50 = setlogin */
#ifdef ACCOUNTING
	"acct",			/* 51 = acct */
#else
	"#51 (unimplemented acct)",		/* 51 = unimplemented acct */
#endif
	"sigpending",			/* 52 = sigpending */
	"fstat",			/* 53 = fstat */
	"ioctl",			/* 54 = ioctl */
	"reboot",			/* 55 = reboot */
	"revoke",			/* 56 = revoke */
	"symlink",			/* 57 = symlink */
	"readlink",			/* 58 = readlink */
	"execve",			/* 59 = execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"getfsstat",			/* 62 = getfsstat */
	"statfs",			/* 63 = statfs */
	"fstatfs",			/* 64 = fstatfs */
	"fhstatfs",			/* 65 = fhstatfs */
	"vfork",			/* 66 = vfork */
	"gettimeofday",			/* 67 = gettimeofday */
	"settimeofday",			/* 68 = settimeofday */
	"setitimer",			/* 69 = setitimer */
	"getitimer",			/* 70 = getitimer */
	"select",			/* 71 = select */
	"kevent",			/* 72 = kevent */
	"munmap",			/* 73 = munmap */
	"mprotect",			/* 74 = mprotect */
	"madvise",			/* 75 = madvise */
	"utimes",			/* 76 = utimes */
	"futimes",			/* 77 = futimes */
	"mincore",			/* 78 = mincore */
	"getgroups",			/* 79 = getgroups */
	"setgroups",			/* 80 = setgroups */
	"getpgrp",			/* 81 = getpgrp */
	"setpgid",			/* 82 = setpgid */
	"sendsyslog",			/* 83 = sendsyslog */
	"utimensat",			/* 84 = utimensat */
	"futimens",			/* 85 = futimens */
	"#86 (obsolete t32_getitimer)",		/* 86 = obsolete t32_getitimer */
	"clock_gettime",			/* 87 = clock_gettime */
	"clock_settime",			/* 88 = clock_settime */
	"clock_getres",			/* 89 = clock_getres */
	"dup2",			/* 90 = dup2 */
	"nanosleep",			/* 91 = nanosleep */
	"fcntl",			/* 92 = fcntl */
	"accept4",			/* 93 = accept4 */
	"__thrsleep",			/* 94 = __thrsleep */
	"fsync",			/* 95 = fsync */
	"setpriority",			/* 96 = setpriority */
	"socket",			/* 97 = socket */
	"connect",			/* 98 = connect */
	"getdents",			/* 99 = getdents */
	"getpriority",			/* 100 = getpriority */
	"pipe2",			/* 101 = pipe2 */
	"dup3",			/* 102 = dup3 */
	"sigreturn",			/* 103 = sigreturn */
	"bind",			/* 104 = bind */
	"setsockopt",			/* 105 = setsockopt */
	"listen",			/* 106 = listen */
	"chflagsat",			/* 107 = chflagsat */
	"#108 (obsolete osigvec)",		/* 108 = obsolete osigvec */
	"ppoll",			/* 109 = ppoll */
	"pselect",			/* 110 = pselect */
	"sigsuspend",			/* 111 = sigsuspend */
	"#112 (obsolete osigstack)",		/* 112 = obsolete osigstack */
	"#113 (obsolete orecvmsg)",		/* 113 = obsolete orecvmsg */
	"#114 (obsolete osendmsg)",		/* 114 = obsolete osendmsg */
	"#115 (obsolete vtrace)",		/* 115 = obsolete vtrace */
	"#116 (obsolete t32_gettimeofday)",		/* 116 = obsolete t32_gettimeofday */
	"#117 (obsolete t32_getrusage)",		/* 117 = obsolete t32_getrusage */
	"getsockopt",			/* 118 = getsockopt */
	"#119 (obsolete resuba)",		/* 119 = obsolete resuba */
	"readv",			/* 120 = readv */
	"writev",			/* 121 = writev */
	"#122 (obsolete t32_settimeofday)",		/* 122 = obsolete t32_settimeofday */
	"fchown",			/* 123 = fchown */
	"fchmod",			/* 124 = fchmod */
	"#125 (obsolete orecvfrom)",		/* 125 = obsolete orecvfrom */
	"setreuid",			/* 126 = setreuid */
	"setregid",			/* 127 = setregid */
	"rename",			/* 128 = rename */
	"#129 (obsolete otruncate)",		/* 129 = obsolete otruncate */
	"#130 (obsolete oftruncate)",		/* 130 = obsolete oftruncate */
	"flock",			/* 131 = flock */
	"mkfifo",			/* 132 = mkfifo */
	"sendto",			/* 133 = sendto */
	"shutdown",			/* 134 = shutdown */
	"socketpair",			/* 135 = socketpair */
	"mkdir",			/* 136 = mkdir */
	"rmdir",			/* 137 = rmdir */
	"#138 (obsolete t32_utimes)",		/* 138 = obsolete t32_utimes */
	"#139 (obsolete 4.2 sigreturn)",		/* 139 = obsolete 4.2 sigreturn */
	"adjtime",			/* 140 = adjtime */
	"#141 (obsolete ogetpeername)",		/* 141 = obsolete ogetpeername */
	"#142 (obsolete ogethostid)",		/* 142 = obsolete ogethostid */
	"#143 (obsolete osethostid)",		/* 143 = obsolete osethostid */
	"#144 (obsolete ogetrlimit)",		/* 144 = obsolete ogetrlimit */
	"#145 (obsolete osetrlimit)",		/* 145 = obsolete osetrlimit */
	"#146 (obsolete okillpg)",		/* 146 = obsolete okillpg */
	"setsid",			/* 147 = setsid */
	"quotactl",			/* 148 = quotactl */
	"#149 (obsolete oquota)",		/* 149 = obsolete oquota */
	"#150 (obsolete ogetsockname)",		/* 150 = obsolete ogetsockname */
	"#151 (unimplemented)",		/* 151 = unimplemented */
	"#152 (unimplemented)",		/* 152 = unimplemented */
	"#153 (unimplemented)",		/* 153 = unimplemented */
	"#154 (unimplemented)",		/* 154 = unimplemented */
#if defined(NFSCLIENT) || defined(NFSSERVER)
	"nfssvc",			/* 155 = nfssvc */
#else
	"#155 (unimplemented)",		/* 155 = unimplemented */
#endif
	"#156 (obsolete ogetdirentries)",		/* 156 = obsolete ogetdirentries */
	"#157 (obsolete statfs25)",		/* 157 = obsolete statfs25 */
	"#158 (obsolete fstatfs25)",		/* 158 = obsolete fstatfs25 */
	"#159 (unimplemented)",		/* 159 = unimplemented */
	"#160 (unimplemented)",		/* 160 = unimplemented */
	"getfh",			/* 161 = getfh */
	"#162 (obsolete ogetdomainname)",		/* 162 = obsolete ogetdomainname */
	"#163 (obsolete osetdomainname)",		/* 163 = obsolete osetdomainname */
	"#164 (unimplemented ouname)",		/* 164 = unimplemented ouname */
	"sysarch",			/* 165 = sysarch */
	"#166 (unimplemented)",		/* 166 = unimplemented */
	"#167 (unimplemented)",		/* 167 = unimplemented */
	"#168 (unimplemented)",		/* 168 = unimplemented */
	"#169 (obsolete semsys10)",		/* 169 = obsolete semsys10 */
	"#170 (obsolete msgsys10)",		/* 170 = obsolete msgsys10 */
	"#171 (obsolete shmsys10)",		/* 171 = obsolete shmsys10 */
	"#172 (unimplemented)",		/* 172 = unimplemented */
	"pread",			/* 173 = pread */
	"pwrite",			/* 174 = pwrite */
	"#175 (unimplemented ntp_gettime)",		/* 175 = unimplemented ntp_gettime */
	"#176 (unimplemented ntp_adjtime)",		/* 176 = unimplemented ntp_adjtime */
	"#177 (unimplemented)",		/* 177 = unimplemented */
	"#178 (unimplemented)",		/* 178 = unimplemented */
	"#179 (unimplemented)",		/* 179 = unimplemented */
	"#180 (unimplemented)",		/* 180 = unimplemented */
	"setgid",			/* 181 = setgid */
	"setegid",			/* 182 = setegid */
	"seteuid",			/* 183 = seteuid */
	"#184 (obsolete lfs_bmapv)",		/* 184 = obsolete lfs_bmapv */
	"#185 (obsolete lfs_markv)",		/* 185 = obsolete lfs_markv */
	"#186 (obsolete lfs_segclean)",		/* 186 = obsolete lfs_segclean */
	"#187 (obsolete lfs_segwait)",		/* 187 = obsolete lfs_segwait */
	"#188 (obsolete stat35)",		/* 188 = obsolete stat35 */
	"#189 (obsolete fstat35)",		/* 189 = obsolete fstat35 */
	"#190 (obsolete lstat35)",		/* 190 = obsolete lstat35 */
	"pathconf",			/* 191 = pathconf */
	"fpathconf",			/* 192 = fpathconf */
	"swapctl",			/* 193 = swapctl */
	"getrlimit",			/* 194 = getrlimit */
	"setrlimit",			/* 195 = setrlimit */
	"#196 (obsolete ogetdirentries48)",		/* 196 = obsolete ogetdirentries48 */
	"mmap",			/* 197 = mmap */
	"__syscall",			/* 198 = __syscall */
	"lseek",			/* 199 = lseek */
	"truncate",			/* 200 = truncate */
	"ftruncate",			/* 201 = ftruncate */
	"__sysctl",			/* 202 = __sysctl */
	"mlock",			/* 203 = mlock */
	"munlock",			/* 204 = munlock */
	"#205 (unimplemented sys_undelete)",		/* 205 = unimplemented sys_undelete */
	"#206 (obsolete t32_futimes)",		/* 206 = obsolete t32_futimes */
	"getpgid",			/* 207 = getpgid */
	"#208 (obsolete nnpfspioctl)",		/* 208 = obsolete nnpfspioctl */
	"utrace",			/* 209 = utrace */
	"#210 (unimplemented)",		/* 210 = unimplemented */
	"#211 (unimplemented)",		/* 211 = unimplemented */
	"#212 (unimplemented)",		/* 212 = unimplemented */
	"#213 (unimplemented)",		/* 213 = unimplemented */
	"#214 (unimplemented)",		/* 214 = unimplemented */
	"#215 (unimplemented)",		/* 215 = unimplemented */
	"#216 (unimplemented)",		/* 216 = unimplemented */
	"#217 (unimplemented)",		/* 217 = unimplemented */
	"#218 (unimplemented)",		/* 218 = unimplemented */
	"#219 (unimplemented)",		/* 219 = unimplemented */
#ifdef SYSVSEM
	"#220 (unimplemented)",		/* 220 = unimplemented */
	"semget",			/* 221 = semget */
#else
	"#220 (unimplemented semctl)",		/* 220 = unimplemented semctl */
	"#221 (unimplemented semget)",		/* 221 = unimplemented semget */
#endif
	"#222 (obsolete semop35)",		/* 222 = obsolete semop35 */
	"#223 (obsolete semconfig35)",		/* 223 = obsolete semconfig35 */
#ifdef SYSVMSG
	"#224 (unimplemented)",		/* 224 = unimplemented */
	"msgget",			/* 225 = msgget */
	"msgsnd",			/* 226 = msgsnd */
	"msgrcv",			/* 227 = msgrcv */
#else
	"#224 (unimplemented msgctl)",		/* 224 = unimplemented msgctl */
	"#225 (unimplemented msgget)",		/* 225 = unimplemented msgget */
	"#226 (unimplemented msgsnd)",		/* 226 = unimplemented msgsnd */
	"#227 (unimplemented msgrcv)",		/* 227 = unimplemented msgrcv */
#endif
#ifdef SYSVSHM
	"shmat",			/* 228 = shmat */
	"#229 (unimplemented)",		/* 229 = unimplemented */
	"shmdt",			/* 230 = shmdt */
#else
	"#228 (unimplemented shmat)",		/* 228 = unimplemented shmat */
	"#229 (unimplemented shmctl)",		/* 229 = unimplemented shmctl */
	"#230 (unimplemented shmdt)",		/* 230 = unimplemented shmdt */
#endif
	"#231 (obsolete shmget35)",		/* 231 = obsolete shmget35 */
	"#232 (obsolete t32_clock_gettime)",		/* 232 = obsolete t32_clock_gettime */
	"#233 (obsolete t32_clock_settime)",		/* 233 = obsolete t32_clock_settime */
	"#234 (obsolete t32_clock_getres)",		/* 234 = obsolete t32_clock_getres */
	"#235 (unimplemented timer_create)",		/* 235 = unimplemented timer_create */
	"#236 (unimplemented timer_delete)",		/* 236 = unimplemented timer_delete */
	"#237 (unimplemented timer_settime)",		/* 237 = unimplemented timer_settime */
	"#238 (unimplemented timer_gettime)",		/* 238 = unimplemented timer_gettime */
	"#239 (unimplemented timer_getoverrun)",		/* 239 = unimplemented timer_getoverrun */
	"#240 (obsolete t32_nanosleep)",		/* 240 = obsolete t32_nanosleep */
	"#241 (unimplemented)",		/* 241 = unimplemented */
	"#242 (unimplemented)",		/* 242 = unimplemented */
	"#243 (unimplemented)",		/* 243 = unimplemented */
	"#244 (unimplemented)",		/* 244 = unimplemented */
	"#245 (unimplemented)",		/* 245 = unimplemented */
	"#246 (unimplemented)",		/* 246 = unimplemented */
	"#247 (unimplemented)",		/* 247 = unimplemented */
	"#248 (unimplemented)",		/* 248 = unimplemented */
	"#249 (unimplemented)",		/* 249 = unimplemented */
	"minherit",			/* 250 = minherit */
	"#251 (obsolete rfork)",		/* 251 = obsolete rfork */
	"poll",			/* 252 = poll */
	"issetugid",			/* 253 = issetugid */
	"lchown",			/* 254 = lchown */
	"getsid",			/* 255 = getsid */
	"msync",			/* 256 = msync */
	"#257 (obsolete semctl35)",		/* 257 = obsolete semctl35 */
	"#258 (obsolete shmctl35)",		/* 258 = obsolete shmctl35 */
	"#259 (obsolete msgctl35)",		/* 259 = obsolete msgctl35 */
	"#260 (unimplemented)",		/* 260 = unimplemented */
	"#261 (unimplemented)",		/* 261 = unimplemented */
	"#262 (unimplemented)",		/* 262 = unimplemented */
	"pipe",			/* 263 = pipe */
	"fhopen",			/* 264 = fhopen */
	"#265 (unimplemented)",		/* 265 = unimplemented */
	"#266 (unimplemented)",		/* 266 = unimplemented */
	"preadv",			/* 267 = preadv */
	"pwritev",			/* 268 = pwritev */
	"kqueue",			/* 269 = kqueue */
	"#270 (obsolete t32_kevent)",		/* 270 = obsolete t32_kevent */
	"mlockall",			/* 271 = mlockall */
	"munlockall",			/* 272 = munlockall */
	"#273 (unimplemented sys_getpeereid)",		/* 273 = unimplemented sys_getpeereid */
	"#274 (unimplemented sys_extattrctl)",		/* 274 = unimplemented sys_extattrctl */
	"#275 (unimplemented sys_extattr_set_file)",		/* 275 = unimplemented sys_extattr_set_file */
	"#276 (unimplemented sys_extattr_get_file)",		/* 276 = unimplemented sys_extattr_get_file */
	"#277 (unimplemented sys_extattr_delete_file)",		/* 277 = unimplemented sys_extattr_delete_file */
	"#278 (unimplemented sys_extattr_set_fd)",		/* 278 = unimplemented sys_extattr_set_fd */
	"#279 (unimplemented sys_extattr_get_fd)",		/* 279 = unimplemented sys_extattr_get_fd */
	"#280 (unimplemented sys_extattr_delete_fd)",		/* 280 = unimplemented sys_extattr_delete_fd */
	"getresuid",			/* 281 = getresuid */
	"setresuid",			/* 282 = setresuid */
	"getresgid",			/* 283 = getresgid */
	"setresgid",			/* 284 = setresgid */
	"#285 (obsolete sys_omquery)",		/* 285 = obsolete sys_omquery */
	"mquery",			/* 286 = mquery */
	"closefrom",			/* 287 = closefrom */
	"sigaltstack",			/* 288 = sigaltstack */
#ifdef SYSVSHM
	"shmget",			/* 289 = shmget */
#else
	"#289 (unimplemented shmget)",		/* 289 = unimplemented shmget */
#endif
#ifdef SYSVSEM
	"semop",			/* 290 = semop */
#else
	"#290 (unimplemented semop)",		/* 290 = unimplemented semop */
#endif
	"#291 (obsolete t32_stat)",		/* 291 = obsolete t32_stat */
	"#292 (obsolete t32_fstat)",		/* 292 = obsolete t32_fstat */
	"#293 (obsolete t32_lstat)",		/* 293 = obsolete t32_lstat */
	"fhstat",			/* 294 = fhstat */
#ifdef SYSVSEM
	"__semctl",			/* 295 = __semctl */
#else
	"#295 (unimplemented)",		/* 295 = unimplemented */
#endif
#ifdef SYSVSHM
	"shmctl",			/* 296 = shmctl */
#else
	"#296 (unimplemented)",		/* 296 = unimplemented */
#endif
#ifdef SYSVMSG
	"msgctl",			/* 297 = msgctl */
#else
	"#297 (unimplemented)",		/* 297 = unimplemented */
#endif
	"sched_yield",			/* 298 = sched_yield */
	"getthrid",			/* 299 = getthrid */
	"#300 (obsolete t32___thrsleep)",		/* 300 = obsolete t32___thrsleep */
	"__thrwakeup",			/* 301 = __thrwakeup */
	"__threxit",			/* 302 = __threxit */
	"__thrsigdivert",			/* 303 = __thrsigdivert */
	"__getcwd",			/* 304 = __getcwd */
	"adjfreq",			/* 305 = adjfreq */
	"#306 (obsolete getfsstat53)",		/* 306 = obsolete getfsstat53 */
	"#307 (obsolete statfs53)",		/* 307 = obsolete statfs53 */
	"#308 (obsolete fstatfs53)",		/* 308 = obsolete fstatfs53 */
	"#309 (obsolete fhstatfs53)",		/* 309 = obsolete fhstatfs53 */
	"setrtable",			/* 310 = setrtable */
	"getrtable",			/* 311 = getrtable */
	"#312 (obsolete t32_getdirentries)",		/* 312 = obsolete t32_getdirentries */
	"faccessat",			/* 313 = faccessat */
	"fchmodat",			/* 314 = fchmodat */
	"fchownat",			/* 315 = fchownat */
	"#316 (obsolete t32_fstatat)",		/* 316 = obsolete t32_fstatat */
	"linkat",			/* 317 = linkat */
	"mkdirat",			/* 318 = mkdirat */
	"mkfifoat",			/* 319 = mkfifoat */
	"mknodat",			/* 320 = mknodat */
	"openat",			/* 321 = openat */
	"readlinkat",			/* 322 = readlinkat */
	"renameat",			/* 323 = renameat */
	"symlinkat",			/* 324 = symlinkat */
	"unlinkat",			/* 325 = unlinkat */
	"#326 (obsolete t32_utimensat)",		/* 326 = obsolete t32_utimensat */
	"#327 (obsolete t32_futimens)",		/* 327 = obsolete t32_futimens */
	"#328 (obsolete __tfork51)",		/* 328 = obsolete __tfork51 */
	"__set_tcb",			/* 329 = __set_tcb */
	"__get_tcb",			/* 330 = __get_tcb */
};

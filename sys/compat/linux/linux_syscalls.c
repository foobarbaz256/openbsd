/*	$OpenBSD: linux_syscalls.c,v 1.46 2003/09/04 02:57:44 tedu Exp $	*/

/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.45 2003/09/04 02:57:22 tedu Exp 
 */

char *linux_syscallnames[] = {
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"open",			/* 5 = open */
	"close",			/* 6 = close */
	"waitpid",			/* 7 = waitpid */
	"creat",			/* 8 = creat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"execve",			/* 11 = execve */
	"chdir",			/* 12 = chdir */
	"time",			/* 13 = time */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"lchown16",			/* 16 = lchown16 */
	"break",			/* 17 = break */
	"ostat",			/* 18 = ostat */
	"lseek",			/* 19 = lseek */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"umount",			/* 22 = umount */
	"linux_setuid16",			/* 23 = linux_setuid16 */
	"linux_getuid16",			/* 24 = linux_getuid16 */
	"stime",			/* 25 = stime */
#ifdef PTRACE
	"ptrace",			/* 26 = ptrace */
#else
	"#26 (unimplemented ptrace)",		/* 26 = unimplemented ptrace */
#endif
	"alarm",			/* 27 = alarm */
	"ofstat",			/* 28 = ofstat */
	"pause",			/* 29 = pause */
	"utime",			/* 30 = utime */
	"stty",			/* 31 = stty */
	"gtty",			/* 32 = gtty */
	"access",			/* 33 = access */
	"nice",			/* 34 = nice */
	"ftime",			/* 35 = ftime */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"rename",			/* 38 = rename */
	"mkdir",			/* 39 = mkdir */
	"rmdir",			/* 40 = rmdir */
	"dup",			/* 41 = dup */
	"pipe",			/* 42 = pipe */
	"times",			/* 43 = times */
	"prof",			/* 44 = prof */
	"brk",			/* 45 = brk */
	"linux_setgid16",			/* 46 = linux_setgid16 */
	"linux_getgid16",			/* 47 = linux_getgid16 */
	"signal",			/* 48 = signal */
	"linux_geteuid16",			/* 49 = linux_geteuid16 */
	"linux_getegid16",			/* 50 = linux_getegid16 */
	"acct",			/* 51 = acct */
	"phys",			/* 52 = phys */
	"lock",			/* 53 = lock */
	"ioctl",			/* 54 = ioctl */
	"fcntl",			/* 55 = fcntl */
	"mpx",			/* 56 = mpx */
	"setpgid",			/* 57 = setpgid */
	"ulimit",			/* 58 = ulimit */
	"oldolduname",			/* 59 = oldolduname */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"ustat",			/* 62 = ustat */
	"dup2",			/* 63 = dup2 */
	"getppid",			/* 64 = getppid */
	"getpgrp",			/* 65 = getpgrp */
	"setsid",			/* 66 = setsid */
	"sigaction",			/* 67 = sigaction */
	"siggetmask",			/* 68 = siggetmask */
	"sigsetmask",			/* 69 = sigsetmask */
	"setreuid16",			/* 70 = setreuid16 */
	"setregid16",			/* 71 = setregid16 */
	"sigsuspend",			/* 72 = sigsuspend */
	"sigpending",			/* 73 = sigpending */
	"sethostname",			/* 74 = sethostname */
	"setrlimit",			/* 75 = setrlimit */
	"getrlimit",			/* 76 = getrlimit */
	"getrusage",			/* 77 = getrusage */
	"gettimeofday",			/* 78 = gettimeofday */
	"settimeofday",			/* 79 = settimeofday */
	"linux_getgroups",			/* 80 = linux_getgroups */
	"linux_setgroups",			/* 81 = linux_setgroups */
	"oldselect",			/* 82 = oldselect */
	"symlink",			/* 83 = symlink */
	"olstat",			/* 84 = olstat */
	"readlink",			/* 85 = readlink */
	"uselib",			/* 86 = uselib */
	"swapon",			/* 87 = swapon */
	"reboot",			/* 88 = reboot */
	"readdir",			/* 89 = readdir */
	"mmap",			/* 90 = mmap */
	"munmap",			/* 91 = munmap */
	"truncate",			/* 92 = truncate */
	"ftruncate",			/* 93 = ftruncate */
	"fchmod",			/* 94 = fchmod */
	"fchown16",			/* 95 = fchown16 */
	"getpriority",			/* 96 = getpriority */
	"setpriority",			/* 97 = setpriority */
	"profil",			/* 98 = profil */
	"statfs",			/* 99 = statfs */
	"fstatfs",			/* 100 = fstatfs */
#ifdef __i386__
	"ioperm",			/* 101 = ioperm */
#else
	"ioperm",			/* 101 = ioperm */
#endif
	"socketcall",			/* 102 = socketcall */
	"klog",			/* 103 = klog */
	"setitimer",			/* 104 = setitimer */
	"getitimer",			/* 105 = getitimer */
	"stat",			/* 106 = stat */
	"lstat",			/* 107 = lstat */
	"fstat",			/* 108 = fstat */
	"olduname",			/* 109 = olduname */
#ifdef __i386__
	"iopl",			/* 110 = iopl */
#else
	"iopl",			/* 110 = iopl */
#endif
	"vhangup",			/* 111 = vhangup */
	"idle",			/* 112 = idle */
	"vm86old",			/* 113 = vm86old */
	"wait4",			/* 114 = wait4 */
	"swapoff",			/* 115 = swapoff */
	"sysinfo",			/* 116 = sysinfo */
	"ipc",			/* 117 = ipc */
	"fsync",			/* 118 = fsync */
	"sigreturn",			/* 119 = sigreturn */
	"clone",			/* 120 = clone */
	"setdomainname",			/* 121 = setdomainname */
	"uname",			/* 122 = uname */
#ifdef __i386__
	"modify_ldt",			/* 123 = modify_ldt */
#else
	"modify_ldt",			/* 123 = modify_ldt */
#endif
	"adjtimex",			/* 124 = adjtimex */
	"mprotect",			/* 125 = mprotect */
	"sigprocmask",			/* 126 = sigprocmask */
	"create_module",			/* 127 = create_module */
	"init_module",			/* 128 = init_module */
	"delete_module",			/* 129 = delete_module */
	"get_kernel_syms",			/* 130 = get_kernel_syms */
	"quotactl",			/* 131 = quotactl */
	"getpgid",			/* 132 = getpgid */
	"fchdir",			/* 133 = fchdir */
	"bdflush",			/* 134 = bdflush */
	"sysfs",			/* 135 = sysfs */
	"personality",			/* 136 = personality */
	"afs_syscall",			/* 137 = afs_syscall */
	"linux_setfsuid16",			/* 138 = linux_setfsuid16 */
	"linux_getfsuid16",			/* 139 = linux_getfsuid16 */
	"llseek",			/* 140 = llseek */
	"getdents",			/* 141 = getdents */
	"select",			/* 142 = select */
	"flock",			/* 143 = flock */
	"msync",			/* 144 = msync */
	"readv",			/* 145 = readv */
	"writev",			/* 146 = writev */
	"getsid",			/* 147 = getsid */
	"fdatasync",			/* 148 = fdatasync */
	"__sysctl",			/* 149 = __sysctl */
	"mlock",			/* 150 = mlock */
	"munlock",			/* 151 = munlock */
	"mlockall",			/* 152 = mlockall */
	"munlockall",			/* 153 = munlockall */
	"sched_setparam",			/* 154 = sched_setparam */
	"sched_getparam",			/* 155 = sched_getparam */
	"sched_setscheduler",			/* 156 = sched_setscheduler */
	"sched_getscheduler",			/* 157 = sched_getscheduler */
	"sched_yield",			/* 158 = sched_yield */
	"sched_get_priority_max",			/* 159 = sched_get_priority_max */
	"sched_get_priority_min",			/* 160 = sched_get_priority_min */
	"sched_rr_get_interval",			/* 161 = sched_rr_get_interval */
	"nanosleep",			/* 162 = nanosleep */
	"mremap",			/* 163 = mremap */
	"setresuid16",			/* 164 = setresuid16 */
	"getresuid16",			/* 165 = getresuid16 */
	"vm86",			/* 166 = vm86 */
	"query_module",			/* 167 = query_module */
	"poll",			/* 168 = poll */
	"nfsservctl",			/* 169 = nfsservctl */
	"setresgid16",			/* 170 = setresgid16 */
	"getresgid16",			/* 171 = getresgid16 */
	"prctl",			/* 172 = prctl */
	"rt_sigreturn",			/* 173 = rt_sigreturn */
	"rt_sigaction",			/* 174 = rt_sigaction */
	"rt_sigprocmask",			/* 175 = rt_sigprocmask */
	"rt_sigpending",			/* 176 = rt_sigpending */
	"rt_sigtimedwait",			/* 177 = rt_sigtimedwait */
	"rt_queueinfo",			/* 178 = rt_queueinfo */
	"rt_sigsuspend",			/* 179 = rt_sigsuspend */
	"pread",			/* 180 = pread */
	"pwrite",			/* 181 = pwrite */
	"chown16",			/* 182 = chown16 */
	"getcwd",			/* 183 = getcwd */
	"capget",			/* 184 = capget */
	"capset",			/* 185 = capset */
	"sigaltstack",			/* 186 = sigaltstack */
	"sendfile",			/* 187 = sendfile */
	"getpmsg",			/* 188 = getpmsg */
	"putpmsg",			/* 189 = putpmsg */
	"vfork",			/* 190 = vfork */
	"ugetrlimit",			/* 191 = ugetrlimit */
	"mmap2",			/* 192 = mmap2 */
	"truncate64",			/* 193 = truncate64 */
	"ftruncate64",			/* 194 = ftruncate64 */
	"stat64",			/* 195 = stat64 */
	"lstat64",			/* 196 = lstat64 */
	"fstat64",			/* 197 = fstat64 */
	"lchown",			/* 198 = lchown */
	"getuid",			/* 199 = getuid */
	"getgid",			/* 200 = getgid */
	"geteuid",			/* 201 = geteuid */
	"getegid",			/* 202 = getegid */
	"setreuid",			/* 203 = setreuid */
	"setregid",			/* 204 = setregid */
	"getgroups",			/* 205 = getgroups */
	"setgroups",			/* 206 = setgroups */
	"fchown",			/* 207 = fchown */
	"setresuid",			/* 208 = setresuid */
	"getresuid",			/* 209 = getresuid */
	"setresgid",			/* 210 = setresgid */
	"getresgid",			/* 211 = getresgid */
	"chown",			/* 212 = chown */
	"setuid",			/* 213 = setuid */
	"setgid",			/* 214 = setgid */
	"setfsuid",			/* 215 = setfsuid */
	"setfsgid",			/* 216 = setfsgid */
	"pivot_root",			/* 217 = pivot_root */
	"mincore",			/* 218 = mincore */
	"madvise",			/* 219 = madvise */
	"getdents64",			/* 220 = getdents64 */
	"fcntl64",			/* 221 = fcntl64 */
	"#222 (unimplemented)",		/* 222 = unimplemented */
	"#223 (unimplemented)",		/* 223 = unimplemented */
	"#224 (unimplemented linux_sys_gettid)",		/* 224 = unimplemented linux_sys_gettid */
	"#225 (unimplemented linux_sys_readahead)",		/* 225 = unimplemented linux_sys_readahead */
	"setxattr",			/* 226 = setxattr */
	"lsetxattr",			/* 227 = lsetxattr */
	"fsetxattr",			/* 228 = fsetxattr */
	"getxattr",			/* 229 = getxattr */
	"lgetxattr",			/* 230 = lgetxattr */
	"fgetxattr",			/* 231 = fgetxattr */
	"listxattr",			/* 232 = listxattr */
	"llistxattr",			/* 233 = llistxattr */
	"flistxattr",			/* 234 = flistxattr */
	"removexattr",			/* 235 = removexattr */
	"lremovexattr",			/* 236 = lremovexattr */
	"fremovexattr",			/* 237 = fremovexattr */
	"#238 (unimplemented linux_sys_tkill)",		/* 238 = unimplemented linux_sys_tkill */
	"#239 (unimplemented linux_sys_sendfile64)",		/* 239 = unimplemented linux_sys_sendfile64 */
	"#240 (unimplemented linux_sys_futex)",		/* 240 = unimplemented linux_sys_futex */
	"#241 (unimplemented linux_sys_sched_setaffinity)",		/* 241 = unimplemented linux_sys_sched_setaffinity */
	"#242 (unimplemented linux_sys_sched_getaffinity)",		/* 242 = unimplemented linux_sys_sched_getaffinity */
	"#243 (unimplemented linux_sys_set_thread_area)",		/* 243 = unimplemented linux_sys_set_thread_area */
	"#244 (unimplemented linux_sys_get_thread_area)",		/* 244 = unimplemented linux_sys_get_thread_area */
	"#245 (unimplemented linux_sys_io_setup)",		/* 245 = unimplemented linux_sys_io_setup */
	"#246 (unimplemented linux_sys_io_destroy)",		/* 246 = unimplemented linux_sys_io_destroy */
	"#247 (unimplemented linux_sys_io_getevents)",		/* 247 = unimplemented linux_sys_io_getevents */
	"#248 (unimplemented linux_sys_io_submit)",		/* 248 = unimplemented linux_sys_io_submit */
	"#249 (unimplemented linux_sys_io_cancel)",		/* 249 = unimplemented linux_sys_io_cancel */
	"fadvise64",			/* 250 = fadvise64 */
	"#251 (unimplemented)",		/* 251 = unimplemented */
	"linux_exit_group",			/* 252 = linux_exit_group */
	"#253 (unimplemented linux_sys_lookup_dcookie)",		/* 253 = unimplemented linux_sys_lookup_dcookie */
	"#254 (unimplemented linux_sys_epoll_create)",		/* 254 = unimplemented linux_sys_epoll_create */
	"#255 (unimplemented linux_sys_epoll_ctl)",		/* 255 = unimplemented linux_sys_epoll_ctl */
	"#256 (unimplemented linux_sys_epoll_wait)",		/* 256 = unimplemented linux_sys_epoll_wait */
	"#257 (unimplemented linux_sys_remap_file_pages)",		/* 257 = unimplemented linux_sys_remap_file_pages */
	"#258 (unimplemented linux_sys_set_tid_address)",		/* 258 = unimplemented linux_sys_set_tid_address */
	"#259 (unimplemented linux_sys_timer_create)",		/* 259 = unimplemented linux_sys_timer_create */
	"#260 (unimplemented linux_sys_timer_settime)",		/* 260 = unimplemented linux_sys_timer_settime */
	"#261 (unimplemented linux_sys_timer_gettime)",		/* 261 = unimplemented linux_sys_timer_gettime */
	"#262 (unimplemented linux_sys_timer_getoverrun)",		/* 262 = unimplemented linux_sys_timer_getoverrun */
	"#263 (unimplemented linux_sys_timer_delete)",		/* 263 = unimplemented linux_sys_timer_delete */
	"#264 (unimplemented linux_sys_clock_settime)",		/* 264 = unimplemented linux_sys_clock_settime */
	"#265 (unimplemented linux_sys_clock_gettime)",		/* 265 = unimplemented linux_sys_clock_gettime */
	"#266 (unimplemented linux_sys_clock_getres)",		/* 266 = unimplemented linux_sys_clock_getres */
	"#267 (unimplemented linux_sys_clock_nanosleep)",		/* 267 = unimplemented linux_sys_clock_nanosleep */
};

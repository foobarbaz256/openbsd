/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.4 1997/01/26 23:05:12 downsj Exp 
 */

#define	OSF1_SYS_syscall	0
#define	OSF1_SYS_exit	1
#define	OSF1_SYS_fork	2
#define	OSF1_SYS_read	3
#define	OSF1_SYS_write	4
#define	OSF1_SYS_close	6
#define	OSF1_SYS_wait4	7
#define	OSF1_SYS_link	9
#define	OSF1_SYS_unlink	10
#define	OSF1_SYS_chdir	12
#define	OSF1_SYS_fchdir	13
#define	OSF1_SYS_mknod	14
#define	OSF1_SYS_chmod	15
#define	OSF1_SYS_chown	16
#define	OSF1_SYS_obreak	17
#define	OSF1_SYS_getfsstat	18
#define	OSF1_SYS_lseek	19
#define	OSF1_SYS_getpid	20
#define	OSF1_SYS_mount	21
#define	OSF1_SYS_unmount	22
#define	OSF1_SYS_setuid	23
#define	OSF1_SYS_getuid	24
#define	OSF1_SYS_access	33
#define	OSF1_SYS_sync	36
#define	OSF1_SYS_kill	37
#define	OSF1_SYS_setpgid	39
#define	OSF1_SYS_dup	41
#define	OSF1_SYS_pipe	42
#define	OSF1_SYS_open	45
				/* 46 is obsolete sigaction */
#define	OSF1_SYS_getgid	47
#define	OSF1_SYS_sigprocmask	48
#define	OSF1_SYS_getlogin	49
#define	OSF1_SYS_setlogin	50
#define	OSF1_SYS_acct	51
#define	OSF1_SYS_ioctl	54
#define	OSF1_SYS_reboot	55
#define	OSF1_SYS_revoke	56
#define	OSF1_SYS_symlink	57
#define	OSF1_SYS_readlink	58
#define	OSF1_SYS_execve	59
#define	OSF1_SYS_umask	60
#define	OSF1_SYS_chroot	61
#define	OSF1_SYS_getpgrp	63
#define	OSF1_SYS_getpagesize	64
#define	OSF1_SYS_vfork	66
#define	OSF1_SYS_stat	67
#define	OSF1_SYS_lstat	68
#define	OSF1_SYS_mmap	71
#define	OSF1_SYS_munmap	73
#define	OSF1_SYS_madvise	75
#define	OSF1_SYS_getgroups	79
#define	OSF1_SYS_setgroups	80
#define	OSF1_SYS_setpgrp	82
#define	OSF1_SYS_setitimer	83
#define	OSF1_SYS_gethostname	87
#define	OSF1_SYS_sethostname	88
#define	OSF1_SYS_getdtablesize	89
#define	OSF1_SYS_dup2	90
#define	OSF1_SYS_fstat	91
#define	OSF1_SYS_fcntl	92
#define	OSF1_SYS_select	93
#define	OSF1_SYS_poll	94
#define	OSF1_SYS_fsync	95
#define	OSF1_SYS_setpriority	96
#define	OSF1_SYS_socket	97
#define	OSF1_SYS_connect	98
#define	OSF1_SYS_getpriority	100
#define	OSF1_SYS_send	101
#define	OSF1_SYS_recv	102
#define	OSF1_SYS_sigreturn	103
#define	OSF1_SYS_bind	104
#define	OSF1_SYS_setsockopt	105
#define	OSF1_SYS_sigsuspend	111
#define	OSF1_SYS_sigstack	112
#define	OSF1_SYS_gettimeofday	116
#define	OSF1_SYS_getrusage	117
#define	OSF1_SYS_getsockopt	118
#define	OSF1_SYS_readv	120
#define	OSF1_SYS_writev	121
#define	OSF1_SYS_settimeofday	122
#define	OSF1_SYS_fchown	123
#define	OSF1_SYS_fchmod	124
#define	OSF1_SYS_recvfrom	125
#define	OSF1_SYS_rename	128
#define	OSF1_SYS_truncate	129
#define	OSF1_SYS_ftruncate	130
#define	OSF1_SYS_setgid	132
#define	OSF1_SYS_sendto	133
#define	OSF1_SYS_shutdown	134
#define	OSF1_SYS_mkdir	136
#define	OSF1_SYS_rmdir	137
#define	OSF1_SYS_utimes	138
				/* 139 is obsolete 4.2 sigreturn */
#define	OSF1_SYS_gethostid	142
#define	OSF1_SYS_sethostid	143
#define	OSF1_SYS_getrlimit	144
#define	OSF1_SYS_setrlimit	145
#define	OSF1_SYS_setsid	147
#define	OSF1_SYS_quota	149
#define	OSF1_SYS_sigaction	156
#define	OSF1_SYS_getdirentries	159
#define	OSF1_SYS_statfs	160
#define	OSF1_SYS_fstatfs	161
#define	OSF1_SYS_lchown	208
#define	OSF1_SYS_getsid	234
#define	OSF1_SYS_sigaltstack	235
#define	OSF1_SYS_usleep_thread	251
#define	OSF1_SYS_setsysinfo	257
#define	OSF1_SYS_MAXSYSCALL	261

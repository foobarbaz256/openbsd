/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.3 1996/08/02 20:20:31 niklas Exp 
 */

#define	syscallarg(x)	union { x datum; register_t pad; }

struct sunos_sys_open_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
	syscallarg(int) mode;
};

struct sunos_sys_wait4_args {
	syscallarg(int) pid;
	syscallarg(int *) status;
	syscallarg(int) options;
	syscallarg(struct rusage *) rusage;
};

struct sunos_sys_creat_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
};

struct sunos_sys_execv_args {
	syscallarg(char *) path;
	syscallarg(char **) argp;
};

struct sunos_sys_mknod_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
	syscallarg(int) dev;
};

struct sunos_sys_ptrace_args {
	syscallarg(int) req;
	syscallarg(pid_t) pid;
	syscallarg(caddr_t) addr;
	syscallarg(int) data;
	syscallarg(char *) addr2;
};

struct sunos_sys_access_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct sunos_sys_stat_args {
	syscallarg(char *) path;
	syscallarg(struct ostat *) ub;
};

struct sunos_sys_lstat_args {
	syscallarg(char *) path;
	syscallarg(struct ostat *) ub;
};

struct sunos_sys_mctl_args {
	syscallarg(caddr_t) addr;
	syscallarg(int) len;
	syscallarg(int) func;
	syscallarg(void *) arg;
};

struct sunos_sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(u_long) com;
	syscallarg(caddr_t) data;
};

struct sunos_sys_reboot_args {
	syscallarg(int) howto;
	syscallarg(char *) bootstr;
};

struct sunos_sys_omsync_args {
	syscallarg(caddr_t) addr;
	syscallarg(size_t) len;
	syscallarg(int) flags;
};

struct sunos_sys_mmap_args {
	syscallarg(caddr_t) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
	syscallarg(int) flags;
	syscallarg(int) fd;
	syscallarg(long) pos;
};

struct sunos_sys_setpgrp_args {
	syscallarg(int) pid;
	syscallarg(int) pgid;
};

struct sunos_sys_fcntl_args {
	syscallarg(int) fd;
	syscallarg(int) cmd;
	syscallarg(void *) arg;
};

struct sunos_sys_setsockopt_args {
	syscallarg(int) s;
	syscallarg(int) level;
	syscallarg(int) name;
	syscallarg(caddr_t) val;
	syscallarg(int) valsize;
};

struct sunos_sys_sigvec_args {
	syscallarg(int) signum;
	syscallarg(struct sigvec *) nsv;
	syscallarg(struct sigvec *) osv;
};

struct sunos_sys_sigreturn_args {
	syscallarg(struct sigcontext *) sigcntxp;
};

struct sunos_sys_getrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct orlimit *) rlp;
};

struct sunos_sys_setrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct orlimit *) rlp;
};

struct sys_poll_args {
	syscallarg(struct pollfd *) fds;
	syscallarg(unsigned long) nfds;
	syscallarg(int) timeout;
};

struct sunos_sys_nfssvc_args {
	syscallarg(int) fd;
};

struct sunos_sys_statfs_args {
	syscallarg(char *) path;
	syscallarg(struct sunos_statfs *) buf;
};

struct sunos_sys_fstatfs_args {
	syscallarg(int) fd;
	syscallarg(struct sunos_statfs *) buf;
};

struct sunos_sys_unmount_args {
	syscallarg(char *) path;
};

struct sunos_sys_quotactl_args {
	syscallarg(int) cmd;
	syscallarg(char *) special;
	syscallarg(int) uid;
	syscallarg(caddr_t) addr;
};

struct sunos_sys_exportfs_args {
	syscallarg(char *) path;
	syscallarg(char *) ex;
};

struct sunos_sys_mount_args {
	syscallarg(char *) type;
	syscallarg(char *) dir;
	syscallarg(int) flags;
	syscallarg(caddr_t) data;
};

struct sunos_sys_ustat_args {
	syscallarg(int) dev;
	syscallarg(struct sunos_ustat *) buf;
};

struct sunos_sys_auditsys_args {
	syscallarg(char *) record;
};

struct sunos_sys_getdents_args {
	syscallarg(int) fd;
	syscallarg(char *) buf;
	syscallarg(int) nbytes;
};

struct sunos_sys_fchroot_args {
	syscallarg(int) fd;
};

struct sunos_sys_sigpending_args {
	syscallarg(int *) mask;
};

struct sunos_sys_sysconf_args {
	syscallarg(int) name;
};

struct sunos_sys_uname_args {
	syscallarg(struct sunos_utsname *) name;
};

/*
 * System call prototypes.
 */

int	sys_nosys	__P((struct proc *, void *, register_t *));
int	sys_exit	__P((struct proc *, void *, register_t *));
int	sys_fork	__P((struct proc *, void *, register_t *));
int	sys_read	__P((struct proc *, void *, register_t *));
int	sys_write	__P((struct proc *, void *, register_t *));
int	sunos_sys_open	__P((struct proc *, void *, register_t *));
int	sys_close	__P((struct proc *, void *, register_t *));
int	sunos_sys_wait4	__P((struct proc *, void *, register_t *));
int	sunos_sys_creat	__P((struct proc *, void *, register_t *));
int	sys_link	__P((struct proc *, void *, register_t *));
int	sys_unlink	__P((struct proc *, void *, register_t *));
int	sunos_sys_execv	__P((struct proc *, void *, register_t *));
int	sys_chdir	__P((struct proc *, void *, register_t *));
int	sunos_sys_mknod	__P((struct proc *, void *, register_t *));
int	sys_chmod	__P((struct proc *, void *, register_t *));
int	sys_chown	__P((struct proc *, void *, register_t *));
int	sys_obreak	__P((struct proc *, void *, register_t *));
int	compat_43_sys_lseek	__P((struct proc *, void *, register_t *));
int	sys_getpid	__P((struct proc *, void *, register_t *));
int	sys_setuid	__P((struct proc *, void *, register_t *));
int	sys_getuid	__P((struct proc *, void *, register_t *));
int	sunos_sys_ptrace	__P((struct proc *, void *, register_t *));
int	sunos_sys_access	__P((struct proc *, void *, register_t *));
int	sys_sync	__P((struct proc *, void *, register_t *));
int	sys_kill	__P((struct proc *, void *, register_t *));
int	sunos_sys_stat	__P((struct proc *, void *, register_t *));
int	sunos_sys_lstat	__P((struct proc *, void *, register_t *));
int	sys_dup	__P((struct proc *, void *, register_t *));
int	sys_pipe	__P((struct proc *, void *, register_t *));
int	sys_profil	__P((struct proc *, void *, register_t *));
int	sys_setgid	__P((struct proc *, void *, register_t *));
int	sys_getgid	__P((struct proc *, void *, register_t *));
int	sys_acct	__P((struct proc *, void *, register_t *));
int	sunos_sys_mctl	__P((struct proc *, void *, register_t *));
int	sunos_sys_ioctl	__P((struct proc *, void *, register_t *));
int	sunos_sys_reboot	__P((struct proc *, void *, register_t *));
int	sys_symlink	__P((struct proc *, void *, register_t *));
int	sys_readlink	__P((struct proc *, void *, register_t *));
int	sys_execve	__P((struct proc *, void *, register_t *));
int	sys_umask	__P((struct proc *, void *, register_t *));
int	sys_chroot	__P((struct proc *, void *, register_t *));
int	compat_43_sys_fstat	__P((struct proc *, void *, register_t *));
int	compat_43_sys_getpagesize	__P((struct proc *, void *, register_t *));
int	sunos_sys_omsync	__P((struct proc *, void *, register_t *));
int	sys_vfork	__P((struct proc *, void *, register_t *));
int	sys_sbrk	__P((struct proc *, void *, register_t *));
int	sys_sstk	__P((struct proc *, void *, register_t *));
int	sunos_sys_mmap	__P((struct proc *, void *, register_t *));
int	sys_ovadvise	__P((struct proc *, void *, register_t *));
int	sys_munmap	__P((struct proc *, void *, register_t *));
int	sys_mprotect	__P((struct proc *, void *, register_t *));
int	sys_madvise	__P((struct proc *, void *, register_t *));
int	sunos_sys_vhangup	__P((struct proc *, void *, register_t *));
int	sys_mincore	__P((struct proc *, void *, register_t *));
int	sys_getgroups	__P((struct proc *, void *, register_t *));
int	sys_setgroups	__P((struct proc *, void *, register_t *));
int	sys_getpgrp	__P((struct proc *, void *, register_t *));
int	sunos_sys_setpgrp	__P((struct proc *, void *, register_t *));
int	sys_setitimer	__P((struct proc *, void *, register_t *));
int	sys_swapon	__P((struct proc *, void *, register_t *));
int	sys_getitimer	__P((struct proc *, void *, register_t *));
int	compat_43_sys_gethostname	__P((struct proc *, void *, register_t *));
int	compat_43_sys_sethostname	__P((struct proc *, void *, register_t *));
int	compat_43_sys_getdtablesize	__P((struct proc *, void *, register_t *));
int	sys_dup2	__P((struct proc *, void *, register_t *));
int	sunos_sys_fcntl	__P((struct proc *, void *, register_t *));
int	sys_select	__P((struct proc *, void *, register_t *));
int	sys_fsync	__P((struct proc *, void *, register_t *));
int	sys_setpriority	__P((struct proc *, void *, register_t *));
int	sys_socket	__P((struct proc *, void *, register_t *));
int	sys_connect	__P((struct proc *, void *, register_t *));
int	compat_43_sys_accept	__P((struct proc *, void *, register_t *));
int	sys_getpriority	__P((struct proc *, void *, register_t *));
int	compat_43_sys_send	__P((struct proc *, void *, register_t *));
int	compat_43_sys_recv	__P((struct proc *, void *, register_t *));
int	sys_bind	__P((struct proc *, void *, register_t *));
int	sunos_sys_setsockopt	__P((struct proc *, void *, register_t *));
int	sys_listen	__P((struct proc *, void *, register_t *));
int	sunos_sys_sigvec	__P((struct proc *, void *, register_t *));
int	compat_43_sys_sigblock	__P((struct proc *, void *, register_t *));
int	compat_43_sys_sigsetmask	__P((struct proc *, void *, register_t *));
int	sys_sigsuspend	__P((struct proc *, void *, register_t *));
int	compat_43_sys_sigstack	__P((struct proc *, void *, register_t *));
int	compat_43_sys_recvmsg	__P((struct proc *, void *, register_t *));
int	compat_43_sys_sendmsg	__P((struct proc *, void *, register_t *));
int	sys_gettimeofday	__P((struct proc *, void *, register_t *));
int	sys_getrusage	__P((struct proc *, void *, register_t *));
int	sys_getsockopt	__P((struct proc *, void *, register_t *));
int	sys_readv	__P((struct proc *, void *, register_t *));
int	sys_writev	__P((struct proc *, void *, register_t *));
int	sys_settimeofday	__P((struct proc *, void *, register_t *));
int	sys_fchown	__P((struct proc *, void *, register_t *));
int	sys_fchmod	__P((struct proc *, void *, register_t *));
int	compat_43_sys_recvfrom	__P((struct proc *, void *, register_t *));
int	compat_43_sys_setreuid	__P((struct proc *, void *, register_t *));
int	compat_43_sys_setregid	__P((struct proc *, void *, register_t *));
int	sys_rename	__P((struct proc *, void *, register_t *));
int	compat_43_sys_truncate	__P((struct proc *, void *, register_t *));
int	compat_43_sys_ftruncate	__P((struct proc *, void *, register_t *));
int	sys_flock	__P((struct proc *, void *, register_t *));
int	sys_sendto	__P((struct proc *, void *, register_t *));
int	sys_shutdown	__P((struct proc *, void *, register_t *));
int	sys_socketpair	__P((struct proc *, void *, register_t *));
int	sys_mkdir	__P((struct proc *, void *, register_t *));
int	sys_rmdir	__P((struct proc *, void *, register_t *));
int	sys_utimes	__P((struct proc *, void *, register_t *));
int	sunos_sys_sigreturn	__P((struct proc *, void *, register_t *));
int	sys_adjtime	__P((struct proc *, void *, register_t *));
int	compat_43_sys_getpeername	__P((struct proc *, void *, register_t *));
int	compat_43_sys_gethostid	__P((struct proc *, void *, register_t *));
int	sunos_sys_getrlimit	__P((struct proc *, void *, register_t *));
int	sunos_sys_setrlimit	__P((struct proc *, void *, register_t *));
int	compat_43_sys_killpg	__P((struct proc *, void *, register_t *));
int	compat_43_sys_getsockname	__P((struct proc *, void *, register_t *));
int	sys_poll	__P((struct proc *, void *, register_t *));
#ifdef NFSSERVER
int	sunos_sys_nfssvc	__P((struct proc *, void *, register_t *));
#else
#endif
int	sys_getdirentries	__P((struct proc *, void *, register_t *));
int	sunos_sys_statfs	__P((struct proc *, void *, register_t *));
int	sunos_sys_fstatfs	__P((struct proc *, void *, register_t *));
int	sunos_sys_unmount	__P((struct proc *, void *, register_t *));
#ifdef NFSCLIENT
int	async_daemon	__P((struct proc *, void *, register_t *));
int	sys_getfh	__P((struct proc *, void *, register_t *));
#else
#endif
int	compat_09_sys_getdomainname	__P((struct proc *, void *, register_t *));
int	compat_09_sys_setdomainname	__P((struct proc *, void *, register_t *));
int	sunos_sys_quotactl	__P((struct proc *, void *, register_t *));
int	sunos_sys_exportfs	__P((struct proc *, void *, register_t *));
int	sunos_sys_mount	__P((struct proc *, void *, register_t *));
int	sunos_sys_ustat	__P((struct proc *, void *, register_t *));
#ifdef SYSVSEM
int	compat_10_sys_semsys	__P((struct proc *, void *, register_t *));
#else
#endif
#ifdef SYSVMSG
int	compat_10_sys_msgsys	__P((struct proc *, void *, register_t *));
#else
#endif
#ifdef SYSVSHM
int	compat_10_sys_shmsys	__P((struct proc *, void *, register_t *));
#else
#endif
int	sunos_sys_auditsys	__P((struct proc *, void *, register_t *));
int	sunos_sys_getdents	__P((struct proc *, void *, register_t *));
int	sys_setsid	__P((struct proc *, void *, register_t *));
int	sys_fchdir	__P((struct proc *, void *, register_t *));
int	sunos_sys_fchroot	__P((struct proc *, void *, register_t *));
int	sunos_sys_sigpending	__P((struct proc *, void *, register_t *));
int	sys_setpgid	__P((struct proc *, void *, register_t *));
int	sys_pathconf	__P((struct proc *, void *, register_t *));
int	sys_fpathconf	__P((struct proc *, void *, register_t *));
int	sunos_sys_sysconf	__P((struct proc *, void *, register_t *));
int	sunos_sys_uname	__P((struct proc *, void *, register_t *));

# hints/vmesa.sh
#
# VM/ESA hints by Neale Ferguson (neale@mailbox.tabnsw.com.au)
#
# Currently (1999-Jan-09) Configure cannot be used in VM/ESA because
# too many things are done differently in the C compiler environment.
# Therefore the hints file is hand-crafted. --jhi@iki.fi
# 

case "$archname" in
'') archname="$osname" ;;
esac
bin='/usr/local/bin'
binexp='/usr/local/bin'
byacc='byacc'
c='\c'
cc='c89'
ccflags="-D_OE_SOCKETS -DOLD_PTHREADS_API -DYYDYNAMIC -DDEBUGGING -I.." \
	"-I/usr/local/include -W c,hwopts\\\(string\\\),langlvl\\\(ansi\\\)"
clocktype='clock_t'
cryptlib="n"
d_Gconvert='gcvt((x),(n),(b))'
d_access='define'
d_alarm='define'
d_archlib='define'
# randbits='15'
archobjs="vmesa.o"
d_attribut='undef'
d_bcmp='define'
d_bcopy='define'
d_bsd='undef'
d_bsdgetpgrp='undef'
d_bsdsetpgrp='undef'
d_bzero='define'
d_casti32='define'
d_castneg='define'
d_charvspr='undef'
d_chown='define'
d_chroot='undef'
d_chsize='undef'
d_closedir='define'
d_const='define'
d_crypt='undef'
d_csh='undef'
d_cuserid='define'
d_dbl_dig='define'
d_difftime='define'
d_dirnamlen='undef'
d_dlerror='define'
d_dlopen='define'
d_dlsymun='define'
d_dosuid='undef'
d_dup2='define'
d_endgrent='undef'
d_endpwent='undef'
d_eofnblk='define'
d_eunice='undef'
d_fchmod='define'
d_fchown='define'
d_fcntl='define'
d_fd_macros='define'
d_fd_set='define'
d_fds_bits='define'
d_fgetpos='define'
d_flexfnam='define'
d_flock='undef'
d_fork='undef'
d_fpathconf='define'
d_fsetpos='define'
d_ftime='undef'
d_getgrent='undef'
d_gethent='define'
d_gethname='undef'
d_getlogin='define'
d_getpgid='undef'
d_getpgrp='define'
d_getpgrp2='undef'
d_getppid='define'
d_getprior='undef'
d_getpwent='undef'
d_gettimeod='define'
d_gnulibc='undef'
d_htonl='define'
d_index='define'
d_inetaton='undef'
d_isascii='define'
d_killpg='define'
d_link='define'
d_locconv='define'
d_lockf='define'
d_longdbl='undef'
d_longllong='undef'
d_lstat='define'
d_mblen='define'
d_mbstowcs='define'
d_mbtowc='define'
d_memcmp='define'
d_memcpy='define'
d_memmove='define'
d_memset='define'
d_mkdir='define'
d_mkfifo='define'
d_mktime='define'
d_msg='define'
d_msgctl='define'
d_msgget='define'
d_msgrcv='define'
d_msgsnd='define'
d_mymalloc='undef'
d_nice='undef'
d_oldsock='undef'
d_open3='define'
d_pathconf='define'
d_pause='define'
d_phostname='undef'
d_pipe='define'
d_poll='undef'
d_portable='define'
d_pwage='undef'
d_pwchange='undef'
d_pwclass='undef'
d_pwcomment='undef'
d_pwexpire='undef'
d_pwquota='undef'
d_readdir='define'
d_readlink='define'
d_rename='define'
d_rewinddir='define'
d_rmdir='define'
d_safebcpy='define'
d_safemcpy='undef'
d_sanemcmp='define'
d_sched_yield='undef'
d_seekdir='undef'
d_select='define'
d_sem='define'
d_semctl='define'
d_semctl_semid_ds='define'
d_semget='define'
d_semop='define'
d_setegid='define'
d_seteuid='define'
d_setgrent='undef'
d_setgrps='undef'
d_setlinebuf='undef'
d_setlocale='define'
d_setpgid='define'
d_setpgrp='define'
d_setpgrp2='undef'
d_setprior='undef'
d_setpwent='undef'
d_setregid='undef'
d_setresgid='undef'
d_setresuid='undef'
d_setreuid='undef'
d_setrgid='undef'
d_setruid='undef'
d_setsid='define'
d_sfio='undef'
d_shm='define'
d_shmat='define'
d_shmatprototype='define'
d_shmctl='define'
d_shmdt='define'
d_shmget='define'
d_sigaction='define'
d_sigsetjmp='define'
d_socket='define'
d_sockpair='undef'
d_statblks='undef'
d_stdio_cnt_lval='undef'
d_stdio_ptr_lval='undef'
d_stdiobase='undef'
d_stdstdio='undef'
d_strchr='define'
d_strcoll='define'
d_strctcpy='undef'
d_strerrm='strerror(e)'
d_strerror='define'
d_strtod='define'
d_strtol='define'
d_strtoul='define'
d_strxfrm='define'
d_suidsafe='undef'
d_symlink='define'
d_syscall='undef'
d_sysconf='define'
d_sysernlst="n"
d_syserrlst='undef'
d_system='define'
d_tcgetpgrp='define'
d_tcsetpgrp='define'
d_telldir='undef'
d_time='define'
d_times='define'
d_truncate='define'
d_tzname='define'
d_umask='define'
d_uname='define'
d_union_semun='undef'
d_vfork='define'
d_void_closedir='undef'
d_voidsig='define'
d_voidtty="n"
d_volatile='define'
d_vprintf='define'
d_waitpid='define'
d_wait4='undef'
d_wcstombs='define'
d_wctomb='define'
d_xenix='undef'
db_hashtype='u_int32_t'
db_prefixtype='size_t'
direntrytype='struct dirent'
dlext='none'
dlsrc='dl_vmesa.xs'
dynamic_ext=''
eagain='EAGAIN'
ebcdic='define'
exe_ext=''
fpostype='fpos_t'
freetype='void'
groupstype='gid_t'
h_fcntl='false'
h_sysfile='true'
hint='recommended'
i_arpainet="define"
i_bsdioctl="n"
i_db='undef'
i_dbm='define'
i_dirent='define'
i_dld='define'
i_dlfcn='define'
i_fcntl='undef'
i_float='define'
i_gdbm='define'
i_grp='define'
i_limits='define'
i_locale='define'
i_malloc='undef'
i_math='define'
i_memory='define'
i_ndbm='define'
i_neterrno='undef'
i_niin='define'
i_pwd='define'
i_rpcsvcdbm='undef'
i_sfio='undef'
i_sgtty='undef'
i_stdarg='define'
i_stddef='define'
i_stdlib='define'
i_string='define'
i_sysdir='define'
i_sysfile='define'
i_sysfilio='undef'
i_sysin='undef'
i_sysioctl='define'
i_sysndir='undef'
i_sysparam='undef'
i_sysresrc='define'
i_sysselct='undef'
i_syssockio="n"
i_sysstat='define'
i_systime='define'
i_systimek='undef'
i_systimes='define'
i_systypes='define'
i_sysun='define'
i_syswait='define'
i_termio='undef'
i_termios='define'
i_time='undef'
i_unistd='define'
i_utime='define'
i_values='undef'
i_varargs='undef'
i_varhdr='stdarg.h'
i_vfork='undef'
ld='c89'
ldflags='-L/usr/local/lib -L.'
lib_ext='.a'
libc=''
libperl='libperl.a'
libpth='/usr/local/lib /lib /usr/lib'
libs='-l//posxsock -l//vmmtlib -lgdbm -lxpg4'
libswanted='gdbm'
lint="n"
locincpth='/usr/local/include /opt/local/include /usr/gnu/include /opt/gnu/include /usr/GNU/include /opt/GNU/include'
loclibpth='/usr/local/lib /opt/local/lib /usr/gnu/lib /opt/gnu/lib /usr/GNU/lib /opt/GNU/lib'
make_set_make='#'
make='gnumake'
mallocobj=''
mallocsrc=''
malloctype='void *'
netdb_hlen_type='size_t'
netdb_host_type='char *'
netdb_name_type='const char *'
netdb_net_type='in_addr_t'
o_nonblock='O_NONBLOCK'
obj_ext='.o'
optimize='undef'
prefix='/usr/local'
prefixexp='/usr/local'
prototype='define'
ranlib=':'
rd_nodata='-1'
scriptdir='/usr/local/bin'
scriptdirexp='/usr/local/bin'
selecttype='fd_set *'
shmattype='void *'
shrpenv=''
signal_t='void'
sig_name_init='"ZERO","HUP","INT","ABRT","ILL","POLL","URG","STOP","FPE","KILL","BUS","SEGV","SYS","PIPE","ALRM","TERM","USR1","USR2","NUM18","CONT","CHLD","TTIN","TTOU","IO","QUIT","TSTP","TRAP","NUM27","WINCH","XCPU","XFSZ","VTALRM","PROF","NUM33","NUM34","NUM35","NUM36","NUM3","NUM38","NUM39","NUM40","NUM41","NUM42","NUM43","NUM44","NUM45","NUM46","NUM47","NUM48","NUM49","CLD"'
sig_num_init='0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,20 '
sizetype='size_t'
so='.a'
ssizetype='ssize_t'
static_ext='Data/Dumper Digest/MD5 Fcntl Filter/Util/Call GDBM_File IO IPC/SysV List/Util MIME/Base64 NDBM_File Opcode PerlIO/scalar POSIX Socket Storable Thread Time/HiRes Time/Piece attrs re'
stdchar='char'
stdio_cnt='(fp)->__countIn'
stdio_ptr='(fp)->__bufPtr'
timeincl='sys/time.h '
timetype='time_t'
uidtype='uid_t'
usedl='define'
usemymalloc='n'
usenm='false'
useopcode='true'
useperlio='undef'
useposix='true'
usesfio='false'
useshrplib='false'
usethreads='y'
usevfork='true'
vi='x'

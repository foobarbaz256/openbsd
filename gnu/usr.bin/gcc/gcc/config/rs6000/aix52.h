/* Definitions of target machine for GNU compiler,
   for IBM RS/6000 POWER running AIX V5.2.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Contributed by David Edelsohn (edelsohn@gnu.org).

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


/* AIX V5 and above support 64-bit executables.  */
#undef  SUBSUBTARGET_SWITCHES
#define SUBSUBTARGET_SWITCHES					\
  {"aix64", 		MASK_64BIT | MASK_POWERPC64 | MASK_POWERPC,	\
   N_("Compile for 64-bit pointers") },					\
  {"aix32",		- (MASK_64BIT | MASK_POWERPC64),		\
   N_("Compile for 32-bit pointers") },					\
  {"pe",		0,						\
   N_("Support message passing with the Parallel Environment") },

/* Sometimes certain combinations of command options do not make sense
   on a particular target machine.  You can define a macro
   `OVERRIDE_OPTIONS' to take account of this.  This macro, if
   defined, is executed once just after all the command options have
   been parsed.

   The macro SUBTARGET_OVERRIDE_OPTIONS is provided for subtargets, to
   get control.  */

#define NON_POWERPC_MASKS (MASK_POWER | MASK_POWER2)
#define SUBTARGET_OVERRIDE_OPTIONS					\
do {									\
  if (TARGET_64BIT && (target_flags & NON_POWERPC_MASKS))		\
    {									\
      target_flags &= ~NON_POWERPC_MASKS;				\
      warning ("-maix64 and POWER architecture are incompatible");	\
    }									\
  if (TARGET_64BIT && ! TARGET_POWERPC64)				\
    {									\
      target_flags |= MASK_POWERPC64;					\
      warning ("-maix64 requires PowerPC64 architecture remain enabled"); \
    }									\
  if (TARGET_POWERPC64 && ! TARGET_64BIT)				\
    {									\
      error ("-maix64 required: 64-bit computation with 32-bit addressing not yet supported"); \
    }									\
} while (0);

#undef ASM_SPEC
#define ASM_SPEC "-u %{maix64:-a64 -mppc64} %(asm_cpu)"

/* Common ASM definitions used by ASM_SPEC amonst the various targets
   for handling -mcpu=xxx switches.  */
#undef ASM_CPU_SPEC
#define ASM_CPU_SPEC \
"%{!mcpu*: %{!maix64: \
  %{mpowerpc64: -mppc64} \
  %{!mpower64: %(asm_default)}}} \
%{mcpu=power3: -m604} \
%{mcpu=power4: -m604} \
%{mcpu=powerpc: -mppc} \
%{mcpu=rs64a: -mppc} \
%{mcpu=603: -m603} \
%{mcpu=603e: -m603} \
%{mcpu=604: -m604} \
%{mcpu=604e: -m604} \
%{mcpu=620: -mppc} \
%{mcpu=630: -m604}"

#undef	ASM_DEFAULT_SPEC
#define ASM_DEFAULT_SPEC "-mppc"

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()      \
  do                                  \
    {                                 \
      builtin_define ("_IBMR2");      \
      builtin_define ("_POWER");      \
      builtin_define ("_LONG_LONG");  \
      builtin_define ("_AIX");        \
      builtin_define ("_AIX32");      \
      builtin_define ("_AIX41");      \
      builtin_define ("_AIX43");      \
      builtin_define ("_AIX51");      \
      builtin_define ("_AIX52");      \
      builtin_assert ("system=unix"); \
      builtin_assert ("system=aix");  \
    }                                 \
  while (0)

#undef CPP_SPEC
#define CPP_SPEC "%{posix: -D_POSIX_SOURCE}	\
  %{ansi: -D_ANSI_C_SOURCE}			\
  %{maix64: -D__64BIT__}			\
  %{mpe: -I/usr/lpp/ppe.poe/include}		\
  %{pthread: -D_THREAD_SAFE}"

/* The GNU C++ standard library requires that these macros be 
   defined.  */
#undef CPLUSPLUS_CPP_SPEC                       
#define CPLUSPLUS_CPP_SPEC			\
  "-D_XOPEN_SOURCE=500				\
   -D_XOPEN_SOURCE_EXTENDED=1			\
   -D_LARGE_FILE_API				\
   -D_ALL_SOURCE				\
   %{maix64: -D__64BIT__}			\
   %{mpe: -I/usr/lpp/ppe.poe/include}		\
   %{pthread: -D_THREAD_SAFE}"

#undef  TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_POWERPC | MASK_NEW_MNEMONICS)

#undef  PROCESSOR_DEFAULT
#define PROCESSOR_DEFAULT PROCESSOR_PPC630
#undef  PROCESSOR_DEFAULT64
#define PROCESSOR_DEFAULT64 PROCESSOR_POWER4

#undef  TARGET_POWER
#define TARGET_POWER 0

/* Define this macro as a C expression for the initializer of an
   array of string to tell the driver program which options are
   defaults for this target and thus do not need to be handled
   specially when using `MULTILIB_OPTIONS'.

   Do not define this macro if `MULTILIB_OPTIONS' is not defined in
   the target makefile fragment or if none of the options listed in
   `MULTILIB_OPTIONS' are set by default.  *Note Target Fragment::.  */

#undef	MULTILIB_DEFAULTS

#undef LIB_SPEC
#define LIB_SPEC "%{pg:-L/lib/profiled -L/usr/lib/profiled}\
   %{p:-L/lib/profiled -L/usr/lib/profiled}\
   %{!maix64:%{!shared:%{g*:-lg}}}\
   %{mpe:-L/usr/lpp/ppe.poe/lib -lmpi -lvtd}\
   %{pthread:-lpthreads} -lc"

#undef LINK_SPEC
#define LINK_SPEC "-bpT:0x10000000 -bpD:0x20000000 %{!r:-btextro} -bnodelcsect\
   %{static:-bnso %(link_syscalls) } %{shared:-bM:SRE %{!e:-bnoentry}}\
   %{!maix64:%{!shared:%{g*: %(link_libg) }}} %{maix64:-b64}\
   %{mpe:-binitfini:poe_remote_main}"

#undef STARTFILE_SPEC
#define STARTFILE_SPEC "%{!shared:\
   %{maix64:%{pg:gcrt0_64%O%s}%{!pg:%{p:mcrt0_64%O%s}%{!p:crt0_64%O%s}}}\
   %{!maix64:\
     %{pthread:%{pg:gcrt0_r%O%s}%{!pg:%{p:mcrt0_r%O%s}%{!p:crt0_r%O%s}}}\
     %{!pthread:%{pg:gcrt0%O%s}%{!pg:%{p:mcrt0%O%s}%{!p:crt0%O%s}}}}}"

/* AIX V5 typedefs ptrdiff_t as "long" while earlier releases used "int".  */

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE "long int"

/* Type used for wchar_t, as a string used in a declaration.  */
#undef  WCHAR_TYPE
#define WCHAR_TYPE (!TARGET_64BIT ? "short unsigned int" : "unsigned int")

/* Width of wchar_t in bits.  */
#undef  WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE (!TARGET_64BIT ? 16 : 32)
#define MAX_WCHAR_TYPE_SIZE 32

/* AIX V5 uses PowerPC nop (ori 0,0,0) instruction as call glue for PowerPC
   and "cror 31,31,31" for POWER architecture.  */

#undef RS6000_CALL_GLUE
#define RS6000_CALL_GLUE "{cror 31,31,31|nop}"

/* AIX 4.2 and above provides initialization and finalization function
   support from linker command line.  */
#undef HAS_INIT_SECTION
#define HAS_INIT_SECTION

#undef LD_INIT_SWITCH
#define LD_INIT_SWITCH "-binitfini"

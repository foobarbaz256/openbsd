dnl $Id: win32.m4,v 1.2 2013/06/17 18:57:41 robert Exp $
dnl rk_WIN32_EXPORT buildsymbol symbol-that-export
AC_DEFUN([rk_WIN32_EXPORT],[AH_TOP([#ifdef $1
#ifndef $2
#ifdef _WIN32_
#define $2_FUNCTION __declspec(dllexport)
#define $2_CALL __stdcall
#define $2_VARIABLE __declspec(dllexport)
#else
#define $2_FUNCTION
#define $2_CALL
#define $2_VARIABLE
#endif
#endif
#endif
])])

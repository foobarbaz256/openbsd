# longdouble.m4 serial 1 (gettext-0.12)
dnl Copyright (C) 2002-2003 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl From Bruno Haible.
dnl Test whether the compiler supports the 'long double' type.
dnl Prerequisite: AC_PROG_CC

AC_DEFUN([gt_TYPE_LONGDOUBLE],
[
  AC_CACHE_CHECK([for long double], gt_cv_c_long_double,
    [if test "$GCC" = yes; then
       gt_cv_c_long_double=yes
     else
       AC_TRY_COMPILE([
         /* The Stardent Vistra knows sizeof(long double), but does not support it.  */
         long double foo = 0.0;
         /* On Ultrix 4.3 cc, long double is 4 and double is 8.  */
         int array [2*(sizeof(long double) >= sizeof(double)) - 1];
         ], ,
         gt_cv_c_long_double=yes, gt_cv_c_long_double=no)
     fi])
  if test $gt_cv_c_long_double = yes; then
    AC_DEFINE(HAVE_LONG_DOUBLE, 1, [Define if you have the 'long double' type.])
  fi
])

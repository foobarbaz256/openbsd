/* Basic data types for Objective C.
   Copyright (C) 1993 Free Software Foundation, Inc.

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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 

	$Id: objc.h,v 1.1.1.1 1995/10/18 08:39:50 deraadt Exp $
*/

/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#ifndef __objc_INCLUDE_GNU
#define __objc_INCLUDE_GNU

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IN_GCC
#include "gstddef.h"
#else
#include "stddef.h"
#endif

/*
** Definition of the boolean type.  
*/
typedef char  BOOL;
#define YES   (BOOL)1
#define NO    (BOOL)0

/*
** Definition of a selector.  Selectors are really of type unsigned int.
** The runtime does this mapping from SEL's to names internally in the
** sel_... operations.  You should never use the fact that it is actually
** an integer, since other Objective-C implementations use other conventions.
*/
typedef void* SEL;

/*
** ObjC uses this typedef for untyped instances.
*/
typedef struct objc_object {
  struct objc_class*  class_pointer;
} *id;

/*
** Definition of method type.  When retrieving the implementation of a
** method, this is type of the pointer returned
*/
typedef id (*IMP)(id, SEL, ...); 

/*
** More simple types...
*/
#define nil (id)0                               /* id of Nil instance */
#define Nil (Class*)0                          /* id of Nil class */
typedef char *STR;                              /* String alias */

/*
** The compiler generates one of these structures for each class.  
** 
** This structure is the definition for classes. 
** 
** This structure is generated by the compiler in the executable and used by
** the run-time during normal messaging operations.  Therefore some members
** change type. The compiler generates "char* const" and places a string in
** the following member variables:  super_class. 
*/
typedef struct objc_class MetaClass;
typedef struct objc_class Class;
struct objc_class {     
  MetaClass*         class_pointer;          /* Pointer to the class's
                                                meta class. */
  struct objc_class*  super_class;            /* Pointer to the super 
                                                class. NULL for class 
                                                Object. */
  const char*         name;                   /* Name of the class. */
  long                version;                /* Unknown. */
  unsigned long       info;                   /* Bit mask.  See class masks 
                                                defined above. */
  long                instance_size;          /* Size in bytes of the class.  
                                                The sum of the class definition 
                                                and all super class 
                                                definitions. */
  struct objc_ivar_list* ivars;               /* Pointer to a structure that
                                                describes the instance 
                                                variables in the class
                                                definition.  NULL indicates
                                                no instance variables.  Does
                                                not include super class
                                                variables. */
  struct objc_method_list*  methods;          /* Linked list of instance
                                                methods defined for the 
                                                class. */
  struct sarray *    dtable;                  /* Pointer to instance 
					         method dispatch table. */  
  struct objc_class* subclass_list;           /* Subclasses */
  struct objc_class* sibling_class;

  struct objc_protocol_list *protocols;	      /* Protocols conformed to */
};

#ifndef __OBJC__
typedef struct objc_protocol {
  struct objc_class* class_pointer;
  char *protocol_name;
  struct objc_protocol_list *protocol_list;
  struct objc_method_description_list *instance_methods, *class_methods; 
} Protocol; 

#else

@class Protocol;
#endif 

typedef void* retval_t;		/* return value */
typedef void(*apply_t)(void);	/* function pointer */

#if defined(REG_ARGS) || defined(STACK_ARGS) 

typedef struct {
  char* arg_pointer;
#ifdef STRUCT_RETURN
  void* struct_return;
#endif
#ifdef REG_ARGS
  void* regs[2];
#endif
} *arglist_t;

#ifdef REG_ARGS
#define __objc_frame_receiver(FRAME)  (FRAME)->regs[0]
#define __objc_frame_selector(FRAME)  ((SEL)(FRAME)->regs[1])

#else
#define __objc_frame_receiver(FRAME) ((id*)(FRAME)->arg_pointer)[0]
#define __objc_frame_selector(FRAME) ((SEL*)(FRAME)->arg_pointer)[1]
#endif
#else

typedef void* arglist_t;

#endif

#if defined(__OBJC__) 

#include "objc/sarray.h"

static id nil_method(id rcv, SEL op, ...) { return rcv; }

extern __inline__ IMP
objc_msg_lookup(id receiver, SEL op)
{
  if(receiver)
    return sarray_get(receiver->class_pointer->dtable, (size_t) op);
  else
    return nil_method;
}

#else

IMP objc_msg_lookup(id receiver, SEL op);

#endif

#ifdef __cplusplus
}
#endif

#endif /* not __objc_INCLUDE_GNU */

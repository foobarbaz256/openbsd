/*	$OpenBSD: m_item_nam.c,v 1.4 1998/07/24 16:39:00 millert Exp $	*/

/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *   Author: Juergen Pfeifer <Juergen.Pfeifer@T-Online.de> 1995,1997        *
 ****************************************************************************/

/***************************************************************************
* Module m_item_nam                                                        *
* Get menus item name and description                                      *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$From: m_item_nam.c,v 1.7 1998/02/11 12:13:49 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  char *item_name(const ITEM *item)
|   
|   Description   :  Return name of menu item
|
|   Return Values :  See above; returns NULL if item is invalid
+--------------------------------------------------------------------------*/
const char *item_name(const ITEM * item) 
{
  return ((item) ? item->name.str : (char *)0);
}
		
/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  char *item_description(const ITEM *item)
|   
|   Description   :  Returns description of item
|
|   Return Values :  See above; Returns NULL if item is invalid
+--------------------------------------------------------------------------*/
const char *item_description(const ITEM * item)
{
  return ((item) ? item->description.str : (char *)0);
}

/* m_item_nam.c ends here */

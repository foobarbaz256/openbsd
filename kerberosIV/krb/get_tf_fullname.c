/*
 * This software may now be redistributed outside the US.
 *
 * $Source: /cvs/src/kerberosIV/krb/Attic/get_tf_fullname.c,v $
 *
 * $Locker:  $
 */

/* 
  Copyright (C) 1989 by the Massachusetts Institute of Technology

   Export of this software from the United States of America is assumed
   to require a specific license from the United States Government.
   It is the responsibility of any person or organization contemplating
   export to obtain such a license before exporting.

WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
distribute this software and its documentation for any purpose and
without fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.  M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without express
or implied warranty.

  */

#include "krb_locl.h"

/*
 * This file contains a routine to extract the fullname of a user
 * from the ticket file.
 */

/*
 * krb_get_tf_fullname() takes four arguments: the name of the 
 * ticket file, and variables for name, instance, and realm to be
 * returned in.  Since the realm of a ticket file is not really fully 
 * supported, the realm used will be that of the first ticket in the
 * file as this is the one that was obtained with a password by
 * krb_get_in_tkt().
 */

int
krb_get_tf_fullname(ticket_file, name, instance, realm)
	char *ticket_file;
	char *name;
	char *instance;
	char *realm;
{
    int tf_status;
    CREDENTIALS c;

    if ((tf_status = tf_init(ticket_file, R_TKT_FIL)) != KSUCCESS)
	return(tf_status);

    if (((tf_status = tf_get_pname(c.pname)) != KSUCCESS) ||
	((tf_status = tf_get_pinst(c.pinst)) != KSUCCESS))
	return (tf_status);
    
    if (name)
	strcpy(name, c.pname);
    if (instance)
	strcpy(instance, c.pinst);
    if ((tf_status = tf_get_cred(&c)) == KSUCCESS) {
	if (realm)
	    strcpy(realm, c.realm);
    }
    else {
	if (tf_status == EOF)
	    return(KFAILURE);
	else
	    return(tf_status);
    }    
    (void) tf_close();
    
    return(tf_status);
}

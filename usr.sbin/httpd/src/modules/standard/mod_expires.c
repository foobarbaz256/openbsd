/* ====================================================================
 * Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * IT'S CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */

/*
 * mod_expires.c
 * version 0.0.11
 * status beta
 * 
 * Andrew Wilson <Andrew.Wilson@cm.cf.ac.uk> 26.Jan.96
 *
 * This module allows you to control the form of the Expires: header
 * that Apache issues for each access.  Directives can appear in
 * configuration files or in .htaccess files so expiry semantics can
 * be defined on a per-directory basis.  
 *
 * DIRECTIVE SYNTAX
 *
 * Valid directives are:
 *
 *     ExpiresActive on | off
 *     ExpiresDefault <code><seconds>
 *     ExpiresByType type/encoding <code><seconds>
 *
 * Valid values for <code> are:
 *
 *     'M'      expires header shows file modification date + <seconds>
 *     'A'      expires header shows access time + <seconds>
 *
 *              [I'm not sure which of these is best under different
 *              circumstances, I guess it's for other people to explore.
 *              The effects may be indistinguishable for a number of cases]
 *
 * <seconds> should be an integer value [acceptable to atoi()]
 *
 * There is NO space between the <code> and <seconds>.
 *
 * For example, a directory which contains information which changes
 * frequently might contain:
 *
 *     # reports generated by cron every hour.  don't let caches
 *     # hold onto stale information
 *     ExpiresDefault M3600
 *
 * Another example, our html pages can change all the time, the gifs
 * tend not to change often:
 * 
 *     # pages are hot (1 week), images are cold (1 month)
 *     ExpiresByType text/html A604800
 *     ExpiresByType image/gif A2592000
 *
 * Expires can be turned on for all URLs on the server by placing the
 * following directive in a conf file:
 *
 *     ExpiresActive on
 *
 * ExpiresActive can also appear in .htaccess files, enabling the
 * behaviour to be turned on or off for each chosen directory.
 *
 *     # turn off Expires behaviour in this directory
 *     # and subdirectories
 *     ExpiresActive off
 *
 * Directives defined for a directory are valid in subdirectories
 * unless explicitly overridden by new directives in the subdirectory
 * .htaccess files.
 *
 * ALTERNATIVE DIRECTIVE SYNTAX
 *
 * Directives can also be defined in a more readable syntax of the form:
 *
 *     ExpiresDefault "<base> [plus] {<num> <type>}*"
 *     ExpiresByType type/encoding "<base> [plus] {<num> <type>}*"
 *
 * where <base> is one of:
 *      access  
 *      now             equivalent to 'access'
 *      modification
 *
 * where the 'plus' keyword is optional
 *
 * where <num> should be an integer value [acceptable to atoi()]
 *
 * where <type> is one of:
 *      years
 *      months
 *      weeks
 *      days
 *      hours
 *      minutes
 *      seconds
 *
 * For example, any of the following directives can be used to make
 * documents expire 1 month after being accessed, by default:
 *
 *      ExpiresDefault "access plus 1 month"
 *      ExpiresDefault "access plus 4 weeks"
 *      ExpiresDefault "access plus 30 days"
 *
 * The expiry time can be fine-tuned by adding several '<num> <type>'
 * clauses:
 *
 *      ExpiresByType text/html "access plus 1 month 15 days 2 hours"
 *      ExpiresByType image/gif "modification plus 5 hours 3 minutes"
 *
 * ---
 *
 * Change-log:
 * 29.Jan.96    Hardened the add_* functions.  Server will now bail out
 *              if bad directives are given in the conf files.
 * 02.Feb.96    Returns DECLINED if not 'ExpiresActive on', giving other
 *              expires-aware modules a chance to play with the same
 *              directives. [Michael Rutman]
 * 03.Feb.96    Call tzset() before localtime().  Trying to get the module
 *              to work properly in non GMT timezones.
 * 12.Feb.96    Modified directive syntax to allow more readable commands:
 *                ExpiresDefault "now plus 10 days 20 seconds"
 *                ExpiresDefault "access plus 30 days"
 *                ExpiresDefault "modification plus 1 year 10 months 30 days"
 * 13.Feb.96    Fix call to table_get() with NULL 2nd parameter [Rob Hartill]
 * 19.Feb.96    Call gm_timestr_822() to get time formatted correctly, can't
 *              rely on presence of HTTP_TIME_FORMAT in Apache 1.1+.
 * 21.Feb.96    This version (0.0.9) reverses assumptions made in 0.0.8
 *              about star/star handlers.  Reverting to 0.0.7 behaviour.
 * 08.Jun.96    allows ExpiresDefault to be used with responses that use 
 *              the DefaultType by not DECLINING, but instead skipping 
 *              the table_get check and then looking for an ExpiresDefault.
 *              [Rob Hartill]
 * 04.Nov.96    'const' definitions added.
 *
 * TODO
 * add support for Cache-Control: max-age=20 from the HTTP/1.1
 * proposal (in this case, a ttl of 20 seconds) [ask roy]
 * add per-file expiry and explicit expiry times - duplicates some
 * of the mod_cern_meta.c functionality.  eg:
 *              ExpiresExplicit index.html "modification plus 30 days"
 *
 * BUGS
 * Hi, welcome to the internet.
 */

#include <ctype.h>
#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

typedef struct {
    int active;
    char *expiresdefault;
    table *expiresbytype;
} expires_dir_config;

/* from mod_dir, why is this alias used?
 */
#define DIR_CMD_PERMS OR_INDEXES

#define ACTIVE_ON       1
#define ACTIVE_OFF      0
#define ACTIVE_DONTCARE 2

module MODULE_VAR_EXPORT expires_module;

static void *create_dir_expires_config(pool *p, char *dummy)
{
    expires_dir_config *new =
    (expires_dir_config *) ap_pcalloc(p, sizeof(expires_dir_config));
    new->active = ACTIVE_DONTCARE;
    new->expiresdefault = "";
    new->expiresbytype = ap_make_table(p, 4);
    return (void *) new;
}

static const char *set_expiresactive(cmd_parms *cmd, expires_dir_config * dir_config, int arg)
{
    /* if we're here at all it's because someone explicitly
     * set the active flag
     */
    dir_config->active = ACTIVE_ON;
    if (arg == 0) {
        dir_config->active = ACTIVE_OFF;
    };
    return NULL;
}

/* check_code() parse 'code' and return NULL or an error response
 * string.  If we return NULL then real_code contains code converted
 * to the cnnnn format.
 */
static char *check_code(pool *p, const char *code, char **real_code)
{
    char *word;
    char base = 'X';
    int modifier = 0;
    int num = 0;
    int factor = 0;

    /* 0.0.4 compatibility?
     */
    if ((code[0] == 'A') || (code[0] == 'M')) {
        *real_code = (char *)code;
        return NULL;
    };

    /* <base> [plus] {<num> <type>}*
     */

    /* <base>
     */
    word = ap_getword_conf(p, &code);
    if (!strncasecmp(word, "now", 1) ||
        !strncasecmp(word, "access", 1)) {
        base = 'A';
    }
    else if (!strncasecmp(word, "modification", 1)) {
        base = 'M';
    }
    else {
        return ap_pstrcat(p, "bad expires code, unrecognised <base> '",
                       word, "'", NULL);
    };

    /* [plus]
     */
    word = ap_getword_conf(p, &code);
    if (!strncasecmp(word, "plus", 1)) {
        word = ap_getword_conf(p, &code);
    };

    /* {<num> <type>}*
     */
    while (word[0]) {
        /* <num>
         */
        if (ap_isdigit(word[0])) {
            num = atoi(word);
        }
        else {
            return ap_pstrcat(p, "bad expires code, numeric value expected <num> '",
                           word, "'", NULL);
        };

        /* <type>
         */
        word = ap_getword_conf(p, &code);
        if (word[0]) {
            /* do nothing */
        }
        else {
            return ap_pstrcat(p, "bad expires code, missing <type>", NULL);
        };

        factor = 0;
        if (!strncasecmp(word, "years", 1)) {
            factor = 60 * 60 * 24 * 365;
        }
        else if (!strncasecmp(word, "months", 2)) {
            factor = 60 * 60 * 24 * 30;
        }
        else if (!strncasecmp(word, "weeks", 1)) {
            factor = 60 * 60 * 24 * 7;
        }
        else if (!strncasecmp(word, "days", 1)) {
            factor = 60 * 60 * 24;
        }
        else if (!strncasecmp(word, "hours", 1)) {
            factor = 60 * 60;
        }
        else if (!strncasecmp(word, "minutes", 2)) {
            factor = 60;
        }
        else if (!strncasecmp(word, "seconds", 1)) {
            factor = 1;
        }
        else {
            return ap_pstrcat(p, "bad expires code, unrecognised <type>",
                           "'", word, "'", NULL);
        };

        modifier = modifier + factor * num;

        /* next <num>
         */
        word = ap_getword_conf(p, &code);
    };

    *real_code = ap_psprintf(p, "%c%d", base, modifier);

    return NULL;
}

static const char *set_expiresbytype(cmd_parms *cmd, expires_dir_config * dir_config, char *mime, char *code)
{
    char *response, *real_code;

    if ((response = check_code(cmd->pool, code, &real_code)) == NULL) {
        ap_table_setn(dir_config->expiresbytype, mime, real_code);
        return NULL;
    };
    return ap_pstrcat(cmd->pool,
                 "'ExpiresByType ", mime, " ", code, "': ", response, NULL);
}

static const char *set_expiresdefault(cmd_parms *cmd, expires_dir_config * dir_config, char *code)
{
    char *response, *real_code;

    if ((response = check_code(cmd->pool, code, &real_code)) == NULL) {
        dir_config->expiresdefault = real_code;
        return NULL;
    };
    return ap_pstrcat(cmd->pool,
                   "'ExpiresDefault ", code, "': ", response, NULL);
}

static const command_rec expires_cmds[] =
{
    {"ExpiresActive", set_expiresactive, NULL, DIR_CMD_PERMS, FLAG,
     "Limited to 'on' or 'off'"},
    {"ExpiresBytype", set_expiresbytype, NULL, DIR_CMD_PERMS, TAKE2,
     "a MIME type followed by an expiry date code"},
    {"ExpiresDefault", set_expiresdefault, NULL, DIR_CMD_PERMS, TAKE1,
     "an expiry date code"},
    {NULL}
};

static void *merge_expires_dir_configs(pool *p, void *basev, void *addv)
{
    expires_dir_config *new = (expires_dir_config *) ap_pcalloc(p, sizeof(expires_dir_config));
    expires_dir_config *base = (expires_dir_config *) basev;
    expires_dir_config *add = (expires_dir_config *) addv;

    if (add->active == ACTIVE_DONTCARE) {
        new->active = base->active;
    }
    else {
        new->active = add->active;
    };

    if (add->expiresdefault != '\0') {
        new->expiresdefault = add->expiresdefault;
    };

    new->expiresbytype = ap_overlay_tables(p, add->expiresbytype,
                                        base->expiresbytype);
    return new;
}

static int add_expires(request_rec *r)
{
    expires_dir_config *conf;
    char *code;
    time_t base;
    time_t additional;
    time_t expires;
    char age[20];

    if (ap_is_HTTP_ERROR(r->status))       /* Don't add Expires headers to errors */
        return DECLINED;

    if (r->main != NULL)        /* Say no to subrequests */
        return DECLINED;

    conf = (expires_dir_config *) ap_get_module_config(r->per_dir_config, &expires_module);
    if (conf == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
                    "internal error: %s", r->filename);
        return SERVER_ERROR;
    };

    if (conf->active != ACTIVE_ON)
        return DECLINED;

    /* we perhaps could use the default_type(r) in its place but that
     * may be 2nd guesing the desired configuration...  calling table_get
     * with a NULL key will SEGV us
     *
     * I still don't know *why* r->content_type would ever be NULL, this
     * is possibly a result of fixups being called in many different
     * places.  Fixups is probably the wrong place to be doing all this
     * work...  Bah.
     *
     * Changed as of 08.Jun.96 don't DECLINE, look for an ExpiresDefault.
     */
    if (r->content_type == NULL)
        code = NULL;
    else
        code = (char *) ap_table_get(conf->expiresbytype, 
		ap_field_noparam(r->pool, r->content_type));

    if (code == NULL) {
        /* no expires defined for that type, is there a default? */
        code = conf->expiresdefault;

        if (code[0] == '\0')
            return OK;
    };

    /* we have our code */

    switch (code[0]) {
    case 'M':
	if (r->finfo.st_mode == 0) { 
	    /* file doesn't exist on disk, so we can't do anything based on
	     * modification time.  Note that this does _not_ log an error.
	     */
	    return DECLINED;
	}
        base = r->finfo.st_mtime;
        additional = atoi(&code[1]);
        break;
    case 'A':
        /* there's been some discussion and it's possible that 
         * 'access time' will be stored in request structure
         */
        base = r->request_time;
        additional = atoi(&code[1]);
        break;
    default:
        /* expecting the add_* routines to be case-hardened this 
         * is just a reminder that module is beta
         */
        ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
                    "internal error: bad expires code: %s", r->filename);
        return SERVER_ERROR;
    };

    expires = base + additional;
    ap_snprintf(age, sizeof(age), "max-age=%d", (int) expires - (int) r->request_time);
    ap_table_setn(r->headers_out, "Cache-Control", ap_pstrdup(r->pool, age));
    tzset();                    /* redundant? called implicitly by localtime, at least 
                                 * under FreeBSD
                                 */
    ap_table_setn(r->headers_out, "Expires", ap_gm_timestr_822(r->pool, expires));
    return OK;
}

module MODULE_VAR_EXPORT expires_module =
{
    STANDARD_MODULE_STUFF,
    NULL,                       /* initializer */
    create_dir_expires_config,  /* dir config creater */
    merge_expires_dir_configs,  /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server configs */
    expires_cmds,               /* command table */
    NULL,                       /* handlers */
    NULL,                       /* filename translation */
    NULL,                       /* check_user_id */
    NULL,                       /* check auth */
    NULL,                       /* check access */
    NULL,                       /* type_checker */
    add_expires,                /* fixups */
    NULL,                       /* logger */
    NULL,                       /* header parser */
    NULL,                       /* child_init */
    NULL,                       /* child_exit */
    NULL                        /* post read-request */
};


#ifdef NETWARE
int main(int argc, char *argv[]) 
{
    ExitThread(TSR_THREAD, 0);
}
#endif

 
/* ====================================================================
 * Copyright (c) 1996,1997 The Apache Group.  All rights reserved.
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
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
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


/*                       _                            _ _ 
**   _ __ ___   ___   __| |    _ __ _____      ___ __(_) |_ ___ 
**  | '_ ` _ \ / _ \ / _` |   | '__/ _ \ \ /\ / / '__| | __/ _ \
**  | | | | | | (_) | (_| |   | | |  __/\ V  V /| |  | | ||  __/
**  |_| |_| |_|\___/ \__,_|___|_|  \___| \_/\_/ |_|  |_|\__\___|
**                       |_____|
**
**  URL Rewriting Module
**
**  This module uses a rule-based rewriting engine (based on a
**  regular-expression parser) to rewrite requested URLs on the fly. 
**  
**  It supports an unlimited number of additional rule conditions (which can
**  operate on a lot of variables, even on HTTP headers) for granular
**  matching and even external database lookups (either via plain text
**  tables, DBM hash files or even external processes) for advanced URL
**  substitution.
**  
**  It operates on the full URLs (including the PATH_INFO part) both in
**  per-server context (httpd.conf) and per-dir context (.htaccess) and even
**  can generate QUERY_STRING parts on result.   The rewriting result finally
**  can lead to internal subprocessing, external request redirection or even
**  to internal proxy throughput.
**
**  This module was originally written in April 1996 and 
**  gifted exclusively to the The Apache Group in July 1997 by
**
**      Ralf S. Engelschall
**      rse@engelschall.com
**      www.engelschall.com
*/




    /* from the underlaying Unix system ... */
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

    /* from the Apache server ... */
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_log.h"

    /* now our own stuff ... */
#include "mod_rewrite.h"

 


/*
** +-------------------------------------------------------+
** |                                                       |
** |             static module configuration
** |                                                       |
** +-------------------------------------------------------+
*/


/*
**
**  our interface to the Apache server kernel
**
**  keep in mind: 
**
**  o  Runtime logic of a request is as following:
**
**       while(request or subrequest) {
**           foreach(stage #1...#9) {
**               foreach(module) { (**)
**                   try to run hook
**               }
**           }
**       }
**
**  o  the order of modules at (**) is the inverted order as
**     given in the "Configuration" file, i.e. the last module
**     specified is the first one called for each hook!
**     The core module is always the last!
**
**  o  there are two different types of result checking and 
**     continue processing:
**     for hook #1,#4,#5,#6,#8:
**         hook run loop stops on first modules which gives
**         back a result != DECLINED, i.e. it usually returns OK
**         which says "OK, module has handled this _stage_" and for #1
**         this have not to mean "Ok, the filename is now valid".
**     for hook #2,#3,#7,#9:
**         all hooks are run, independend of result
**
**  o  at the last stage, the core module always 
**       - says "BAD_REQUEST" if r->filename does not begin with "/"
**       - prefix URL with document_root or replaced server_root
**         with document_root and sets r->filename
**       - always return a "OK" independed if the file really exists
**         or not!
**
*/

    /* the table of commands we provide */
static command_rec command_table[] = {
    { "RewriteEngine",   cmd_rewriteengine,   NULL, OR_FILEINFO, FLAG, 
      "On or Off to enable or disable (default) the whole rewriting engine" },
    { "RewriteOptions",  cmd_rewriteoptions,  NULL, OR_FILEINFO, ITERATE, 
      "List of option strings to set" },
    { "RewriteBase",     cmd_rewritebase,     NULL, OR_FILEINFO, TAKE1, 
      "the base URL of the per-directory context" },
    { "RewriteCond",     cmd_rewritecond,     NULL, OR_FILEINFO, RAW_ARGS, 
      "a input string and a to be applied regexp-pattern" },
    { "RewriteRule",     cmd_rewriterule,     NULL, OR_FILEINFO, RAW_ARGS, 
      "a URL-applied regexp-pattern and a substitution URL" },
    { "RewriteMap",      cmd_rewritemap,      NULL, RSRC_CONF,   TAKE2, 
      "a mapname and a filename" },
    { "RewriteLog",      cmd_rewritelog,      NULL, RSRC_CONF,   TAKE1, 
      "the filename of the rewriting logfile" },
    { "RewriteLogLevel", cmd_rewriteloglevel, NULL, RSRC_CONF,   TAKE1, 
      "the level of the rewriting logfile verbosity (0=none, 1=std, .., 9=max)" },
    { NULL }
};

    /* the table of content handlers we provide */
static handler_rec handler_table[] = {
    { "redirect-handler", handler_redirect },
    { NULL }
};

    /* the main config structure */
module rewrite_module = {
   STANDARD_MODULE_STUFF, 

   init_module,                 /* module initializer */

   config_perdir_create,        /* create per-dir    config structures */
   config_perdir_merge,         /* merge  per-dir    config structures */
   config_server_create,        /* create per-server config structures */
   config_server_merge,         /* merge  per-server config structures */
   command_table,               /* table of config file commands */

   handler_table,               /* [#8] table of MIME-typed-dispatched request action handlers */

   hook_uri2file,               /* [#1] URI to filename translation */

   NULL,                        /* [#4] check_user_id: get and validate user id from the HTTP request */
   NULL,                        /* [#5] check_auth:    check if the user is ok _here_ */
   NULL,                        /* [#2] check_access:  check access by host address, etc. */

   hook_mimetype,               /* [#6] determine MIME type */

   hook_fixup,                  /* [#7] pre-run fixups */
   NULL,                        /* [#9] log a transaction */
   NULL                         /* [#3] header parser */
};

    /* the cache */
static cache *cachep;

    /* whether proxy module is available or not */
static int proxy_available;

    /* the txt mapfile parsing stuff */
#define MAPFILE_PATTERN "^([^ \t]+)[ \t]+([^ \t]+).*$"
#define MAPFILE_OUTPUT "$1,$2"
static regex_t   *lookup_map_txtfile_regexp = NULL;
static regmatch_t lookup_map_txtfile_regmatch[10];




/*
** +-------------------------------------------------------+
** |                                                       |
** |           configuration directive handling
** |                                                       |
** +-------------------------------------------------------+
*/


/*
**
**  per-server configuration structure handling
**
*/

static void *config_server_create(pool *p, server_rec *s)
{
    rewrite_server_conf *a;

    a = (rewrite_server_conf *)pcalloc(p, sizeof(rewrite_server_conf));

    a->state           = ENGINE_DISABLED;
    a->options         = OPTION_NONE;
    a->rewritelogfile  = NULL;
    a->rewritelogfp    = -1;
    a->rewriteloglevel = 1;
    a->rewritemaps     = make_array(p, 2, sizeof(rewritemap_entry));
    a->rewriteconds    = make_array(p, 2, sizeof(rewritecond_entry));
    a->rewriterules    = make_array(p, 2, sizeof(rewriterule_entry));

    return (void *)a;
}

static void *config_server_merge(pool *p, void *basev, void *overridesv)
{
    rewrite_server_conf *a, *base, *overrides;

    a         = (rewrite_server_conf *)pcalloc(p, sizeof(rewrite_server_conf));
    base      = (rewrite_server_conf *)basev;
    overrides = (rewrite_server_conf *)overridesv;

    a->state           = overrides->state;
    a->options         = overrides->options;
    a->rewritelogfile  = base->rewritelogfile  != NULL ? base->rewritelogfile  : overrides->rewritelogfile;
    a->rewritelogfp    = base->rewritelogfp    != -1   ? base->rewritelogfp    : overrides->rewritelogfp;
    a->rewriteloglevel = overrides->rewriteloglevel;

    if (a->options & OPTION_INHERIT) {
        a->rewritemaps  = append_arrays(p, overrides->rewritemaps,  base->rewritemaps);
        a->rewriteconds = append_arrays(p, overrides->rewriteconds, base->rewriteconds);
        a->rewriterules = append_arrays(p, overrides->rewriterules, base->rewriterules);
    }
    else {
        a->rewritemaps  = overrides->rewritemaps;
        a->rewriteconds = overrides->rewriteconds;
        a->rewriterules = overrides->rewriterules;
    }

    return (void *)a;
}


/*
**
**  per-directory configuration structure handling
**
*/

static void *config_perdir_create(pool *p, char *path)
{
    rewrite_perdir_conf *a;

    a = (rewrite_perdir_conf *)pcalloc(p, sizeof(rewrite_perdir_conf));

    a->state           = ENGINE_DISABLED;
    a->options         = OPTION_NONE;
    a->baseurl         = NULL;
    a->rewriteconds    = make_array(p, 2, sizeof(rewritecond_entry));
    a->rewriterules    = make_array(p, 2, sizeof(rewriterule_entry));

    if (path == NULL)
        a->directory = NULL;
    else {
        /* make sure it has a trailing slash */
        if (path[strlen(path)-1] == '/')
            a->directory = pstrdup(p, path);
        else
            a->directory = pstrcat(p, path, "/", NULL);
    }

    return (void *)a;
}

static void *config_perdir_merge(pool *p, void *basev, void *overridesv)
{
    rewrite_perdir_conf *a, *base, *overrides;

    a         = (rewrite_perdir_conf *)pcalloc(p, sizeof(rewrite_perdir_conf));
    base      = (rewrite_perdir_conf *)basev;
    overrides = (rewrite_perdir_conf *)overridesv;

    a->state           = overrides->state;
    a->options         = overrides->options;
    a->directory       = overrides->directory;
    a->baseurl         = overrides->baseurl;

    if (a->options & OPTION_INHERIT) {
        a->rewriteconds = append_arrays(p, overrides->rewriteconds, base->rewriteconds);
        a->rewriterules = append_arrays(p, overrides->rewriterules, base->rewriterules);
    }
    else {
        a->rewriteconds = overrides->rewriteconds;
        a->rewriterules = overrides->rewriterules;
    }

    return (void *)a;
}


/*
**
**  the configuration commands
**
*/

static const char *cmd_rewriteengine(cmd_parms *cmd, rewrite_perdir_conf *dconf, int flag)
{
    rewrite_server_conf *sconf;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);
    if (cmd->path == NULL) /* is server command */
        sconf->state = (flag ? ENGINE_ENABLED : ENGINE_DISABLED);
    else                   /* is per-directory command */
        dconf->state = (flag ? ENGINE_ENABLED : ENGINE_DISABLED);

    return NULL;
}

static const char *cmd_rewriteoptions(cmd_parms *cmd, rewrite_perdir_conf *dconf, char *option)
{
    rewrite_server_conf *sconf;
    const char *err;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);
    if (cmd->path == NULL) /* is server command */
        err = cmd_rewriteoptions_setoption(cmd->pool, &(sconf->options), option);
    else                   /* is per-directory command */
        err = cmd_rewriteoptions_setoption(cmd->pool, &(dconf->options), option);

    return err;
}

static const char *cmd_rewriteoptions_setoption(pool *p, int *options, char *name)
{
    if (strcasecmp(name, "inherit") == 0)
        *options |= OPTION_INHERIT;
    else
        return pstrcat(p, "RewriteOptions: unknown option '", name, "'\n", NULL);
    return NULL;
}

static const char *cmd_rewritelog(cmd_parms *cmd, void *dconf, char *a1)
{
    rewrite_server_conf *sconf;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);
    sconf->rewritelogfile = a1;

    return NULL;
}

static const char *cmd_rewriteloglevel(cmd_parms *cmd, void *dconf, char *a1)
{
    rewrite_server_conf *sconf;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);
    sconf->rewriteloglevel = atoi(a1);

    return NULL;
}

static const char *cmd_rewritemap(cmd_parms *cmd, void *dconf, char *a1, char *a2)
{
    rewrite_server_conf *sconf;
    rewritemap_entry *new;
    struct stat st;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);
    new = push_array(sconf->rewritemaps);

    new->name = a1;
    if (strncmp(a2, "txt:", 4) == 0) {
        new->type      = MAPTYPE_TXT;
        new->datafile  = a2+4;
        new->checkfile = a2+4;
    }
    else if (strncmp(a2, "dbm:", 4) == 0) {
#ifdef HAS_NDBM_LIB
        new->type      = MAPTYPE_DBM;
        new->datafile  = a2+4;
        new->checkfile = pstrcat(cmd->pool, a2+4, NDBM_FILE_SUFFIX, NULL);
#else
        return pstrdup(cmd->pool, "RewriteMap: cannot use NDBM mapfile, because no NDBM support compiled in");
#endif
    }
    else if (strncmp(a2, "prg:", 4) == 0) {
        new->type = MAPTYPE_PRG;
        new->datafile = a2+4;
        new->checkfile = a2+4;
    }
    else {
        new->type      = MAPTYPE_TXT;
        new->datafile  = a2;
        new->checkfile = a2;
    }
    new->fpin  = 0;
    new->fpout = 0;

    if (new->checkfile)
        if (stat(new->checkfile, &st) == -1)
            return pstrcat(cmd->pool, "RewriteMap: map file or program not found:", new->checkfile, NULL);

    return NULL;
}

static const char *cmd_rewritebase(cmd_parms *cmd, rewrite_perdir_conf *dconf, char *a1)
{
    if (cmd->path == NULL || dconf == NULL)
        return "RewriteBase: only valid in per-directory config files";
    if (a1[0] != '/') 
        return "RewriteBase: argument is not a valid URL";
    if (a1[0] == '\0')
        return "RewriteBase: empty URL not allowed";

    dconf->baseurl = pstrdup(cmd->pool, a1);

    return NULL;
}

static const char *cmd_rewritecond(cmd_parms *cmd, rewrite_perdir_conf *dconf, char *str)
{
    rewrite_server_conf *sconf;
    rewritecond_entry *new;
    regex_t *regexp;
    char *a1;
    char *a2;
    char *a3;
    char *cp;
    const char *err;
    int rc;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);

    /*  make a new entry in the internal temporary rewrite rule list */
    if (cmd->path == NULL)   /* is server command */
        new = push_array(sconf->rewriteconds);
    else                     /* is per-directory command */
        new = push_array(dconf->rewriteconds);

    /*  parse the argument line ourself */
    if (parseargline(str, &a1, &a2, &a3)) 
        return pstrcat(cmd->pool, "RewriteCond: bad argument line '", str, "'\n", NULL);

    /*  arg1: the input string */
    new->input = pstrdup(cmd->pool, a1);

    /* arg3: optional flags field 
       (this have to be first parsed, because we need to
        know if the regex should be compiled with ICASE!) */
    new->flags = CONDFLAG_NONE;
    if (a3 != NULL) {
        if ((err = cmd_rewritecond_parseflagfield(cmd->pool, new, a3)) != NULL)
            return err;
    }

    /*  arg2: the pattern
        try to compile the regexp to test if is ok */
    cp = a2;
    if (cp[0] == '!') {
        new->flags |= CONDFLAG_NOTMATCH;
        cp++;
    }

    /* now be careful: Under the POSIX regex library
       we can compile the pattern for case-insensitive matching,
       under the old V8 library we have to do it self via a hack */
    if (new->flags & CONDFLAG_NOCASE)
        rc = ((regexp = pregcomp(cmd->pool, cp, REG_EXTENDED|REG_ICASE)) == NULL);
    else
        rc = ((regexp = pregcomp(cmd->pool, cp, REG_EXTENDED)) == NULL);
    if (rc)
        return pstrcat(cmd->pool, "RewriteCond: cannot compile regular expression '", a2, "'\n", NULL);
    new->pattern = pstrdup(cmd->pool, cp);
    new->regexp  = regexp;

    return NULL;
}

static const char *cmd_rewritecond_parseflagfield(pool *p, rewritecond_entry *cfg, char *str)
{
    char *cp;
    char *cp1;
    char *cp2;
    char *cp3;
    char *key;
    char *val;
    const char *err;

    if (str[0] != '[' || str[strlen(str)-1] != ']')
        return pstrdup(p, "RewriteCond: bad flag delimiters");

    cp = str+1;
    str[strlen(str)-1] = ','; /* for simpler parsing */
    for ( ; *cp != '\0'; ) {
        /* skip whitespaces */
        for ( ; (*cp == ' ' || *cp == '\t') && *cp != '\0'; cp++)
            ;
        if (*cp == '\0')
            break;
        cp1 = cp;
        if ((cp2 = strchr(cp, ',')) != NULL) {
            cp = cp2+1;
            for ( ; (*(cp2-1) == ' ' || *(cp2-1) == '\t'); cp2--)
                ;
            *cp2 = '\0';
            if ((cp3 = strchr(cp1, '=')) != NULL) {
                *cp3 = '\0';
                key = cp1;
                val = cp3+1;
            }
            else {
                key = cp1;
                val = "";
            }
            if ((err = cmd_rewritecond_setflag(p, cfg, key, val)) != NULL)
                return err;
        }
        else
            break;
    }
    
    return NULL;
}

static const char *cmd_rewritecond_setflag(pool *p, rewritecond_entry *cfg, char *key, char *val)
{
    if (   strcasecmp(key, "nocase") == 0
        || strcasecmp(key, "NC") == 0    ) {
        cfg->flags |= CONDFLAG_NOCASE;
    }
    else if (   strcasecmp(key, "ornext") == 0
             || strcasecmp(key, "OR") == 0    ) {
        cfg->flags |= CONDFLAG_ORNEXT;
    }
    else {
        return pstrcat(p, "RewriteCond: unknown flag '", key, "'\n", NULL);
    }
    return NULL;
}

/* NON static */
const char *cmd_rewriterule(cmd_parms *cmd, rewrite_perdir_conf *dconf, char *str)
{
    rewrite_server_conf *sconf;
    rewriterule_entry *new;
    regex_t *regexp;
    char *a1;
    char *a2;
    char *a3;
    char *cp;
    const char *err;

    sconf = (rewrite_server_conf *)get_module_config(cmd->server->module_config, &rewrite_module);

    /*  make a new entry in the internal rewrite rule list */
    if (cmd->path == NULL)   /* is server command */
        new = push_array(sconf->rewriterules);
    else                     /* is per-directory command */
        new = push_array(dconf->rewriterules);

    /*  parse the argument line ourself */
    if (parseargline(str, &a1, &a2, &a3)) 
        return pstrcat(cmd->pool, "RewriteRule: bad argument line '", str, "'\n", NULL);

    /*  arg1: the pattern
        try to compile the regexp to test if is ok */
    new->flags = RULEFLAG_NONE;
    cp = a1;
    if (cp[0] == '!') {
        new->flags |= RULEFLAG_NOTMATCH;
        cp++;
    }
    if ((regexp = pregcomp(cmd->pool, cp, REG_EXTENDED)) == NULL)
        return pstrcat(cmd->pool, "RewriteRule: cannot compile regular expression '", a1, "'\n", NULL);
    new->pattern = pstrdup(cmd->pool, cp);
    new->regexp  = regexp;

    /*  arg2: the output string
        replace the $<N> by \<n> which is needed by the currently
        used Regular Expression library */
    new->output = pstrdup(cmd->pool, a2);

    /* arg3: optional flags field */
    new->forced_mimetype = NULL;
    new->forced_responsecode = HTTP_MOVED_TEMPORARILY;
    new->env[0] = NULL;
    new->skip = 0;
    if (a3 != NULL) {
        if ((err = cmd_rewriterule_parseflagfield(cmd->pool, new, a3)) != NULL)
            return err;
    }

    /* now, if the server or per-dir config holds an
       array of RewriteCond entries, we take it for us 
       and clear the array */
    if (cmd->path == NULL) {  /* is server command */
        new->rewriteconds   = sconf->rewriteconds;
        sconf->rewriteconds = make_array(cmd->pool, 2, sizeof(rewritecond_entry));
    }
    else {                    /* is per-directory command */
        new->rewriteconds   = dconf->rewriteconds;
        dconf->rewriteconds = make_array(cmd->pool, 2, sizeof(rewritecond_entry));
    }

    return NULL;
}

static const char *cmd_rewriterule_parseflagfield(pool *p, rewriterule_entry *cfg, char *str)
{
    char *cp;
    char *cp1;
    char *cp2;
    char *cp3;
    char *key;
    char *val;
    const char *err;

    if (str[0] != '[' || str[strlen(str)-1] != ']')
        return pstrdup(p, "RewriteRule: bad flag delimiters");

    cp = str+1;
    str[strlen(str)-1] = ','; /* for simpler parsing */
    for ( ; *cp != '\0'; ) {
        /* skip whitespaces */
        for ( ; (*cp == ' ' || *cp == '\t') && *cp != '\0'; cp++)
            ;
        if (*cp == '\0')
            break;
        cp1 = cp;
        if ((cp2 = strchr(cp, ',')) != NULL) {
            cp = cp2+1;
            for ( ; (*(cp2-1) == ' ' || *(cp2-1) == '\t'); cp2--)
                ;
            *cp2 = '\0';
            if ((cp3 = strchr(cp1, '=')) != NULL) {
                *cp3 = '\0';
                key = cp1;
                val = cp3+1;
            }
            else {
                key = cp1;
                val = "";
            }
            if ((err = cmd_rewriterule_setflag(p, cfg, key, val)) != NULL)
                return err;
        }
        else
            break;
    }
    
    return NULL;
}

static const char *cmd_rewriterule_setflag(pool *p, rewriterule_entry *cfg, char *key, char *val)
{
    int status = 0;
    int i;

    if (   strcasecmp(key, "redirect") == 0
        || strcasecmp(key, "R") == 0       ) {
        cfg->flags |= RULEFLAG_FORCEREDIRECT;
        if (strlen(val) > 0) {
            if (strcasecmp(val, "permanent") == 0)
                status = HTTP_MOVED_PERMANENTLY;
            else if (strcasecmp(val, "temp") == 0)
                status = HTTP_MOVED_TEMPORARILY;
            else if (strcasecmp(val, "seeother") == 0)
                status = HTTP_SEE_OTHER;
            else if (isdigit(*val))
                status = atoi(val);
            if (!is_HTTP_REDIRECT(status))
                return pstrdup(p, "RewriteRule: invalid HTTP response code for flag 'R'");
            cfg->forced_responsecode = status;
        }
    }
    else if (   strcasecmp(key, "last") == 0
             || strcasecmp(key, "L") == 0   ) {
        cfg->flags |= RULEFLAG_LASTRULE;
    }
    else if (   strcasecmp(key, "next") == 0
             || strcasecmp(key, "N") == 0   ) {
        cfg->flags |= RULEFLAG_NEWROUND;
    }
    else if (   strcasecmp(key, "chain") == 0
             || strcasecmp(key, "C") == 0    ) {
        cfg->flags |= RULEFLAG_CHAIN;
    }
    else if (   strcasecmp(key, "type") == 0
             || strcasecmp(key, "T") == 0   ) {
        cfg->forced_mimetype = pstrdup(p, val);
    }
    else if (   strcasecmp(key, "env") == 0
             || strcasecmp(key, "E") == 0   ) {
        for (i = 0; (cfg->env[i] != NULL) && (i < MAX_ENV_FLAGS); i++)
            ;
        if (i < MAX_ENV_FLAGS) {
            cfg->env[i] = pstrdup(p, val);
            cfg->env[i+1] = NULL;
        }
        else 
            return pstrdup(p, "RewriteRule: to much environment flags 'E'");
    }
    else if (   strcasecmp(key, "nosubreq") == 0
             || strcasecmp(key, "NS") == 0      ) {
        cfg->flags |= RULEFLAG_IGNOREONSUBREQ;
    }
    else if (   strcasecmp(key, "proxy") == 0
             || strcasecmp(key, "P") == 0      ) {
        cfg->flags |= RULEFLAG_PROXY;
    }
    else if (   strcasecmp(key, "passthrough") == 0
             || strcasecmp(key, "PT") == 0      ) {
        cfg->flags |= RULEFLAG_PASSTHROUGH;
    }
    else if (   strcasecmp(key, "skip") == 0
             || strcasecmp(key, "S") == 0   ) {
        cfg->skip = atoi(val);
    }
    else if (   strcasecmp(key, "forbidden") == 0
             || strcasecmp(key, "F") == 0   ) {
        cfg->flags |= RULEFLAG_FORBIDDEN;
    }
    else if (   strcasecmp(key, "gone") == 0
             || strcasecmp(key, "G") == 0   ) {
        cfg->flags |= RULEFLAG_GONE;
    }
    else if (   strcasecmp(key, "qsappend") == 0
             || strcasecmp(key, "QSA") == 0   ) {
        cfg->flags |= RULEFLAG_QSAPPEND;
    }
    else {
        return pstrcat(p, "RewriteRule: unknown flag '", key, "'\n", NULL);
    }
    return NULL;
}


/*
**
**  module initialisation 
**  [called from read_config() after all 
**  config commands were already called]
**
*/

static void init_module(server_rec *s, pool *p)
{
    /* step through the servers and
       - open eachs rewriting logfile 
       - open the RewriteMap prg:xxx programs */
    for (; s; s = s->next) {
        open_rewritelog(s, p);
        run_rewritemap_programs(s, p);
    }

    /* create the lookup cache */
    cachep = init_cache(p);

    /* check if proxy module is available */
    proxy_available = is_proxy_available(s);

    /* precompile a static pattern 
       for the txt mapfile parsing */
    lookup_map_txtfile_regexp = pregcomp(p, MAPFILE_PATTERN, REG_EXTENDED);
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |                     runtime hooks
** |                                                       |
** +-------------------------------------------------------+
*/


/*
**
**  URI-to-filename hook
**
**  [used for the rewriting engine triggered by
**  the per-server 'RewriteRule' directives]
**
*/

static int hook_uri2file(request_rec *r)
{
    void *sconf;
    rewrite_server_conf *conf;
    char *var;
    char *thisserver, *thisport, *thisurl;
    char buf[512];
    char docroot[512];
    char *cp, *cp2;
    struct stat finfo;
    int n;
    int l;

    /*
     *  retrieve the config structures
     */
    sconf = r->server->module_config;
    conf  = (rewrite_server_conf *)get_module_config(sconf, &rewrite_module);

    /*
     *  only do something under runtime if the engine is really enabled,
     *  else return immediately!
     */
    if (conf->state == ENGINE_DISABLED)
        return DECLINED;

    /*
     *  add the SCRIPT_URL variable to the env. this is a bit complicated
     *  due to the fact that apache uses subrequests and internal redirects
     */

    if (r->main == NULL) {
         var = pstrcat(r->pool, "REDIRECT_", ENVVAR_SCRIPT_URL, NULL);
         var = table_get(r->subprocess_env, var);
         if (var == NULL) 
             table_set(r->subprocess_env, ENVVAR_SCRIPT_URL, pstrdup(r->pool, r->uri));
         else 
             table_set(r->subprocess_env, ENVVAR_SCRIPT_URL, pstrdup(r->pool, var));
    } 
    else {
         var = table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         table_set(r->subprocess_env, ENVVAR_SCRIPT_URL, pstrdup(r->pool, var));
    }

    /*
     *  create the SCRIPT_URI variable for the env
     */

    /* add the canonical URI of this URL */
    thisserver = r->server->server_hostname;
#ifdef APACHE_SSL
    if (((!r->connection->client->ssl) && (r->server->port == DEFAULT_PORT)) ||
         ((r->connection->client->ssl) && (r->server->port == 443)))
#else
    if (r->server->port == DEFAULT_PORT)
#endif 
        thisport = "";
    else {
        ap_snprintf(buf, sizeof(buf), ":%u", r->server->port);
        thisport = pstrdup(r->pool, buf);
    }
    thisurl = table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);

    /* set the variable */
#ifdef APACHE_SSL
    var = pstrcat(r->pool, http_method(r), "://", thisserver, thisport, thisurl, NULL);
#else
    var = pstrcat(r->pool, "http://", thisserver, thisport, thisurl, NULL);
#endif
    table_set(r->subprocess_env, ENVVAR_SCRIPT_URI, pstrdup(r->pool, var));


    /* if filename was not initially set,
       we start with the requested URI */
    if (r->filename == NULL) {
        r->filename = pstrdup(r->pool, r->uri);
        rewritelog(r, 2, "init rewrite engine with requested uri %s", r->filename);
    }

    /*
     *  now apply the rules ... 
     */
    if (apply_rewrite_list(r, conf->rewriterules, NULL)) {

        if (strlen(r->filename) > 6 &&
            strncmp(r->filename, "proxy:", 6) == 0) {
            /* it should be go on as an internal proxy request */

            /* check if the proxy module is enabled, so
               we can actually use it! */
            if (!proxy_available) {
		log_reason("attempt to make remote request from mod_rewrite "
    	    	    "without proxy enabled", r->filename, r);
                return FORBIDDEN;
    	    }

            /* make sure the QUERY_STRING and
               PATH_INFO parts get incorporated */
            r->filename = pstrcat(r->pool, r->filename, 
                                           r->path_info ? r->path_info : "", 
                                           r->args ? "?" : NULL, r->args, 
                                           NULL);

            /* now make sure the request gets handled by the
               proxy handler */
            r->proxyreq = 1;
            r->handler  = "proxy-server";

            rewritelog(r, 1, "go-ahead with proxy request %s [OK]", r->filename);
            return OK; 
        }
        else if (  (strlen(r->filename) > 7 &&
                    strncasecmp(r->filename, "http://", 7) == 0)
                || (strlen(r->filename) > 8 &&
                    strncasecmp(r->filename, "https://", 8) == 0)
                || (strlen(r->filename) > 9 &&
                    strncasecmp(r->filename, "gopher://", 9) == 0)
                || (strlen(r->filename) > 6 &&
                    strncasecmp(r->filename, "ftp://", 6) == 0)    ) {
            /* it was finally rewritten to a remote URL */

            /* skip 'scheme:' */
            for (cp = r->filename; *cp != ':' && *cp != '\0'; cp++)
                ;
            /* skip '://' */
            cp += 3;
            /* skip host part */
            for ( ; *cp != '/' && *cp != '\0'; cp++)
                ;
            if (*cp != '\0') {
                rewritelog(r, 1, "escaping %s for redirect", r->filename);
                cp2 = escape_uri(r->pool, cp);
                *cp = '\0';
                r->filename = pstrcat(r->pool, r->filename, cp2, NULL);
            }

            /* append the QUERY_STRING part */
            if (r->args != NULL)
               r->filename = pstrcat(r->pool, r->filename, "?", r->args, NULL);

            /* determine HTTP redirect response code */
            if (is_HTTP_REDIRECT(r->status)) {
                n = r->status; 
                r->status = HTTP_OK; /* make Apache kernel happy */
            }
            else
                n = REDIRECT;

            /* now do the redirection */
            table_set(r->headers_out, "Location", r->filename);
            rewritelog(r, 1, "redirect to %s [REDIRECT/%d]", r->filename, n);
            return n;
        }
        else if (strlen(r->filename) > 10 &&
                 strncmp(r->filename, "forbidden:", 10) == 0) {
            /* This URLs is forced to be forbidden for the requester */
            return FORBIDDEN; 
        }
        else if (strlen(r->filename) > 5 &&
                 strncmp(r->filename, "gone:", 5) == 0) {
            /* This URLs is forced to be gone */
            return HTTP_GONE; 
        }
        else if (strlen(r->filename) > 12 &&
                 strncmp(r->filename, "passthrough:", 12) == 0) {
            /* Hack because of underpowered API: passing the current
               rewritten filename through to other URL-to-filename handlers
               just as it were the requested URL. This is to enable
               post-processing by mod_alias, etc.  which always act on
               r->uri! The difference here is: We do not try to
               add the document root */
            r->uri = pstrdup(r->pool, r->filename+12);
            return DECLINED; 
        }
        else {
            /* it was finally rewritten to a local path */

            /* expand "/~user" prefix */
            r->filename = expand_tildepaths(r, r->filename);  

            rewritelog(r, 2, "local path result: %s", r->filename);

            /* the filename has to start with a slash! */
            if (r->filename[0] != '/')
                return BAD_REQUEST;

            /* if there is no valid prefix, we have 
               to emulate the translator from the core and
               prefix the filename with document_root

               NOTICE:
               We cannot leave out the prefix_stat because
               - when we always prefix with document_root
                 then no absolute path can be created, e.g. via 
                 emulating a ScriptAlias directive, etc.
               - when we always NOT prefix with document_root
                 then the files under document_root have to
                 be references directly and document_root
                 gets never used and will be a dummy parameter -
                 this is also bad

               BUT:
               Under real Unix systems this is no problem,
               because we only do stat() on the first directory
               and this gets cached by the kernel for along time!
             */
            n = prefix_stat(r->filename, &finfo);
            if (n == 0) {
                if ((cp = document_root(r)) != NULL) {
                    strncpy(docroot, cp, sizeof(docroot)-1);
                    EOS_PARANOIA(docroot);

                    /* always NOT have a trailing slash */
                    l = strlen(docroot);
                    if (docroot[l-1] == '/') {
                        docroot[l-1] = '\0';
                    }
                    if (r->server->path && !strncmp(r->filename, r->server->path, r->server->pathlen))
                        r->filename = pstrcat(r->pool, docroot, (r->filename + r->server->pathlen), NULL);
                    else
                        r->filename = pstrcat(r->pool, docroot, r->filename, NULL);
                    rewritelog(r, 2, "prefixed with document_root to %s", r->filename);
                }
            }

            rewritelog(r, 1, "go-ahead with %s [OK]", r->filename);
            return OK;
        }
    }
    else {
        rewritelog(r, 1, "pass through %s", r->filename);
        return DECLINED;
    }
}


/*
**
**  MIME-type hook
**
**  [used to support the forced-MIME-type feature]
**
*/

static int hook_mimetype(request_rec *r)
{
    char *t;
    
    /* now check if we have to force a MIME-type */
    t = table_get(r->notes, REWRITE_FORCED_MIMETYPE_NOTEVAR);
    if (t == NULL) 
        return DECLINED;
    else {
        rewritelog(r, 1, "force filename %s to have MIME-type '%s'", r->filename, t);
        r->content_type = t;
        return OK;
    }
}


/*
**
**  Fixup hook
**
**  [used for the rewriting engine triggered by
**  the per-directory 'RewriteRule' directives]
**
*/

static int hook_fixup(request_rec *r)
{
    rewrite_perdir_conf *dconf;
    char *cp;
    char *cp2;
    char *prefix;
    int l;
    int n;
    char *ofilename;

    dconf = (rewrite_perdir_conf *)get_module_config(r->per_dir_config, &rewrite_module);

    /* if there is no per-dir config we return immediately */
    if (dconf == NULL)
        return DECLINED;

    /* we shouldn't do anything in subrequests */
    if (r->main != NULL) 
        return DECLINED;

    /* if there are no real (i.e. no RewriteRule directives!)
       per-dir config of us, we return also immediately */
    if (dconf->directory == NULL) 
        return DECLINED;

    /*
     *  only do something under runtime if the engine is really enabled,
     *  for this directory, else return immediately!
     */
    if (!(allow_options(r) & (OPT_SYM_LINKS | OPT_SYM_OWNER))) {
        /* FollowSymLinks is mandatory! */
        log_reason("Options FollowSymLinks or SymLinksIfOwnerMatch is off which implies that RewriteRule directive is forbidden", r->filename, r);
        return FORBIDDEN;
    }
    else {
        /* FollowSymLinks is given, but the user can
           still turn off the rewriting engine */
        if (dconf->state == ENGINE_DISABLED)
            return DECLINED;
    }

    /*
     *  remember the current filename before rewriting for later check
     *  to prevent deadlooping because of internal redirects
     *  on final URL/filename which can be equal to the inital one.
     */
    ofilename = r->filename;

    /*
     *  now apply the rules ... 
     */
    if (apply_rewrite_list(r, dconf->rewriterules, dconf->directory)) {

        if (strlen(r->filename) > 6 &&
            strncmp(r->filename, "proxy:", 6) == 0) {
            /* it should go on as an internal proxy request */

            /* make sure the QUERY_STRING and
               PATH_INFO parts get incorporated */
            r->filename = pstrcat(r->pool, r->filename, 
                                           /* r->path_info was already
                                              appended by the rewriting engine
                                              because of the per-dir context! */
                                           r->args ? "?" : NULL, r->args, 
                                           NULL);

            /* now make sure the request gets handled by the
               proxy handler */
            r->proxyreq = 1;
            r->handler  = "proxy-server";

            rewritelog(r, 1, "[per-dir %s] go-ahead with proxy request %s [OK]", dconf->directory, r->filename);
            return OK; 
        }
        else if (  (strlen(r->filename) > 7 &&
                    strncasecmp(r->filename, "http://", 7) == 0)
                || (strlen(r->filename) > 8 &&
                    strncasecmp(r->filename, "https://", 8) == 0)
                || (strlen(r->filename) > 9 &&
                    strncasecmp(r->filename, "gopher://", 9) == 0)
                || (strlen(r->filename) > 6 &&
                    strncasecmp(r->filename, "ftp://", 6) == 0)    ) {
            /* it was finally rewritten to a remote URL */

            /* because we are in a per-dir context
               first try to replace the directory with its base-URL
               if there is a base-URL available */
            if (dconf->baseurl != NULL) {
                /* skip 'scheme:' */
                for (cp = r->filename; *cp != ':' && *cp != '\0'; cp++)
                    ;
                /* skip '://' */
                cp += 3;
                if ((cp = strchr(cp, '/')) != NULL) {
                    rewritelog(r, 2, "[per-dir %s] trying to replace prefix %s with %s", dconf->directory, dconf->directory, dconf->baseurl);
                    cp2 = subst_prefix_path(r, cp, dconf->directory, dconf->baseurl);
                    if (strcmp(cp2, cp) != 0) {
                        *cp = '\0';
                        r->filename = pstrcat(r->pool, r->filename, cp2, NULL);
                    }
                }
            }

            /* now prepare the redirect... */

            /* skip 'scheme:' */
            for (cp = r->filename; *cp != ':' && *cp != '\0'; cp++)
                ;
            /* skip '://' */
            cp += 3;
            /* skip host part */
            for ( ; *cp != '/' && *cp != '\0'; cp++)
                ;
            if (*cp != '\0') {
                rewritelog(r, 1, "[per-dir %s] escaping %s for redirect", dconf->directory, r->filename);
                cp2 = escape_uri(r->pool, cp);
                *cp = '\0';
                r->filename = pstrcat(r->pool, r->filename, cp2, NULL);
            }

            /* append the QUERY_STRING part */
            if (r->args != NULL)
               r->filename = pstrcat(r->pool, r->filename, "?", r->args, NULL);

            /* determine HTTP redirect response code */
            if (is_HTTP_REDIRECT(r->status)) {
                n = r->status; 
                r->status = HTTP_OK; /* make Apache kernel happy */
            }
            else
                n = REDIRECT;

            /* now do the redirection */
            table_set(r->headers_out, "Location", r->filename);
            rewritelog(r, 1, "[per-dir %s] redirect to %s [REDIRECT/%d]", dconf->directory, r->filename, n);
            return n;
        }
        else if (strlen(r->filename) > 10 &&
                 strncmp(r->filename, "forbidden:", 10) == 0) {
            /* This URLs is forced to be forbidden for the requester */
            return FORBIDDEN; 
        }
        else if (strlen(r->filename) > 5 &&
                 strncmp(r->filename, "gone:", 5) == 0) {
            /* This URLs is forced to be gone */
            return HTTP_GONE; 
        }
        else {
            /* it was finally rewritten to a local path */

            /* if someone used the PASSTHROUGH flag in per-dir
               context we just ignore it. It is only useful
               in per-server context */
            if (strlen(r->filename) > 12 &&
                strncmp(r->filename, "passthrough:", 12) == 0) {
                r->filename = pstrdup(r->pool, r->filename+12);
            }

            /* the filename has to start with a slash! */
            if (r->filename[0] != '/')
                return BAD_REQUEST;

            /* Check for deadlooping:
             * At this point we KNOW that at least one rewriting
             * rule was applied, but when the resulting URL is
             * the same as the initial URL, we are not allowed to
             * use the following internal redirection stuff because
             * this would lead to a deadloop.
             */
            if (strcmp(r->filename, ofilename) == 0) {
                rewritelog(r, 1, "[per-dir %s] initial URL equal rewritten URL: %s [IGNORING REWRITE]", dconf->directory, r->filename);
                return OK;
            }

            /* if there is a valid base-URL then substitute
               the per-dir prefix with this base-URL if the
               current filename still is inside this per-dir 
               context. If not then treat the result as a 
               plain URL */
            if (dconf->baseurl != NULL) {
                rewritelog(r, 2, "[per-dir %s] trying to replace prefix %s with %s", dconf->directory, dconf->directory, dconf->baseurl);
                r->filename = subst_prefix_path(r, r->filename, dconf->directory, dconf->baseurl);
            }
            else {
                /* if no explicit base-URL exists we assume
                   that the directory prefix is also a valid URL
                   for this webserver and only try to remove the
                   document_root if it is prefix */

                if ((cp = document_root(r)) != NULL) {
                    prefix = pstrdup(r->pool, cp);
                    /* always NOT have a trailing slash */
                    l = strlen(prefix);
                    if (prefix[l-1] == '/') {
                        prefix[l-1] = '\0';
                        l--;
                    }
                    if (strncmp(r->filename, prefix, l) == 0) {
                        rewritelog(r, 2, "[per-dir %s] strip document_root prefix: %s -> %s", dconf->directory, r->filename, r->filename+l);
                        r->filename = pstrdup(r->pool, r->filename+l); 
                    }
                }
            }

            /* now initiate the internal redirect */
            rewritelog(r, 1, "[per-dir %s] internal redirect with %s [INTERNAL REDIRECT]", dconf->directory, r->filename);
            r->filename = pstrcat(r->pool, "redirect:", r->filename, NULL);
            r->handler = "redirect-handler";
            return OK; 
        }
    }
    else {
        rewritelog(r, 1, "[per-dir %s] pass through %s", dconf->directory, r->filename);
        return DECLINED;
    }
}


/*
**
**  Content-Handlers
**
**  [used for redirect support]
**
*/

static int handler_redirect(request_rec *r)
{
    /* just make sure that we are really meant! */
    if (strncmp(r->filename, "redirect:", 9) != 0) 
        return DECLINED;

    /* now do the internal redirect */
    internal_redirect(pstrcat(r->pool, r->filename+9, 
                                       r->args ? "?" : NULL, r->args, NULL), r);

    /* and return gracefully */
    return OK;
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |                  rewriting engine
** |                                                       |
** +-------------------------------------------------------+
*/


static int apply_rewrite_list(request_rec *r, array_header *rewriterules, char *perdir)
{
    rewriterule_entry *entries;
    rewriterule_entry *p;
    int i;
    int changed;
    int rc;
    int s;
    
    entries = (rewriterule_entry *)rewriterules->elts;
    changed = 0;
    loop:
    for (i = 0; i < rewriterules->nelts; i++) {
        p = &entries[i];

        /* ignore this rule on subrequests if we are explicitly asked to do so
           or this is a proxy throughput or a forced redirect rule */
        if (r->main != NULL &&
            (p->flags & RULEFLAG_IGNOREONSUBREQ ||
             p->flags & RULEFLAG_PROXY          ||
             p->flags & RULEFLAG_FORCEREDIRECT    ))
            continue;

        /* apply the current rule */
        rc = apply_rewrite_rule(r, p, perdir);
        if (rc) {
            if (rc != 2) /* not a match-only rule */
                changed = 1;
            if (p->flags & RULEFLAG_PASSTHROUGH) {
                rewritelog(r, 2, "forcing '%s' to get passed through to next URI-to-filename handler", r->filename);
                r->filename = pstrcat(r->pool, "passthrough:", r->filename, NULL);
                changed = 1;
                break;
            }
            if (p->flags & RULEFLAG_FORBIDDEN) {
                rewritelog(r, 2, "forcing '%s' to be forbidden", r->filename);
                r->filename = pstrcat(r->pool, "forbidden:", r->filename, NULL);
                changed = 1;
                break;
            }
            if (p->flags & RULEFLAG_GONE) {
                rewritelog(r, 2, "forcing '%s' to be gone", r->filename);
                r->filename = pstrcat(r->pool, "gone:", r->filename, NULL);
                changed = 1;
                break;
            }
            if (p->flags & RULEFLAG_PROXY) 
                break;
            if (p->flags & RULEFLAG_LASTRULE) 
                break;
            if (p->flags & RULEFLAG_NEWROUND) 
                goto loop;

            /* if we are forced to skip N next rules, do it now */
            if (p->skip > 0) {
                s = p->skip;
                while (   i < rewriterules->nelts
                       && s > 0) {
                    i++;
                    p = &entries[i];
                    s--;
                }
            }
        }
        else {
            /* if current rule is chained with next rule(s),
               skip all this next rule(s) */
            while (   i < rewriterules->nelts
                   && p->flags & RULEFLAG_CHAIN) {
                i++;
                p = &entries[i];
            }
        }
    }
    return changed;
}

static int apply_rewrite_rule(request_rec *r, rewriterule_entry *p, char *perdir)
{
    char *uri;
    char *output;
    int flags;
    char newuri[MAX_STRING_LEN];
    char env[MAX_STRING_LEN];
    char port[32];
    char env2[MAX_STRING_LEN];
    regex_t *regexp;
    regmatch_t regmatch[10];
    int rc;
    int prefixstrip;
    int i;
    int failed;
    array_header *rewriteconds;
    rewritecond_entry *conds;
    rewritecond_entry *c;

    uri     = r->filename;
    regexp  = p->regexp;
    output  = p->output;
    flags   = p->flags;

    if (perdir != NULL && r->path_info != NULL && r->path_info[0] != '\0') {
        rewritelog(r, 3, "[per-dir %s] add path-info postfix: %s -> %s%s", perdir, uri, uri, r->path_info);
        uri = pstrcat(r->pool, uri, r->path_info, NULL);
    }

    prefixstrip = 0;
    if (perdir != NULL) {
        /* this is a per-directory match */
        if (   strlen(uri) >= strlen(perdir)
            && strncmp(uri, perdir, strlen(perdir)) == 0) {
            rewritelog(r, 3, "[per-dir %s] strip per-dir prefix: %s -> %s", perdir, uri, uri+strlen(perdir));
            uri = uri+strlen(perdir);
            prefixstrip = 1;
        }
    }

    if (perdir != NULL) 
        rewritelog(r, 3, "[per-dir %s] applying pattern '%s' to uri '%s'", perdir, p->pattern, uri);

    rc = (regexec(regexp, uri, regexp->re_nsub+1, regmatch, 0) == 0);   /* try to match the pattern */
    if (( rc && !(p->flags & RULEFLAG_NOTMATCH)) ||
        (!rc &&  (p->flags & RULEFLAG_NOTMATCH))   ) {     

        /* ok, the pattern matched, but we now additionally have to check 
           for any preconditions which have to be also true. We do this
           at this very late stage to avoid unnessesary checks which
           slow down the rewriting engine!! */
        rewriteconds = p->rewriteconds;
        conds = (rewritecond_entry *)rewriteconds->elts;
        failed = 0;
        for (i = 0; i < rewriteconds->nelts; i++) {
            c = &conds[i];
            rc = apply_rewrite_cond(r, c, perdir);
            if (c->flags & CONDFLAG_ORNEXT) {
                /* there is a "or" flag */
                if (rc == 0) {
                    /* one cond is false, but another can be true... */
                    continue;
                }
                else {
                    /* one true cond is enough, so skip the other conds
                       of the "ornext" chained conds */
                    while (   i < rewriteconds->nelts
                           && c->flags & CONDFLAG_ORNEXT) {
                        i++;
                        c = &conds[i];
                    }
                    continue;
                }
            }
            else {
                /* no "or" flag, so a single fail means total fail */
                if (rc == 0) { /* failed */
                    failed = 1;
                    break;
                }
            }
        }
        if (failed) 
            return 0; /* if any condition fails this complete rule fails */

        /* if this is a pure matching rule we return immediately */
        if (strcmp(output, "-") == 0) {
            /* but before we set the env variables... */
            for (i = 0; p->env[i] != NULL; i++) {
                strncpy(env2, p->env[i], sizeof(env2)-1);
                EOS_PARANOIA(env2);
                strncpy(env, pregsub(r->pool, env2, uri, regexp->re_nsub+1, regmatch), sizeof(env)-1);    /* substitute in output */
                EOS_PARANOIA(env);
                add_env_variable(r, env);
            }
            return 2;
        }

        /* if this is a forced proxy request ... */
        if (p->flags & RULEFLAG_PROXY) {
            if (p->flags & RULEFLAG_NOTMATCH) {
                output = pstrcat(r->pool, "proxy:", output, NULL);
                strncpy(newuri, output, sizeof(newuri)-1);
                EOS_PARANOIA(newuri);
                expand_variables_inbuffer(r, newuri, sizeof(newuri));/* expand %{...} */
                expand_map_lookups(r, newuri, sizeof(newuri));       /* expand ${...} */
            }
            else {
                output = pstrcat(r->pool, "proxy:", output, NULL);
                strncpy(newuri, pregsub(r->pool, output, uri, regexp->re_nsub+1, regmatch), sizeof(newuri)-1);    /* substitute in output */
                EOS_PARANOIA(newuri);
                for (i = 0; p->env[i] != NULL; i++) {
                    strncpy(env2, p->env[i], sizeof(env2)-1);
                    EOS_PARANOIA(env2);
                    strncpy(env, pregsub(r->pool, env2, uri, regexp->re_nsub+1, regmatch), sizeof(env)-1);    /* substitute in output */
                    EOS_PARANOIA(env);
                    add_env_variable(r, env);
                }
                expand_variables_inbuffer(r, newuri, sizeof(newuri));   /* expand %{...} */
                expand_map_lookups(r, newuri, sizeof(newuri));          /* expand ${...} */
            }
            if (perdir == NULL)
                rewritelog(r, 2, "rewrite %s -> %s", r->filename, newuri);
            else
                rewritelog(r, 2, "[per-dir %s] rewrite %s -> %s", perdir, r->filename, newuri);
            r->filename = pstrdup(r->pool, newuri);
            return 1;
        }

        /* if this is an implicit redirect in a per-dir rule */
        i = strlen(output);
        if (perdir != NULL
            && (   (i > 7 && strncasecmp(output, "http://", 7) == 0)
                || (i > 8 && strncasecmp(output, "https://", 8) == 0)
                || (i > 9 && strncasecmp(output, "gopher://", 9) == 0)
                || (i > 6 && strncasecmp(output, "ftp://", 6) == 0)   ) ) {
            if (p->flags & RULEFLAG_NOTMATCH) {
                strncpy(newuri, output, sizeof(newuri)-1);
                EOS_PARANOIA(newuri);
                expand_variables_inbuffer(r, newuri, sizeof(newuri));/* expand %{...} */
                expand_map_lookups(r, newuri, sizeof(newuri));       /* expand ${...} */
            }
            else {
                strncpy(newuri, pregsub(r->pool, output, uri, regexp->re_nsub+1, regmatch), sizeof(newuri)-1);    /* substitute in output */
                EOS_PARANOIA(newuri);
                for (i = 0; p->env[i] != NULL; i++) {
                    strncpy(env2, p->env[i], sizeof(env2)-1);
                    EOS_PARANOIA(env2);
                    strncpy(env, pregsub(r->pool, env2, uri, regexp->re_nsub+1, regmatch), sizeof(env)-1);    /* substitute in output */
                    EOS_PARANOIA(env);
                    add_env_variable(r, env);
                }
                expand_variables_inbuffer(r, newuri, sizeof(newuri));/* expand %{...} */
                expand_map_lookups(r, newuri, sizeof(newuri));       /* expand ${...} */
            }
            rewritelog(r, 2, "[per-dir %s] redirect %s -> %s", perdir, r->filename, newuri);
            r->filename = pstrdup(r->pool, newuri);
            splitout_queryargs(r, p->flags & RULEFLAG_QSAPPEND);
            r->status = p->forced_responsecode;
            return 1;
        }

        /* add again the previously stripped perdir prefix if the new 
           URI is not a new one (i.e. prefixed by a slash which means 
           that it is not for this per-dir context)                    */
        if (prefixstrip && output[0] != '/') {
            rewritelog(r, 3, "[per-dir %s] add per-dir prefix: %s -> %s%s", perdir, output, perdir, output);
            output = pstrcat(r->pool, perdir, output, NULL);
        }

        if (p->flags & RULEFLAG_NOTMATCH) {
            /* just overtake the URI */
            strncpy(newuri, output, sizeof(newuri)-1);
            EOS_PARANOIA(newuri);
        }
        else {
            /* substitute in output */
            strncpy(newuri, pregsub(r->pool, output, uri, regexp->re_nsub+1, regmatch), sizeof(newuri)-1);    /* substitute in output */
            EOS_PARANOIA(newuri);
            for (i = 0; p->env[i] != NULL; i++) {
                strncpy(env2, p->env[i], sizeof(env2)-1);
                EOS_PARANOIA(env2);
                strncpy(env, pregsub(r->pool, env2, uri, regexp->re_nsub+1, regmatch), sizeof(env)-1);    /* substitute in output */
                EOS_PARANOIA(env);
                add_env_variable(r, env);
            }
        }
        expand_variables_inbuffer(r, newuri, sizeof(newuri));  /* expand %{...} */
        expand_map_lookups(r, newuri, sizeof(newuri));         /* expand ${...} */

        if (perdir == NULL)
            rewritelog(r, 2, "rewrite %s -> %s", uri, newuri);
        else
            rewritelog(r, 2, "[per-dir %s] rewrite %s -> %s", perdir, uri, newuri);

        r->filename = pstrdup(r->pool, newuri);

        /* reduce http[s]://<ourhost>[:<port>] */
        reduce_uri(r);

        /* split out on-the-fly generated QUERY_STRING '....?xxxxx&xxxx...' */
        splitout_queryargs(r, p->flags & RULEFLAG_QSAPPEND);

        /* if a MIME-type should be later forced for this URL, then remember this */
        if (p->forced_mimetype != NULL) {
            table_set(r->notes, REWRITE_FORCED_MIMETYPE_NOTEVAR, p->forced_mimetype);
            if (perdir == NULL)
                rewritelog(r, 2, "remember %s to have MIME-type '%s'", r->filename, p->forced_mimetype);
            else
                rewritelog(r, 2, "[per-dir %s] remember %s to have MIME-type '%s'", perdir, r->filename, p->forced_mimetype);
        }

        /* if we are forced to do a explicit redirect by [R] flag
           and the current URL still is not a fully qualified one we
           finally prefix it with http[s]://<ourname> explicitly */
        if (flags & RULEFLAG_FORCEREDIRECT) {
            r->status = p->forced_responsecode;
            if (  !(strlen(r->filename) > 7 &&
                    strncasecmp(r->filename, "http://", 7) == 0)
               && !(strlen(r->filename) > 8 &&
                    strncasecmp(r->filename, "https://", 8) == 0)
               && !(strlen(r->filename) > 9 &&
                    strncasecmp(r->filename, "gopher://", 9) == 0)
               && !(strlen(r->filename) > 6 &&
                    strncasecmp(r->filename, "ftp://", 6) == 0)    ) {

#ifdef APACHE_SSL
                if ((!r->connection->client->ssl && r->server->port == DEFAULT_PORT) ||
                    ( r->connection->client->ssl && r->server->port == 443)  )
#else
                if (r->server->port == DEFAULT_PORT)
#endif
                    port[0] = '\0';
                else 
                    ap_snprintf(port, sizeof(port), ":%u", r->server->port);
                if (r->filename[0] == '/')
#ifdef APACHE_SSL
                    ap_snprintf(newuri, sizeof(newuri), "%s://%s%s%s", http_method(r), r->server->server_hostname, port, r->filename);
#else
                    ap_snprintf(newuri, sizeof(newuri), "http://%s%s%s", r->server->server_hostname, port, r->filename);
#endif
                else
#ifdef APACHE_SSL
                    ap_snprintf(newuri, sizeof(newuri), "%s://%s%s/%s", http_method(r), r->server->server_hostname, port, r->filename);
#else
                    ap_snprintf(newuri, sizeof(newuri), "http://%s%s/%s", r->server->server_hostname, port, r->filename);
#endif
                if (perdir == NULL) 
                    rewritelog(r, 2, "prepare forced redirect %s -> %s", r->filename, newuri);
                else
                    rewritelog(r, 2, "[per-dir %s] prepare forced redirect %s -> %s", perdir, r->filename, newuri);
                r->filename = pstrdup(r->pool, newuri);
                return 1;
            }
        }

        return 1;
    }
    return 0;
}

static int apply_rewrite_cond(request_rec *r, rewritecond_entry *p, char *perdir)
{
    char *input;
    int rc;
    struct stat sb;
    request_rec *rsub;

    /* first, we have to expand the input string to match */
    input = expand_variables(r, p->input);

    rc = 0;
    if (strcmp(p->pattern, "-f") == 0) {
        if (stat(input, &sb) == 0)
            if (S_ISREG(sb.st_mode))
                rc = 1;
    }
    else if (strcmp(p->pattern, "-s") == 0) {
        if (stat(input, &sb) == 0)
            if (S_ISREG(sb.st_mode) && sb.st_size > 0) 
                rc = 1;
    }
    else if (strcmp(p->pattern, "-l") == 0) {
#ifndef __EMX__
/* OS/2 dosen't support links. */
        if (stat(input, &sb) == 0)
            if (S_ISLNK(sb.st_mode))
                rc = 1;
#endif
    }
    else if (strcmp(p->pattern, "-d") == 0) {
        if (stat(input, &sb) == 0)
            if (S_ISDIR(sb.st_mode))
                rc = 1;
    }
    else if (strcmp(p->pattern, "-U") == 0) {
        /* avoid infinite subrequest recursion */
        if (strlen(input) > 0               /* nonempty path, and */
            && (   r->main == NULL          /* - either not in a subrequest */
                || (   r->main->uri != NULL /* - or in a subrequest...*/
                    && r->uri != NULL       /*   ...and then URIs aren't NULL... */
                                            /*   ...and sub and main URIs differ */
                    && strcmp(r->main->uri, r->uri) != 0) ) ) {

            /* run a URI-based subrequest */
            rsub = sub_req_lookup_uri(input, r);

            /* URI exists for any result up to 3xx, redirects allowed */
            if (rsub->status < 400)
                rc = 1;

            /* log it */
            rewritelog(r, 5, "RewriteCond URI (-U) check: path=%s -> status=%d", input, rsub->status);

            /* cleanup by destroying the subrequest */
            destroy_sub_req(rsub);
        }
    }
    else if (strcmp(p->pattern, "-F") == 0) {
        /* avoid infinite subrequest recursion */
        if (strlen(input) > 0               /* nonempty path, and */
            && (   r->main == NULL          /* - either not in a subrequest */
                || (   r->main->uri != NULL /* - or in a subrequest...*/
                    && r->uri != NULL       /*   ...and then URIs aren't NULL... */
                                            /*   ...and sub and main URIs differ */
                    && strcmp(r->main->uri, r->uri) != 0) ) ) {

            /* process a file-based subrequest: 
               this differs from -U in that no path translation is done. */
            rsub = sub_req_lookup_file(input, r);
 
            /* file exists for any result up to 2xx, no redirects */
            if (rsub->status < 300 &&
                /* double-check that file exists since default result is 200 */
                stat(rsub->filename, &sb) == 0)
                rc = 1;

            /* log it */
            rewritelog(r, 5, "RewriteCond file (-F) check: path=%s -> file=%s status=%d", input, rsub->filename, rsub->status);

            /* cleanup by destroying the subrequest */
            destroy_sub_req(rsub);
        }
    }
    else if (strlen(p->pattern) > 1 && *(p->pattern) == '>') {
        rc = (compare_lexicography(input, p->pattern+1) == 1 ? 1 : 0);
    }
    else if (strlen(p->pattern) > 1 && *(p->pattern) == '<') {
        rc = (compare_lexicography(input, p->pattern+1) == -1 ? 1 : 0);
    }
    else if (strlen(p->pattern) > 1 && *(p->pattern) == '=') {
        rc = (strcmp(input, p->pattern+1) == 0 ? 1 : 0);
    }
    else {
        /* it is really a regexp pattern, so apply it */
        rc = (regexec(p->regexp, input, 0, NULL, 0) == 0);
    }

    /* if this is a non-matching regexp, just negate the result */ 
    if (p->flags & CONDFLAG_NOTMATCH) 
        rc = !rc;

    rewritelog(r, 4, "RewriteCond: input='%s' pattern='%s%s' => %s", 
               input, (p->flags & CONDFLAG_NOTMATCH ? "!" : ""), 
               p->pattern, rc ? "matched" : "not-matched");

    /* end just return the result */
    return rc;
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |              URL transformation functions
** |                                                       |
** +-------------------------------------------------------+
*/


/*
**
**  split out a QUERY_STRING part from
**  the current URI string
**
*/

static void splitout_queryargs(request_rec *r, int qsappend)
{
    char *q;
    char *olduri;

    q = strchr(r->filename, '?');
    if (q != NULL) {
        olduri = pstrdup(r->pool, r->filename);
        *q++ = '\0';
        if (qsappend)
            r->args = pstrcat(r->pool, q, "&", r->args, NULL);
        else
            r->args = pstrdup(r->pool, q);
        if (strlen(r->args) == 0) {
            r->args = NULL;
            rewritelog(r, 3, "split uri=%s -> uri=%s, args=<none>", olduri, r->filename);
        }
        else {
            if (r->args[strlen(r->args)-1] == '&')
                r->args[strlen(r->args)-1] = '\0';
            rewritelog(r, 3, "split uri=%s -> uri=%s, args=%s", olduri, r->filename, r->args);
        }
    }
    return;            
}


/*
**
**  strip 'http[s]://ourhost/' from URI
**
*/

static void reduce_uri(request_rec *r)
{
    char *cp;
    unsigned short port;
    char *portp;
    char *hostp;
    char *url;
    char c;
    char host[LONG_STRING_LEN];
    char buf[MAX_STRING_LEN];
    char *olduri;

#ifdef APACHE_SSL
    if (   (!r->connection->client->ssl &&
            strncasecmp(r->filename, "http://", 7) == 0)
        || (r->connection->client->ssl &&
            strncasecmp(r->filename, "https://", 8) == 0)) {
#else
    if (strncasecmp(r->filename, "http://", 7) == 0) {
#endif
        /* there was really a rewrite to a remote path */

        olduri = pstrdup(r->pool, r->filename); /* save for logging */

        /* cut the hostname and port out of the URI */
#ifdef APACHE_SSL
        strncpy(buf, r->filename+strlen(http_method(r))+3, sizeof(buf)-1);
#else
        strncpy(buf, r->filename+7, sizeof(buf)-1);
#endif
        EOS_PARANOIA(buf);
        hostp = buf;
        for (cp = hostp; *cp != '\0' && *cp != '/' && *cp != ':'; cp++)
            ;
        if (*cp == ':') {
            /* set host */
            *cp++ = '\0';
            strncpy(host, hostp, sizeof(host)-1);
            EOS_PARANOIA(host);
            /* set port */
            portp = cp;
            for (; *cp != '\0' && *cp != '/'; cp++)
                ;
            c = *cp;
            *cp = '\0';
            port = atoi(portp);
            *cp = c;
            /* set remaining url */
            url = cp;
        }
        else if (*cp == '/') {
            /* set host */
            *cp = '\0';
            strncpy(host, hostp, sizeof(host)-1);
            EOS_PARANOIA(host);
            *cp = '/';
            /* set port */
            port = DEFAULT_PORT;
            /* set remaining url */
            url = cp;
        }
        else {
            /* set host */
            strncpy(host, hostp, sizeof(host)-1);
            EOS_PARANOIA(host);
            /* set port */
            port = DEFAULT_PORT;
            /* set remaining url */
            url = "/";
        }

        /* now check whether we could reduce it to a local path... */
        if (is_this_our_host(r, host) && port == r->server->port) {
            /* this is our host, so only the URL remains */
            r->filename = pstrdup(r->pool, url);
            rewritelog(r, 3, "reduce %s -> %s", olduri, r->filename);
        }
    }
    return;            
}


/*
**
**  Expand tilde-paths (~user) through
**  Unix /etc/passwd database information
**
*/

static char *expand_tildepaths(request_rec *r, char *uri)
{
    char user[LONG_STRING_LEN];
    struct passwd *pw;
    char *newuri;
    int i, j;

    newuri = uri;
    if (uri != NULL && strlen(uri) > 2 && uri[0] == '/' && uri[1] == '~') {
        /* cut out the username */
        for (j = 0, i = 2; j < sizeof(user)-1 && uri[i] != '\0' && 
                       (   (uri[i] >= '0' && uri[i] <= '9')
                        || (uri[i] >= 'a' && uri[i] <= 'z')
                        || (uri[i] >= 'A' && uri[i] <= 'Z')); )
            user[j++] = uri[i++];
        user[j] = '\0';

        /* lookup username in systems passwd file */
        if ((pw = getpwnam(user)) != NULL) {
            /* ok, user was found, so expand the ~user string */
            if (uri[i] != '\0') {
                /* ~user/anything...  has to be expanded */
                if (pw->pw_dir[strlen(pw->pw_dir)-1] == '/') 
                    pw->pw_dir[strlen(pw->pw_dir)-1] = '\0';
                newuri = pstrcat(r->pool, pw->pw_dir, uri+i, NULL);
            }
            else {
                /* only ~user has to be expanded */
                newuri = pstrdup(r->pool, pw->pw_dir);
            }
        }
    }
    return newuri;
}


/*
**
**  mapfile expansion support
**  i.e. expansion of MAP lookup directives
**  ${<mapname>:<key>} in RewriteRule rhs
**
*/

#define limit_length(n) (n > LONG_STRING_LEN-1 ? LONG_STRING_LEN-1 : n)

static void expand_map_lookups(request_rec *r, char *uri, int uri_len)
{
    char newuri[MAX_STRING_LEN];
    char *cpI;
    char *cpIE;
    char *cpO;
    char *cpT;
    char *cpT2;
    char mapname[LONG_STRING_LEN];
    char mapkey[LONG_STRING_LEN];
    char defaultvalue[LONG_STRING_LEN];
    int n;

    cpI = uri;
    cpIE = cpI+strlen(cpI);
    cpO = newuri;
    while (cpI < cpIE) {
        if (cpI+6 < cpIE && strncmp(cpI, "${", 2) == 0) {
            /* missing delimiter -> take it as plain text */
            if (   strchr(cpI+2, ':') == NULL
                || strchr(cpI+2, '}') == NULL) {
                memcpy(cpO, cpI, 2);
                cpO += 2;
                cpI += 2;
                continue;
            }
            cpI += 2;

            cpT = strchr(cpI, ':');
            n = cpT-cpI;
            memcpy(mapname, cpI, limit_length(n));
            mapname[limit_length(n)] = '\0';
            cpI += n+1;

            cpT2 = strchr(cpI, '|');
            cpT = strchr(cpI, '}');
            if (cpT2 != NULL && cpT2 < cpT) {
                n = cpT2-cpI;
                memcpy(mapkey, cpI, limit_length(n));
                mapkey[limit_length(n)] = '\0';
                cpI += n+1;

                n = cpT-cpI;
                memcpy(defaultvalue, cpI, limit_length(n));
                defaultvalue[limit_length(n)] = '\0';
                cpI += n+1;
            }
            else {
                n = cpT-cpI;
                memcpy(mapkey, cpI, limit_length(n));
                mapkey[limit_length(n)] = '\0';
                cpI += n+1;

                defaultvalue[0] = '\0';
            }

            cpT = lookup_map(r, mapname, mapkey);
            if (cpT != NULL) {
                n = strlen(cpT);
                if (cpO + n >= newuri + sizeof(newuri)) {
                    log_printf(r->server, "insufficient space in expand_map_lookups, aborting");
                    return;
                }
                memcpy(cpO, cpT, n);
                cpO += n;
            }
            else {
                n = strlen(defaultvalue);
                if (cpO + n >= newuri + sizeof(newuri)) {
                    log_printf(r->server, "insufficient space in expand_map_lookups, aborting");
                    return;
                }
                memcpy(cpO, defaultvalue, n);
                cpO += n;
            }
        }
        else {
            cpT = strstr(cpI, "${");
            if (cpT == NULL)
                cpT = cpI+strlen(cpI);
            n = cpT-cpI;
            if (cpO + n >= newuri + sizeof(newuri)) {
                log_printf(r->server, "insufficient space in expand_map_lookups, aborting");
                return;
            }
            memcpy(cpO, cpI, n);
            cpO += n;
            cpI += n;
        }
    }
    *cpO = '\0';
    strncpy(uri, newuri, uri_len-1);
    uri[uri_len-1] = '\0';
    return;
}

#undef limit_length



/*
** +-------------------------------------------------------+
** |                                                       |
** |              DBM hashfile support
** |                                                       |
** +-------------------------------------------------------+
*/


static char *lookup_map(request_rec *r, char *name, char *key)
{
    void *sconf;
    rewrite_server_conf *conf;
    array_header *rewritemaps;
    rewritemap_entry *entries;
    rewritemap_entry *s;
    char *value;
    struct stat st;
    int i;

    /* get map configuration */
    sconf = r->server->module_config;
    conf  = (rewrite_server_conf *)get_module_config(sconf, &rewrite_module);
    rewritemaps = conf->rewritemaps;

    entries = (rewritemap_entry *)rewritemaps->elts;
    for (i = 0; i < rewritemaps->nelts; i++) {
        s = &entries[i];
        if (strcmp(s->name, name) == 0) {
            if (s->type == MAPTYPE_TXT) {
                stat(s->checkfile, &st); /* existence was checked at startup! */
                value = get_cache_string(cachep, s->name, CACHEMODE_TS, st.st_mtime, key);
                if (value == NULL) {
                    rewritelog(r, 6, "cache lookup FAILED, forcing new map lookup");
                    if ((value = lookup_map_txtfile(r, s->datafile, key)) != NULL) {
                        rewritelog(r, 5, "map lookup OK: map=%s key=%s[txt] -> val=%s", s->name, key, value);
                        set_cache_string(cachep, s->name, CACHEMODE_TS, st.st_mtime, key, value);
                        return value;
                    }
                    else {
                        rewritelog(r, 5, "map lookup FAILED: map=%s[txt] key=%s", s->name, key);
                        return NULL;
                    }
                }
                else {
                    rewritelog(r, 5, "cache lookup OK: map=%s[txt] key=%s -> val=%s", s->name, key, value);
                    return value;
                }
            }
            else if (s->type == MAPTYPE_DBM) {
#if HAS_NDBM_LIB
                stat(s->checkfile, &st); /* existence was checked at startup! */
                value = get_cache_string(cachep, s->name, CACHEMODE_TS, st.st_mtime, key);
                if (value == NULL) {
                    rewritelog(r, 6, "cache lookup FAILED, forcing new map lookup");
                    if ((value = lookup_map_dbmfile(r, s->datafile, key)) != NULL) {
                        rewritelog(r, 5, "map lookup OK: map=%s[dbm] key=%s -> val=%s", s->name, key, value);
                        set_cache_string(cachep, s->name, CACHEMODE_TS, st.st_mtime, key, value);
                        return value;
                    }
                    else {
                        rewritelog(r, 5, "map lookup FAILED: map=%s[dbm] key=%s", s->name, key);
                        return NULL;
                    }
                }
                else {
                    rewritelog(r, 5, "cache lookup OK: map=%s[dbm] key=%s -> val=%s", s->name, key, value);
                    return value;
                }
#else
                return NULL;
#endif
            }
            else if (s->type == MAPTYPE_PRG) {
                if ((value = lookup_map_program(r, s->fpin, s->fpout, key)) != NULL) {
                    rewritelog(r, 5, "map lookup OK: map=%s key=%s -> val=%s", s->name, key, value);
                    return value;
                }
                else {
                    rewritelog(r, 5, "map lookup FAILED: map=%s key=%s", s->name, key);
                }
            }
        }
    }
    return NULL;
}


static char *lookup_map_txtfile(request_rec *r, char *file, char *key)
{
    FILE *fp = NULL;
    char line[1024];
    char output[1024];
    char result[1024];
    char *value = NULL;
    char *cpT;
    char *curkey;
    char *curval;

    if ((fp = pfopen(r->pool, file, "r")) == NULL)
        return NULL;

    strncpy(output, MAPFILE_OUTPUT, sizeof(output)-1);
    EOS_PARANOIA(output);
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = '\0';
        if (regexec(lookup_map_txtfile_regexp, line, lookup_map_txtfile_regexp->re_nsub+1, lookup_map_txtfile_regmatch, 0) == 0) {
            strncpy(result, pregsub(r->pool, output, line, lookup_map_txtfile_regexp->re_nsub+1, lookup_map_txtfile_regmatch), sizeof(result)-1); /* substitute in output */
            EOS_PARANOIA(result);
            cpT = strchr(result, ',');
            *cpT = '\0';
            curkey = result;
            curval = cpT+1;

            if (strcmp(curkey, key) == 0) {
                value = pstrdup(r->pool, curval);
                break;
            }
        }
    }
    pfclose(r->pool, fp);
    return value;
}

#if HAS_NDBM_LIB
static char *lookup_map_dbmfile(request_rec *r, char *file, char *key)
{
    DBM *dbmfp = NULL;
    datum dbmkey;
    datum dbmval;
    char *value = NULL;
    char buf[MAX_STRING_LEN];

    dbmkey.dptr  = key;
    dbmkey.dsize = (strlen(key) < sizeof(buf) - 1 ? strlen(key) : sizeof(buf)-1);
    if ((dbmfp = dbm_open(file, O_RDONLY, 0666)) != NULL) {
        dbmval = dbm_fetch(dbmfp, dbmkey);
        if (dbmval.dptr != NULL) {
            memcpy(buf, dbmval.dptr, dbmval.dsize);
            buf[dbmval.dsize] = '\0';
            value = pstrdup(r->pool, buf);
        }
        dbm_close(dbmfp);
    }
    return value;
}
#endif

static char *lookup_map_program(request_rec *r, int fpin, int fpout, char *key)
{
    char buf[LONG_STRING_LEN];
    char c;
    int i;

    /* lock the channel */
#ifdef USE_PIPE_LOCKING
    fd_lock(fpin);
#endif

    /* write out the request key */
    write(fpin, key, strlen(key));
    write(fpin, "\n", 1);

    /* read in the response value */
    i = 0;
    while (read(fpout, &c, 1) == 1 && (i < LONG_STRING_LEN-1)) {
        if (c == '\n')
            break;
        buf[i++] = c;
    }
    buf[i] = '\0';

    /* unlock the channel */
#ifdef USE_PIPE_LOCKING
    fd_unlock(fpin);
#endif

    if (strcasecmp(buf, "NULL") == 0)
        return NULL;
    else
        return pstrdup(r->pool, buf);
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |              rewriting logfile support
** |                                                       |
** +-------------------------------------------------------+
*/


static void open_rewritelog(server_rec *s, pool *p)
{
    rewrite_server_conf *conf;
    char *fname;
    FILE *fp;
    int    rewritelog_flags = ( O_WRONLY|O_APPEND|O_CREAT );
    mode_t rewritelog_mode  = ( S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
  
    conf = get_module_config(s->module_config, &rewrite_module);
    
    if (conf->rewritelogfile == NULL)
        return; 
    if (*(conf->rewritelogfile) == '\0')
        return;
    if (conf->rewritelogfp > 0)
        return; /* virtual log shared w/main server */

    fname = server_root_relative(p, conf->rewritelogfile);
    
    if (*conf->rewritelogfile == '|') {
        if (!spawn_child(p, rewritelog_child, (void *)(conf->rewritelogfile+1),
                    kill_after_timeout, &fp, NULL)) {
            perror("spawn_child");
            fprintf (stderr, "mod_rewrite: could not fork child for RewriteLog process\n");
            exit (1);
        }
        conf->rewritelogfp = fileno(fp);
    }
    else if (*conf->rewritelogfile != '\0') {
        if ((conf->rewritelogfp = popenf(p, fname, rewritelog_flags, rewritelog_mode)) < 0) {
            perror("open");
            fprintf(stderr, "mod_rewrite: could not open RewriteLog file %s.\n", fname);
            exit(1);
        }
    }
    return;
}

/* Child process code for 'RewriteLog "|..."' */
static void rewritelog_child(void *cmd)
{
    cleanup_for_exec();
    signal(SIGHUP, SIG_IGN);
#ifdef __EMX__
    /* OS/2 needs a '/' */
    execl(SHELL_PATH, SHELL_PATH, "/c", (char *)cmd, NULL);
#else
    execl(SHELL_PATH, SHELL_PATH, "-c", (char *)cmd, NULL);
#endif
    exit(1);
}

static void rewritelog(request_rec *r, int level, const char *text, ...)
{
    rewrite_server_conf *conf;
    conn_rec *connect;
    char *str1;
    char str2[512];
    char str3[1024];
    char type[20];
    char redir[20];
    va_list ap;
    int i;
    request_rec *req;
    char *ruser;
    const char *rhost;
    
    va_start(ap, text);
    conf = get_module_config(r->server->module_config, &rewrite_module);
    connect = r->connection;

    if (conf->rewritelogfp <0)
        return;
    if (conf->rewritelogfile == NULL)
        return;
    if (*(conf->rewritelogfile) == '\0')
        return;

    if (level > conf->rewriteloglevel)
        return;

    if (connect->user == NULL) {
        ruser = "-";
    }
    else if (strlen (connect->user) != 0) {
        ruser = connect->user;
    }
    else {
        ruser = "\"\"";
    }

    rhost = get_remote_host(connect, r->server->module_config, REMOTE_NOLOOKUP);
    if (rhost == NULL)
        rhost = "UNKNOWN-HOST";

    str1 = pstrcat(r->pool, rhost, " ",
                            (connect->remote_logname != NULL ? connect->remote_logname : "-"), " ",
                            ruser, NULL);
    ap_vsnprintf(str2, sizeof(str2), text, ap);

    if (r->main == NULL)
        strcpy(type, "initial");
    else
        strcpy(type, "subreq");

    for (i = 0, req = r; req->prev != NULL; req = req->prev) 
        i++;
    if (i == 0)
        redir[0] = '\0';
    else
        ap_snprintf(redir, sizeof(redir), "/redir#%d", i);

    ap_snprintf(str3, sizeof(str3), "%s %s [%s/sid#%x][rid#%x/%s%s] (%d) %s\n", str1, current_logtime(r), r->server->server_hostname, (unsigned int)(r->server), (unsigned int)r, type, redir, level, str2);

    fd_lock(conf->rewritelogfp);
    write(conf->rewritelogfp, str3, strlen(str3));
    fd_unlock(conf->rewritelogfp);

    va_end(ap);
    return;
}

static char *current_logtime(request_rec *r)
{
    int timz;
    struct tm *t;
    char tstr[80];
    char sign;
    
    t = get_gmtoff(&timz);
    sign = (timz < 0 ? '-' : '+');
    if(timz < 0) 
        timz = -timz;

    strftime(tstr, 80, "[%d/%b/%Y:%H:%M:%S ", t);
    ap_snprintf(tstr + strlen(tstr), 80-strlen(tstr), "%c%.2d%.2d]", sign, timz/60, timz%60);
    return pstrdup(r->pool, tstr);
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |                  program map support
** |                                                       |
** +-------------------------------------------------------+
*/

static void run_rewritemap_programs(server_rec *s, pool *p)
{
    rewrite_server_conf *conf;
    char *fname;
    FILE *fpin;
    FILE *fpout;
    array_header *rewritemaps;
    rewritemap_entry *entries;
    rewritemap_entry *map;
    int i;
    int rc;
  
    conf = get_module_config(s->module_config, &rewrite_module);

    rewritemaps = conf->rewritemaps;
    entries = (rewritemap_entry *)rewritemaps->elts;
    for (i = 0; i < rewritemaps->nelts; i++) {
        map = &entries[i];
        if (map->type != MAPTYPE_PRG)
            continue;
        if (map->datafile == NULL    ||
            *(map->datafile) == '\0' ||
            map->fpin > 0        ||
            map->fpout > 0         )
            continue;
        fname = server_root_relative(p, map->datafile);
        fpin = NULL;
        fpout = NULL;
        rc = spawn_child(p, rewritemap_program_child, (void *)map->datafile, kill_after_timeout, &fpin, &fpout);
        if (rc == 0 || fpin == NULL || fpout == NULL) {
            perror("spawn_child");
            fprintf(stderr, "mod_rewrite: could not fork child for RewriteMap process\n");
            exit(1);
        }
        map->fpin  = fileno(fpin);
        map->fpout = fileno(fpout);
    }
    return;
}

/* child process code */
static void rewritemap_program_child(void *cmd)
{
    cleanup_for_exec();
    signal(SIGHUP, SIG_IGN);
#ifdef __EMX__
    /* OS/2 needs a '/' */
    execl(SHELL_PATH, SHELL_PATH, "/c", (char *)cmd, NULL);
#else
    execl(SHELL_PATH, SHELL_PATH, "-c", (char *)cmd, NULL);
#endif
    exit(1);
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |             environment variable support
** |                                                       |
** +-------------------------------------------------------+
*/


static void expand_variables_inbuffer(request_rec *r, char *buf, int buf_len)
{
    char *newbuf;
    newbuf = expand_variables(r, buf);
    if (strcmp(newbuf, buf) != 0) {
        strncpy(buf, newbuf, buf_len-1);
        buf[buf_len-1] = '\0';
    }
    return;
}

static char *expand_variables(request_rec *r, char *str)
{
    char output[MAX_STRING_LEN];
    char input[MAX_STRING_LEN];
    char *cp;
    char *cp2;
    char *cp3;
    int expanded;

    strncpy(input, str, sizeof(input)-1);
    EOS_PARANOIA(input);
    output[0] = '\0';
    expanded = 0;
    for (cp = input; cp < input+MAX_STRING_LEN; ) {
        if ((cp2 = strstr(cp, "%{")) != NULL) {
            if ((cp3 = strstr(cp2, "}")) != NULL) {
                *cp2 = '\0';
                strncpy(&output[strlen(output)], cp, sizeof(output)-strlen(output)-1);

                cp2 += 2;
                *cp3 = '\0';
                strncpy(&output[strlen(output)], lookup_variable(r, cp2), sizeof(output)-strlen(output)-1);

                cp = cp3+1;
                expanded = 1;
                continue;
            }
        }
        strncpy(&output[strlen(output)], cp, sizeof(output)-strlen(output)-1);
        EOS_PARANOIA(output);
        break;
    }
    return expanded ? pstrdup(r->pool, output) : str;
}

static char *lookup_variable(request_rec *r, char *var)
{
    char *result;
    char resultbuf[LONG_STRING_LEN];
    time_t tc;
    struct tm *tm;
    request_rec *rsub;
    struct passwd *pw;
    struct group *gr;
    struct stat finfo;

    result = NULL;

    /* HTTP headers */
    if (strcasecmp(var, "HTTP_USER_AGENT") == 0) {
        result = lookup_header(r, "User-Agent");
    }
    else if (strcasecmp(var, "HTTP_REFERER") == 0) {
        result = lookup_header(r, "Referer");
    }
    else if (strcasecmp(var, "HTTP_COOKIE") == 0) {
        result = lookup_header(r, "Cookie");
    }
    else if (strcasecmp(var, "HTTP_FORWARDED") == 0) {
        result = lookup_header(r, "Forwarded");
    }
    else if (strcasecmp(var, "HTTP_HOST") == 0) {
        result = lookup_header(r, "Host");
    }
    else if (strcasecmp(var, "HTTP_PROXY_CONNECTION") == 0) {
        result = lookup_header(r, "Proxy-Connection");
    }
    else if (strcasecmp(var, "HTTP_ACCEPT") == 0) {
        result = lookup_header(r, "Accept");
    }
    /* all other headers from which we are still not know about */
    else if (strlen(var) > 5 && strncasecmp(var, "HTTP:", 5) == 0) {
        result = lookup_header(r, var+5);
    }

    /* connection stuff */
    else if (strcasecmp(var, "REMOTE_ADDR") == 0) {
        result = r->connection->remote_ip;
    }
    else if (strcasecmp(var, "REMOTE_HOST") == 0) {
        result = (char *)get_remote_host(r->connection, r->per_dir_config, REMOTE_NAME);
    }
    else if (strcasecmp(var, "REMOTE_USER") == 0) {
        result = r->connection->user;
    }
    else if (strcasecmp(var, "REMOTE_IDENT") == 0) {
        result = (char *)get_remote_logname(r);
    }

    /* request stuff */
    else if (strcasecmp(var, "THE_REQUEST") == 0) { /* non-standard */
        result = r->the_request;
    }
    else if (strcasecmp(var, "REQUEST_METHOD") == 0) {
        result = r->method;
    }
    else if (strcasecmp(var, "REQUEST_URI") == 0) { /* non-standard */
        result = r->uri;
    }
    else if (strcasecmp(var, "SCRIPT_FILENAME") == 0 ||
             strcasecmp(var, "REQUEST_FILENAME") == 0  ) {
        result = r->filename;
    }
    else if (strcasecmp(var, "PATH_INFO") == 0) {
        result = r->path_info;
    }
    else if (strcasecmp(var, "QUERY_STRING") == 0) {
        result = r->args;
    }
    else if (strcasecmp(var, "AUTH_TYPE") == 0) {
        result = r->connection->auth_type;
    }
    else if (strcasecmp(var, "IS_SUBREQ") == 0) { /* non-standard */
        result = (r->main != NULL ? "true" : "false");
    }

    /* internal server stuff */
    else if (strcasecmp(var, "DOCUMENT_ROOT") == 0) {
        result = document_root(r);
    }
    else if (strcasecmp(var, "SERVER_ADMIN") == 0) {
        result = r->server->server_admin;
    }
    else if (strcasecmp(var, "SERVER_NAME") == 0) {
        result = r->server->server_hostname;
    }
    else if (strcasecmp(var, "SERVER_PORT") == 0) {
        ap_snprintf(resultbuf, sizeof(resultbuf), "%u", r->server->port);
        result = resultbuf;
    }
    else if (strcasecmp(var, "SERVER_PROTOCOL") == 0) {
        result = r->protocol;
    }
    else if (strcasecmp(var, "SERVER_SOFTWARE") == 0) {
        result = pstrdup(r->pool, SERVER_VERSION);
    }
    else if (strcasecmp(var, "API_VERSION") == 0) { /* non-standard */
        ap_snprintf(resultbuf, sizeof(resultbuf), "%d", MODULE_MAGIC_NUMBER);
        result = resultbuf;
    }

    /* underlaying Unix system stuff */
    else if (strcasecmp(var, "TIME_YEAR") == 0) {
        tc = time(NULL); 
        tm = localtime(&tc); 
        ap_snprintf(resultbuf, sizeof(resultbuf), "%02d%02d", (tm->tm_year / 100) + 19, tm->tm_year % 100);
        result = resultbuf;
    }
#define MKTIMESTR(format, tmfield) \
    tc = time(NULL); \
    tm = localtime(&tc); \
    ap_snprintf(resultbuf, sizeof(resultbuf), format, tm->tmfield); \
    result = resultbuf;
    else if (strcasecmp(var, "TIME_MON") == 0) {
        MKTIMESTR("%02d", tm_mon+1)
    }
    else if (strcasecmp(var, "TIME_DAY") == 0) {
        MKTIMESTR("%02d", tm_mday)
    }
    else if (strcasecmp(var, "TIME_HOUR") == 0) {
        MKTIMESTR("%02d", tm_hour)
    }
    else if (strcasecmp(var, "TIME_MIN") == 0) {
        MKTIMESTR("%02d", tm_min)
    }
    else if (strcasecmp(var, "TIME_SEC") == 0) {
        MKTIMESTR("%02d", tm_sec)
    }
    else if (strcasecmp(var, "TIME_WDAY") == 0) {
        MKTIMESTR("%d", tm_wday)
    }
    else if (strcasecmp(var, "TIME") == 0) {
        tc = time(NULL);
        tm = localtime(&tc);
        ap_snprintf(resultbuf, sizeof(resultbuf), "%02d%02d%02d%02d%02d%02d%02d",
            (tm->tm_year / 100) + 19, (tm->tm_year % 100),
            tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        result = resultbuf;
        rewritelog(r, 1, "RESULT='%s'", result);
    }

    /* all other env-variables from the parent Apache process */
    else if (strlen(var) > 4 && strncasecmp(var, "ENV:", 4) == 0) {
        /* first try the internal Apache notes structure */
        result = table_get(r->notes, var+4);
        /* second try the internal Apache env structure  */
        if (result == NULL) 
            result = table_get(r->subprocess_env, var+4);
        /* third try the external OS env */
        if (result == NULL) 
            result = getenv(var+4);
    }

#define LOOKAHEAD(subrecfunc) \
        if ( \
          /* filename is safe to use */ \
          r->filename != NULL \
              /* - and we're either not in a subrequest */ \
              && ( r->main == NULL \
                  /* - or in a subrequest where paths are non-NULL... */ \
                    || ( r->main->uri != NULL && r->uri != NULL \
                        /*   ...and sub and main paths differ */ \
                        && strcmp(r->main->uri, r->uri) != 0))) { \
            /* process a file-based subrequest */ \
            rsub = subrecfunc(r->filename, r); \
            /* now recursively lookup the variable in the sub_req */ \
            result = lookup_variable(rsub, var+5); \
            /* copy it up to our scope before we destroy the sub_req's pool */ \
            result = pstrdup(r->pool, result); \
            /* cleanup by destroying the subrequest */ \
            destroy_sub_req(rsub); \
            /* log it */ \
            rewritelog(r, 5, "lookahead: path=%s var=%s -> val=%s", r->filename, var+5, result); \
            /* return ourself to prevent re-pstrdup */ \
            return result; \
        }

    /* look-ahead for parameter through URI-based sub-request */
    else if (strlen(var) > 5 && strncasecmp(var, "LA-U:", 5) == 0) {
        LOOKAHEAD(sub_req_lookup_uri)
    }
    /* look-ahead for parameter through file-based sub-request */
    else if (strlen(var) > 5 && strncasecmp(var, "LA-F:", 5) == 0) {
        LOOKAHEAD(sub_req_lookup_file)
    }

    /* file stuff */
    else if (strcasecmp(var, "SCRIPT_USER") == 0) {
        result = pstrdup(r->pool, "<unknown>");
        if (r->finfo.st_mode != 0) {
            if ((pw = getpwuid(r->finfo.st_uid)) != NULL) { 
                result = pstrdup(r->pool, pw->pw_name);
            }
        }
        else {
            if (stat(r->filename, &finfo) == 0) {
                if ((pw = getpwuid(finfo.st_uid)) != NULL) { 
                    result = pstrdup(r->pool, pw->pw_name);
                }
            }
        }
    }
    else if (strcasecmp(var, "SCRIPT_GROUP") == 0) {
        result = pstrdup(r->pool, "<unknown>");
        if (r->finfo.st_mode != 0) {
            if ((gr = getgrgid(r->finfo.st_gid)) != NULL) { 
                result = pstrdup(r->pool, gr->gr_name);
            }
        }
        else {
            if (stat(r->filename, &finfo) == 0) {
                if ((gr = getgrgid(finfo.st_gid)) != NULL) { 
                    result = pstrdup(r->pool, gr->gr_name);
                }
            }
        }
    }

    if (result == NULL)
        return pstrdup(r->pool, "");
    else
        return pstrdup(r->pool, result);
}
 
static char *lookup_header(request_rec *r, const char *name)
{
    array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    hdrs_arr = table_elts(r->headers_in);
    hdrs = (table_entry *)hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; ++i) {
       if (hdrs[i].key == NULL)
          continue;
       if (strcasecmp(hdrs[i].key, name) == 0) 
          return hdrs[i].val;
    }
    return NULL;
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |                    caching support
** |                                                       |
** +-------------------------------------------------------+
*/


static cache *init_cache(pool *p)
{
    cache *c;

    c = (cache *)palloc(p, sizeof(cache));
    c->pool = make_sub_pool(p);
    c->lists = make_array(c->pool, 2, sizeof(cachelist));
    return c;
}

static void set_cache_string(cache *c, char *res, int mode, time_t time, char *key, char *value)
{
    cacheentry ce;

    ce.time  = time;
    ce.key   = key;
    ce.value = value;
    store_cache_string(c, res, &ce);
    return;
}

static char *get_cache_string(cache *c, char *res, int mode, time_t time, char *key)
{
    cacheentry *ce;

    ce = retrieve_cache_string(c, res, key);
    if (ce == NULL)
        return NULL;
    if (mode & CACHEMODE_TS) {
        if (time != ce->time)
            return NULL;
    }
    else if (mode & CACHEMODE_TTL) {
        if (time > ce->time)
            return NULL;
    }
    return pstrdup(c->pool, ce->value);
}

static void store_cache_string(cache *c, char *res, cacheentry *ce)
{
    int i;
    int j;
    cachelist *l;
    cacheentry *e;
    int found_list;

    found_list = 0;
    /* first try to edit an existing entry */
    for (i = 0; i < c->lists->nelts; i++) {
        l = &(((cachelist *)c->lists->elts)[i]);
        if (strcmp(l->resource, res) == 0) {
            found_list = 1;
            for (j = 0; j < l->entries->nelts; j++) {
                e = &(((cacheentry *)l->entries->elts)[j]);
                if (strcmp(e->key, ce->key) == 0) {
                    e->time  = ce->time;
                    e->value = pstrdup(c->pool, ce->value);
                    return;
                }
            }
        }
    }

    /* create a needed new list */
    if (!found_list) {
        l = push_array(c->lists);
        l->resource = pstrdup(c->pool, res);
        l->entries  = make_array(c->pool, 2, sizeof(cacheentry));
    }

    /* create the new entry */
    for (i = 0; i < c->lists->nelts; i++) {
        l = &(((cachelist *)c->lists->elts)[i]);
        if (strcmp(l->resource, res) == 0) {
            e = push_array(l->entries);
            e->time  = ce->time;
            e->key   = pstrdup(c->pool, ce->key);
            e->value = pstrdup(c->pool, ce->value);
            return;
        }
    }

    /* not reached, but when it is no problem... */
    return;
}

static cacheentry *retrieve_cache_string(cache *c, char *res, char *key)
{
    int i;
    int j;
    cachelist *l;
    cacheentry *e;

    for (i = 0; i < c->lists->nelts; i++) {
        l = &(((cachelist *)c->lists->elts)[i]);
        if (strcmp(l->resource, res) == 0) {
            for (j = 0; j < l->entries->nelts; j++) {
                e = &(((cacheentry *)l->entries->elts)[j]);
                if (strcmp(e->key, key) == 0) {
                    return e;
                }
            }
        }
    }
    return NULL;
}




/*
** +-------------------------------------------------------+
** |                                                       |
** |                    misc functions
** |                                                       |
** +-------------------------------------------------------+
*/

static char *subst_prefix_path(request_rec *r, char *input, char *match, char *subst)
{
    char matchbuf[LONG_STRING_LEN];
    char substbuf[LONG_STRING_LEN];
    char *output;
    int l;

    output = input;

    /* first create a match string which always has a trailing slash */
    strncpy(matchbuf, match, sizeof(matchbuf)-1);
    EOS_PARANOIA(matchbuf);
    l = strlen(matchbuf);
    if (matchbuf[l-1] != '/') {
       matchbuf[l] = '/';
       matchbuf[l+1] = '\0';
       l++;
    }
    /* now compare the prefix */
    if (strncmp(input, matchbuf, l) == 0) {
        rewritelog(r, 5, "strip matching prefix: %s -> %s", output, output+l);
        output = pstrdup(r->pool, output+l); 

        /* and now add the base-URL as replacement prefix */
        strncpy(substbuf, subst, sizeof(substbuf)-1);
        EOS_PARANOIA(substbuf);
        l = strlen(substbuf);
        if (substbuf[l-1] != '/') {
           substbuf[l] = '/';
           substbuf[l+1] = '\0';
           l++;
        }
        if (output[0] == '/') {
            rewritelog(r, 4, "add subst prefix: %s -> %s%s", output, substbuf, output+1);
            output = pstrcat(r->pool, substbuf, output+1, NULL);
        }
        else {
            rewritelog(r, 4, "add subst prefix: %s -> %s%s", output, substbuf, output);
            output = pstrcat(r->pool, substbuf, output, NULL);
        }
    }
    return output;
}


/*
**
**  own command line parser which don't have the '\\' problem
**
*/

static int parseargline(char *str, char **a1, char **a2, char **a3)
{
    char *cp;
    int isquoted;

#define SKIP_WHITESPACE(cp) \
    for ( ; *cp == ' ' || *cp == '\t'; ) \
        cp++;

#define CHECK_QUOTATION(cp,isquoted) \
    isquoted = 0; \
    if (*cp == '"') { \
        isquoted = 1; \
        cp++; \
    }

#define DETERMINE_NEXTSTRING(cp,isquoted) \
    for ( ; *cp != '\0'; cp++) { \
        if (   (isquoted    && (*cp     == ' ' || *cp     == '\t')) \
            || (*cp == '\\' && (*(cp+1) == ' ' || *(cp+1) == '\t'))) { \
            cp++; \
            continue; \
        } \
        if (   (!isquoted && (*cp == ' ' || *cp == '\t')) \
            || (isquoted  && *cp == '"')                  ) \
            break; \
    } 

    cp = str;
    SKIP_WHITESPACE(cp);

    /*  determine first argument */
    CHECK_QUOTATION(cp, isquoted);
    *a1 = cp;
    DETERMINE_NEXTSTRING(cp, isquoted);
    if (*cp == '\0')
        return 1;
    *cp++ = '\0';

    SKIP_WHITESPACE(cp);

    /*  determine second argument */
    CHECK_QUOTATION(cp, isquoted);
    *a2 = cp;
    DETERMINE_NEXTSTRING(cp, isquoted);
    if (*cp == '\0') {
        *cp++ = '\0';
        *a3 = NULL;
        return 0;
    }
    *cp++ = '\0';

    SKIP_WHITESPACE(cp);

    /* again check if there are only two arguments */
    if (*cp == '\0') {
        *cp++ = '\0';
        *a3 = NULL;
        return 0;
    }

    /*  determine second argument */
    CHECK_QUOTATION(cp, isquoted);
    *a3 = cp;
    DETERMINE_NEXTSTRING(cp, isquoted);
    *cp++ = '\0';

    return 0;
}


static void add_env_variable(request_rec *r, char *s)
{
    char var[MAX_STRING_LEN];
    char val[MAX_STRING_LEN];
    char *cp;
    int n;

    if ((cp = strchr(s, ':')) != NULL) {
        n = ((cp-s) > MAX_STRING_LEN-1 ? MAX_STRING_LEN-1 : (cp-s));
        memcpy(var, s, n);
        var[n] = '\0';
        strncpy(val, cp+1, sizeof(val)-1);
        EOS_PARANOIA(val);
        table_set(r->subprocess_env, pstrdup(r->pool, var), pstrdup(r->pool, val));
        rewritelog(r, 5, "setting env variable '%s' to '%s'", var, val);
    }
}



/*
**
**  stat() for only the prefix of a path
**
*/

static int prefix_stat(const char *path, struct stat *sb)
{
    char curpath[LONG_STRING_LEN];
    char *cp;

    strncpy(curpath, path, sizeof(curpath)-1);
    EOS_PARANOIA(curpath);
    if (curpath[0] != '/') 
        return 0;
    if ((cp = strchr(curpath+1, '/')) != NULL)
        *cp = '\0';
    if (stat(curpath, sb) == 0)
        return 1;
    else
        return 0;
}


/*
**
**  special DNS lookup functions
**
*/

static int is_this_our_host(request_rec *r, char *testhost)
{
    char **cppHNLour;
    char **cppHNLtest;
    char *ourhostname;
    char *ourhostip;
    const char *names;
    char *name;
    int i, j;
    server_addr_rec *sar;

    /* we can check:
       r->
            char *hostname            Host, as set by full URI or Host: 
            int hostlen               Length of http://host:port in full URI 
       r->server->
            int is_virtual            0=main, 1=ip-virtual, 2=non-ip-virtual
            char *server_hostname     used on compare to r->hostname
            inet_ntoa(r->connection->local_addr.sin_addr)
                                      used on compare to r->hostname
            unsigned short port       for redirects
            char *path                name of ServerPath
            int pathlen               len of ServerPath
            char *names               Wildcarded names for ServerAlias servers 
       r->server->addrs->
            struct in_addr host_addr  The bound address, for this server
            short host_port           The bound port, for this server 
            char *virthost            The name given in <VirtualHost> 
    */

    ourhostname = r->server->server_hostname;
    ourhostip   = inet_ntoa(r->connection->local_addr.sin_addr);

    /* just a simple common case */
    if (strcmp(testhost, ourhostname) == 0 ||
        strcmp(testhost, ourhostip)   == 0   )
       return YES;

    /* now the complicated cases */
    if (!r->server->is_virtual) {
        /* main servers */

        /* check for the alternative IP addresses */
        if ((cppHNLour = resolv_ipaddr_list(r, ourhostname)) == NULL)
            return NO;
        if ((cppHNLtest = resolv_ipaddr_list(r, testhost)) == NULL)
            return NO;
        for (i = 0; cppHNLtest[i] != NULL; i++) {
            for (j = 0; cppHNLour[j] != NULL; j++) {
                if (strcmp(cppHNLtest[i], cppHNLour[j]) == 0) {
                    return YES;
                }
            }
        }
    }
    else if (r->server->is_virtual) {
        /* virtual servers */

        /* check for the names supplied in the VirtualHost directive */
        for(sar = r->server->addrs; sar != NULL; sar = sar->next) {
            if(strcasecmp(sar->virthost, testhost) == 0)
                return YES;
        }

        /* check for the virtual-server aliases */
        if (r->server->names != NULL && r->server->names[0] != '\0') {
            names = r->server->names;
            while (*names != '\0') {
                name = getword_conf(r->pool, &names);
                if ((is_matchexp(name) && !strcasecmp_match(testhost, name)) ||
                    (strcasecmp(testhost, name) == 0)                          ) {
                    return YES;
                }
            }
        }
    }
    return NO;
}

static int isaddr(char *host)
{
    char *cp;

    /* Null pointers and empty strings 
       are not addresses. */
    if (host == NULL)
        return NO;
    if (*host == '\0')
        return NO;
    /* Make sure it has only digits and dots. */
    for (cp = host; *cp; cp++) {
        if (!isdigit(*cp) && *cp != '.')
            return NO;
    }
    /* If it has a trailing dot, 
       don't treat it as an address. */
    if (*(cp-1) == '.')
       return NO;
    return YES;
}

static char **resolv_ipaddr_list(request_rec *r, char *name)
{
    char **cppHNL;
    struct hostent *hep;
    int i;

    if (isaddr(name)) 
        hep = gethostbyaddr(name, sizeof(struct in_addr), AF_INET);
    else
        hep = gethostbyname(name);
    if (hep == NULL)
        return NULL;
    for (i = 0; hep->h_addr_list[i]; i++)
        ;
    cppHNL = (char **)palloc(r->pool, sizeof(char *)*(i+1));
    for (i = 0; hep->h_addr_list[i]; i++)
        cppHNL[i] = pstrdup(r->pool, inet_ntoa(*((struct in_addr *)(hep->h_addr_list[i]))) );
    cppHNL[i] = NULL;
    return cppHNL;
}


/*
**
**  check if proxy module is available
**  i.e. if it is compiled in and turned on
**
*/

static int is_proxy_available(server_rec *s)
{
    return (find_linked_module("mod_proxy.c") != NULL); 
}


/*
**
**  File locking
**
*/

#ifdef USE_FCNTL
static struct flock   lock_it;
static struct flock unlock_it;
#endif 

static void fd_lock(int fd)
{
    int rc;

#ifdef USE_FCNTL
    lock_it.l_whence = SEEK_SET; /* from current point */
    lock_it.l_start  = 0;        /* -"- */
    lock_it.l_len    = 0;        /* until end of file */
    lock_it.l_type   = F_WRLCK;  /* set exclusive/write lock */
    lock_it.l_pid    = 0;        /* pid not actually interesting */

    while (   ((rc = fcntl(fd, F_SETLKW, &lock_it)) < 0) 
           && (errno == EINTR)                               )
        continue;
#endif
#ifdef USE_FLOCK
    while (   ((rc = flock(fd, LOCK_EX)) < 0) 
           && (errno == EINTR)               )
        continue;
#endif

    if (rc < 0) {
#ifdef USE_FLOCK
        perror("flock");
#else
        perror("fcntl");
#endif
        fprintf(stderr, "Error getting lock. Exiting!");
        exit(1);
    }
    return;
}

static void fd_unlock(int fd)
{
    int rc;

#ifdef USE_FCNTL 
    unlock_it.l_whence = SEEK_SET; /* from current point */
    unlock_it.l_start  = 0;        /* -"- */
    unlock_it.l_len    = 0;        /* until end of file */
    unlock_it.l_type   = F_UNLCK;  /* unlock */
    unlock_it.l_pid    = 0;        /* pid not actually interesting */

    rc = fcntl(fd, F_SETLKW, &unlock_it);
#endif
#ifdef USE_FLOCK 
    rc = flock(fd, LOCK_UN);
#endif 

    if (rc < 0) {
#ifdef USE_FLOCK
        perror("flock");
#else
        perror("fcntl");
#endif
        fprintf(stderr, "Error freeing lock. Exiting!");
        exit(1);
    }
}

/*
**
**  Lexicographic Compare
**
*/

static int compare_lexicography(char *cpNum1, char *cpNum2)
{
    int i;
    int n1, n2;

    n1 = strlen(cpNum1);
    n2 = strlen(cpNum2);
    if (n1 > n2)
        return 1;
    if (n1 < n2)
        return -1;
    for (i = 0; i < n1; i++) {
        if (cpNum1[i] > cpNum2[i])
            return 1;
        if (cpNum1[i] < cpNum2[i])
            return -1;
    }
    return 0;
}


/*EOF*/

/*	$OpenBSD: resp.c,v 1.2 2004/08/03 05:08:45 jfb Exp $	*/
/*
 * Copyright (c) 2004 Jean-Francois Brousseau <jfb@openbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>

#include "buf.h"
#include "cvs.h"
#include "log.h"
#include "file.h"
#include "proto.h"


#define CVS_MTSTK_MAXDEPTH   16


static int  cvs_resp_validreq  (struct cvsroot *, int, char *);
static int  cvs_resp_cksum     (struct cvsroot *, int, char *);
static int  cvs_resp_modtime   (struct cvsroot *, int, char *);
static int  cvs_resp_m         (struct cvsroot *, int, char *);
static int  cvs_resp_ok        (struct cvsroot *, int, char *);
static int  cvs_resp_error     (struct cvsroot *, int, char *);
static int  cvs_resp_statdir   (struct cvsroot *, int, char *);
static int  cvs_resp_sticky    (struct cvsroot *, int, char *);
static int  cvs_resp_newentry  (struct cvsroot *, int, char *);
static int  cvs_resp_updated   (struct cvsroot *, int, char *);
static int  cvs_resp_removed   (struct cvsroot *, int, char *);
static int  cvs_resp_mode      (struct cvsroot *, int, char *);
static int  cvs_resp_modxpand  (struct cvsroot *, int, char *);
static int  cvs_resp_rcsdiff   (struct cvsroot *, int, char *);
static int  cvs_resp_template  (struct cvsroot *, int, char *);

static const char *cvs_months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};



struct cvs_resphdlr {
	int (*hdlr)(struct cvsroot *, int, char *);
} cvs_resp_swtab[CVS_RESP_MAX + 1] = {
	{ NULL              },
	{ cvs_resp_ok       },
	{ cvs_resp_error    },
	{ cvs_resp_validreq },
	{ cvs_resp_newentry },
	{ cvs_resp_newentry },
	{ cvs_resp_cksum    },
	{ NULL              },
	{ cvs_resp_updated  },
	{ cvs_resp_updated  },
	{ cvs_resp_updated  },	/* 10 */
	{ cvs_resp_updated  },
	{ cvs_resp_updated  },
	{ cvs_resp_rcsdiff  },
	{ cvs_resp_mode     },
	{ cvs_resp_modtime  },
	{ cvs_resp_removed  },
	{ cvs_resp_removed  },
	{ cvs_resp_statdir  },
	{ cvs_resp_statdir  },
	{ cvs_resp_sticky   },	/* 20 */
	{ cvs_resp_sticky   },
	{ cvs_resp_template },
	{ NULL              },
	{ NULL              },
	{ NULL              },
	{ cvs_resp_modxpand },
	{ NULL              },
	{ cvs_resp_m        },
	{ cvs_resp_m        },
	{ cvs_resp_m        },	/* 30 */
	{ cvs_resp_m        },
	{ cvs_resp_m        },
};



/*
 * The MT command uses scoping to tag the data.  Whenever we encouter a '+',
 * we push the name of the tag on the stack, and we pop it when we encounter
 * a '-' with the same name.
 */

static char *cvs_mt_stack[CVS_MTSTK_MAXDEPTH];
static u_int cvs_mtstk_depth = 0;

static time_t cvs_modtime = 0;


/* last checksum received */
char *cvs_fcksum = NULL;

mode_t  cvs_lastmode = 0;

/* hack to receive the remote version without outputting it */
extern u_int cvs_version_sent;


/*
 * cvs_resp_handle()
 *
 * Generic response handler dispatcher.  The handler expects the first line
 * of the command as single argument.
 * Returns the return value of the command on success, or -1 on failure.
 */

int
cvs_resp_handle(struct cvsroot *root, char *line)
{
	char *cp, *cmd;
	struct cvs_resp *resp;

	cmd = line;

	cp = strchr(cmd, ' ');
	if (cp != NULL)
		*(cp++) = '\0';

	resp = cvs_resp_getbyname(cmd);
	if (resp == NULL) {
		return (-1);
	}
	else if (cvs_resp_swtab[resp->resp_id].hdlr == NULL) {
		cvs_log(LP_ERRNO, "handler for `%s' not implemented", cmd);
		return (-1);
	}

	return (*cvs_resp_swtab[resp->resp_id].hdlr)(root, resp->resp_id, cp);
}


/*
 * cvs_resp_validreq()
 *
 * Handler for the `Valid-requests' response.  The list of valid requests is
 * split on spaces and each request's entry in the valid request array is set
 * to 1 to indicate the validity.
 * Returns 0 on success, or -1 on failure.
 */

static int
cvs_resp_validreq(struct cvsroot *root, int type, char *line)
{
	char *sp, *ep;
	struct cvs_req *req;

	/* parse the requests */
	sp = line;
	do {
		ep = strchr(sp, ' ');
		if (ep != NULL)
			*ep = '\0';

		req = cvs_req_getbyname(sp);
		if (req != NULL)
			CVS_SETVR(root, req->req_id);

		if (ep != NULL)
			sp = ep + 1;
	} while (ep != NULL);

	return (0);
}


/*
 * cvs_resp_m()
 *
 * Handler for the `M', 'MT', `F' and `E' responses.
 */

static int
cvs_resp_m(struct cvsroot *root, int type, char *line)
{
	char *cp;
	FILE *stream;

	stream = NULL;

	switch (type) {
	case CVS_RESP_F:
		fflush(stderr);
		return (0);
	case CVS_RESP_M:
		if (cvs_version_sent) {
			/*
			 * Instead of outputting the line, we save it as the
			 * remote server's version string.
			 */
			cvs_version_sent = 0;
			root->cr_version = strdup(line);
			return (0);
		}
		stream = stdout;
		break;
	case CVS_RESP_E:
		stream = stderr;
		break;
	case CVS_RESP_MT:
		if (*line == '+') {
			if (cvs_mtstk_depth == CVS_MTSTK_MAXDEPTH) {
				cvs_log(LP_ERR,
				    "MT scope stack has reached max depth");
				return (-1);
			}
			cvs_mt_stack[cvs_mtstk_depth] = strdup(line + 1);
			if (cvs_mt_stack[cvs_mtstk_depth] == NULL)
				return (-1);
			cvs_mtstk_depth++;
		}
		else if (*line == '-') {
			if (cvs_mtstk_depth == 0) {
				cvs_log(LP_ERR, "MT scope stack underflow");
				return (-1);
			}
			else if (strcmp(line + 1,
			    cvs_mt_stack[cvs_mtstk_depth - 1]) != 0) {
				cvs_log(LP_ERR, "mismatch in MT scope stack");
				return (-1);
			}
			free(cvs_mt_stack[cvs_mtstk_depth--]);
		}
		else {
			if (strcmp(line, "newline") == 0)
				putc('\n', stdout);
			else if (strncmp(line, "fname ", 6) == 0)
				printf("%s", line + 6);
			else {
				/* assume text */
				cp = strchr(line, ' ');
				if (cp != NULL)
					printf("%s", cp + 1);
			}
		}

		return (0);
	case CVS_RESP_MBINARY:
		cvs_log(LP_WARN, "Mbinary not supported in client yet");
		break;
	}

	fputs(line, stream);
	fputc('\n', stream);

	return (0);
}


/*
 * cvs_resp_ok()
 *
 * Handler for the `ok' response.  This handler's job is to 
 */

static int
cvs_resp_ok(struct cvsroot *root, int type, char *line)
{
	return (1);
}


/*
 * cvs_resp_error()
 *
 * Handler for the `error' response.  This handler's job is to 
 */

static int
cvs_resp_error(struct cvsroot *root, int type, char *line)
{
	return (1);
}


/*
 * cvs_resp_statdir()
 *
 * Handler for the `Clear-static-directory' and `Set-static-directory'
 * responses.
 */

static int
cvs_resp_statdir(struct cvsroot *root, int type, char *line)
{
	int fd;
	char rpath[MAXPATHLEN], statpath[MAXPATHLEN];

	cvs_getln(root, rpath, sizeof(rpath));

	snprintf(statpath, sizeof(statpath), "%s/%s", line,
	    CVS_PATH_STATICENTRIES);

	if ((type == CVS_RESP_CLRSTATDIR) &&
	    (unlink(statpath) == -1) && (errno != ENOENT)) {
		cvs_log(LP_ERRNO, "failed to unlink %s file",
		    CVS_PATH_STATICENTRIES);
		return (-1);
	}
	else if (type == CVS_RESP_SETSTATDIR) {
		fd = open(statpath, O_CREAT|O_TRUNC|O_WRONLY, 0400);
		if (fd == -1) {
			cvs_log(LP_ERRNO, "failed to create %s file",
			    CVS_PATH_STATICENTRIES);
			return (-1);
		}
		(void)close(fd);

	}

	return (0);
}

/*
 * cvs_resp_sticky()
 *
 * Handler for the `Clear-sticky' and `Set-sticky' responses.
 */

static int
cvs_resp_sticky(struct cvsroot *root, int type, char *line)
{
	size_t len;
	char rpath[MAXPATHLEN];
	struct stat st;
	CVSFILE *cf;

	/* remove trailing slash */
	len = strlen(line);
	if ((len > 0) && (line[len - 1] == '/'))
		line[--len] = '\0';

	/* get the remote path */
	cvs_getln(root, rpath, sizeof(rpath));

	/* if the directory doesn't exist, create it */
	if (stat(line, &st) == -1) {
		/* attempt to create it */
		if (errno != ENOENT) {
			cvs_log(LP_ERRNO, "failed to stat %s", line);
		}
		else {
			cf = cvs_file_create(line, DT_DIR, 0755);
			if (cf == NULL)
				return (-1);
			cf->cf_ddat->cd_repo = strdup(line);
			cf->cf_ddat->cd_root = root;
			root->cr_ref++;
			cvs_mkadmin(cf, 0755);

			cvs_file_free(cf);
		}
	}

	if (type == CVS_RESP_CLRSTICKY) {
	}
	else if (type == CVS_RESP_SETSTICKY) {
	}

	return (0);
}


/*
 * cvs_resp_newentry()
 *
 * Handler for the `New-entry' response and `Checked-in' responses.
 */

static int
cvs_resp_newentry(struct cvsroot *root, int type, char *line)
{
	char entbuf[128];
	CVSENTRIES *entfile;

	/* get the remote path */
	cvs_getln(root, entbuf, sizeof(entbuf));

	/* get the new Entries line */
	if (cvs_getln(root, entbuf, sizeof(entbuf)) < 0)
		return (-1);

	entfile = cvs_ent_open(line, O_WRONLY);
	if (entfile == NULL)
		return (-1);
	cvs_ent_addln(entfile, entbuf);
	cvs_ent_close(entfile);

	return (0);
}


/*
 * cvs_resp_cksum()
 *
 * Handler for the `Checksum' response.  We store the checksum received for
 * the next file in a dynamically-allocated buffer pointed to by <cvs_fcksum>.
 * Upon next file reception, the handler checks to see if there is a stored
 * checksum.
 * The file handler must make sure that the checksums match and free the
 * checksum buffer once it's done to indicate there is no further checksum.
 */

static int
cvs_resp_cksum(struct cvsroot *root, int type, char *line)
{
	if (cvs_fcksum != NULL) {
		cvs_log(LP_WARN, "unused checksum");
		free(cvs_fcksum);
	}

	cvs_fcksum = strdup(line);
	if (cvs_fcksum == NULL) {
		cvs_log(LP_ERRNO, "failed to copy checksum string");
		return (-1);
	}

	return (0);
}


/*
 * cvs_resp_modtime()
 *
 * Handler for the `Mod-time' file update modifying response.  The timestamp
 * given is used to set the last modification time on the next file that
 * will be received.
 */

static int
cvs_resp_modtime(struct cvsroot *root, int type, char *line)
{
	int i;
	long off;
	char sign, mon[8], gmt[8], hr[4], min[4], *ep;
	struct tm cvs_tm;

	memset(&cvs_tm, 0, sizeof(cvs_tm));
	sscanf(line, "%d %3s %d %2d:%2d:%2d %5s", &cvs_tm.tm_mday, mon,
	    &cvs_tm.tm_year, &cvs_tm.tm_hour, &cvs_tm.tm_min,
	    &cvs_tm.tm_sec, gmt);
	cvs_tm.tm_year -= 1900;
	cvs_tm.tm_isdst = -1;

	if (*gmt == '-') {
		sscanf(gmt, "%c%2s%2s", &sign, hr, min);
		cvs_tm.tm_gmtoff = strtol(hr, &ep, 10);
		if ((cvs_tm.tm_gmtoff == LONG_MIN) ||
		    (cvs_tm.tm_gmtoff == LONG_MAX) ||
		    (*ep != '\0')) {
			cvs_log(LP_ERR,
			    "parse error in GMT hours specification `%s'", hr);
			cvs_tm.tm_gmtoff = 0;
		}
		else {
			/* get seconds */
			cvs_tm.tm_gmtoff *= 3600;

			/* add the minutes */
			off = strtol(min, &ep, 10);
			if ((cvs_tm.tm_gmtoff == LONG_MIN) ||
			    (cvs_tm.tm_gmtoff == LONG_MAX) ||
			    (*ep != '\0')) {
				cvs_log(LP_ERR,
				    "parse error in GMT minutes "
				    "specification `%s'", min);
			}
			else
				cvs_tm.tm_gmtoff += off * 60;
		}
	}
	if (sign == '-')
		cvs_tm.tm_gmtoff = -cvs_tm.tm_gmtoff;

	for (i = 0; i < (int)(sizeof(cvs_months)/sizeof(cvs_months[0])); i++) {
		if (strcmp(cvs_months[i], mon) == 0) {
			cvs_tm.tm_mon = i;
			break;
		}
	}

	cvs_modtime = mktime(&cvs_tm);
	return (0);
}


/*
 * cvs_resp_updated()
 *
 * Handler for the `Updated' and `Created' responses.
 */

static int
cvs_resp_updated(struct cvsroot *root, int type, char *line)
{
	size_t len;
	mode_t fmode;
	char tbuf[32], path[MAXPATHLEN], cksum_buf[CVS_CKSUM_LEN];
	BUF *fbuf;
	CVSENTRIES *ef;
	struct cvs_ent *ep;
	struct timeval tv[2];

	ep = NULL;

	/* read the remote path of the file */
	cvs_getln(root, path, sizeof(path));

	/* read the new entry */
	cvs_getln(root, path, sizeof(path));
	ep = cvs_ent_parse(path);
	if (ep == NULL)
		return (-1);
	snprintf(path, sizeof(path), "%s%s", line, ep->ce_name);

	if (type == CVS_RESP_CREATED) {
		/* set the timestamp as the last one received from Mod-time */
		ep->ce_timestamp = ctime_r(&cvs_modtime, tbuf);
		len = strlen(tbuf);
		if ((len > 0) && (tbuf[len - 1] == '\n'))
			tbuf[--len] = '\0';

		ef = cvs_ent_open(line, O_WRONLY);
		if (ef == NULL)
			return (-1);

		cvs_ent_add(ef, ep);
		cvs_ent_close(ef);
	}
	else if (type == CVS_RESP_UPDEXIST) {
	}
	else if (type == CVS_RESP_UPDATED) {
	}

	fbuf = cvs_recvfile(root, &fmode);
	if (fbuf == NULL)
		return (-1);

	cvs_buf_write(fbuf, path, fmode);

	tv[0].tv_sec = (long)cvs_modtime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = (long)cvs_modtime;
	tv[1].tv_usec = 0;
	if (utimes(path, tv) == -1)
		cvs_log(LP_ERRNO, "failed to set file timestamps");

	/* now see if there is a checksum */
	if (cvs_fcksum != NULL) {
		if (cvs_cksum(path, cksum_buf, sizeof(cksum_buf)) < 0) {
		}

		if (strcmp(cksum_buf, cvs_fcksum) != 0) {
			cvs_log(LP_ERR, "checksum error on received file");
			(void)unlink(line);
		}

		free(cvs_fcksum);
		cvs_fcksum = NULL;
	}

	return (0);
}


/*
 * cvs_resp_removed()
 *
 * Handler for the `Removed' and `Remove-entry' responses.
 */

static int
cvs_resp_removed(struct cvsroot *root, int type, char *line)
{
	char base[MAXPATHLEN], file[MAXNAMLEN];
	CVSENTRIES *ef;

	cvs_splitpath(line, base, sizeof(base), file, sizeof(file));

	ef = cvs_ent_open(base, O_RDWR);
	if (ef == NULL)
		return (-1);

	printf("Received a `Remove' on %s\n", line);
	cvs_ent_remove(ef, file);
	cvs_ent_close(ef);

	return (0);
}


/*
 * cvs_resp_mode()
 *
 * Handler for the `Mode' response.
 */

static int
cvs_resp_mode(struct cvsroot *root, int type, char *line)
{
	if (cvs_strtomode(line, &cvs_lastmode) < 0) {
		return (-1);
	}
	return (0);
}


/*
 * cvs_resp_modxpand()
 *
 * Handler for the `Module-expansion' response.
 */

static int
cvs_resp_modxpand(struct cvsroot *root, int type, char *line)
{
	return (0);
}

/*
 * cvs_resp_rcsdiff()
 *
 * Handler for the `Rcs-diff' response.
 */

static int
cvs_resp_rcsdiff(struct cvsroot *root, int type, char *line)
{
	char file[MAXPATHLEN], buf[MAXPATHLEN], cksum_buf[CVS_CKSUM_LEN];
	char *fname, *orig, *patch;
	mode_t fmode;
	BUF *res, *fcont, *patchbuf;
	CVSENTRIES *entf;
	struct cvs_ent *ent;

	/* get remote path and build local path of file to be patched */
	cvs_getln(root, buf, sizeof(buf));
	fname = strrchr(buf, '/');
	if (fname == NULL)
		fname = buf;
	snprintf(file, sizeof(file), "%s%s", line, fname);

	/* get updated entry fields */
	cvs_getln(root, buf, sizeof(buf));
	ent = cvs_ent_parse(buf);
	if (ent == NULL) {
		return (-1);
	}

	patchbuf = cvs_recvfile(root, &fmode);
	fcont = cvs_buf_load(file, BUF_AUTOEXT);
	if (fcont == NULL)
		return (-1);

	cvs_buf_putc(patchbuf, '\0');
	cvs_buf_putc(fcont, '\0');
	orig = cvs_buf_release(fcont);
	patch = cvs_buf_release(patchbuf);

	res = rcs_patch(orig, patch);
	if (res == NULL)
		return (-1);

	cvs_buf_write(res, file, fmode);

	/* now see if there is a checksum */
	if (cvs_fcksum != NULL) {
		if (cvs_cksum(file, cksum_buf, sizeof(cksum_buf)) < 0) {
		}

		if (strcmp(cksum_buf, cvs_fcksum) != 0) {
			cvs_log(LP_ERR, "checksum error on received file");
			(void)unlink(file);
		}

		free(cvs_fcksum);
		cvs_fcksum = NULL;
	}

	/* update revision in entries */
	entf = cvs_ent_open(line, O_WRONLY);
	if (entf == NULL)
		return (-1);

	cvs_ent_close(entf);

	return (0);
}


/*
 * cvs_resp_template()
 *
 * Handler for the `Template' response.
 */

static int
cvs_resp_template(struct cvsroot *root, int type, char *line)
{
	mode_t mode;
	BUF *tmpl;

	tmpl = cvs_recvfile(root, &mode);
	if (tmpl == NULL)
		return (-1);

	return (0);
}

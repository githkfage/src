/*	$OpenBSD: cvs.h,v 1.16 2004/07/30 01:49:22 jfb Exp $	*/
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

#ifndef CVS_H
#define CVS_H

#include <sys/param.h>
#include <stdio.h>
#include <dirent.h>

#include "rcs.h"

#define CVS_VERSION    "OpenCVS 0.1"


#define CVS_HIST_CACHE     128
#define CVS_HIST_NBFLD     6


#define CVS_CKSUM_LEN      33     /* length of a CVS checksum string */


/* operations */
#define CVS_OP_ANY          0     /* all operations */
#define CVS_OP_ADD          1
#define CVS_OP_ANNOTATE     2
#define CVS_OP_CHECKOUT     3
#define CVS_OP_COMMIT       4
#define CVS_OP_DIFF         5
#define CVS_OP_HISTORY      6
#define CVS_OP_IMPORT       7
#define CVS_OP_INIT         8
#define CVS_OP_LOG          9
#define CVS_OP_REMOVE      10
#define CVS_OP_SERVER      11
#define CVS_OP_STATUS      12
#define CVS_OP_TAG         13
#define CVS_OP_UPDATE      14
#define CVS_OP_VERSION     15




/* methods */
#define CVS_METHOD_NONE       0
#define CVS_METHOD_LOCAL      1    /* local access */
#define CVS_METHOD_SERVER     2    /* tunnel through CVS_RSH */
#define CVS_METHOD_PSERVER    3    /* cvs pserver */
#define CVS_METHOD_KSERVER    4    /* kerberos */
#define CVS_METHOD_GSERVER    5    /* gssapi server */
#define CVS_METHOD_EXT        6
#define CVS_METHOD_FORK       7    /* local but fork */

#define CVS_CMD_MAXNAMELEN   16
#define CVS_CMD_MAXALIAS      2
#define CVS_CMD_MAXDESCRLEN  64


/* defaults */
#define CVS_RSH_DEFAULT     "ssh"
#define CVS_EDITOR_DEFAULT  "vi"


/* server-side paths */
#define CVS_PATH_ROOT         "CVSROOT"
#define CVS_PATH_COMMITINFO   CVS_PATH_ROOT "/commitinfo"
#define CVS_PATH_CONFIG       CVS_PATH_ROOT "/config"
#define CVS_PATH_CVSIGNORE    CVS_PATH_ROOT "/cvsignore"
#define CVS_PATH_CVSWRAPPERS  CVS_PATH_ROOT "/cvswrappers"
#define CVS_PATH_EDITINFO     CVS_PATH_ROOT "/editinfo"
#define CVS_PATH_HISTORY      CVS_PATH_ROOT "/history"
#define CVS_PATH_LOGINFO      CVS_PATH_ROOT "/loginfo"
#define CVS_PATH_MODULES      CVS_PATH_ROOT "/modules"
#define CVS_PATH_NOTIFY       CVS_PATH_ROOT "/notify"
#define CVS_PATH_RCSINFO      CVS_PATH_ROOT "/rcsinfo"
#define CVS_PATH_TAGINFO      CVS_PATH_ROOT "/taginfo"
#define CVS_PATH_VERIFYMSG    CVS_PATH_ROOT "/verifymsg"


/* client-side paths */
#define CVS_PATH_RC             ".cvsrc"
#define CVS_PATH_CVSDIR         "CVS"
#define CVS_PATH_ENTRIES        CVS_PATH_CVSDIR "/Entries"
#define CVS_PATH_STATICENTRIES  CVS_PATH_CVSDIR "/Entries.Static"
#define CVS_PATH_LOGENTRIES     CVS_PATH_CVSDIR "/Entries.Log"
#define CVS_PATH_ROOTSPEC       CVS_PATH_CVSDIR "/Root"
#define CVS_PATH_REPOSITORY     CVS_PATH_CVSDIR "/Repository"


struct cvs_file;
struct cvs_dir;


struct cvs_op {
	u_int  co_op;
	uid_t  co_uid;    /* user performing the operation */
	char  *co_path;   /* target path of the operation */
	char  *co_tag;    /* tag or branch, NULL if HEAD */
};



struct cvsroot {
	char   *cr_str;
	u_int   cr_method;
	char   *cr_buf;
	char   *cr_user;
	char   *cr_pass;
	char   *cr_host;
	char   *cr_dir;
	u_int   cr_port;
	u_int   cr_ref;

	/* connection data */
	FILE   *cr_srvin;
	FILE   *cr_srvout;
};


#define CF_STAT     0x01    /* allocate space for file stats */
#define CF_IGNORE   0x02    /* apply regular ignore rules */
#define CF_RECURSE  0x04    /* recurse on directory operations */
#define CF_SORT     0x08    /* all files are sorted alphabetically */
#define CF_KNOWN    0x10    /* only recurse in directories known to CVS */
#define CF_CREATE   0x20    /* create if file does not exist */


/*
 * The cvs_file structure is used to represent any file or directory within
 * the CVS tree's hierarchy.  The <cf_path> field is a path relative to the
 * directory in which the cvs command was executed.  The <cf_parent> field
 * points back to the parent node in the directory tree structure (it is
 * NULL if the directory is at the wd of the command).
 *
 * The <cf_cvstat> field gives the file's status with regards to the CVS
 * repository.  The file can be in any one of the CVS_FST_* states.
 * If the file's type is DT_DIR, then the <cf_ddat> pointer will point to
 * a cvs_dir structure containing data specific to the directory (such as
 * the contents of the directory's CVS/Entries, CVS/Root, etc.).
 */

#define CVS_FST_UNKNOWN   0
#define CVS_FST_UPTODATE  1
#define CVS_FST_MODIFIED  2
#define CVS_FST_ADDED     3
#define CVS_FST_REMOVED   4
#define CVS_FST_CONFLICT  5
#define CVS_FST_PATCHED   6


TAILQ_HEAD(cvs_flist, cvs_file);


typedef struct cvs_file {
	char            *cf_path;
	struct cvs_file *cf_parent;  /* parent directory (NULL if none) */
	char            *cf_name;
	u_int16_t        cf_cvstat;  /* cvs status of the file */
	u_int16_t        cf_type;    /* uses values from dirent.h */
	struct stat     *cf_stat;    /* only available with CF_STAT flag */
	struct cvs_dir  *cf_ddat;    /* only for directories */

	TAILQ_ENTRY(cvs_file)  cf_list;
} CVSFILE;


struct cvs_dir {
	struct cvsroot  *cd_root;
	char            *cd_repo;
	struct cvs_flist cd_files;
};

#define CVS_HIST_ADDED    'A'
#define CVS_HIST_EXPORT   'E'
#define CVS_HIST_RELEASE  'F'
#define CVS_HIST_MODIFIED 'M'
#define CVS_HIST_CHECKOUT 'O'
#define CVS_HIST_COMMIT   'R'
#define CVS_HIST_TAG      'T'


#define CVS_ENT_NONE    0
#define CVS_ENT_FILE    1
#define CVS_ENT_DIR     2


struct cvs_ent {
	char    *ce_line;
	char    *ce_buf;
	u_int    ce_type;
	char    *ce_name;
	RCSNUM  *ce_rev;
	char    *ce_timestamp;
	char    *ce_opts;
	char    *ce_tag;
	TAILQ_ENTRY(cvs_ent) ce_list;
};

typedef struct cvs_entries {
	char    *cef_path;
	FILE    *cef_file;

	TAILQ_HEAD(, cvs_ent) cef_ent;
	struct cvs_ent       *cef_cur;
} CVSENTRIES;



struct cvs_hent {
	char    ch_event;
	time_t  ch_date;
	uid_t   ch_uid;
	char   *ch_user;
	char   *ch_curdir;
	char   *ch_repo;
	RCSNUM *ch_rev;
	char   *ch_arg;
};


typedef struct cvs_histfile {
	int     chf_fd;
	char   *chf_buf;       /* read buffer */
	size_t  chf_blen;      /* buffer size */
	size_t  chf_bused;     /* bytes used in buffer */

	off_t   chf_off;       /* next read */
	u_int   chf_sindex;    /* history entry index of first in array */
	u_int   chf_cindex;    /* current index (for getnext()) */
	u_int   chf_nbhent;    /* number of valid entries in the array */

	struct cvs_hent chf_hent[CVS_HIST_CACHE];

} CVSHIST;


#ifdef CVS
extern struct cvsroot *cvs_root;
#endif




/* client command handlers */
int  cvs_add      (int, char **);
int  cvs_checkout (int, char **);
int  cvs_commit   (int, char **);
int  cvs_diff     (int, char **);
int  cvs_getlog   (int, char **);
int  cvs_history  (int, char **);
int  cvs_init     (int, char **);
int  cvs_server   (int, char **);
int  cvs_status   (int, char **);
int  cvs_update   (int, char **);
int  cvs_version  (int, char **);


/* from client.c */
int     cvs_client_connect     (struct cvsroot *);
void    cvs_client_disconnect  (struct cvsroot *);
int     cvs_client_sendreq     (u_int, const char *, int);
int     cvs_client_sendarg     (const char *, int);
int     cvs_client_sendln      (const char *);
int     cvs_client_sendraw     (const void *, size_t);
ssize_t cvs_client_recvraw     (void *, size_t);
int     cvs_client_getln       (char *, size_t);
int     cvs_client_senddir     (const char *);


/* from root.c */
struct cvsroot*  cvsroot_parse (const char *);
void             cvsroot_free  (struct cvsroot *);
struct cvsroot*  cvsroot_get   (const char *);


/* from file.c */
int      cvs_file_init    (void);
int      cvs_file_ignore  (const char *);
int      cvs_file_chkign  (const char *);
CVSFILE* cvs_file_create  (const char *, u_int, mode_t);
CVSFILE* cvs_file_get     (const char *, int);
CVSFILE* cvs_file_getspec (char **, int, int);
void     cvs_file_free    (struct cvs_file *);
int      cvs_file_examine (CVSFILE *, int (*)(CVSFILE *, void *), void *);


/* Entries API */
CVSENTRIES*      cvs_ent_open   (const char *, int);
struct cvs_ent*  cvs_ent_get    (CVSENTRIES *, const char *);
struct cvs_ent*  cvs_ent_next   (CVSENTRIES *);
int              cvs_ent_add    (CVSENTRIES *, struct cvs_ent *);
int              cvs_ent_addln  (CVSENTRIES *, const char *);
int              cvs_ent_remove (CVSENTRIES *, const char *);
struct cvs_ent*  cvs_ent_parse  (const char *);
void             cvs_ent_close  (CVSENTRIES *);
void             cvs_ent_free   (struct cvs_ent *);
struct cvs_ent*  cvs_ent_getent (const char *);

/* history API */
CVSHIST*         cvs_hist_open    (const char *);
void             cvs_hist_close   (CVSHIST *);
int              cvs_hist_parse   (CVSHIST *);
struct cvs_hent* cvs_hist_getnext (CVSHIST *);
int              cvs_hist_append  (CVSHIST *, struct cvs_hent *);


/* from util.c */
int    cvs_readrepo   (const char *, char *, size_t);
int    cvs_splitpath  (const char *, char *, size_t, char *, size_t);
int    cvs_modetostr  (mode_t, char *, size_t);
int    cvs_strtomode  (const char *, mode_t *);
int    cvs_mkadmin    (struct cvs_file *, mode_t);
int    cvs_cksum      (const char *, char *, size_t);
int    cvs_exec       (int, char **, int []);
int    cvs_getargv    (const char *, char **, int);
void   cvs_freeargv   (char **, int);


#endif /* CVS_H */

/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "logserv.h"
#include "logdefault.h"
#include "logmirc.h"
#include "logxchat.h"
#include "logeggdrop.h"

/* only check logfile size every X calls */
#ifdef DEBUG
#define DOSIZE 1
#else 
#define DOSIZE 50
#endif


char timebuf[TIMEBUFSIZE];
char startlog[BUFSIZE];

logtype_proc logging_funcs[] = {
	{logserv_joinproc, logserv_partproc, logserv_msgproc, logserv_quitproc, logserv_topicproc, logserv_kickproc, logserv_nickproc, logserv_modeproc},
	{egg_joinproc, egg_partproc, egg_msgproc, egg_quitproc, egg_topicproc, egg_kickproc, egg_nickproc, egg_modeproc},
	{mirc_joinproc, mirc_partproc, mirc_msgproc, mirc_quitproc, mirc_topicproc, mirc_kickproc, mirc_nickproc, mirc_modeproc},
	{xchat_joinproc, xchat_partproc, xchat_msgproc, xchat_quitproc, xchat_topicproc, xchat_kickproc, xchat_nickproc, xchat_modeproc},
};

static int lgs_open_log(ChannelLog *cl);
static void lgs_stat_file(ChannelLog *cl);

/* @brief write a formatted log to the log files, and check if we should switch th logfile
 * 
 * writes a message to a log file. If the logfile hasn't been opened yet, we 
 * open it, and write out any headers required.
 * 
 * @params cl The ChannelLog structreu for the channel we are loggin
 * @params fmt the printf style format of the log message
 */


void lgs_write_log(ChannelLog *cl, char *fmt, ...) {
	va_list ap;
	char log_buf[BUFSIZE];
	
	/* format the string to write */
	va_start (ap, fmt);
	ircvsnprintf (log_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	                                
	/* if the FD isn't opened yet, lets open a log file */
	if (!(cl->flags & LGSFDOPENED)) {
		if (lgs_open_log(cl) != NS_SUCCESS) {
			return;
		}
		/* ok, we just opened the file, write the start out */
		switch (LogServ.logtype) {
			case 0:
				fprintf(cl->logfile, "%s", logserv_startlog(cl));
				break;
			case 1:
				fprintf(cl->logfile, "%s", egg_startlog(cl));
				break;
			case 2:
				fprintf(cl->logfile, "%s", mirc_startlog(cl));
				break;
			case 3:
				fprintf(cl->logfile, "%s", xchat_startlog(cl));
				break;
			default:
				nlog (LOG_WARNING, "Unknown LogType");
		}
	}
	/* ok, file is opened. write the string to it */
#ifdef DEBUG
	printf("%s\n", log_buf);
#endif
	fprintf(cl->logfile, "%s", log_buf);
	cl->dostat++;
#ifdef DEBUG
	/* only flush the logfile in debug mode */
	fflush(cl->logfile);
#endif
	/* ok, now stat the file to check size */
	if (cl->dostat >= DOSIZE) { 
		fflush(cl->logfile);
		lgs_stat_file(cl);
	}
}

/* @brief Opens the log file, creating directories where necessary
 *
 * @param cl the channel log function
 *
 * @returns a boolen indicating success/failure
 */ 

static int lgs_open_log(ChannelLog *cl) {
	struct stat st;
	int res;
	char fname[MAXPATH];

	/* first, make sure the logdir dir exists */
	res = stat(LogServ.logdir, &st);
	if (res != 0) {
		/* hrm, error */
		if (errno == ENOENT) {
			/* ok, it doesn't exist, create it */
			res = sys_mkdir (LogServ.logdir, 0700);
			if (res != 0) {
				/* error */
				nlog (LOG_CRITICAL, "Couldn't create LogDir Directory: %s", strerror(errno));
				return NS_FAILURE;
			}
			nlog (LOG_NOTICE, "Created Channel Logging Dir %s", LogServ.logdir);
		} else {
			nlog (LOG_CRITICAL, "Stat Returned A error: %s", strerror(errno));
			return NS_FAILURE;
		}
	} else if (!S_ISDIR(st.st_mode))	{
		nlog (LOG_CRITICAL, "%s is not a Directory", LogServ.logdir);
		return NS_FAILURE;
	}
	
	/* copy name to the filename holder (in case of invalid paths) */
	strlcpy(cl->filename, cl->channame, MAXPATH);
	ircsnprintf(fname, MAXPATH, "%s/%s.log", LogServ.logdir, make_safe_filename (cl->filename));

	/* open the file */
	cl->logfile = fopen(fname, "a");
	if (!cl->logfile) {
		nlog (LOG_CRITICAL, "Could not open %s for Appending: %s", cl->filename, strerror(errno));
		return NS_FAILURE;
	}
	dlog (DEBUG1, "Opened %s for Appending", cl->filename);
	/* set hte flag */
	cl->flags |= LGSFDOPENED;
	cl->fdopened = me.now;
	return NS_SUCCESS;
}

/* @brief check the logfile size, and rotate if necessary
 * 
 * @param cl the ChannelLog struct for the channel we are checking 
 */ 
static void lgs_stat_file(ChannelLog *cl) {
	struct stat st;
	int res;
	char fname[MAXPATH];
	
	/* reset this counter */
	cl->dostat = 0;
	/* construct the filename to stat */
	ircsnprintf(fname, MAXPATH, "%s/%s.log", LogServ.logdir, cl->filename);
	res = stat(fname, &st);
	if (res != 0) {
		if (errno == ENOENT) {
			/* wtf, this is bad */
			nlog (LOG_CRITICAL, "LogFile went away: %s", fname);
			return;
		} else {
			nlog (LOG_CRITICAL, "Logfile Error: %s", strerror(errno));
			return;
		}
	}
	dlog (DEBUG1, "Logfile Size of %s is %d", fname, (int)st.st_size);
	if (st.st_size > LogServ.maxlogsize) {
		dlog (DEBUG1, "Switching Logfile %s", fname);
		/* ok, the file exceeds out limits, lets switch it */
		lgs_switch_file(cl);
	}
}

/* @brief actually switch the logfile, saving the old log in a different directory
 *
 * @param cl the ChannelLog struct 
 */
void lgs_switch_file(ChannelLog *cl) {
	struct stat st;
	char tmbuf[MAXPATH];
	char newfname[MAXPATH];
	char oldfname[MAXPATH];
	char savedir[MAXPATH];
	int res;

	if (!(cl->flags & LGSFDOPENED)) {
		/* no need to switch, its not opened */
		return;
	}
	/* close the logfile */
	fclose(cl->logfile);
	cl->fdopened = 0;
	cl->flags &= ~ LGSFDOPENED;
	/* check if the target directory exists */
	ircsnprintf(savedir, MAXPATH, "%s/%s", LogServ.savedir, cl->filename);
	
	res = stat(savedir, &st);
	if (res != 0) {
		/* hrm, error */
		if (errno == ENOENT) {
			/* ok, it doesn't exist, create it */
			res = sys_mkdir (savedir, 0700);
			if (res != 0) {
				/* error */
				nlog (LOG_CRITICAL, "Couldn't create LogDir Directory: %s", strerror(errno));
				return;
			}
			nlog (LOG_NOTICE, "Created Channel Logging Dir %s", savedir);
		} else {
			nlog (LOG_CRITICAL, "Stat Returned A error: %s", strerror(errno));
			return;
		}
	} else if (!S_ISDIR(st.st_mode))	{
		nlog (LOG_CRITICAL, "%s is not a Directory", savedir);
		return;
	}
	strftime(tmbuf, MAXPATH, "%d%m%Y%H%M%S", localtime(&me.now));
	ircsnprintf(newfname, MAXPATH, "%s/%s-%s.log", savedir, cl->filename, tmbuf);
	ircsnprintf(oldfname, MAXPATH, "%s/%s.log", LogServ.logdir, cl->filename);
	res = rename(oldfname, newfname);
	if (res != 0) {
		nlog (LOG_CRITICAL, "Couldn't Rename file %s: %s", oldfname, strerror(errno));
	}	
	nlog (LOG_NORMAL, "Switched Logfile for %s from %s to %s", cl->channame, oldfname, newfname);
}
/* @brief Close all logfiles and delete the struct assocated with them
 *
 * Called from ModFini when we are unloaded, to cleanup
 */

void lgs_close_logs() {
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;
	Channel *c;
	
	/* scan through the log files */
	hash_scan_begin(&hs, lgschans);
	while (( hn = hash_scan_next(&hs)) != NULL) {
		cl = hnode_get(hn);
		/* if the log is opened then close it */
		if (cl->flags & LGSFDOPENED) {
			fclose(cl->logfile);
		}
		/* delete them from the hash */
		c = cl->c;
		if (c) {
			c->moddata[lgs_module->modnum] = NULL;
			cl->c = NULL;
		}
		hash_delete(lgschans, hn);
		hnode_destroy(hn);
		free(cl);
	}

}

/* @brief Rotate the log files if they have been opened longer than the pre-defined time
 * 
 * Runs through all active opened logfiles only
 */
int lgs_RotateLogs(void) 
{
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;
	
	/* if Logage is 0, just bail out */
	if (LogServ.maxopentime <= 0) {
		return NS_SUCCESS;
	}

	/* scan through the log files */
	hash_scan_begin(&hs, lgschans);
	while (( hn = hash_scan_next(&hs)) != NULL) {
		cl = hnode_get(hn);
		/* if the log has been opened more than X, then rotate */
		if ((cl->flags & LGSFDOPENED) && ((me.now - cl->fdopened) > LogServ.maxopentime)) {
			lgs_switch_file(cl);
		}
	}
	return NS_SUCCESS;
}

/* @breif Send the Log Message to the relevent Log Processor 
 *
 * @param msgtype the type of message (join, part etc)
 * @param av contents of the message 
 * @param ac message size 
 * @returns NS_SUCCESS or NS_FAILURE
 */
int lgs_send_to_logproc( logmsgtype msgtype, ChannelLog *lgschan, CmdParams* cmdparams) {
	switch (msgtype) {
		case 0:
			return logging_funcs[LogServ.logtype].joinproc(lgschan, cmdparams);
			break;
		case 1:
			return logging_funcs[LogServ.logtype].partproc(lgschan, cmdparams);
			break;
		case 2:
			return logging_funcs[LogServ.logtype].msgproc(lgschan, cmdparams);
			break;
		case 3:
			return logging_funcs[LogServ.logtype].quitproc(lgschan, cmdparams);
			break;
		case 4: 
			return logging_funcs[LogServ.logtype].topicproc(lgschan, cmdparams);
			break;
		case 5:
			return logging_funcs[LogServ.logtype].kickproc(lgschan, cmdparams);
			break;
		case 6:
			return logging_funcs[LogServ.logtype].nickproc(lgschan, cmdparams);
			break;
		case 7:
			return logging_funcs[LogServ.logtype].modeproc(lgschan, cmdparams);
			break;
		default:
			nlog (LOG_WARNING, "Unknown msgtype %d", msgtype);
			return NS_FAILURE;
			break;
	}	
	return NS_FAILURE;
}

#ifdef WIN32
/* temporary work around for linker error */
void main(void) {}
#endif

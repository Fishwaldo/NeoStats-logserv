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

#include "logserv.h"
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
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

log_proc logging_funcs[][LGSMSG_NUMTYPES] = {
	{logserv_joinproc, logserv_partproc, logserv_msgproc, logserv_noticeproc, logserv_ctcpaction, logserv_quitproc, logserv_topicproc, logserv_kickproc, logserv_nickproc, logserv_modeproc},
	{egg_joinproc, egg_partproc, egg_msgproc, egg_noticeproc, egg_ctcpaction, egg_quitproc, egg_topicproc, egg_kickproc, egg_nickproc, egg_modeproc},
	{mirc_joinproc, mirc_partproc, mirc_msgproc, mirc_noticeproc, mirc_ctcpaction, mirc_quitproc, mirc_topicproc, mirc_kickproc, mirc_nickproc, mirc_modeproc},
	{xchat_joinproc, xchat_partproc, xchat_msgproc, xchat_noticeproc, xchat_ctcpaction, xchat_quitproc, xchat_topicproc, xchat_kickproc, xchat_nickproc, xchat_modeproc},
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
	if (!(cl->logfile)) {
		if (lgs_open_log(cl) != NS_SUCCESS) {
			return;
		}
		/* ok, we just opened the file, write the start out */
		switch (LogServ.logtype) {
			case 0:
				os_fprintf (cl->logfile, "%s", logserv_startlog(cl));
				break;
			case 1:
				os_fprintf (cl->logfile, "%s", egg_startlog(cl));
				break;
			case 2:
				os_fprintf (cl->logfile, "%s", mirc_startlog(cl));
				break;
			case 3:
				os_fprintf (cl->logfile, "%s", xchat_startlog(cl));
				break;
			default:
				nlog (LOG_WARNING, "Unknown LogType");
		}
	}
	/* ok, file is opened. write the string to it */
#ifdef DEBUG
	printf("%s\n", log_buf);
#endif
	os_fprintf (cl->logfile, "%s", log_buf);
	cl->dostat++;
#ifdef DEBUG
	/* only flush the logfile in debug mode */
	os_fflush (cl->logfile);
#endif
	/* ok, now stat the file to check size */
	if (cl->dostat >= DOSIZE) { 
		os_fflush (cl->logfile);
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
	char fname[MAXPATH];

	/* first, make sure the logdir dir exists */
	if (os_check_create_dir (LogServ.logdir) != NS_SUCCESS)
	{
		return NS_FAILURE;
	}
	/* copy name to the filename holder (in case of invalid paths) */
	strlcpy(cl->filename, cl->channame, MAXPATH);
	ircsnprintf(fname, MAXPATH, "%s/%s.log", LogServ.logdir, make_safe_filename (cl->filename));

	/* open the file */
	cl->logfile = os_fopen (fname, "a");
	if (!cl->logfile) {
		nlog (LOG_CRITICAL, "Could not open %s for Appending: %s", cl->filename, os_strerror ());
		return NS_FAILURE;
	}
	dlog (DEBUG1, "Opened %s for Appending", cl->filename);
	cl->fdopened = me.now;
	return NS_SUCCESS;
}

/* @brief check the logfile size, and rotate if necessary
 * 
 * @param cl the ChannelLog struct for the channel we are checking 
 */ 
static void lgs_stat_file(ChannelLog *cl) 
{
	int filesize = 0;
	char fname[MAXPATH];
	
	/* reset this counter */
	cl->dostat = 0;
	/* construct the filename to stat */
	ircsnprintf(fname, MAXPATH, "%s/%s.log", LogServ.logdir, cl->filename);
	filesize = os_file_get_size (fname);
	if (filesize <= 0) {
		return;
	}
	dlog (DEBUG1, "Logfile Size of %s is %d", fname, filesize);
	if (filesize > LogServ.maxlogsize) {
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
	char tmbuf[MAXPATH];
	char newfname[MAXPATH];
	char oldfname[MAXPATH];
	int res;

	if (!(cl->logfile)) {
		/* no need to switch, its not opened */
		return;
	}
	/* close the logfile */
	os_fclose (cl->logfile);
	cl->fdopened = 0;
	/* check if the target directory exists */
	if (os_check_create_dir (LogServ.savedir) != NS_SUCCESS)
	{
		return;
	}
	os_strftime (tmbuf, MAXPATH, "%d%m%Y%H%M%S", os_localtime (&me.now));
	ircsnprintf(newfname, MAXPATH, "%s/%s-%s.log", LogServ.savedir, cl->filename, tmbuf);
	ircsnprintf(oldfname, MAXPATH, "%s/%s.log", LogServ.logdir, cl->filename);
	res = os_rename (oldfname, newfname);
	if (res != 0) {
		nlog (LOG_CRITICAL, "Couldn't rename file %s: %s", oldfname, os_strerror ());
		return;
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
		if (cl->logfile) {
			os_fclose (cl->logfile);
		}
		/* delete them from the hash */
		c = cl->c;
		if (c) {
			clear_channel_moddata (c);
			cl->c = NULL;
		}
		hash_delete(lgschans, hn);
		hnode_destroy(hn);
		ns_free (cl);
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
		if ((cl->logfile) && ((me.now - cl->fdopened) > LogServ.maxopentime)) {
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
int lgs_send_to_logproc(LGSMSG_TYPE msgtype, Channel *c, CmdParams* cmdparams) 
{
	ChannelLog *cl;

	if (c)
	{
		cl = (ChannelLog *)get_channel_moddata (c);
		if (cl)
		{
			return logging_funcs[LogServ.logtype][msgtype](cl, cmdparams);
		}
	}
	return NS_FAILURE;
}

/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Justin Hammond
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

/** template.c 
 * You can copy this file as a template for writing your own modules
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dl.h"       /* Required for module */
#include "stats.h"    /* Required for bot support */
#include "log.h"      /* Log systems support */
#include "conf.h"     /* GetConf support */
#include "logserv.h"  /* LogServ Definitions */

/* only check logfile size every X calls */
#ifdef DEBUG
#define DOSIZE 1
#else 
#define DOSIZE 50
#endif


char timebuf[TIMEBUFSIZE];
char startlog[BUFSIZE];

static void lgs_write_log(ChannelLog *cl, char *fmt, ...) __attribute__((format(printf,2,3)));
static int lgs_open_log(ChannelLog *cl);
static void lgs_stat_file(ChannelLog *cl);

static char *logserv_startlog(ChannelLog *cl);
static char *mirc_startlog(ChannelLog *cl);
static char *egg_startlog(ChannelLog *cl);
static char *xchat_startlog(ChannelLog *cl);

/* This was taken from DircProxy 
 * but is actually a pretty handy function. 
 * maybe it should go in the core? 
 */

#ifndef safe_channame

/* @brief Make the name of a logfile safe for a filename
 * 
 * given a name, make sure its a safe name for a filename
 * 
 * @params name the name to check. Warning, the name is modified
 *
 * @returns a modified version of the name, that is safe to use as a filename
 */

static char *_irclog_safe(char *name) {
  char *ptr;

  /* Channel names are allowed to contain . and / according to the IRC
     protocol.  These are nasty as it means someone could theoretically
     create a channel called #/../../etc/passwd and the program would try
     to unlink "/tmp/#/../../etc/passwd" = "/etc/passwd".  If running as root
     this could be bad.  So to compensate we replace '/' with ':' as thats not
     valid in channel names. */
  ptr = name;
  while (*ptr) {
    switch (*ptr) {
      case '/':
        *ptr = ':';
        break;
    }

    ptr++;
  }

  return name;
}

#define safe_channame(x) _irclog_safe(x) 
#endif

/* @brief write a formatted log to the log files, and check if we should switch th logfile
 * 
 * writes a message to a log file. If the logfile hasn't been opened yet, we 
 * open it, and write out any headers required.
 * 
 * @params cl The ChannelLog structreu for the channel we are loggin
 * @params fmt the printf style format of the log message
 */


static void lgs_write_log(ChannelLog *cl, char *fmt, ...) {
	va_list ap;
	char log_buf[BUFSIZE];
	
	/* format the string to write */
        va_start (ap, fmt);
        ircvsnprintf (log_buf, BUFSIZE, fmt, ap);
        va_end (ap);
	                                
	/* if the FD isn't opened yet, lets open a log file */
	if (!(cl->flags & LGSFDOPENED)) {
		if (!lgs_open_log(cl)) {
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
				nlog(LOG_WARNING, LOG_MOD, "Unknown LogType");
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
			res = mkdir(LogServ.logdir, 0700);
			if (res != 0) {
				/* error */
				nlog(LOG_CRITICAL, LOG_MOD, "Couldn't create LogDir Directory: %s", strerror(errno));
				return NS_FAILURE;
			}
			nlog(LOG_NOTICE, LOG_MOD, "Created Channel Logging Dir %s", LogServ.logdir);
			return NS_SUCCESS;
		} else {
			nlog(LOG_CRITICAL, LOG_MOD, "Stat Returned A error: %s", strerror(errno));
			return NS_FAILURE;
		}
	} else if (!S_ISDIR(st.st_mode))	{
		nlog(LOG_CRITICAL, LOG_MOD, "%s is not a Directory", LogServ.logdir);
		return NS_FAILURE;
	}
	
	/* copy name to the filename holder (in case of invalid paths) */
	strlcpy(cl->filename, cl->channame, MAXPATH);
	ircsnprintf(fname, MAXPATH, "%s/%s.log", LogServ.logdir, safe_channame(cl->filename));

	/* open the file */
	cl->logfile = fopen(fname, "a");
	if (!cl->logfile) {
		nlog(LOG_CRITICAL, LOG_MOD, "Could not open %s for Appending: %s", cl->filename, strerror(errno));
		return NS_FAILURE;
	}
	nlog(LOG_DEBUG1, LOG_MOD, "Opened %s for Appending", cl->filename);
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
			nlog(LOG_CRITICAL, LOG_MOD, "LogFile went away: %s", fname);
			return;
		} else {
			nlog(LOG_CRITICAL, LOG_MOD, "Logfile Error: %s", strerror(errno));
			return;
		}
	}
	nlog(LOG_DEBUG1, LOG_MOD, "Logfile Size of %s is %d", fname, st.st_size);
	if (st.st_size > LogServ.maxlogsize) {
		nlog(LOG_DEBUG1, LOG_MOD, "Switching Logfile %s", fname);
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
	ircsnprintf(savedir, MAXPATH, "%s/\%s", LogServ.savedir, cl->filename);
	
	res = stat(savedir, &st);
	if (res != 0) {
		/* hrm, error */
		if (errno == ENOENT) {
			/* ok, it doesn't exist, create it */
			res = mkdir(savedir, 0700);
			if (res != 0) {
				/* error */
				nlog(LOG_CRITICAL, LOG_MOD, "Couldn't create LogDir Directory: %s", strerror(errno));
				return;
			}
			nlog(LOG_NOTICE, LOG_MOD, "Created Channel Logging Dir %s", savedir);
		} else {
			nlog(LOG_CRITICAL, LOG_MOD, "Stat Returned A error: %s", strerror(errno));
			return;
		}
	} else if (!S_ISDIR(st.st_mode))	{
		nlog(LOG_CRITICAL, LOG_MOD, "%s is not a Directory", savedir);
		return;
	}
	strftime(tmbuf, MAXPATH, "%d%m%Y%H%M%S", localtime(&me.now));
	ircsnprintf(newfname, MAXPATH, "%s/%s-%s.log", savedir, cl->filename, tmbuf);
	ircsnprintf(oldfname, MAXPATH, "%s/%s.log", LogServ.logdir, cl->filename);
	res = rename(oldfname, newfname);
	if (res != 0) {
		nlog(LOG_CRITICAL, LOG_MOD, "Couldn't Rename file %s: %s", oldfname, strerror(errno));
	}	
	nlog(LOG_NORMAL, LOG_MOD, "Switched Logfile for %s from %s to %s", cl->channame, oldfname, newfname);
}
/* @brief Close all logfiles and delete the struct assocated with them
 *
 * Called from ModFini when we are unloaded, to cleanup
 */

void lgs_close_logs() {
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;
	Chans *c;
	
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
			c->moddata[LogServ.modnum] = NULL;
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
void lgs_RotateLogs() {
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;
	
	/* scan through the log files */
	hash_scan_begin(&hs, lgschans);
	while (( hn = hash_scan_next(&hs)) != NULL) {
		cl = hnode_get(hn);
		/* if the log has been opened more than X, then rotate */
		if ((cl->flags & LGSFDOPENED) && ((me.now - cl->fdopened) > LogServ.maxopentime)) {
			lgs_switch_file(cl);
		}
	}
}


char *logserv_startlog(ChannelLog *chandata) {
	return startlog;
}


int logserv_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int logserv_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int logserv_msgproc(ChannelLog *chandata, char **av, int ac) {
printf("doing msgproc\n");
	return NS_SUCCESS;
}

int logserv_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int logserv_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int logserv_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
int logserv_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
int logserv_modeproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [00:00] --- Fri Jan  2 2004 */
/* this usually goes at the end of a logfile. Hrm */

char *egg_startlog(ChannelLog *cl) {
	strftime(startlog, BUFSIZE, "[%H:%M] --- %a %b %d %Y\n", localtime(&me.now));

	return startlog;
}
#define EGGTIME "[%H:%M]"

char *egg_time() {
	strftime(timebuf, TIMEBUFSIZE, EGGTIME, localtime(&me.now));
	return timebuf;
}


/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) joined #neostats. */
#define EJOINPROC "%s %s (%s@%s) joined %s\n"

int egg_joinproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u)
		lgs_write_log(chandata, EJOINPROC, egg_time(), u->nick, u->username, u->vhost, av[0]);
	return NS_SUCCESS;
}

/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) left #neostats. */
#define EPARTPROC "%s %s (%s@%s) left %s\n"

int egg_partproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u) 
		lgs_write_log(chandata, EPARTPROC, egg_time(), u->nick, u->username, u->vhost, av[0]);
	
	return NS_SUCCESS;
}

/* [22:04] <Fish> I think that is it
 * [22:04] Action: Fish waits for the eggdrop to flush its logs now
 */
#define EMSGPROC "%s <%s> %s\n"
#define EACTPROC "%s Action: %s %s\n"

int egg_msgproc(ChannelLog *chandata, char **av, int ac) {
	if (ac == 3) 
		lgs_write_log(chandata, EACTPROC, egg_time(), av[0], av[2]);
	else 
		lgs_write_log(chandata, EMSGPROC, egg_time(), av[0], av[1]);
	return NS_SUCCESS;
}

/* [22:03] Fishwaldo (~Fish@3cbc6b2b.22d996b6.singnet.com.sg) left irc: Client closed */
#define EQUITPROC "%s %s (%s@%s) left irc: %s\n"

int egg_quitproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[0]);
	if (u) 
		lgs_write_log(chandata, EQUITPROC, egg_time(), u->nick, u->username, u->vhost, ac == 2 ? av[1] : "");
		
	return NS_SUCCESS;
}

/* [22:02] Topic changed on #neostats by Fish!~Fish@Server-Admin.irc-chat.net: <LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net */
#define ETOPICPROC "%s Topic changed on %s by %s!%s@%s: %s\n"
#define ENOUSERTOPICPROC "%s Topic changed on %s by %s: %s\n"

int egg_topicproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u)
		lgs_write_log(chandata, ETOPICPROC, egg_time(), av[0], u->nick, u->username, u->vhost, ac == 3 ? av[2] : "");
	else 
		lgs_write_log(chandata, ENOUSERTOPICPROC, egg_time(), av[0], av[1], ac == 3 ? av[2] : "");
	return NS_SUCCESS;
}

/* [22:02] Fish kicked from #neostats by Fish: Fish */
#define EKICKPROC "%s %s kicked from %s by %s: %s\n"

int egg_kickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, EKICKPROC, egg_time(), av[1], av[0], av[2], ac == 4 ? av[3] : ""); 
	return NS_SUCCESS;
}

/* [22:02] #NeoStats: mode change '+oa Fish Fish' by SuperSexSquirrel!super@supersexsquirrel.org */
#define EMODEPROC "%s %s: mode change '%s' by %s!%s@%s\n"
#define ENOUSERMODEPROC "%s %s: mode change '%s' by %s\n"
int egg_modeproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	char *modebuf;
	
	modebuf = joinbuf(av, ac, 2);
	u = finduser(av[0]);
	if (u) {
		lgs_write_log(chandata, EMODEPROC, egg_time(), av[1], modebuf, u->nick, u->username, u->vhost);
	} else {
		lgs_write_log(chandata, ENOUSERMODEPROC, egg_time(), av[1], modebuf, av[0]);
	}	
	return NS_SUCCESS;
}

/* [22:02] Nick change: Fish -> haha */
#define ENICKPROC "%s Nick change: %s -> %s\n"

int egg_nickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, ENICKPROC, egg_time(), av[0], av[1]);
	return NS_SUCCESS;
}

/* Session Start: Fri Jan 02 21:46:22 2004
 * Session Ident: #neostats
 * [21:46] * Now talking in #neostats
 * [21:46] * Topic is '<LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net :: NeoStats-2.5.11 :: OPSB 2.0 Beta1 Released :: StupidServ 1.2 Released :: SecureServ 1.0 :: Happy Holidays from the entire NeoStats Team, including the trained monkey'
 * [21:46] * Set by LuShes on Fri Jan 02 11:11:57
 */

#define MSTARTLOG "Session Start: %s\nSession Ident: %s\n%s * Now talking in %s\n%s * Topic is '%s'\n%s * Set by %s on %s\n"

char *mirc_startlog(ChannelLog *cl) {
	char timebuf2[TIMEBUFSIZE];
	char timebuf3[TIMEBUFSIZE];
	
	strftime(timebuf, TIMEBUFSIZE, "%a %b %d %H:%M:%S %Y", localtime(&me.now));
	strftime(timebuf2, TIMEBUFSIZE, "[%H:%M]", localtime(&me.now));
	strftime(timebuf3, TIMEBUFSIZE, "%a %b %d %H:%M:%S", localtime(&cl->c->topictime));
	ircsnprintf(startlog, BUFSIZE, MSTARTLOG, timebuf, cl->channame, timebuf2, cl->channame, timebuf2, cl->c->topic[0] != '0' ? cl->c->topic : "", timebuf2, cl->c->topicowner[0] != '0' ? cl->c->topicowner: "", timebuf3);

	return startlog;
}

#define MIRCTIME "[%H:%M]"
char *mirc_time() {
	strftime(timebuf, TIMEBUFSIZE, MIRCTIME, localtime(&me.now));
	return timebuf;
}


/* [21:47] * Dirk-Digler has joined #neostats */
#define MJOINPROC "%s * %s (%s@%s) has joined %s\n"

int mirc_joinproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	lgs_write_log(chandata, MJOINPROC, mirc_time(), u->nick, u->username, u->vhost, av[0]);
	return NS_SUCCESS;
}

/* [22:07] * DigiGuy has left #neostats */
#define MPARTPROC "%s * %s (%s@%s) has left %s (%s)\n"

int mirc_partproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	lgs_write_log(chandata, MPARTPROC, mirc_time(), u->nick, u->username, u->vhost, av[0], ac == 3 ? av[2] : "");
	return NS_SUCCESS;
}

/* [21:47] <Digi|Away> yes we are feeling nice today */
/* [21:47] * Fish does a action for Digi|Away's log */
#define MMSGPROC "%s <%s> %s\n"
#define MACTPROC "%s * %s %s\n"

int mirc_msgproc(ChannelLog *chandata, char **av, int ac) {
	if (ac == 3) {
		lgs_write_log(chandata, MACTPROC, mirc_time(), av[0], av[2]);
	} else {
		lgs_write_log(chandata, MMSGPROC, mirc_time(), av[0], av[1]);
	}
	return NS_SUCCESS;
}

/* [21:49] * DigiGuy has quit IRC (Quit: ha) */
#define MQUITPROC "%s * %s has quit IRC (%s)\n"

int mirc_quitproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, MQUITPROC, mirc_time(), av[0], ac == 2 ? av[1] : "");
	return NS_SUCCESS;
}

/* [21:48] * Digi|Away changes topic to 'FREE PORN - DETAILS ' */
#define MTOPICPROC "%s * %s changes topic to '%s'\n"

int mirc_topicproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, MTOPICPROC, mirc_time(), av[1], ac == 3 ? av[2] : "");
	return NS_SUCCESS;
}

/* [21:47] * Dirk-Digler was kicked by Fish (Fish) */
#define MKICKPROC "%s * %s was kicked by %s (%s)\n"

int mirc_kickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, MKICKPROC, mirc_time(), av[1], av[2], ac == 4 ? av[3] : "");
	return NS_SUCCESS;
}

/* [21:48] * Fish is now known as Fishy */
#define MNICKPROC "%s * %s is now known as %s\n"

int mirc_nickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, MNICKPROC, mirc_time(), av[0], av[1]);
	return NS_SUCCESS;
}

/* [21:47] * Fish sets mode: +o Dirk-Digler */
#define MMODEPROC "%s * %s sets mode: %s\n"

int mirc_modeproc(ChannelLog *chandata, char **av, int ac) {
	char *modebuf;
	
	modebuf = joinbuf(av, ac, 2);
	lgs_write_log(chandata, MMODEPROC, mirc_time(), av[0], modebuf);
	free(modebuf);
	return NS_SUCCESS;
}

/* XCHAT FORMAT LOGGING BELOW */

#define XCHATTIME "%b %d %H:%M:%S"

char *xchat_time() {
	strftime(timebuf, TIMEBUFSIZE, XCHATTIME, localtime(&me.now));
	return timebuf;
}
/* **** BEGIN LOGGING AT Fri Jan  2 17:25:15 2004
 * \n
 */
#define XSTARTLOG "**** BEGIN LOGGING AT %s\n\n"
 
char *xchat_startlog(ChannelLog *cl) {
	char tmbuf[TIMEBUFSIZE];
	strftime(tmbuf, TIMEBUFSIZE, "%a %b %d %H:%M:%S %Y", localtime(&me.now));
	ircsnprintf(startlog, BUFSIZE, XSTARTLOG, tmbuf);
	return startlog;
}


/* Jan 02 17:27:10 -->     Dirk-Digler (fish@Dirk-Digler.Users.irc-chat.net) has joined #neostats */
#define XJOINFMT "%s -->\t%s (%s@%s) has joined %s\n"

int xchat_joinproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u) 
		lgs_write_log(chandata, XJOINFMT, xchat_time(), u->nick, u->username, u->vhost, av[0]); return NS_SUCCESS;
}

/* Jan 02 17:56:52 <--     DigiGuy (~b.dole@Oper.irc-chat.net) has left #neostats (part)*/
#define XPARTPROC "%s <--\t%s (%s@%s) has left %s (%s)\n"

int xchat_partproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u)
		lgs_write_log(chandata, XPARTPROC, xchat_time(), u->nick, u->username, u->vhost, av[0], ac == 3 ? av[2] : "");

	return NS_SUCCESS;
}


/* Jan 02 17:25:43 <SecureServ>    Akilling jojo!~jojo@pD9E60152.dip.t-dialin.net for Virus IRCORK */

#define XMSGFMT "%s <%s>\t%s\n"

/* Action: 
 * Jan 02 17:28:52 *       Fish-Away sighs */
#define XACTFMT "%s *\t%s %s\n"

int xchat_msgproc(ChannelLog *chandata, char **av, int ac) {
	if (ac == 3) {
		lgs_write_log(chandata, XACTFMT, xchat_time(), av[0], av[2]);
	} else {
		lgs_write_log(chandata, XMSGFMT, xchat_time(), av[0], av[1]);
	}
	return NS_SUCCESS;
}
/* Jan 02 17:47:26 <--     Dirk-Digler has quit (Killed (Fish (get lost))) */
#define XQUITFMT "%s <--\t%s has quit (%s)\n"

int xchat_quitproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XQUITFMT, xchat_time(), av[0], ac == 2 ? av[1] : "");
	return NS_SUCCESS;
}

/* Jan 02 17:48:12 ---     Digi|Away has changed the topic to: FREE PORN - DETAILS INSIDE */
#define XTOPICPROC "%s ---\t%s has changed the topic to: %s\n"

int xchat_topicproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XTOPICPROC, xchat_time(), av[1], ac == 3 ? av[2] : "");
	return NS_SUCCESS;
}

/* Jan 02 17:27:10 <-- Fish-Away has kicked Dirk-Digler from #neostats (ha) */
#define XKICKPROC "%s <--\t%s has kicked %s from %s (%s)\n"

int xchat_kickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XKICKPROC, xchat_time(), av[2], av[1], av[0], ac == 4 ? av[3] : "");
	return NS_SUCCESS;
}

/* Jan 02 17:50:32 ---     DigiGuy is now known as Bob */
#define XNICKPROC "%s ---\t%s is now known as %s\n"

int xchat_nickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XNICKPROC, xchat_time(), av[0], av[1]);
	return NS_SUCCESS;
}

/* Jan 02 17:27:10 ---     SuperSexSquirrel sets modes [#NeoStats +v Dirk-Digler] */
#define XMODEPROC "%s ---\t%s sets modes[%s %s]\n"

int xchat_modeproc(ChannelLog *chandata, char **av, int ac) {
	char *modebuf;
	
	modebuf = joinbuf(av, ac, 2);
	lgs_write_log(chandata, XMODEPROC, xchat_time(), av[0], chandata->channame, modebuf);
	free(modebuf);
	return NS_SUCCESS;
}

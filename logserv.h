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

#ifndef LOGSERV_H
#define LOGSERV_H

#include "neostats.h"

/* Channel Logging Struct. */
typedef struct CL_ {
	char channame[MAXCHANLEN];
	Channel *c;
	FILE *logfile;
	char filename[MAXPATH];
	unsigned long flags;
	char statsurl[MAXPATH];
	time_t fdopened;
	int dostat;
} ChannelLog;


/* settings for LogServ */
struct LogServ {
	int logtype;
	int modnum;
	char logdir[MAXPATH];
	char savedir[MAXPATH];
	long maxlogsize;
	long maxopentime;
} LogServ;

/* Definitions for flags */
#define LGSFDOPENED 0x1
#define LGSFDNEEDFLUSH 0x2
#define LGSPUBSTATS 0x4
#define LGSACTIVE 0x8

/* the hash that stores the monitored channels */
hash_t *lgschans;

typedef enum {
	LGSMSG_JOIN=0,
	LGSMSG_PART,
	LGSMSG_MSG,
	LGSMSG_QUIT,
	LGSMSG_TOPIC,
	LGSMSG_KICK,
	LGSMSG_NICK,
	LGSMSG_CHANMODE
} logmsgtype;
	

/* logging function prototype */
typedef int (log_proc) (ChannelLog *chandata, CmdParams* cmdparams);

/* LogType Proc table */

typedef struct logtype_proc {
	log_proc *joinproc;
	log_proc *partproc;
	log_proc *msgproc;
	log_proc *quitproc;
	log_proc *topicproc;
	log_proc *kickproc;
	log_proc *nickproc;
	log_proc *modeproc;
} logtype_proc;

/* log_procssing.c decl */
int lgs_send_to_logproc (logmsgtype msgtype, ChannelLog *lgschan, CmdParams* cmdparams);
int lgs_RotateLogs(void);
void lgs_close_logs();
void lgs_switch_file(ChannelLog *cl);
void lgs_write_log(ChannelLog *cl, char *fmt, ...) __attribute__((format(printf,2,3)));

extern const char lgs_help_chan_oneline[];
extern const char lgs_help_stats_oneline[];
extern const char *ls_about[];
extern const char *lgs_help_chan[];
extern const char *lgs_help_stats[];
extern const char *lgs_help_set_logtype[];
extern const char *lgs_help_set_logsize[];
extern const char *lgs_help_set_logtime[];

extern char timebuf[TIMEBUFSIZE];
extern char startlog[BUFSIZE];

extern Module *lgs_module;

#endif

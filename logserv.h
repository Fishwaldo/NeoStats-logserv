/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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

#include "dl.h"
#include "stats.h"

#ifndef NS_ULEVEL_LOCOPER
#define NS_ULEVEL_LOCOPER 40
#endif


/** 
 * A string to hold the name of our bot
 */
char s_LogServ[MAXNICK];

/** 
 * out ModUser struct 
 */
ModUser *lgs_bot;

/* Channel Logging Struct. */
typedef struct CL_ {
	char channame[CHANLEN];
	Chans *c;
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
typedef int (log_proc) (ChannelLog *, char **, int);

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


int logserv_joinproc(ChannelLog *, char **, int);
int logserv_partproc(ChannelLog *, char **, int);
int logserv_msgproc(ChannelLog *, char **, int);
int logserv_quitproc(ChannelLog *, char **, int);
int logserv_topicproc(ChannelLog *, char **, int);
int logserv_kickproc(ChannelLog *, char **, int);
int logserv_nickproc(ChannelLog *, char **, int);
int logserv_modeproc(ChannelLog *, char **, int);
int egg_joinproc(ChannelLog *, char **, int);
int egg_partproc(ChannelLog *, char **, int);
int egg_msgproc(ChannelLog *, char **, int);
int egg_quitproc(ChannelLog *, char **, int);
int egg_topicproc(ChannelLog *, char **, int);
int egg_kickproc(ChannelLog *, char **, int);
int egg_nickproc(ChannelLog *, char **, int);
int egg_modeproc(ChannelLog *, char **, int);
int mirc_joinproc(ChannelLog *, char **, int);
int mirc_partproc(ChannelLog *, char **, int);
int mirc_msgproc(ChannelLog *, char **, int);
int mirc_quitproc(ChannelLog *, char **, int);
int mirc_topicproc(ChannelLog *, char **, int);
int mirc_kickproc(ChannelLog *, char **, int);
int mirc_nickproc(ChannelLog *, char **, int);
int mirc_modeproc(ChannelLog *, char **, int);
int xchat_joinproc(ChannelLog *, char **, int);
int xchat_partproc(ChannelLog *, char **, int);
int xchat_msgproc(ChannelLog *, char **, int);
int xchat_quitproc(ChannelLog *, char **, int);
int xchat_topicproc(ChannelLog *, char **, int);
int xchat_kickproc(ChannelLog *, char **, int);
int xchat_nickproc(ChannelLog *, char **, int);
int xchat_modeproc(ChannelLog *, char **, int);

void lgs_RotateLogs();
void lgs_close_logs();
void lgs_switch_file(ChannelLog *cl);

#endif

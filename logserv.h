/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
	unsigned long flags;
	char statsurl[MAXPATH];
	time_t fdopened;
} ChannelLog;


/* settings for LogServ */
struct LogServ {
	int logtype;
	int modnum;
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
	LGSMSG_KICK
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
} logtype_proc;


/* log_procssing.c decl */
int dirc_joinproc(ChannelLog *, char **, int);
int dirc_partproc(ChannelLog *, char **, int);
int dirc_msgproc(ChannelLog *, char **, int);
int dirc_quitproc(ChannelLog *, char **, int);
int dirc_topicproc(ChannelLog *, char **, int);
int dirc_kickproc(ChannelLog *, char **, int);
int egg_joinproc(ChannelLog *, char **, int);
int egg_partproc(ChannelLog *, char **, int);
int egg_msgproc(ChannelLog *, char **, int);
int egg_quitproc(ChannelLog *, char **, int);
int egg_topicproc(ChannelLog *, char **, int);
int egg_kickproc(ChannelLog *, char **, int);
int mirc_joinproc(ChannelLog *, char **, int);
int mirc_partproc(ChannelLog *, char **, int);
int mirc_msgproc(ChannelLog *, char **, int);
int mirc_quitproc(ChannelLog *, char **, int);
int mirc_topicproc(ChannelLog *, char **, int);
int mirc_kickproc(ChannelLog *, char **, int);
int xchat_joinproc(ChannelLog *, char **, int);
int xchat_partproc(ChannelLog *, char **, int);
int xchat_msgproc(ChannelLog *, char **, int);
int xchat_quitproc(ChannelLog *, char **, int);
int xchat_topicproc(ChannelLog *, char **, int);
int xchat_kickproc(ChannelLog *, char **, int);



#endif

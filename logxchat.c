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

#include "logserv.h"  /* LogServ Definitions */

/* XCHAT FORMAT LOGGING BELOW */

#define XCHATTIME "%b %d %H:%M:%S"

char *xchat_time() {
	sys_strftime (timebuf, TIMEBUFSIZE, XCHATTIME, sys_localtime (&me.now));
	return timebuf;
}
/* **** BEGIN LOGGING AT Fri Jan  2 17:25:15 2004
 * \n
 */
#define XSTARTLOG "**** BEGIN LOGGING AT %s\n\n"
 
char *xchat_startlog(ChannelLog *cl) {
	char tmbuf[TIMEBUFSIZE];
	sys_strftime (tmbuf, TIMEBUFSIZE, "%a %b %d %H:%M:%S %Y", sys_localtime (&me.now));
	ircsnprintf(startlog, BUFSIZE, XSTARTLOG, tmbuf);
	return startlog;
}


/* Jan 02 17:27:10 -->     Dirk-Digler (fish@Dirk-Digler.Users.irc-chat.net) has joined #neostats */
#define XJOINFMT "%s -->\t%s (%s@%s) has joined %s\n"

int xchat_joinproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	lgs_write_log(chandata, XJOINFMT, xchat_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name); return NS_SUCCESS;
}

/* Jan 02 17:56:52 <--     DigiGuy (~b.dole@Oper.irc-chat.net) has left #neostats (part)*/
#define XPARTPROC "%s <--\t%s (%s@%s) has left %s (%s)\n"

int xchat_partproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	lgs_write_log(chandata, XPARTPROC, xchat_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name, cmdparams->param);
	return NS_SUCCESS;
}

/* Jan 02 17:25:43 <SecureServ>    Akilling jojo!~jojo@pD9E60152.dip.t-dialin.net for Virus IRCORK */

#define XMSGFMT "%s <%s>\t%s\n"

/* Action: 
 * Jan 02 17:28:52 *       Fish-Away sighs */
#define XACTFMT "%s *\t%s %s\n"

int xchat_msgproc(ChannelLog *chandata, CmdParams* cmdparams) {
	if (cmdparams->ac == 3) {
		lgs_write_log(chandata, XACTFMT, xchat_time(), cmdparams->source->name, cmdparams->param);
	} else {
		lgs_write_log(chandata, XMSGFMT, xchat_time(), cmdparams->source->name, cmdparams->param);
	}
	return NS_SUCCESS;
}

int xchat_noticeproc(ChannelLog *chandata, CmdParams* cmdparams) {
	if (cmdparams->ac == 3) {
		lgs_write_log(chandata, XACTFMT, xchat_time(), cmdparams->source->name, cmdparams->param);
	} else {
		lgs_write_log(chandata, XMSGFMT, xchat_time(), cmdparams->source->name, cmdparams->param);
	}
	return NS_SUCCESS;
}
/* Jan 02 17:47:26 <--     Dirk-Digler has quit (Killed (Fish (get lost))) */
#define XQUITFMT "%s <--\t%s has quit (%s)\n"

int xchat_quitproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, XQUITFMT, xchat_time(), cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

/* Jan 02 17:48:12 ---     Digi|Away has changed the topic to: FREE PORN - DETAILS INSIDE */
#define XTOPICPROC "%s ---\t%s has changed the topic to: %s\n"

int xchat_topicproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, XTOPICPROC, xchat_time(), cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

/* Jan 02 17:27:10 <-- Fish-Away has kicked Dirk-Digler from #neostats (ha) */
#define XKICKPROC "%s <--\t%s has kicked %s from %s (%s)\n"

int xchat_kickproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, XKICKPROC, xchat_time(), cmdparams->source->name, cmdparams->target->name, cmdparams->channel->name, cmdparams->param);
	return NS_SUCCESS;
}

/* Jan 02 17:50:32 ---     DigiGuy is now known as Bob */
#define XNICKPROC "%s ---\t%s is now known as %s\n"

int xchat_nickproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, XNICKPROC, xchat_time(), cmdparams->param, cmdparams->source->name);
	return NS_SUCCESS;
}

/* Jan 02 17:27:10 ---     SuperSexSquirrel sets modes [#NeoStats +v Dirk-Digler] */
#define XMODEPROC "%s ---\t%s sets modes[%s %s]\n"

int xchat_modeproc(ChannelLog *chandata, CmdParams* cmdparams) {
	char *modebuf;
	
	modebuf = joinbuf(cmdparams->av, cmdparams->ac, 0);
	lgs_write_log(chandata, XMODEPROC, xchat_time(), cmdparams->source->name, chandata->channame, modebuf);
	ns_free (modebuf);
	return NS_SUCCESS;
}


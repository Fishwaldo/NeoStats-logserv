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

/* [00:00] --- Fri Jan  2 2004 */
/* this usually goes at the end of a logfile. Hrm */

char *egg_startlog(ChannelLog *cl) {
	os_strftime (startlog, BUFSIZE, "[%H:%M] --- %a %b %d %Y\n", os_localtime (&me.now));

	return startlog;
}
#define EGGTIME "[%H:%M]"

char *egg_time() {
	os_strftime (timebuf, TIMEBUFSIZE, EGGTIME, os_localtime (&me.now));
	return timebuf;
}


/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) joined #neostats. */
#define EJOINPROC "%s %s (%s@%s) joined %s.\n"

int egg_joinproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	lgs_write_log(chandata, EJOINPROC, egg_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name);
	return NS_SUCCESS;
}

/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) left #neostats (ha). */
#define EPARTPROC "%s %s (%s@%s) left %s (%s).\n"

int egg_partproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	lgs_write_log(chandata, EPARTPROC, egg_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name, cmdparams->param);
	return NS_SUCCESS;
}

/* [22:04] <Fish> I think that is it */
#define EMSGPROC "%s <%s> %s\n"

int egg_msgproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, EMSGPROC, egg_time(), cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

int egg_noticeproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, EMSGPROC, egg_time(), cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

/* [22:04] Action: Fish waits for the eggdrop to flush its logs now */
#define EACTPROC "%s Action: %s %s\n"

int egg_ctcpaction(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, EACTPROC, egg_time(), cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

/* [22:03] Fishwaldo (~Fish@3cbc6b2b.22d996b6.singnet.com.sg) left irc: Client closed */
#define EQUITPROC "%s %s (%s@%s) left irc: %s\n"

int egg_quitproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	lgs_write_log(chandata, EQUITPROC, egg_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->param);	
	return NS_SUCCESS;
}

/* [22:02] Topic changed on #neostats by Fish!~Fish@Server-Admin.irc-chat.net: <LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net */
#define ETOPICPROC "%s Topic changed on %s by %s!%s@%s: %s\n"
#define ENOUSERTOPICPROC "%s Topic changed on %s by %s: %s\n"

int egg_topicproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	if (cmdparams->source)
		lgs_write_log(chandata, ETOPICPROC, egg_time(), cmdparams->channel->name, cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->param);
	else 
		lgs_write_log(chandata, ENOUSERTOPICPROC, egg_time(), cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
	return NS_SUCCESS;
}

/* [22:02] Fish kicked from #neostats by Fish: Fish */
#define EKICKPROC "%s %s kicked from %s by %s: %s\n"

int egg_kickproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, EKICKPROC, egg_time(), cmdparams->target->name, cmdparams->channel->name, cmdparams->source->name, cmdparams->param); 
	return NS_SUCCESS;
}

/* [22:02] #NeoStats: mode change '+oa Fish Fish' by SuperSexSquirrel!super@supersexsquirrel.org */
#define EMODEPROC "%s %s: mode change '%s' by %s!%s@%s\n"
#define ENOUSERMODEPROC "%s %s: mode change '%s' by %s\n"
int egg_modeproc(ChannelLog *chandata, CmdParams* cmdparams) 
{
	char *modebuf;
	
	modebuf = joinbuf(cmdparams->av, cmdparams->ac, 0);
	if (cmdparams->source) {
		lgs_write_log(chandata, EMODEPROC, egg_time(), cmdparams->channel->name, modebuf, cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost);
	} else {
		lgs_write_log(chandata, ENOUSERMODEPROC, egg_time(), cmdparams->channel->name, modebuf, cmdparams->source->name);
	}	
	return NS_SUCCESS;
}

/* [22:02] Nick change: Fish -> haha */
#define ENICKPROC "%s Nick change: %s -> %s\n"

int egg_nickproc(ChannelLog *chandata, CmdParams* cmdparams) {
	lgs_write_log(chandata, ENICKPROC, egg_time(), cmdparams->param, cmdparams->source->name);
	return NS_SUCCESS;
}


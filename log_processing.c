/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Justin Hammond
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
#include "dl.h"       /* Required for module */
#include "stats.h"    /* Required for bot support */
#include "log.h"      /* Log systems support */
#include "conf.h"     /* GetConf support */
#include "logserv.h"  /* LogServ Definitions */

char timebuf[TIMEBUFSIZE];
char startlog[BUFSIZE];


int dirc_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int dirc_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int dirc_msgproc(ChannelLog *chandata, char **av, int ac) {
printf("doing msgproc\n");
	return NS_SUCCESS;
}

int dirc_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int dirc_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int dirc_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
int dirc_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
int dirc_modeproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
int egg_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_msgproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

int egg_modeproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] * Dirk-Digler has joined #neostats */

int mirc_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* <Digi> [22:07] * DigiGuy has left #neostats */

int mirc_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] <Digi|Away> yes we are feeling nice today */
/* [21:47] * Fish does a action for Digi|Away's log */

int mirc_msgproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:49] * DigiGuy has quit IRC (Quit: ha) */

int mirc_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:48] * Digi|Away changes topic to 'FREE PORN - DETAILS ' */

int mirc_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] * Dirk-Digler was kicked by Fish (Fish) */

int mirc_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:48] * Fish is now known as Fishy */
int mirc_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] * Fish sets mode: +o Dirk-Digler */
int mirc_modeproc(ChannelLog *chandata, char **av, int ac) {

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
 
char *xchat_startlog() {
	char tmbuf[TIMEBUFSIZE];
	strftime(tmbuf, TIMEBUFSIZE, "%a %b %d %H:%M:%S", localtime(&me.now));
	ircsnprintf(startlog, BUFSIZE, XSTARTLOG, tmbuf);
	return startlog;
}


/* Jan 02 17:27:10 -->     Dirk-Digler (fish@Dirk-Digler.Users.irc-chat.net) has joined #neostats */
#define XJOINFMT "%s -->\t%s (%s@%s) has joined %s\n"

int xchat_joinproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u) 
		printf(XJOINFMT, xchat_time(), u->nick, u->username, u->vhost, av[0]);
	return NS_SUCCESS;
}

/* Jan 02 17:56:52 <--     DigiGuy (~b.dole@Oper.irc-chat.net) has left #neostats */
#define XPARTPROC "%s <--\t%s (%s@%s) has left %s\n"

int xchat_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}


/* Jan 02 17:25:43 <SecureServ>    Akilling jojo!~jojo@pD9E60152.dip.t-dialin.net for Virus IRCORK */

#define XMSGFMT "%s <%s>\t %s\n"

/* Action: 
 * Jan 02 17:28:52 *       Fish-Away sighs */
#define XACTFMT "%s *\t %s %s\n"

int xchat_msgproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}
/* Jan 02 17:47:26 <--     Dirk-Digler has quit (Killed (Fish (get lost))) */
#define XQUITFMT "%s <--\t %s has quit (%s)\n"

int xchat_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* Jan 02 17:48:12 ---     Digi|Away has changed the topic to: FREE PORN - DETAILS INSIDE */
#define XTOPICPROC "%s ---\t %s has changed the topic to: %s\n"

int xchat_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* Jan 02 17:27:10 <-- Fish-Away has kicked Dirk-Digler from #neostats (ha) */
#define XKICKPROC "%s <--\t%s has kicked %s from %s (%s)\n"

int xchat_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* Jan 02 17:50:32 ---     DigiGuy is now known as Bob */
#define XNICKPROC "%s ---\t%s is now known as %s\n"

int xchat_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* Jan 02 17:27:10 ---     SuperSexSquirrel sets modes [#NeoStats +v Dirk-Digler] */
#define XMODEPROC "%s ---\t%s sets modes[%s %s]\n"

int xchat_modeproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

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

static void lgs_write_log(ChannelLog *cl, char *fmt, ...) __attribute__((format(printf,2,3)));



static void lgs_write_log(ChannelLog *cl, char *fmt, ...) {
	va_list ap;
	char log_buf[BUFSIZE];
	
        va_start (ap, fmt);
        ircvsnprintf (log_buf, BUFSIZE, fmt, ap);
        va_end (ap);
	                                                
	printf("%s", log_buf);

}

static void lgs_close_logs() {

}




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

/* [00:00] --- Fri Jan  2 2004 */
/* this usually goes at the end of a logfile. Hrm */

char *egg_startlog(ChannelLog *cl) {


	return startlog;
}

char *egg_time() {

	return timebuf;
}

/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) joined #neostats. */
#define EJOINPROC "%s %s (%s@%s) joined %s\n"

int egg_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:02] Fish (~Fish@Server-Admin.irc-chat.net) left #neostats. */
#define EPARTPROC "%s %s (%s@%s) left %s\n"

int egg_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:04] <Fish> I think that is it
 * [22:04] Action: Fish waits for the eggdrop to flush its logs now
 */
#define EMSGPROC "%s <%s> %s\n"
#define EACTPROC "%s Action: %s %s\n"

int egg_msgproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:03] Fishwaldo (~Fish@3cbc6b2b.22d996b6.singnet.com.sg) left irc: Client closed */
#define EQUITPROC "%s %s (%s@%s) left irc: %s\n"

int egg_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:02] Topic changed on #neostats by Fish!~Fish@Server-Admin.irc-chat.net: <LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net */
#define ETOPICPROC "%s Topic changed on %s by %s!%s@%s: %s\n"

int egg_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:02] Fish kicked from #neostats by Fish: Fish */
#define EKICKPROC "%s %s kicked from %s by %s: %s\n"

int egg_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:02] #NeoStats: mode change '+oa Fish Fish' by SuperSexSquirrel!super@supersexsquirrel.org */
#define EMODEPROC "%s %s: mode change '%s' by %s!%s@%s\n"
int egg_modeproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:02] Nick change: Fish -> haha */
#define ENICKPROC "%s Nick change: %s -> %s\n"

int egg_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* Session Start: Fri Jan 02 21:46:22 2004
 * Session Ident: #neostats
 * [21:46] * Now talking in #neostats
 * [21:46] * Topic is '<LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net :: NeoStats-2.5.11 :: OPSB 2.0 Beta1 Released :: StupidServ 1.2 Released :: SecureServ 1.0 :: Happy Holidays from the entire NeoStats Team, including the trained monkey'
 * [21:46] * Set by LuShes on Fri Jan 02 11:11:57
 */

#define MSTARTLOG "Session Start:%s\nSession Ident: %s\n%s * Now talking in %s\n%s * Topic is '%s'\n%s * Set by %s on %s\n"

char *mirc_startlog(ChannelLog *cl) {


	return startlog;
}

#define MIRCTIME "[%H:%M]"
char *mirc_time() {
	strftime(timebuf, TIMEBUFSIZE, MIRCTIME, localtime(&me.now));
	return timebuf;
}


/* [21:47] * Dirk-Digler has joined #neostats */
#define MJOINPROC "%s * %s has joined %s\n"

int mirc_joinproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [22:07] * DigiGuy has left #neostats */
#define MPARTPROC "%s * %s has left %s\n"

int mirc_partproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] <Digi|Away> yes we are feeling nice today */
/* [21:47] * Fish does a action for Digi|Away's log */
#define MMSGPROC "%s <%s> %s\n"
#define MACTPROC "%s * %s %s\n"

int mirc_msgproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:49] * DigiGuy has quit IRC (Quit: ha) */
#define MQUITPROC "%s * %s has quit IRC (%s)\n"

int mirc_quitproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:48] * Digi|Away changes topic to 'FREE PORN - DETAILS ' */
#define MTOPICPROC "%s * %s changes topic to '%s'\n"

int mirc_topicproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] * Dirk-Digler was kicked by Fish (Fish) */
#define MKICKPROC "%s * %s was kicked by %s (%s)\n"

int mirc_kickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:48] * Fish is now known as Fishy */
#define MNICKPROC "%s * %s is now known as %s\n"

int mirc_nickproc(ChannelLog *chandata, char **av, int ac) {

	return NS_SUCCESS;
}

/* [21:47] * Fish sets mode: +o Dirk-Digler */
#define MMODEPROC "%s * %s sets mode: %s\n"

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
 
char *xchat_startlog(ChannelLog *cl) {
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
		lgs_write_log(chandata, XJOINFMT, xchat_time(), u->nick, u->username, u->vhost, av[0]);
	return NS_SUCCESS;
}

/* Jan 02 17:56:52 <--     DigiGuy (~b.dole@Oper.irc-chat.net) has left #neostats */
#define XPARTPROC "%s <--\t%s (%s@%s) has left %s\n"

int xchat_partproc(ChannelLog *chandata, char **av, int ac) {
	User *u;
	u = finduser(av[1]);
	if (u)
		lgs_write_log(chandata, XPARTPROC, xchat_time(), u->nick, u->username, u->vhost, av[0]);
	return NS_SUCCESS;
}


/* Jan 02 17:25:43 <SecureServ>    Akilling jojo!~jojo@pD9E60152.dip.t-dialin.net for Virus IRCORK */

#define XMSGFMT "%s <%s>\t %s\n"

/* Action: 
 * Jan 02 17:28:52 *       Fish-Away sighs */
#define XACTFMT "%s *\t %s %s\n"

int xchat_msgproc(ChannelLog *chandata, char **av, int ac) {
	if (ac == 3) {
		lgs_write_log(chandata, XACTFMT, xchat_time(), av[0], av[2]);
	} else {
		lgs_write_log(chandata, XMSGFMT, xchat_time(), av[0], av[1]);
	}
	return NS_SUCCESS;
}
/* Jan 02 17:47:26 <--     Dirk-Digler has quit (Killed (Fish (get lost))) */
#define XQUITFMT "%s <--\t %s has quit (%s)\n"

int xchat_quitproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XQUITFMT, xchat_time(), av[0], ac == 2 ? av[1] : "");
	return NS_SUCCESS;
}

/* Jan 02 17:48:12 ---     Digi|Away has changed the topic to: FREE PORN - DETAILS INSIDE */
#define XTOPICPROC "%s ---\t %s has changed the topic to: %s\n"

int xchat_topicproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XTOPICPROC, xchat_time(), av[1], ac == 3 ? av[2] : "");
	return NS_SUCCESS;
}

/* Jan 02 17:27:10 <-- Fish-Away has kicked Dirk-Digler from #neostats (ha) */
#define XKICKPROC "%s <--\t%s has kicked %s from %s (%s)\n"

int xchat_kickproc(ChannelLog *chandata, char **av, int ac) {
	lgs_write_log(chandata, XKICKPROC, xchat_time(), av[1], av[2], av[0], ac == 4 ? av[3] : "");
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

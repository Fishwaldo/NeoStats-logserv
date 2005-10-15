/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "logserv.h"  /* LogServ Definitions */

#define MIRCTIME "[%H:%M]"

static char *mirc_time( void )
{
	os_strftime( timebuf, TIMEBUFSIZE, MIRCTIME, os_localtime( &me.now ) );
	return timebuf;
}

/* Session Start: Fri Jan 02 21:46:22 2004
 * Session Ident: #neostats
 * [21:46] * Now talking in #neostats
 * [21:46] * Topic is '<LuShes> I'mmmm back!!!! [NeoStats Support] http://www.neostats.net :: NeoStats-2.5.11 :: OPSB 2.0 Beta1 Released :: StupidServ 1.2 Released :: SecureServ 1.0 :: Happy Holidays from the entire NeoStats Team, including the trained monkey'
 * [21:46] * Set by LuShes on Fri Jan 02 11:11:57
 */

#define MSTARTLOG "Session Start: %s\nSession Ident: %s\n%s * Now talking in %s\n%s * Topic is '%s'\n%s * Set by %s on %s\n"

char *mirc_startlog( const ChannelLog *cl )
{
	char timebuf2[TIMEBUFSIZE];
	char timebuf3[TIMEBUFSIZE];
	
	os_strftime( timebuf, TIMEBUFSIZE, "%a %b %d %H:%M:%S %Y", os_localtime( &me.now ) );
	os_strftime( timebuf2, TIMEBUFSIZE, "[%H:%M]", os_localtime( &me.now ) );
	os_strftime( timebuf3, TIMEBUFSIZE, "%a %b %d %H:%M:%S", os_localtime( &cl->c->topictime ) );
	ircsnprintf( startlog, BUFSIZE, MSTARTLOG, timebuf, cl->channame, timebuf2, cl->channame, timebuf2, cl->c->topic[0] != '0' ? cl->c->topic : "", timebuf2, cl->c->topicowner[0] != '0' ? cl->c->topicowner: "", timebuf3 );

	return startlog;
}

/* [21:47] * Dirk-Digler has joined #neostats */
#define MJOINPROC "%s * %s( %s@%s ) has joined %s\n"

void mirc_joinproc( ChannelLog *chandata, const CmdParams *cmdparams ) 
{
	ls_write_log( chandata, MJOINPROC, mirc_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name );
}

/* [22:07] * DigiGuy has left #neostats */
#define MPARTPROC "%s * %s( %s@%s ) has left %s( %s )\n"

void mirc_partproc( ChannelLog *chandata, const CmdParams *cmdparams ) 
{
	ls_write_log( chandata, MPARTPROC, mirc_time(), cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->vhost, cmdparams->channel->name, cmdparams->param );
}

/* [21:47] <Digi|Away> yes we are feeling nice today */
#define MMSGPROC "%s <%s> %s\n"

void mirc_msgproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MMSGPROC, mirc_time(), cmdparams->source->name, cmdparams->param );
}

void mirc_noticeproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MMSGPROC, mirc_time(), cmdparams->source->name, cmdparams->param );
}

/* [21:47] * Fish does a action for Digi|Away's log */
#define MACTPROC "%s * %s %s\n"

void mirc_ctcpaction( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MACTPROC, mirc_time(), cmdparams->source->name, cmdparams->param );
}

/* [21:49] * DigiGuy has quit IRC( Quit: ha ) */
#define MQUITPROC "%s * %s has quit IRC( %s )\n"

void mirc_quitproc( ChannelLog *chandata, const CmdParams *cmdparams ) 
{
	ls_write_log( chandata, MQUITPROC, mirc_time(), cmdparams->source->name, cmdparams->param );
}

/* [21:48] * Digi|Away changes topic to 'FREE PORN - DETAILS ' */
#define MTOPICPROC "%s * %s changes topic to '%s'\n"

void mirc_topicproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MTOPICPROC, mirc_time(), cmdparams->source->name, cmdparams->param );
}

/* [21:47] * Dirk-Digler was kicked by Fish( Fish ) */
#define MKICKPROC "%s * %s was kicked by %s( %s )\n"

void mirc_kickproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MKICKPROC, mirc_time(), cmdparams->target->name, cmdparams->source->name, cmdparams->param );
}

/* [21:48] * Fish is now known as Fishy */
#define MNICKPROC "%s * %s is now known as %s\n"

void mirc_nickproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	ls_write_log( chandata, MNICKPROC, mirc_time(), cmdparams->param, cmdparams->source->name );
}

/* [21:47] * Fish sets mode: +o Dirk-Digler */
#define MMODEPROC "%s * %s sets mode: %s\n"

void mirc_modeproc( ChannelLog *chandata, const CmdParams *cmdparams )
{
	char *modebuf;
	
	modebuf = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	ls_write_log( chandata, MMODEPROC, mirc_time(), cmdparams->source->name, modebuf );
	ns_free( modebuf );
}


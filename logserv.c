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
#include "logserv.h"

#include "lgs_help.c"

;



/* forward decl */
static int lgs_about(User * u, char **av, int ac);
static int lgs_version(User * u, char **av, int ac);
static int lgs_chans(User * u, char **av, int ac);
static int lgs_stats(User * u, char **av, int ac);
static int lgs_send_to_logproc( logmsgtype msgtype, ChannelLog *lgschan, char **av, int ac);


logtype_proc logging_funcs[] = {
	{dirc_joinproc, dirc_partproc, dirc_msgproc, dirc_quitproc, dirc_topicproc, dirc_kickproc},
	{egg_joinproc, egg_partproc, egg_msgproc, egg_quitproc, egg_topicproc, egg_kickproc},
	{mirc_joinproc, mirc_partproc, mirc_msgproc, mirc_quitproc, mirc_topicproc, mirc_kickproc},
	{xchat_joinproc, xchat_partproc, xchat_msgproc, xchat_quitproc, xchat_topicproc, xchat_kickproc},
};



/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo __module_info = {
	"LogServ",
	"Channel Logging Bot",
	"$Rev$\0"
	__DATE__,
	__TIME__
};

bot_cmd lgs_commands[]=
{
	{"ABOUT",			lgs_about,		0, 	0,			lgs_help_about, 		lgs_help_about_oneline},
	{"VERSION",			lgs_version,		0, 	0,			lgs_help_version, 	 	lgs_help_version_oneline},
	{"CHANS",			lgs_chans,		1, 	NS_ULEVEL_OPER,		lgs_help_chan, 		 	lgs_help_chan_oneline},
	{"STATS",			lgs_stats,		0, 	0,			lgs_help_stats, 		lgs_help_stats_oneline},
	{NULL,				NULL,			0, 	0,			NULL, 				NULL}
};

bot_setting lgs_settings[]=
{
	{"LOGTYPE",		
	&LogServ.logtype,	
	SET_TYPE_INT,	
	0,	
	3,	
	NS_ULEVEL_ADMIN,
	"LogType",
	"Log Type",
	lgs_help_set_logtype },
	{NULL,			NULL,			0,		0, 	0,	0,			NULL,		NULL,		NULL },
};

/** Module function list
 * A list of IRCd (server) commands that we will respond to
 * e.g. VERSION
 * This table is required for your module to load and run on NeoStats
 * but you do not have to have any functions in it
 */
Functions __module_functions[] = {
	{NULL, NULL, 0}
};

/** Channel message processing
 * What do we do with messages in channes
 * This is required if you want your module to respond to channel messages
 */
int __ChanMessage(char *origin, char **argv, int argc)
{
	char *chan = argv[0];
	return 1;
}

/** Online event processing
 * What we do when we first come online
 */
static int Online(char **av, int ac)
{
	/* Introduce a bot onto the network */
	lgs_bot = init_mod_bot(s_LogServ, "LogBot", me.name, "Channel Logging Bot", services_bot_modes,
		BOT_FLAG_ONLY_OPERS, lgs_commands, lgs_settings, __module_info.module_name);		
	return 1;
};

/** Module event list
 * What events we will act on
 * This is required if you want your module to respond to events on IRC
 * see modules.txt for a list of all events available
 */
EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{NULL, NULL}
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int __ModInit(int modnum, int apiver)
{
	strlcpy(s_LogServ, "LogServ", MAXNICK);
	lgschans = hash_create(-1, 0,0);
	
	/* get the logtype */
	if (GetConf((void *)&LogServ.logtype, CFGINT, "LogType") < 0) {
		LogServ.logtype = 0;
	} 
	
	return 1;
}

/** Init module
 * This is required if you need to do cleanup of your module when it ends
 */
void __ModFini()
{

};



/* ok, now here are logservs functions */

/* @brief Send Description of Module to user
 *
 * @param u The user requesting help
 * @param av the text sent
 * @param ac the number of words in av
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_about(User * u, char **av, int ac) {
	privmsg_list(u->nick, s_LogServ, lgs_help_about);
	return 1; 
}

/* @brief Send version of Module to user
 *
 * @param u The user requesting version
 * @param av the text sent
 * @param ac the number of words in av
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_version(User * u, char **av, int ac) {
	SET_SEGV_LOCATION();
	prefmsg(u->nick, s_LogServ, "\2%s Version Information\2", s_LogServ);
	prefmsg(u->nick, s_LogServ, "%s Version: %s Compiled %s at %s", s_LogServ, __module_info.module_version, __module_info.module_build_date, __module_info.module_build_time);
	return 1;
}

/* @brief manipulate the logged channels
 *
 * @param u The user requesting channel data
 * @param av the text sent
 * @param ac the number of words in av
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_chans(User * u, char **av, int ac) {
	if (!strcasecmp(av[2], "LIST")) {
		prefmsg(u->nick, s_LogServ, "Channel List:");
		/* XXX do channel list */
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "DEL")) {
		if (ac < 4) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		prefmsg(u->nick, s_LogServ, "Del Chan");
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "ADD")) {
		if (ac < 6) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		prefmsg(u->nick, s_LogServ, "Add Chan");
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "SET")) {
		if (ac < 6) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		prefmsg(u->nick, s_LogServ, "Set Chan");
		return NS_SUCCESS;
	} else {
		prefmsg(u->nick, s_LogServ, "Syntax error: Unknown Command %s", av[2]);
		prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
		return NS_FAILURE;
	}
	return NS_FAILURE;	
}

/* @brief Send some very simple stats to the user
 *
 * @param u The user requesting stats data
 * @param av the text sent
 * @param ac the number of words in av
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_stats(User * u, char **av, int ac) {
	prefmsg(u->nick, s_LogServ, "LogServ Stats:");
	prefmsg(u->nick, s_LogServ, "Monitoring %d channels", (int)hash_count(lgschans));
}


/* @breif Send the Log Message to the relevent Log Processor 
 *
 * @param msgtype the type of message (join, part etc)
 * @param av contents of the message 
 * @param ac message size 
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_send_to_logproc( logmsgtype msgtype, ChannelLog *lgschan, char **av, int ac) {
	switch (msgtype) {
		case 0:
			return logging_funcs[LogServ.logtype].joinproc(lgschan, av, ac);
			break;
		case 1:
			return logging_funcs[LogServ.logtype].partproc(lgschan, av, ac);
			break;
		case 2:
			return logging_funcs[LogServ.logtype].msgproc(lgschan, av, ac);
			break;
		case 3:
			return logging_funcs[LogServ.logtype].quitproc(lgschan, av, ac);
			break;
		case 4: 
			return logging_funcs[LogServ.logtype].topicproc(lgschan, av, ac);
			break;
		case 5:
			return logging_funcs[LogServ.logtype].kickproc(lgschan, av, ac);
			break;
		default:
			nlog(LOG_WARNING, LOG_MOD, "Unknown msgtype %d", msgtype);
			return NS_FAILURE;
			break;
	}	
	return NS_FAILURE;
}

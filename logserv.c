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
#include "dl.h"       /* Required for module */
#include "stats.h"    /* Required for bot support */
#include "log.h"      /* Log systems support */
#include "conf.h"     /* GetConf support */
#include "logserv.h"

#include "lgs_help.c"

/* forward decl */
static int lgs_about(User * u, char **av, int ac);
static int lgs_version(User * u, char **av, int ac);
static int lgs_chans(User * u, char **av, int ac);
static int lgs_stats(User * u, char **av, int ac);
static int lgs_send_to_logproc( logmsgtype msgtype, ChannelLog *lgschan, char **av, int ac);
static void lgs_save_channels_data(ChannelLog *cl);
ChannelLog *lgs_newchanlog(User *u, char **av, int ac);
ChannelLog *lgs_findactchanlog(Chans *c);

logtype_proc logging_funcs[] = {
	{logserv_joinproc, logserv_partproc, logserv_msgproc, logserv_quitproc, logserv_topicproc, logserv_kickproc, logserv_nickproc, logserv_modeproc},
	{egg_joinproc, egg_partproc, egg_msgproc, egg_quitproc, egg_topicproc, egg_kickproc, egg_nickproc, egg_modeproc},
	{mirc_joinproc, mirc_partproc, mirc_msgproc, mirc_quitproc, mirc_topicproc, mirc_kickproc, mirc_nickproc, mirc_modeproc},
	{xchat_joinproc, xchat_partproc, xchat_msgproc, xchat_quitproc, xchat_topicproc, xchat_kickproc, xchat_nickproc, xchat_modeproc},
};



/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo __module_info = {
	"LogServ",
	"Channel Logging Bot",
	"$Rev$",
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
	1,	
	3,	
	NS_ULEVEL_ADMIN,
	"LogType",
	"Log Type",
	lgs_help_set_logtype },
	{"LOGSIZE",		
	&LogServ.maxlogsize,	
	SET_TYPE_INT,	
	0,	
	10000000,	
	NS_ULEVEL_ADMIN,
	"LogSwitchSize",
	"Bytes",
	lgs_help_set_logsize },
	{"LOGAGE",		
	&LogServ.maxopentime,	
	SET_TYPE_INT,	
	0,	
	86400,	
	NS_ULEVEL_ADMIN,
	"LogSwitchTime",
	"Seconds",
	lgs_help_set_logtime },
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
	Chans *c;
	ChannelLog *cl;
	char **data;
	int datasize = 0;
	char *buf;

	SET_SEGV_INMODULE(__module_info.module_name);
		
	c = findchan(chan);
	if (c && c->moddata[LogServ.modnum]) {
		cl = c->moddata[LogServ.modnum];
		AddStringToList(&data, origin, &datasize);
		if (argv[1][0] == '\1') {
			AddStringToList(&data, argv[1], &datasize);
			buf = joinbuf(argv, argc, 2);
		} else {
			buf = joinbuf(argv, argc, 1);
		}
		strip_mirc_codes(buf);
		AddStringToList(&data, buf, &datasize);
		lgs_send_to_logproc(LGSMSG_MSG, cl, data, datasize);		
		free(data);
	}
	return 1;
}
static int lgs_PartChan(char **av, int ac) 
{
	ChannelLog *cl;
	Chans *c;
	
	/* this function is called recursively, so ignore parts by LogServ */
	if (!strcasecmp(av[1], s_LogServ)) {
		return NS_FAILURE;
	}

	c = findchan(av[0]);
	if ((cl = lgs_findactchanlog(c)) != NULL) {
		/* process the part message now */
		lgs_send_to_logproc(LGSMSG_PART, cl, av, ac);	
		if (c->cur_users == 2) {
			/* last user just parted, so we leave as well */
			nlog(LOG_DEBUG1, LOG_MOD, "Parting Channel %s as there are no more members", c->name);
			/*close/switch the logfile*/
			lgs_switch_file(cl);

			spart_cmd(s_LogServ, c->name);
			c->moddata[LogServ.modnum] = NULL;
			cl->c = NULL;
			cl->flags &= ~LGSACTIVE;
		}
		return NS_SUCCESS;	
	}
	return NS_SUCCESS;
}
static int lgs_JoinChan(char **av, int ac) {
	ChannelLog *cl;
	
	if (!strcasecmp(av[1], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(findchan(av[0]))) != NULL) {
		lgs_send_to_logproc(LGSMSG_JOIN, cl, av, ac);
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_NewChan(char **av, int ac) 
{
	ChannelLog *cl;
	hnode_t *cn;
	Chans *c;

	c = findchan(av[0]);
	cn = hash_lookup(lgschans, av[0]);
	if (!cn) 
		return NS_FAILURE;
		
	cl = hnode_get(cn);
	if (c && cl) {
		if (join_bot_to_chan(s_LogServ, cl->channame, 0) == NS_SUCCESS) {
			SET_SEGV_INMODULE(__module_info.module_name);
			cl->flags |= LGSACTIVE;
			nlog(LOG_NOTICE, LOG_MOD, "Actived Logging on channel %s", cl->channame);
			if (cl->statsurl[0] != '\0') {
				prefmsg(cl->channame, s_LogServ, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
			}	
		}
		c->moddata[LogServ.modnum] = cl;
		cl->c = c;
	}
	return NS_SUCCESS;
}	


/** Online event processing
 * What we do when we first come online
 */
static int lgs_Online(char **av, int ac)
{
	ChannelLog *cl;
	hnode_t *cn;
	char **row;
	int count;
	Chans *c;
	char *tmp;
	/* Introduce a bot onto the network */
	lgs_bot = init_mod_bot(s_LogServ, LogServ.user, LogServ.host, LogServ.rname, services_bot_modes,
		BOT_FLAG_ONLY_OPERS, lgs_commands, lgs_settings, __module_info.module_name);		

	/* load Channels and join them */
	if (GetTableData("Chans", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			nlog(LOG_DEBUG1, LOG_MOD, "Loading Channel %s", row[count]);
			cl = malloc(sizeof(ChannelLog));
			bzero(cl, sizeof(ChannelLog));
			strlcpy(cl->channame, row[count], CHANLEN);
			if (GetData((void *)&cl->flags, CFGINT, "Chans", cl->channame, "Flags") > 0) {
				cl->flags &= ~LGSACTIVE;
				cl->flags &= ~LGSFDNEEDFLUSH;
				cl->flags &= ~LGSFDOPENED;
			}
			if (GetData((void *)&tmp, CFGSTR, "Chans", cl->channame, "URL") < 0) {
				cl->statsurl[0] = '\0';
			} else {
				strlcpy(cl->statsurl, tmp, MAXPATH);
			}
			c = findchan(cl->channame);
			if (c) {
				if (join_bot_to_chan(s_LogServ, cl->channame, 0) == NS_SUCCESS) {
					SET_SEGV_INMODULE(__module_info.module_name);
					cl->flags |= LGSACTIVE;
					nlog(LOG_NOTICE, LOG_MOD, "Actived Logging on channel %s", cl->channame);
					if (cl->statsurl[0] != '\0') {
						prefmsg(cl->channame, s_LogServ, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
					}	
				}
				c->moddata[LogServ.modnum] = cl;
				cl->c = c;
			}
			cn = hnode_create(cl);
			hash_insert(lgschans, cn, cl->channame);
		}	
	}
	
	/* start a timer to scan the logs for rotation */
	add_mod_timer("lgs_RotateLogs", "Rotate_LogServ_Logs", __module_info.module_name, 300);
			
	return 1;
};

static int lgs_Signoff(char **av, int ac) {
/* This function is kinda useless, because,
 * partchan will be called for each channel the user is a member off
 * before this function is called. - Damn
 */
#if 0
	ChannelLog *cl;

	if (!strcasecmp(av[0], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(findchan(av[0]))) != NULL) {
		lgs_send_to_logproc(LGSMSG_QUIT, cl, av, ac);
		return NS_SUCCESS;
	}	
#endif
	return NS_SUCCESS;
}	

static int lgs_KickChan(char **av, int ac) {
	ChannelLog *cl;

	if (!strcasecmp(av[1], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(findchan(av[0]))) != NULL) {
		lgs_send_to_logproc(LGSMSG_KICK, cl, av, ac);
		if (cl->c->cur_users == 2) {
			/* last user just parted, so we leave as well */
			nlog(LOG_DEBUG1, LOG_MOD, "Parting Channel %s as there are no more members", cl->channame);
			lgs_switch_file(cl);

			spart_cmd(s_LogServ, cl->channame);
			cl->c->moddata[LogServ.modnum] = NULL;
			cl->c = NULL;
			cl->flags &= ~LGSACTIVE;
		}
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_TopicChan(char **av, int ac) {
	ChannelLog *cl;

	if (!strcasecmp(av[1], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(findchan(av[0]))) != NULL) {
		lgs_send_to_logproc(LGSMSG_TOPIC, cl, av, ac);
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_NickChange(char **av, int ac) {
	ChannelLog *cl;
	User *u;
	lnode_t *cm;

	if (!strcasecmp(av[1], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	u = finduser(av[1]);
	if (!u) {
		return NS_FAILURE;
	}
	
	/* ok, move through each of the channels */
	cm = list_first (u->chans);
	while (cm) {
		if ((cl = lgs_findactchanlog(findchan (lnode_get (cm)))) != NULL) {
			lgs_send_to_logproc(LGSMSG_NICK, cl, av, ac);
                }
                cm = list_next (u->chans, cm);
	}
	return NS_SUCCESS;
}

/* damn, 2.5.11 doesn't have a chan mode event. Duh! */
#ifdef EVENT_CHANMODE
static int lgs_ChanMode(char **av, int ac) {
	ChannelLog *cl;

	if (!strcasecmp(av[0], s_LogServ)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(findchan(av[1]))) != NULL) {
		lgs_send_to_logproc(LGSMSG_CHANMODE, cl, av, ac);
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}
#endif


/** Module event list
 * What events we will act on
 * This is required if you want your module to respond to events on IRC
 * see modules.txt for a list of all events available
 */
EventFnList __module_events[] = {
	{EVENT_ONLINE, lgs_Online},
	{EVENT_NEWCHAN, lgs_NewChan},
	{EVENT_JOINCHAN, lgs_JoinChan},
	{EVENT_PARTCHAN, lgs_PartChan},
	{EVENT_SIGNOFF, lgs_Signoff},
	{EVENT_KICK, lgs_KickChan},
	{EVENT_TOPICCHANGE, lgs_TopicChan},
	{EVENT_NICKCHANGE, lgs_NickChange},
#ifdef EVENT_CHANMODE
	{EVENT_CHANMODE, lgs_ChanMode},
#endif
	{NULL, NULL}
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int __ModInit(int modnum, int apiver)
{
	char *tmp;
	strlcpy(s_LogServ, "LogServ", MAXNICK);
	lgschans = hash_create(-1, 0,0);
	
	if (GetConf((void *) &tmp, CFGSTR, "Nick") < 0) {
		strlcpy(s_LogServ, "LogServ", MAXNICK);
	} else {
		strlcpy(s_LogServ, tmp, MAXNICK);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "User") < 0) {
		strlcpy(LogServ.user, "LogBot", MAXUSER);
	} else {
		strlcpy(LogServ.user, tmp, MAXUSER);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Host") < 0) {
		strlcpy(LogServ.host, me.name, MAXHOST);
	} else {
		strlcpy(LogServ.host, tmp, MAXHOST);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Rname") < 0) {
		ircsnprintf(LogServ.rname, MAXREALNAME, "Channel Logging Bot");
	} else {
		strlcpy(LogServ.rname, tmp, MAXREALNAME);
		free(tmp);
	}
	/* get the logtype */
	if (GetConf((void *)&LogServ.logtype, CFGINT, "LogType") < 0) {
		LogServ.logtype = 1;
	} 
	/* get the logsize switch */
	if (GetConf((void *)&LogServ.maxlogsize, CFGINT, "LogSwitchSize") < 0) {
		/* 1Mb, or there abouts is default */
		LogServ.maxlogsize = 1000000;
	}
	/* get the logage time */
	if (GetConf((void *)&LogServ.maxopentime, CFGINT, "LogSwitchTime") < 0) {
		/* switch every 1 hour */
		LogServ.maxopentime = 3600;
	}
	/*XXX TODO */
	ircsnprintf(LogServ.logdir, MAXPATH, "logs/chanlogs");
	ircsnprintf(LogServ.savedir, MAXPATH, "ChanLogs");
	LogServ.modnum = modnum;
	return 1;
}

/** Init module
 * This is required if you need to do cleanup of your module when it ends
 */
void __ModFini()
{
	/* close the log files */
	lgs_close_logs();
	
	/* delete the hash */
	hash_destroy(lgschans);


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
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;

	if (!strcasecmp(av[2], "LIST")) {
		prefmsg(u->nick, s_LogServ, "Monitored Channel List:");
		hash_scan_begin(&hs, lgschans);
		while ((hn = hash_scan_next(&hs)) != NULL) {
			cl = hnode_get(hn);
			if ((cl->flags & LGSPUBSTATS) || (UserLevel(u) >= NS_ULEVEL_LOCOPER)) {
				/* its a priv channel, only show to opers */
				prefmsg(u->nick, s_LogServ, "%s (%c) URL: %s", cl->channame, (cl->flags & LGSACTIVE) ? '*' : '-', (cl->statsurl[0] != 0) ? cl->statsurl : "None");
			}							
		}
		prefmsg(u->nick, s_LogServ, "End Of List.");				
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "DEL")) {
		if (ac < 4) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		hn = hash_lookup(lgschans, av[3]);
		if (hn) {
			cl = hnode_get(hn);
		} else {
			prefmsg(u->nick, s_LogServ, "Can not find channel %s in Logging System", av[3]);
			return NS_FAILURE;
		}
		if (!cl) {
			prefmsg(u->nick, s_LogServ, "Can not find Channel %s in Logging System", av[3]);
			return NS_FAILURE;
		}
		/* rotate out the file */
		
		if (cl->flags & LGSACTIVE) {
			lgs_switch_file(cl);
		}
		if (cl->c) {
			cl->c->moddata[LogServ.modnum] = NULL;
		}
		hash_delete(lgschans, hn);
		hnode_destroy(hn);
		spart_cmd(s_LogServ, cl->channame);
		SET_SEGV_INMODULE(__module_info.module_name);
		free(cl);
		DelRow("Chans", av[3]);
		prefmsg(u->nick, s_LogServ, "Deleted Channel %s", av[3]);
		chanalert(s_LogServ, "%s deleted %s from Channel Logging", u->nick, av[3]);
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "ADD")) {
		if (ac < 6) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		lgs_newchanlog(u, av, ac);
		return NS_SUCCESS;
	} else if (!strcasecmp(av[2], "SET")) {
		if (ac < 6) {
			prefmsg(u->nick, s_LogServ, "Syntax error: insufficient parameters");
			prefmsg(u->nick, s_LogServ, "/msg %s HELP CHANS for more information", s_LogServ);
			return NS_FAILURE;
		}
		hn = hash_lookup(lgschans, av[4]);
		if (hn) {
			cl = hnode_get(hn);
		} else {
			prefmsg(u->nick, s_LogServ, "Can not find channel %s in Logging System", av[3]);
			return NS_FAILURE;
		}
		if (!cl) {
			prefmsg(u->nick, s_LogServ, "Can not find Channel %s in Logging System", av[3]);
			return NS_FAILURE;
		}
		if (!strcasecmp(av[3], "URL")) {
			ircsnprintf(cl->statsurl, MAXPATH, "%s", av[5]);
			prefmsg(u->nick, s_LogServ, "Changed URL for %s to: %s", cl->channame, cl->statsurl);
			chanalert(s_LogServ, "%s changed the URL for %s to: %s", u->nick, cl->channame, cl->statsurl);
			lgs_save_channels_data(cl);
		} else {
			prefmsg(u->nick, s_LogServ, "Unknown Set Option");
		}
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
	return NS_SUCCESS;
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
		case 6:
			return logging_funcs[LogServ.logtype].nickproc(lgschan, av, ac);
			break;
		case 7:
			return logging_funcs[LogServ.logtype].modeproc(lgschan, av, ac);
			break;
		default:
			nlog(LOG_WARNING, LOG_MOD, "Unknown msgtype %d", msgtype);
			return NS_FAILURE;
			break;
	}	
	return NS_FAILURE;
}


ChannelLog *lgs_newchanlog(User *u, char **av, int ac) {
	Chans *c;
	ChannelLog *cl;
	hnode_t *cn;
	
	c = findchan(av[3]);
	if (!c) {
		prefmsg(u->nick, s_LogServ, "Error, Channel %s is not Online at the moment", av[3]);
		return NULL;
	}
	if ((cn = hash_lookup(lgschans, av[3])) != NULL) {
		prefmsg(u->nick, s_LogServ, "Already Logging %s.", av[3]);
		return hnode_get(cn);
	}
	cl = malloc(sizeof(ChannelLog));
	bzero(cl, sizeof(ChannelLog));
	strlcpy(cl->channame, av[3], CHANLEN);
	cl->c = c;
	c->moddata[LogServ.modnum] = cl;
	if (!strcasecmp(av[4], "Public")) {
		cl->flags |= LGSPUBSTATS;
	} else if (!strcasecmp(av[4], "Private")) {
		cl->flags &= ~LGSPUBSTATS;
	} else {
		prefmsg(u->nick, s_LogServ, "Unknown Public Type %s. Setting to Public", av[4]);
		cl->flags |= LGSPUBSTATS;
	}
	if (ac == 6) {
		/* we have a URL */
		strlcpy(cl->statsurl, av[5], MAXPATH);
		prefmsg(u->nick, s_LogServ, "Stats URL is set to %s", cl->statsurl);
	} else {
		prefmsg(u->nick, s_LogServ, "No Stats URL is Set");
	}


	cn = hnode_create(cl);
	hash_insert(lgschans, cn, cl->channame);
	lgs_save_channels_data(cl);

	if (join_bot_to_chan(s_LogServ, cl->channame, 0) == NS_SUCCESS) {
		cl->flags |= LGSACTIVE;
		nlog(LOG_NOTICE, LOG_MOD, "%s actived Logging on channel %s", u->nick, cl->channame);
		prefmsg(cl->channame, s_LogServ, "%s Actived Channel Logging on %s", u->nick, cl->channame);
		if (cl->statsurl[0] != '\0') {
			prefmsg(cl->channame, s_LogServ, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
		}	
	}
	prefmsg(u->nick, s_LogServ, "Now Logging %s", cl->channame);
	chanalert(s_LogServ, "%s Activated Logging on %s", u->nick, cl->channame);
	return cl;
}

static void lgs_save_channels_data(ChannelLog *cl) {
	
	nlog(LOG_DEBUG1, LOG_MOD, "Saving Channel Data for %s", cl->channame);
	SetData((void *)cl->flags, CFGINT, "Chans", cl->channame, "Flags");
	SetData((void *)cl->statsurl, CFGSTR, "Chans", cl->channame, "URL");
}	

/* @brief find a active Channel Log record 
 * 
 * @params c - Chans Struct of the channel we are looking for 
 * 
 * @returns A ChannelLog Struct on success, or NULL on failure
 */

ChannelLog *lgs_findactchanlog(Chans *c) {
	ChannelLog *cl;
	if (c && c->moddata[LogServ.modnum]) {
		cl = c->moddata[LogServ.modnum];
#ifdef DEBUG
		/* paranoid checking this is */
		if (!(cl->c == c)) {
			nlog(LOG_WARNING, LOG_MOD, "Channel Log Channel doesn't match channel.");
			return NULL;
		}
#endif
		return cl;
	} else {
		return NULL;
	}
}

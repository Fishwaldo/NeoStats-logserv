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

#include <stdio.h>
#ifdef WIN32
#include "win32modconfig.h"
#else
#include "modconfig.h"
#endif
#include "logserv.h"

Bot *lgs_bot;
Module *lgs_module;

/* forward decl */
static int lgs_chans (CmdParams* cmdparams);
static int lgs_stats (CmdParams* cmdparams);
static int lgs_send_to_logproc (logmsgtype msgtype, ChannelLog *lgschan, CmdParams* cmdparams);
static void lgs_save_channels_data(ChannelLog *cl);
ChannelLog *lgs_newchanlog(CmdParams* cmdparams);
ChannelLog *lgs_findactchanlog(Channel *c);

logtype_proc logging_funcs[] = {
	{logserv_joinproc, logserv_partproc, logserv_msgproc, logserv_quitproc, logserv_topicproc, logserv_kickproc, logserv_nickproc, logserv_modeproc},
	{egg_joinproc, egg_partproc, egg_msgproc, egg_quitproc, egg_topicproc, egg_kickproc, egg_nickproc, egg_modeproc},
	{mirc_joinproc, mirc_partproc, mirc_msgproc, mirc_quitproc, mirc_topicproc, mirc_kickproc, mirc_nickproc, mirc_modeproc},
	{xchat_joinproc, xchat_partproc, xchat_msgproc, xchat_quitproc, xchat_topicproc, xchat_kickproc, xchat_nickproc, xchat_modeproc},
};


/** Copyright info */
const char *ls_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo module_info = {
	"LogServ",
	"Channel Logging Bot",
	ls_copyright,
	ls_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

bot_cmd lgs_commands[]=
{
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

/** Channel message processing
 * What do we do with messages in channes
 * This is required if you want your module to respond to channel messages
 */
#if 0 //temp
int __ChanMessage(char *origin, char **argv, int argc)
{
	char *chan = argv[0]; 
	Channel *c;
	ChannelLog *cl;
	char **data;
	int datasize = 0;
	char *buf;

	if (argc <= 1) {
		return NS_FAILURE;
	}
		
	c = find_chan (chan);
	if (c && c->moddata[lgs_module->modnum]) {
		cl = c->moddata[lgs_module->modnum];
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
#endif

static int lgs_event_part (CmdParams* cmdparams) 
{
	ChannelLog *cl;
	
	/* this function is called recursively, so ignore parts by LogServ */
	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		return NS_FAILURE;
	}

	if ((cl = lgs_findactchanlog(cmdparams->channel)) != NULL) {
		/* process the part message now */
		lgs_send_to_logproc(LGSMSG_PART, cl, cmdparams);	
		if (cmdparams->channel->users == 2) {
			/* last user just parted, so we leave as well */
			dlog (DEBUG1, "Parting Channel %s as there are no more members", cmdparams->channel->name);
			/*close/switch the logfile*/
			lgs_switch_file(cl);

			irc_part (lgs_bot, cmdparams->channel->name);
			cmdparams->channel->moddata[lgs_module->modnum] = NULL;
			cl->c = NULL;
			cl->flags &= ~LGSACTIVE;
		}
		return NS_SUCCESS;	
	}
	return NS_SUCCESS;
}
static int lgs_event_join (CmdParams* cmdparams) 
{
	ChannelLog *cl;
	
	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(cmdparams->channel)) != NULL) {
		lgs_send_to_logproc(LGSMSG_JOIN, cl, cmdparams);
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_event_newchan (CmdParams* cmdparams) 
{
	ChannelLog *cl;
	hnode_t *cn;

	cn = hash_lookup(lgschans, cmdparams->channel);
	if (!cn) 
		return NS_FAILURE;
		
	cl = hnode_get(cn);
	if (cmdparams->channel && cl) {
		if (irc_join (lgs_bot, cl->channame, 0) == NS_SUCCESS) {
			cl->flags |= LGSACTIVE;
			nlog (LOG_NOTICE, "Actived Logging on channel %s", cl->channame);
			if (cl->statsurl[0] != '\0') {
				irc_chanprivmsg (lgs_bot, cl->channame, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
			}	
		}
		cmdparams->channel->moddata[lgs_module->modnum] = cl;
		cl->c = cmdparams->channel;
	}
	return NS_SUCCESS;
}	


/** BotInfo */
static BotInfo ls_botinfo = 
{
	"LogServ", 
	"LogServ1", 
	"LS", 
	BOT_COMMON_HOST, 
	"Channel Logging Bot", 	
	BOT_FLAG_SERVICEBOT|BOT_FLAG_ONLY_OPERS, 
	lgs_commands, 
	lgs_settings,
};

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	ChannelLog *cl;
	hnode_t *cn;
	char **row;
	int count;
	Channel *c;
	char *tmp;
	/* Introduce a bot onto the network */
	lgs_bot = init_bot(&ls_botinfo);		

	/* load Channels and join them */
	if (GetTableData("Channel", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			dlog (DEBUG1, "Loading Channel %s", row[count]);
			cl = malloc(sizeof(ChannelLog));
			bzero(cl, sizeof(ChannelLog));
			strlcpy(cl->channame, row[count], MAXCHANLEN);
			if (GetData((void *)&cl->flags, CFGINT, "Channel", cl->channame, "Flags") > 0) {
				cl->flags &= ~LGSACTIVE;
				cl->flags &= ~LGSFDNEEDFLUSH;
				cl->flags &= ~LGSFDOPENED;
			}
			if (GetData((void *)&tmp, CFGSTR, "Channel", cl->channame, "URL") < 0) {
				cl->statsurl[0] = '\0';
			} else {
				strlcpy(cl->statsurl, tmp, MAXPATH);
			}
			c = find_chan (cl->channame);
			if (c) {
				if (irc_join (lgs_bot, cl->channame, 0) == NS_SUCCESS) {
					cl->flags |= LGSACTIVE;
					nlog (LOG_NOTICE, "Actived Logging on channel %s", cl->channame);
					if (cl->statsurl[0] != '\0') {
						irc_chanprivmsg (lgs_bot, cl->channame, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
					}	
				}
				c->moddata[lgs_module->modnum] = cl;
				cl->c = c;
			}
			cn = hnode_create(cl);
			hash_insert(lgschans, cn, cl->channame);
		}	
	}
	
	/* start a timer to scan the logs for rotation */
	add_timer (TIMER_TYPE_INTERVAL, lgs_RotateLogs, "lgs_RotateLogs", 300);
		
	return NS_SUCCESS;
};

static int lgs_event_kick(CmdParams* cmdparams) 
{
	ChannelLog *cl;

	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(cmdparams->channel)) != NULL) {
		lgs_send_to_logproc(LGSMSG_KICK, cl, cmdparams);
		if (cl->c->users == 2) {
			/* last user just parted, so we leave as well */
			dlog (DEBUG1, "Parting Channel %s as there are no more members", cl->channame);
			lgs_switch_file(cl);

			irc_part (lgs_bot, cl->channame);
			cl->c->moddata[lgs_module->modnum] = NULL;
			cl->c = NULL;
			cl->flags &= ~LGSACTIVE;
		}
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_event_topic(CmdParams* cmdparams) 
{
	ChannelLog *cl;

	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(cmdparams->channel)) != NULL) {
		lgs_send_to_logproc(LGSMSG_TOPIC, cl, cmdparams);
		return NS_SUCCESS;
	}	
	return NS_SUCCESS;
}

static int lgs_event_nick(CmdParams* cmdparams) 
{
	ChannelLog *cl;
	lnode_t *cm;

	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}

	/* ok, move through each of the channels */
	cm = list_first (cmdparams->source->user->chans);
	while (cm) {
		if ((cl = lgs_findactchanlog(find_chan (lnode_get (cm)))) != NULL) {
			lgs_send_to_logproc(LGSMSG_NICK, cl, cmdparams);
                }
                cm = list_next (cmdparams->source->user->chans, cm);
	}
	return NS_SUCCESS;
}

/* damn, 2.5.11 doesn't have a chan mode event. Duh! */
#ifdef EVENT_CHANMODE
static int lgs_event_cmode(CmdParams* cmdparams) 
{
	ChannelLog *cl;

	if (!strcasecmp(cmdparams->source->name, lgs_bot->name)) {
		/* its me, so ignore */
		return NS_FAILURE;
	}
	
	if ((cl = lgs_findactchanlog(cmdparams->channel)) != NULL) {
		lgs_send_to_logproc(LGSMSG_CHANMODE, cl, cmdparams);
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
ModuleEvent module_events[] = 
{
	{EVENT_NEWCHAN, lgs_event_newchan},
	{EVENT_JOIN, lgs_event_join,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_PART, lgs_event_part},
	{EVENT_KICK, lgs_event_kick},
	{EVENT_TOPIC, lgs_event_topic},
	{EVENT_NICK, lgs_event_nick},
#ifdef EVENT_CHANMODE
	{EVENT_CHANMODE, lgs_event_cmode},
#endif
	{EVENT_NULL, NULL}
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int ModInit (Module *mod_ptr)
{
	ModuleConfig (lgs_settings);
	lgschans = hash_create(-1, 0,0);
	
	/*XXX TODO */
	ircsnprintf(LogServ.logdir, MAXPATH, "logs/chanlogs");
	ircsnprintf(LogServ.savedir, MAXPATH, "ChanLogs");
	lgs_module = mod_ptr;
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


/* @brief manipulate the logged channels
 *
 * @param u The user requesting channel data
 * @param av the text sent
 * @param ac the number of words in av
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_chans(CmdParams* cmdparams)
{
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;

	if (!strcasecmp(cmdparams->av[0], "LIST")) {
		irc_prefmsg (lgs_bot, cmdparams->source, "Monitored Channel List:");
		hash_scan_begin(&hs, lgschans);
		while ((hn = hash_scan_next(&hs)) != NULL) {
			cl = hnode_get(hn);
			if ((cl->flags & LGSPUBSTATS) || (UserLevel(cmdparams->source) >= NS_ULEVEL_LOCOPER)) {
				/* its a priv channel, only show to opers */
				irc_prefmsg (lgs_bot, cmdparams->source, "%s (%c) URL: %s", cl->channame, (cl->flags & LGSACTIVE) ? '*' : '-', (cl->statsurl[0] != 0) ? cl->statsurl : "None");
			}							
		}
		irc_prefmsg (lgs_bot, cmdparams->source, "End Of List.");				
		return NS_SUCCESS;
	} else if (!strcasecmp(cmdparams->av[0], "DEL")) {
		if (cmdparams->ac < 2) {
			irc_prefmsg (lgs_bot, cmdparams->source, "Syntax error: insufficient parameters");
			irc_prefmsg (lgs_bot, cmdparams->source, "/msg %s HELP CHANS for more information", lgs_bot->name);
			return NS_FAILURE;
		}
		hn = hash_lookup(lgschans, cmdparams->av[1]);
		if (hn) {
			cl = hnode_get(hn);
		} else {
			irc_prefmsg (lgs_bot, cmdparams->source, "Can not find channel %s in Logging System", cmdparams->av[1]);
			return NS_FAILURE;
		}
		if (!cl) {
			irc_prefmsg (lgs_bot, cmdparams->source, "Can not find Channel %s in Logging System", cmdparams->av[1]);
			return NS_FAILURE;
		}
		/* rotate out the file */
		
		if (cl->flags & LGSACTIVE) {
			lgs_switch_file(cl);
		}
		if (cl->c) {
			cl->c->moddata[lgs_module->modnum] = NULL;
		}
		hash_delete(lgschans, hn);
		hnode_destroy(hn);
		irc_part (lgs_bot, cl->channame);
		free(cl);
		DelRow("Channel", cmdparams->av[1]);
		irc_prefmsg (lgs_bot, cmdparams->source, "Deleted Channel %s", cmdparams->av[1]);
		irc_chanalert (lgs_bot, "%s deleted %s from Channel Logging", cmdparams->source->name, cmdparams->av[1]);
		return NS_SUCCESS;
	} else if (!strcasecmp(cmdparams->av[0], "ADD")) {
		if (cmdparams->ac < 4) {
			irc_prefmsg (lgs_bot, cmdparams->source, "Syntax error: insufficient parameters");
			irc_prefmsg (lgs_bot, cmdparams->source, "/msg %s HELP CHANS for more information", lgs_bot->name);
			return NS_FAILURE;
		}
		lgs_newchanlog(cmdparams);
		return NS_SUCCESS;
	} else if (!strcasecmp(cmdparams->av[0], "SET")) {
		if (cmdparams->ac < 4) {
			irc_prefmsg (lgs_bot, cmdparams->source, "Syntax error: insufficient parameters");
			irc_prefmsg (lgs_bot, cmdparams->source, "/msg %s HELP CHANS for more information", lgs_bot->name);
			return NS_FAILURE;
		}
		hn = hash_lookup(lgschans, cmdparams->av[2]);
		if (hn) {
			cl = hnode_get(hn);
		} else {
			irc_prefmsg (lgs_bot, cmdparams->source, "Can not find channel %s in Logging System", cmdparams->av[1]);
			return NS_FAILURE;
		}
		if (!cl) {
			irc_prefmsg (lgs_bot, cmdparams->source, "Can not find Channel %s in Logging System", cmdparams->av[1]);
			return NS_FAILURE;
		}
		if (!strcasecmp(cmdparams->av[1], "URL")) {
			ircsnprintf(cl->statsurl, MAXPATH, "%s", cmdparams->av[3]);
			irc_prefmsg (lgs_bot, cmdparams->source, "Changed URL for %s to: %s", cl->channame, cl->statsurl);
			irc_chanalert (lgs_bot, "%s changed the URL for %s to: %s", cmdparams->source->name, cl->channame, cl->statsurl);
			lgs_save_channels_data(cl);
		} else {
			irc_prefmsg (lgs_bot, cmdparams->source, "Unknown Set Option");
		}
		return NS_SUCCESS;
	} else {
		irc_prefmsg (lgs_bot, cmdparams->source, "Syntax error: Unknown Command %s", cmdparams->av[0]);
		irc_prefmsg (lgs_bot, cmdparams->source, "/msg %s HELP CHANS for more information", lgs_bot->name);
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
static int lgs_stats(CmdParams* cmdparams)
{
	irc_prefmsg (lgs_bot, cmdparams->source, "LogServ Stats:");
	irc_prefmsg (lgs_bot, cmdparams->source, "Monitoring %d channels", (int)hash_count(lgschans));
	return NS_SUCCESS;
}


/* @breif Send the Log Message to the relevent Log Processor 
 *
 * @param msgtype the type of message (join, part etc)
 * @param av contents of the message 
 * @param ac message size 
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_send_to_logproc( logmsgtype msgtype, ChannelLog *lgschan, CmdParams* cmdparams) {
	switch (msgtype) {
		case 0:
			return logging_funcs[LogServ.logtype].joinproc(lgschan, cmdparams);
			break;
		case 1:
			return logging_funcs[LogServ.logtype].partproc(lgschan, cmdparams);
			break;
		case 2:
			return logging_funcs[LogServ.logtype].msgproc(lgschan, cmdparams);
			break;
		case 3:
			return logging_funcs[LogServ.logtype].quitproc(lgschan, cmdparams);
			break;
		case 4: 
			return logging_funcs[LogServ.logtype].topicproc(lgschan, cmdparams);
			break;
		case 5:
			return logging_funcs[LogServ.logtype].kickproc(lgschan, cmdparams);
			break;
		case 6:
			return logging_funcs[LogServ.logtype].nickproc(lgschan, cmdparams);
			break;
		case 7:
			return logging_funcs[LogServ.logtype].modeproc(lgschan, cmdparams);
			break;
		default:
			nlog (LOG_WARNING, "Unknown msgtype %d", msgtype);
			return NS_FAILURE;
			break;
	}	
	return NS_FAILURE;
}


ChannelLog *lgs_newchanlog(CmdParams* cmdparams) 
{
	Channel *c;
	ChannelLog *cl;
	hnode_t *cn;

	c = find_chan (cmdparams->av[1]);
	if (!c) {
		irc_prefmsg (lgs_bot, cmdparams->source, "Error, Channel %s is not Online at the moment", cmdparams->av[1]);
		return NULL;
	}
	if ((cn = hash_lookup(lgschans, cmdparams->av[1])) != NULL) {
		irc_prefmsg (lgs_bot, cmdparams->source, "Already Logging %s.", cmdparams->av[1]);
		return hnode_get(cn);
	}
	cl = malloc(sizeof(ChannelLog));
	bzero(cl, sizeof(ChannelLog));
	strlcpy(cl->channame, cmdparams->av[1], MAXCHANLEN);
	cl->c = c;
	c->moddata[lgs_module->modnum] = cl;
	if (!strcasecmp(cmdparams->av[2], "Public")) {
		cl->flags |= LGSPUBSTATS;
	} else if (!strcasecmp(cmdparams->av[2], "Private")) {
		cl->flags &= ~LGSPUBSTATS;
	} else {
		irc_prefmsg (lgs_bot, cmdparams->source, "Unknown Public Type %s. Setting to Public", cmdparams->av[2]);
		cl->flags |= LGSPUBSTATS;
	}
	if (cmdparams->ac == 4) {
		/* we have a URL */
		strlcpy(cl->statsurl, cmdparams->av[3], MAXPATH);
		irc_prefmsg (lgs_bot, cmdparams->source, "Stats URL is set to %s", cl->statsurl);
	} else {
		irc_prefmsg (lgs_bot, cmdparams->source, "No Stats URL is Set");
	}


	cn = hnode_create(cl);
	hash_insert(lgschans, cn, cl->channame);
	lgs_save_channels_data(cl);

	if (irc_join (lgs_bot, cl->channame, 0) == NS_SUCCESS) {
		cl->flags |= LGSACTIVE;
		nlog (LOG_NOTICE, "%s actived Logging on channel %s", cmdparams->source->name, cl->channame);
		irc_chanprivmsg (lgs_bot, cl->channame, "%s Actived Channel Logging on %s", cmdparams->source->name, cl->channame);
		if (cl->statsurl[0] != '\0') {
			irc_chanprivmsg (lgs_bot, cl->channame, "Stats will be avaiable at %s when Logs are processed next", cl->statsurl);
		}	
	}
	irc_prefmsg (lgs_bot, cmdparams->source, "Now Logging %s", cl->channame);
	irc_chanalert (lgs_bot, "%s Activated Logging on %s", cmdparams->source->name, cl->channame);
	return cl;
}

static void lgs_save_channels_data(ChannelLog *cl) {
	
	dlog (DEBUG1, "Saving Channel Data for %s", cl->channame);
	SetData((void *)cl->flags, CFGINT, "Channel", cl->channame, "Flags");
	SetData((void *)cl->statsurl, CFGSTR, "Channel", cl->channame, "URL");
}	

/* @brief find a active Channel Log record 
 * 
 * @params c - Channel Struct of the channel we are looking for 
 * 
 * @returns A ChannelLog Struct on success, or NULL on failure
 */

ChannelLog *lgs_findactchanlog(Channel *c) {
	ChannelLog *cl;
	if (c && c->moddata[lgs_module->modnum]) {
		cl = c->moddata[lgs_module->modnum];
#ifdef DEBUG
		/* paranoid checking this is */
		if (!(cl->c == c)) {
			nlog (LOG_WARNING, "Channel Log Channel doesn't match channel.");
			return NULL;
		}
#endif
		return cl;
	} else {
		return NULL;
	}
}

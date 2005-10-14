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
#include MODULECONFIG
#include "logserv.h"

Bot *lgs_bot;

/* forward decl */
static int lgs_cmd_add( const CmdParams *cmdparams );
static int lgs_cmd_del( const CmdParams *cmdparams );
static int lgs_cmd_list( const CmdParams *cmdparams );
static int lgs_cmd_url( const CmdParams *cmdparams );
static int lgs_cmd_stats( const CmdParams *cmdparams );
static void lgs_save_channel_data( ChannelLog *cl );

/** Copyright info */
const char *ls_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module Info definition */
ModuleInfo module_info =
{
	"LogServ",
	"Channel Logging Bot",
	ls_copyright,
	ls_about,
	NEOSTATS_VERSION,
	MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

static bot_cmd lgs_commands[]=
{
	{"ADD",		lgs_cmd_add ,	3, 	NS_ULEVEL_OPER,	lgs_help_add, 0, NULL, NULL},
	{"DEL",		lgs_cmd_del,	1, 	NS_ULEVEL_OPER,	lgs_help_del, 0, NULL, NULL},
	{"LIST",	lgs_cmd_list,	0, 	NS_ULEVEL_OPER,	lgs_help_list, 0, NULL, NULL},
	{"URL",		lgs_cmd_url,	2, 	NS_ULEVEL_OPER,	lgs_help_url, 0, NULL, NULL},
	{"STATS",	lgs_cmd_stats,	0, 	NS_ULEVEL_OPER,	lgs_help_stats, 0, NULL, NULL},
	NS_CMD_END()
};

bot_setting lgs_settings[]=
{
	{"LOGTYPE",		
	&LogServ.logtype,	
	SET_TYPE_INT,	
	1,	
	3,	
	NS_ULEVEL_ADMIN,
	NULL,
	lgs_help_set_logtype,
	NULL,( void* )1},
	{"LOGSIZE",		
	&LogServ.maxlogsize,	
	SET_TYPE_INT,	
	0,	
	10000000,	
	NS_ULEVEL_ADMIN,
	"Bytes",
	lgs_help_set_logsize,
	NULL,( void* )1000000},
	{"LOGAGE",		
	&LogServ.maxopentime,	
	SET_TYPE_INT,	
	0,	
	TS_ONE_DAY,	
	NS_ULEVEL_ADMIN,
	"Seconds",
	lgs_help_set_logtime ,
	NULL,( void* )TS_ONE_HOUR},
	{"LOGDIR",		
	LogServ.logdir,	
	SET_TYPE_STRING,	
	0,	
	MAXPATH,	
	NS_ULEVEL_ADMIN,
	NULL,
	lgs_help_set_logdir,
	NULL,( void* )"logs/chanlogs"},
	{"SAVEDIR",	
	LogServ.savedir,	
	SET_TYPE_STRING,	
	0,	
	MAXPATH,	
	NS_ULEVEL_ADMIN,
	NULL,
	lgs_help_set_savedir,
	NULL,( void* )"chanlogs"},
	NS_SETTING_END()
};

static void lgs_join_logged_channel( Channel* c, ChannelLog *cl )
{
	if( irc_join( lgs_bot, cl->channame, 0 ) == NS_SUCCESS ) {
		cl->flags |= LGSACTIVE;
		nlog( LOG_NOTICE, "Activated logging on %s", cl->channame );
		if( cl->statsurl[0] != '\0' ) {
			irc_chanprivmsg( lgs_bot, cl->channame, "Stats will be available at %s when Logs are processed next", cl->statsurl );
		}	
	}
	SetChannelModValue( c, cl );
	cl->c = c;
}

static int LoadLogChannel( void *data, int size )
{
	ChannelLog *cl;
	Channel *c;

	cl = ns_calloc( sizeof( ChannelLog ) );
	os_memcpy( cl, data, sizeof( ChannelLog ) );
	dlog( DEBUG1, "Loading Channel %s", cl->channame );
	c = FindChannel( cl->channame );
	if( c ) {
		lgs_join_logged_channel( c, cl );
	}
	hnode_create_insert( lgschans, cl, cl->channame );
	return NS_FALSE;
}

void LoadLogChannels( void )
{
	DBAFetchRows( "Channel", LoadLogChannel );
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

static int lgs_event_cprivate( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_MSG, cmdparams->channel, cmdparams );		
	return NS_SUCCESS;
}

static int lgs_event_cnotice( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_NOTICE, cmdparams->channel, cmdparams );		
	return NS_SUCCESS;
}

static int lgs_event_ctcpaction( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_CTCPACTION, cmdparams->channel, cmdparams );		
	return NS_SUCCESS;
}

static int lgs_event_part( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_PART, cmdparams->channel, cmdparams );	
	return NS_SUCCESS;
}


static int lgs_event_emptychan( const CmdParams *cmdparams ) 
{
	ChannelLog *cl;
	
	cl =( ChannelLog * )GetChannelModValue( cmdparams->channel );
	if( cl ) {
		/*close/switch the logfile*/
		lgs_switch_file( cl );
		ClearChannelModValue( cl->c );
		cl->c = NULL;
		cl->flags &= ~LGSACTIVE;
	}
	return NS_SUCCESS;
}

static int lgs_event_join( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_JOIN, cmdparams->channel, cmdparams );
	return NS_SUCCESS;
}

static int lgs_event_newchan( const CmdParams *cmdparams ) 
{
	ChannelLog *cl;

	cl =( ChannelLog * )hnode_find( lgschans, cmdparams->channel );
	if( cl ) {
		lgs_join_logged_channel( cmdparams->channel, cl );
	}
	return NS_SUCCESS;
}	

static int lgs_event_kick( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_KICK, cmdparams->channel, cmdparams );
	return NS_SUCCESS;
}

static int lgs_event_topic( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_TOPIC, cmdparams->channel, cmdparams );
	return NS_SUCCESS;
}

static int lgs_event_nick( const CmdParams *cmdparams ) 
{
	lnode_t *cm;

	/* ok, move through each of the channels */
	cm = list_first( cmdparams->source->user->chans );
	while( cm ) {
		lgs_send_to_logproc( LGSMSG_NICK,( ( Channel * )FindChannel( lnode_get( cm ) ) ), cmdparams );
        cm = list_next( cmdparams->source->user->chans, cm );
	}
	return NS_SUCCESS;
}

static int lgs_event_cmode( const CmdParams *cmdparams ) 
{
	lgs_send_to_logproc( LGSMSG_CHANMODE, cmdparams->channel, cmdparams );
	return NS_SUCCESS;
}

/** Module event list
 * What events we will act on
 * This is required if you want your module to respond to events on IRC
 * see modules.txt for a list of all events available
 */
ModuleEvent module_events[] = 
{
	{EVENT_CPRIVATE,	lgs_event_cprivate,	0},
	{EVENT_CNOTICE,		lgs_event_cnotice,	0},
	{EVENT_CTCPACTIONREQ,lgs_event_ctcpaction,	0},
	{EVENT_NEWCHAN,		lgs_event_newchan,	0},
	{EVENT_EMPTYCHAN,	lgs_event_emptychan,	0},
	{EVENT_JOIN,		lgs_event_join,		EVENT_FLAG_EXCLUDE_MODME},
	{EVENT_PART,		lgs_event_part,		EVENT_FLAG_EXCLUDE_MODME},
	{EVENT_KICK,		lgs_event_kick,		EVENT_FLAG_EXCLUDE_MODME},
	{EVENT_TOPIC,		lgs_event_topic,	EVENT_FLAG_EXCLUDE_MODME},
	{EVENT_NICK,		lgs_event_nick,		EVENT_FLAG_EXCLUDE_MODME},
	{EVENT_CMODE,		lgs_event_cmode,	EVENT_FLAG_EXCLUDE_MODME},
	NS_EVENT_END()
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int ModInit( void )
{
	ModuleConfig( lgs_settings );
	lgschans = hash_create( HASHCOUNT_T_MAX, 0,0 );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	/* Introduce a bot onto the network */
	lgs_bot = AddBot( &ls_botinfo );		
	if( !lgs_bot ) {
		return NS_FAILURE;
	}
	/* load Channels and join them */
	LoadLogChannels();
	/* start a timer to scan the logs for rotation */
	(void)AddTimer( TIMER_TYPE_INTERVAL, lgs_RotateLogs, "lgs_RotateLogs", 300, NULL );
	return NS_SUCCESS;
}

/** Init module
 * This is required if you need to do cleanup of your module when it ends
 */
int ModFini( void )
{
	/* close the log files */
	lgs_close_logs();
	/* delete the hash */
	hash_destroy( lgschans );
	return NS_SUCCESS;
}

/* ok, now here are logservs functions */

static int lgs_cmd_add( const CmdParams *cmdparams )
{
	Channel *c;
	ChannelLog *cl;

	if( hash_lookup( lgschans, cmdparams->av[1] ) != NULL ) {
		irc_prefmsg( lgs_bot, cmdparams->source, "Already Logging %s.", cmdparams->av[0] );
		return NS_FAILURE;
	}
	cl = ns_calloc( sizeof( ChannelLog ) );
	strlcpy( cl->channame, cmdparams->av[0], MAXCHANLEN );
	if( !ircstrcasecmp( cmdparams->av[1], "Public" ) ) {
		cl->flags |= LGSPUBSTATS;
	} else if( !ircstrcasecmp( cmdparams->av[1], "Private" ) ) {
		cl->flags &= ~LGSPUBSTATS;
	} else {
		irc_prefmsg( lgs_bot, cmdparams->source, "Unknown Public Type %s. Setting to Public", cmdparams->av[1] );
		cl->flags |= LGSPUBSTATS;
	}
	if( cmdparams->ac == 3 ) {
		/* we have a URL */
		strlcpy( cl->statsurl, cmdparams->av[2], MAXPATH );
		irc_prefmsg( lgs_bot, cmdparams->source, "Stats URL is set to %s", cl->statsurl );
	} else {
		irc_prefmsg( lgs_bot, cmdparams->source, "No Stats URL is Set" );
	}
	hnode_create_insert( lgschans, cl, cl->channame );
	lgs_save_channel_data( cl );
	c = FindChannel( cmdparams->av[0] );
	if( c ) {
		lgs_join_logged_channel( c, cl ); 
		irc_chanprivmsg( lgs_bot, cl->channame, "%s activated logging on %s", cmdparams->source->name, cl->channame );
	}
	nlog( LOG_NOTICE, "%s activated logging on %s", cmdparams->source->name, cl->channame );
	irc_prefmsg( lgs_bot, cmdparams->source, "Activated logging on %s", cl->channame );
	CommandReport( lgs_bot, "%s activated logging on %s", cmdparams->source->name, cl->channame );
	return NS_SUCCESS;
}

static int lgs_cmd_del( const CmdParams *cmdparams )
{
	hnode_t *hn;
	ChannelLog *cl;

	hn = hash_lookup( lgschans, cmdparams->av[0] );
	if( !hn ) {
		irc_prefmsg( lgs_bot, cmdparams->source, "Can not find channel %s in Logging System", cmdparams->av[0] );
		return NS_FAILURE;
	}
	cl =( ChannelLog * ) hnode_find( lgschans, cmdparams->av[0] );
	if( !cl ) {
		irc_prefmsg( lgs_bot, cmdparams->source, "Can not find Channel %s in Logging System", cmdparams->av[0] );
		return NS_FAILURE;
	}
	/* rotate out the file */
	if( cl->flags & LGSACTIVE ) {
		lgs_switch_file( cl );
	}
	if( cl->c ) {
		ClearChannelModValue( cl->c );
	}
	hash_delete( lgschans, hn );
	hnode_destroy( hn );
	irc_part( lgs_bot, cl->channame, NULL );
	ns_free( cl );
	DBADelete( "Channel", cmdparams->av[1] );
	irc_prefmsg( lgs_bot, cmdparams->source, "Deleted channel %s", cmdparams->av[0] );
	CommandReport( lgs_bot, "%s deleted %s from logging", cmdparams->source->name, cmdparams->av[0] );
	return NS_SUCCESS;
}

static int lgs_cmd_list( const CmdParams *cmdparams )
{
	hscan_t hs;
	hnode_t *hn;
	ChannelLog *cl;

	irc_prefmsg( lgs_bot, cmdparams->source, "Log channel list:" );
	hash_scan_begin( &hs, lgschans );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		cl = hnode_get( hn );
		if( ( cl->flags & LGSPUBSTATS ) ||( UserLevel( cmdparams->source ) >= NS_ULEVEL_LOCOPER ) ) {
			/* its a priv channel, only show to opers */
			irc_prefmsg( lgs_bot, cmdparams->source, "%s( %c ) URL: %s", cl->channame,( cl->flags & LGSACTIVE ) ? '*' : '-',( cl->statsurl[0] != 0 ) ? cl->statsurl : "None" );
		}							
	}
	irc_prefmsg( lgs_bot, cmdparams->source, "End of list." );				
	return NS_SUCCESS;
}

static int lgs_cmd_url( const CmdParams *cmdparams )
{
	ChannelLog *cl;

	cl =( ChannelLog * ) hnode_find( lgschans, cmdparams->av[0] );
	if( !cl ) {
		irc_prefmsg( lgs_bot, cmdparams->source, "Can not find channel %s in Logging System", cmdparams->av[0] );
		return NS_FAILURE;
	}
	if( ValidateURL( cmdparams->av[1] ) != NS_SUCCESS )
	{
		irc_prefmsg( lgs_bot, cmdparams->source, "%s is an invalid URL", cmdparams->av[1] );
		return NS_FAILURE;
	}
	ircsnprintf( cl->statsurl, MAXPATH, "%s", cmdparams->av[1] );
	irc_prefmsg( lgs_bot, cmdparams->source, "Changed URL for %s to: %s", cl->channame, cl->statsurl );
	CommandReport( lgs_bot, "%s changed the URL for %s to: %s", cmdparams->source->name, cl->channame, cl->statsurl );
	lgs_save_channel_data( cl );
	return NS_SUCCESS;
}

/* @brief Send some very simple stats to the user
 *
 * @param u The user requesting stats data
 * @returns NS_SUCCESS or NS_FAILURE
 */
static int lgs_cmd_stats( const CmdParams *cmdparams )
{
	irc_prefmsg( lgs_bot, cmdparams->source, "Stats:" );
	irc_prefmsg( lgs_bot, cmdparams->source, "Logging %d channels",( int )hash_count( lgschans ) );
	return NS_SUCCESS;
}

static void lgs_save_channel_data( ChannelLog *cl ) {
	
	dlog( DEBUG1, "Saving Channel Data for %s", cl->channame );
	DBAStore( "Channel", cl->channame, cl, sizeof( ChannelLog ) );
}	

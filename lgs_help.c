/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
** $Id: ss_help.c 1086 2004-12-12 20:15:54Z Mark $
*/

#include "stats.h"

const char lgs_help_about_oneline[]="About LogServ";
const char lgs_help_version_oneline[]="Shows you the current LogServ Version";
const char lgs_help_chan_oneline[]="Manipulate Channel Logging List";
const char lgs_help_stats_oneline[]="Simple LogServ Stats";

const char *lgs_help_about[] = {
	"\2LogServ\2 provides a Logging Service for Channels on IRC",
	"That can be used to generate statistical information about the channel",
	NULL
};

const char *lgs_help_chan[] = {
	"Syntax: \2CHANS \37ADD <Channame> <\"public\"/\"private\"> <url>\37\2",
	"        \2CHANS \37DEL <Channame>\37\2",
	"        \2CHANS \37LIST\37\2",
	"        \2CHANS \37SET URL <url>\37\2",
	"",
	"Provides Logging Services for Channels on the network",
	"\2ADD\2 Adds a channel to LogServ for Monitoring. You must specify:",
	"      The Channel Name",
	"      Public/Private to indicate if the logging for this channel should be",
	"      made known to users",
	"      a URL to display when joining the channel showing where users can view",
	"      Statistics about the channel",
	"\2DEL\2 Deletes a logged channel from LogServ",
	"      Requires the channel name",
	"\2LIST\2 Lists the channels that LogServ is currently logging",
	"\2SET\2 Modifies certain features of LogServ on that channel",
	"      Those options include:",
	"      \2URL\2 - Modifies the URL for the stats on that channel",
	NULL
};

const char *lgs_help_stats[] = {
	"Syntax: \2STATS\2",
	"        \2STATS\2 displays some simple stats about LogServ",
	NULL
};

const char *lgs_help_version[] = {
	"Displays LogServ's Version Number",
	NULL
};

const char *lgs_help_set_logtype[] = {
	"\2LOGTYPE\2 <type>",
	"Modifies the logfile format that LogServ uses.",
	"The options are:",
	"\2 1\2 - EggDrop Format",
	"\2 2\2 - Mirc Format",
	"\2 3\2 - Xchat Format",
	"",
	"You should restart NeoStats, or reload the LogServ module after changing",
	"the log file format so new log files will be created",
	NULL
};

const char *lgs_help_set_logsize[] = {
	"\2LOGSIZE\2 <bytes> - Specify the size in bytes at which a logfile is automatically",
	"rotated",
	NULL
};

const char *lgs_help_set_logtime[] = {
	"\2LOGAGE\2 <seconds> - Specify the age in seconds that we rotate the log file, regardless of log file size",
	NULL
};

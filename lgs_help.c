/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
** $Id: ss_help.c 1086 2003-12-12 20:15:54Z Mark $
*/

#include "stats.h"

const char lgs_help_about_oneline[]="About StatServ";
const char lgs_help_version_oneline[]="Shows you the current StatServ Version";
const char lgs_help_chan_oneline[]="Channel Information";
const char lgs_help_stats_oneline[]="Modify Statistic Entries.";

const char *lgs_help_about[] = {
	"\2StatServ\2 provides detailed statistics about your",
	"IRC network users, channels and servers.",
	NULL
};

const char *lgs_help_chan[] = {
	"Syntax: \2CHAN \37<POP/KICKS/TOPICS/<Channame>>\37\2",
	"",
	"Provides Statistics on Channels on the network",
	"\2CHAN\2 By itself provides a list of the top 10 channels",
	"    based on the current number of members",
	"\2CHAN POP\2 gives you information on the most popular",
	"    channels on the network based on the number of joins",
	"\2CHAN KICKS\2 Gives you the top 10 kicking channels",
	"\2CHAN TOPICS\2 Gives you the top 10 topic changing channels",
	"\2CHAN <name>\2 Gives you specific information on a channel",
	NULL
};

const char *lgs_help_stats[] = {
	"Syntax: \2STATS LIST\2",
	"        \2STATS DEL <servername>\2",
	"        \2STATS COPY <oldservername> <newservername>\2",
	"",
	"Use, LIST to list all database entries. DEL to remove an",
	"entry and COPY to copy an entry.",
	NULL
};

const char *lgs_help_version[] = {
	"hrm",
	NULL,
};

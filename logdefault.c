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

#include "logserv.h"  /* LogServ Definitions */

char *logserv_startlog(ChannelLog *chandata) {
	return startlog;
}


int logserv_joinproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}

int logserv_partproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}

int logserv_msgproc(ChannelLog *chandata, CmdParams* cmdparams) {
	return NS_SUCCESS;
}

int logserv_noticeproc(ChannelLog *chandata, CmdParams* cmdparams) {
	return NS_SUCCESS;
}

int logserv_ctcpaction(ChannelLog *chandata, CmdParams* cmdparams) {
	return NS_SUCCESS;
}

int logserv_quitproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}

int logserv_topicproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}

int logserv_kickproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}
int logserv_nickproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}
int logserv_modeproc(ChannelLog *chandata, CmdParams* cmdparams) {

	return NS_SUCCESS;
}


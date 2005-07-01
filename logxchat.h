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

#ifndef LOGXCHAT_H
#define LOGXCHAT_H

char *xchat_startlog(ChannelLog *cl);
int xchat_joinproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_partproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_msgproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_noticeproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_ctcpaction(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_quitproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_topicproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_kickproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_nickproc(ChannelLog *chandata, CmdParams* cmdparams);
int xchat_modeproc(ChannelLog *chandata, CmdParams* cmdparams);

#endif
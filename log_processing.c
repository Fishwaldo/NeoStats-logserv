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

int dirc_joinproc(ChannelLog *chandata, char **av, int ac) {

}

int dirc_partproc(ChannelLog *chandata, char **av, int ac) {

}

int dirc_msgproc(ChannelLog *chandata, char **av, int ac) {
printf("doing msgproc\n");
}

int dirc_quitproc(ChannelLog *chandata, char **av, int ac) {

}

int dirc_topicproc(ChannelLog *chandata, char **av, int ac) {

}

int dirc_kickproc(ChannelLog *chandata, char **av, int ac) {

}
int egg_joinproc(ChannelLog *chandata, char **av, int ac) {

}

int egg_partproc(ChannelLog *chandata, char **av, int ac) {

}

int egg_msgproc(ChannelLog *chandata, char **av, int ac) {

}

int egg_quitproc(ChannelLog *chandata, char **av, int ac) {

}

int egg_topicproc(ChannelLog *chandata, char **av, int ac) {

}

int egg_kickproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_joinproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_partproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_msgproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_quitproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_topicproc(ChannelLog *chandata, char **av, int ac) {

}

int mirc_kickproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_joinproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_partproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_msgproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_quitproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_topicproc(ChannelLog *chandata, char **av, int ac) {

}

int xchat_kickproc(ChannelLog *chandata, char **av, int ac) {

}

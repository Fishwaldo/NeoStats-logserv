#!/bin/sh

# Channel og Directory. Where Logfiles are stored. Only need to change this
#if you installed neostats into a different prefix other than NeoStats
chanlogs="$HOME/NeoStats/ChanLogs/"

# Onis Installation Path
onisinstall="$HOME/onis-0.4.6/" 

# optional Onis Commandline options
onisoptions=""

doonis() {
	#test if output directory exists 
	if [ ! -d $chanlogs/$1/html ]; then
		mkdir $chanlogs/$1/html
	fi
	cd $chanlogs/$1
	files=`ls $chanlogs/$1/*.log`
	command="$onisinstall/onis --config $chanlogs/onisconfig --output $chanlogs/$1/html/index.html --channel \"$1\" --persistency_file $chanlogs/$1/persistancy.data --overwrite true $onisoptions $files"
	echo `$command`
	cd $chanlogs
}

cd $chanlogs


directorys=`ls $chanlogs`
for channel in $directorys; do
    if [ -d $channel ]
	then doonis $channel
    fi
done

#!/bin/sh
echo "
===========================================

 ERA-Polytech team

  NE J.S.Company 
  St.Petersburg State Polytechnical University
  St. Petersburg, Russia
  
  Research Coordinator: Lev Stankevich
  Team Coordinator: Alexei Kritchoun
  Created by: Anton Ivanov, Sergey Serebryakov
  
  mailto: erapolytech@hotbox.ru robocup@mail.ru
 
    (c) 2001-2003 NE JSC

===========================================
"

if [ $# -lt 1 ];
then
    echo "Usage: $0 num servername [g]"
    exit 1
fi
if [ $# -lt 2 ];
then
    SERVER="localhost"
else
    SERVER=$2
fi

PROGRAM=./agent

echo "Server: "$SERVER

num=0
while [ $num -lt $1 ] ; 
do    
    cmdline="$PROGRAM -host $SERVER -file ./client.conf "
    if [ $num = 0 ] ;
    then
        if [ $# = 3 ] ;
        then
    	    if [ $3 = "g" ];
    
	    then
	        echo "Starting goalie"
	        echo $cmdline -goalie &
                $cmdline -goalie &
	    fi
        else
            echo $cmdline &
	    $cmdline &
    
        fi
    else
            echo $cmdline &
            $cmdline &
    fi
    num=`expr $num + 1`
    sleep 1
done

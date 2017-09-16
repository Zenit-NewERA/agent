#!/bin/sh

if [ $# = 1 ]; 
then
    teamname=$1
else
    teamname="STEP"
fi

#set simulator_step = 30
host="localhost"
programname="./agent"

echo "Running with the following parameters"
echo "Hostname: " $host
echo "Team: " $teamname
echo "Program: " $programname
#echo "Sim_step: " $simulator_step

$programname -file client.conf -team_name $teamname -goalie &
sleep 1
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
$programname -file client.conf -host $host -team_name $teamname &
sleep 1
~/coach/coach/coach -file /home/anton/coach/coach/coach.conf &

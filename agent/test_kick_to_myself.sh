#!/bin/sh

if [ $# = 1 ]; 
then
    teamname=$1
else
    teamname="Zenit-NewERA"
fi

#set simulator_step = 30
host="localhost"
programname="./agent"

echo "Running with the following parameters"
echo "Hostname: " $host
echo "Team: " $teamname
echo "Program: " $programname
#echo "Sim_step: " $simulator_step

$programname -file  client.conf  -host $host -team_name $teamname &
echo "<1> -> Initialize"
sleep 1
$programname -file client.conf -host $host -team_name $teamname  &
echo "<2> -> Initialize"
$programname -file client.conf -host $host -team_name $teamname  &
echo "<2> -> Initialize"
sleep 1
$programname -file  client.conf  -host $host -team_name "team2" -goalie &
echo "<1> -> Initialize"

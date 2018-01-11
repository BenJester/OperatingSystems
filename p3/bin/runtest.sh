#!/bin/bash

# script directory
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ "$#" -ne 2 ]; then
    echo "usage: startservers.sh <time to simulate system (seconds)> <test script file>"
    exit;
fi

OUTPUT_FILE="$SCRIPT_DIR/../serverlogs/server.output"
echo "Writing simulation output to $OUTPUT_FILE"

TIME_TO_SIMULATE="$1"
echo "Simulation will last $TIME_TO_SIMULATE seconds"
echo "TIME_TO_SIMULATE=$TIME_TO_SIMULATE" >> "$OUTPUT_FILE"

TEST_FILE="$2"
NUM_SERVERS=0

RMI_PORT="1098"

LOG_DIR="$SCRIPT_DIR/../serverlogs"
echo "Reading log files from $LOG_DIR"
CONFIG_DIR="$SCRIPT_DIR/../serverlogs"
echo "Reading config file from $CONFIG_DIR"

echo "Restarting rmiregistry"
PIDOF="$SCRIPT_DIR/pidof"
RMI_PID=`"$PIDOF" rmiregistry`

if [ -n "$RMI_PID" ] 
then
    echo "Killing old rmiregistry ($RMI_PID)"
    kill -9 "$RMI_PID"
fi

cd "$SCRIPT_DIR"
rmiregistry "$RMI_PORT" &
echo "Waiting for rmiregistry to start."
sleep 5

declare -a SERVER_PIDS

function restart_server {
    id=$1
    if [ -z "${SERVER_PIDS[$id]}" ] 
    then
	java -classpath "$SCRIPT_DIR" edu.duke.raft.StartServer 1098 "$id" "$LOG_DIR" "$CONFIG_DIR" >> "$OUTPUT_FILE" &
	PID="$!"
	SERVER_PIDS[$id]="$PID"
	echo "Started server S$id"
    fi
}

function start_servers {
    NUM_SERVERS=$1
    for (( id=1; id<=$NUM_SERVERS; id++ ))
    do
	# initialize servers' log and config files
	if [ ! -f "$LOG_DIR/$id.log" ]; then
	    cp "$LOG_DIR/init.log" "$LOG_DIR/$id.log"
	fi
	if [ ! -f "$LOG_DIR/$id.config" ]; then
	    cp "$CONFIG_DIR/init.config" "$CONFIG_DIR/$id.config"
	fi
	
	echo "NUM_SERVERS=$NUM_SERVERS" >> "$CONFIG_DIR/$id.config"
	restart_server $id
    done
}

function pause_server {
    kill -SIGSTOP ${SERVER_PIDS[$1]}
    echo "Paused server S$1"
}

function resume_server {
    kill -SIGCONT ${SERVER_PIDS[$1]}
    echo "Resumed server S$1"
}

function fail_server {
    kill -9 ${SERVER_PIDS[$1]}
    SERVER_PIDS[$1]=""
    echo "Failed server S$1"
}

START=`date +%s`
# load the test script
if [ -e "$SCRIPT_DIR/../$TEST_FILE" ]; then
    source "$SCRIPT_DIR/../$TEST_FILE"
    while [ $(( $(date +%s) - $TIME_TO_SIMULATE )) -lt $START ] 
    do
	sleep 5
    done
else
    echo "Could not find test file $SCRIPT_DIR/../$TEST_FILE"
fi

echo "Shutting down simulation"

for (( id=1; id<=$NUM_SERVERS; id++ ))
do
    echo "Shutting down server S$id"
    kill -9 ${SERVER_PIDS[$id]}
done

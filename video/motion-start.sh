#!/bin/bash
date=$(date +%Y-%m-%d_%H:%M:%S%z)
start_dir=`pwd`

function ask() { 
        REPLY=$1
        if ! [[ $REPLY =~ ^[Yy]$ ]]; then
                echo "Aborting."
                exit 0
        fi
}

function start_chainsign_daemon() { 
	echo "[INFO] Starting chainsign daemon" 
	if ! [ -d ../chainsign ]; then
		echo "[ERROR] chainsign directory doesn't exist"
		echo "Run ./do script or(and) check directory"
	fi	

	cd ../chainsign

	if ! [ -f chainsign ]; then
		cmake . && make
	fi

	./chainsign --daemon &
	cd $start_dir
}


if [[ $0 != "./motion-start.sh" ]]; then 
	echo "[WARN] This script should be called using command: ./motion-start"
	echo "[WARN] Continue? "
	read yn
	ask $yn
fi

if [[ $1 != "--dry-run" ]]; then
	start_chainsign_daemon 

	echo "guarding-since-$date[$1]" > ~/stat

	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting..."
	sleep 10
	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting..."
	sleep 60

	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting...3"
	sleep 5
	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting...2"
	sleep 5
	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting...1"
	sleep 5
	./lib_sendxmpp.sh "rfree.mobile@jit.si" "starting...GO"
	sleep 5 
fi

motion

if [[ $1 != "--dry-run" ]]; then
	./lib_sendxmpp.sh "rfree.mobile@jit.si" "end of program"
fi

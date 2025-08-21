#!/bin/sh
if type sleep | grep -q -- "shell builtin" ; then
	true
else
	if ksh93 -c 'true' ; then
		ksh93 "$0" "$*"
		exit
	fi
fi


if [ -z "$1" ]; then
        echo
        echo usage: $0 network-interface
        echo
        echo e.g. $0 eth0
        echo
        echo shows packets-per-second
        exit
fi

IF=$1

if [ -d /sys/class/net/$1/wireless ]
then
	echo "action:xterm -e 'sh -c $HOME/aliases/wifi/wifi.sh'"
	type=wireless
else
	type=wired
fi

while true
do
	upstatus=`cat /sys/class/net/$1/operstate`
        R1=`cat /sys/class/net/$1/statistics/rx_packets`
        T1=`cat /sys/class/net/$1/statistics/tx_packets`
        sleep 0.5
        R2=`cat /sys/class/net/$1/statistics/rx_packets`
        T2=`cat /sys/class/net/$1/statistics/tx_packets`
        TXPPS=`expr $T2 - $T1`
        RXPPS=`expr $R2 - $R1`
	if [ "$upstatus" = "up" ]; then
		if [ "$RXPPS" -eq "0" ]; then
			if [ "$TXPPS" -eq "0" ]; then
				#echo "icon:network-idle";
				echo "icon:idle-$type.xpm"
			else
				#echo "icon:network-transmit";
				echo "icon:send-$type.xpm"
			fi
		else
			if [ "$TXPPS" -eq "0" ]; then
				#echo "icon:network-receive";
				echo "icon:receive-$type.xpm"
			else
				#echo "icon:network-transmit-receive";
				echo "icon:sendreceive-$type.xpm"
			fi
		fi
#		if [ "$type" = "wireless" ]; then
#			#echo menu:Disable!ifup $1||View Available Wireless Networks!xterm -e 'sh -c $HOME/aliases/wifi/wifi.sh']
#			echo "menu:Disable!ifdown $1||View Available Wireless Networks!xterm"
#		else
#			#echo menu:Disable[!"ifup $1"]
#			echo "menu:Disable!bash -c 'yad --about'!gtk-about||Quit!quit!gtk-quit"
#		fi
	else
		#echo "icon:network-offline"
		echo "icon:offline-$type.xpm"
	fi

#        echo "tx $1: $TXPPS pkts/s rx $1: $RXPPS pkts/s"
done

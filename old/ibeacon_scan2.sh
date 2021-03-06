#!/bin/bash
# iBeacon Scan by Radius Networks

if [[ $1 == "parse" ]]; then
  packet=""
  capturing=""
  count=0
  seq1=0
  seq2=0
  seq3=0
  seq4=0
  already1=0
  already2=0
  already3=0
  already4=0
  receiveCount=5
  #echo "" > /home/pi/iBeaconScan/BLE1.txt   #清空
  #echo "" > /home/pi/iBeaconScan/BLE2.txt   #清空
  while read line
  do
    count=$[count + 1]
    if [ "$capturing" ]; then
      if [[ $line =~ ^[0-9a-fA-F]{2}\ [0-9a-fA-F] ]]; then
        packet="$packet $line"
      else
        if [[ $packet =~ ^04\ 3E\ 2A\ 02\ 01\ .{26}\ 02\ 01\ .{14}\ 02\ 15 ]]; then
          UUID=`echo $packet | sed 's/^.\{69\}\(.\{47\}\).*$/\1/'`
          MAJOR=`echo $packet | sed 's/^.\{117\}\(.\{5\}\).*$/\1/'`
          MINOR=`echo $packet | sed 's/^.\{123\}\(.\{5\}\).*$/\1/'`
          POWER=`echo $packet | sed 's/^.\{129\}\(.\{2\}\).*$/\1/'`
		  
		  RSSI=`echo $packet | sed 's/^.\{132\}\(.\{2\}\).*$/\1/'`
		  
          UUID=`echo $UUID | sed -e 's/\ //g' -e 's/^\(.\{8\}\)\(.\{4\}\)\(.\{4\}\)\(.\{4\}\)\(.\{12\}\)$/\1-\2-\3-\4-\5/'`
          MAJOR=`echo $MAJOR | sed 's/\ //g'`
          MAJOR=`echo "ibase=16; $MAJOR" | bc`
          MINOR=`echo $MINOR | sed 's/\ //g'`
          MINOR=`echo "ibase=16; $MINOR" | bc`
          POWER=`echo "ibase=16; $POWER" | bc`
		  
		  RSSI=`echo "ibase=16; $RSSI" | bc`
		  
          POWER=$[POWER - 256]
		  RSSI=$[RSSI - 256]
		  
          if [[ $2 == "-b" ]]; then
			echo "$UUID $MAJOR $MINOR $POWER"
          else
    	    #echo "UUID: $UUID MAJOR: $MAJOR MINOR: $MINOR POWER: $POWER"	
	        #echo "UUID: $UUID MAJOR: $MAJOR MINOR: $MINOR POWER: $POWER RSSI: $RSSI"
			
					if [[ $MAJOR -eq 87 ]]; then
						case $MINOR in
						"1") 
							if [[ $already1 -eq 0 ]]; then
								echo "" > /home/pi/iBeaconScan/BLE1.txt   #清空
								already1=1								
							fi							
							seq1=$[seq1+1]
							echo "$seq1 $MINOR $POWER $RSSI" >> /home/pi/iBeaconScan/BLE1.txt
						;;
						"2") 
							if [[ $already2 -eq 0 ]]; then
								echo "" > /home/pi/iBeaconScan/BLE2.txt   #清空
								already2=1
							fi	
							seq2=$[seq2+1]
							echo "$seq2 $MINOR $POWER $RSSI" >> /home/pi/iBeaconScan/BLE2.txt
						;;						
						"3") 
							if [[ $already3 -eq 0 ]]; then
								echo "" > /home/pi/iBeaconScan/BLE3.txt   #清空
								already3=1
							fi	
							seq3=$[seq3+1]
							echo "$seq3 $MINOR $POWER $RSSI" >> /home/pi/iBeaconScan/BLE3.txt
						;;	
						"4") 
							if [[ $already4 -eq 0 ]]; then
								echo "" > /home/pi/iBeaconScan/BLE4.txt   #清空
								already4=1
							fi	
							seq4=$[seq4+1]
							echo "$seq4 $MINOR $POWER $RSSI" >> /home/pi/iBeaconScan/BLE4.txt
						;;						
						esac
					fi		
		
									
					if [[ $seq1 -eq $receiveCount ]]; then
						seq1=0
						#echo "" > /home/pi/iBeaconScan/BLE1.txt
						echo "start" > /home/pi/iBeaconScan/BLE1.txt
					fi
					if [[ $seq2 -eq $receiveCount ]]; then
						seq2=0
						#echo "" > /home/pi/iBeaconScan/BLE2.txt
					fi	
	 fi
        fi
        capturing=""
        packet=""
      fi
    fi

    if [ ! "$capturing" ]; then
      if [[ $line =~ ^\> ]]; then
        packet=`echo $line | sed 's/^>.\(.*$\)/\1/'`
        capturing=1
      fi
    fi
  done
else
  sudo hcitool lescan --duplicates 1>/dev/null &
  if [ "$(pidof hcitool)" ]; then
    sudo hcidump --raw | ./$0 parse $1
  fi
fi

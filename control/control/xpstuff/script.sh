#!/bin/sh
type=pppoe; for i in idle tx rx txrx; do composite ${i}.ico ${type}.ico xp_ncpa_${type}_${i}.png; done; composite xp_ncpa_${type}_txrx.png nohw.ico xp_ncpa_${type}_nohw.png; ln -s xp_ncpa_${type}_idle.png xp_ncpa_${type}_down.png

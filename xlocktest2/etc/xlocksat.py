#!/usr/bin/python3
# xlocksat

import datetime
import sys
def put(string):
  sys.stdout.write(string)

t = datetime.datetime.now().time()
#print t.hour
saturation = round((12.0 - abs(t.hour - 12.0)) / 12.0, 2)
put(str(saturation))

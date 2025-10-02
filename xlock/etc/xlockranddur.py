#!/usr/bin/python3
# xarand

import subprocess
import sys
def cmd_exists(cmd):
  return subprocess.call("type " + cmd, shell=True,
    stdout=subprocess.PIPE, stderr=subprocess.PIPE) == 0
def put(string):
  sys.stdout.write(string)

if cmd_exists("/usr/games/fortune"):
  p1 = subprocess.Popen(["/usr/games/fortune"], shell=True, stdout=subprocess.PIPE)
else:
  if cmd_exists("/usr/bin/fortune"):
    p1 = subprocess.Popen(["/usr/bin/fortune"], shell=True, stdout=subprocess.PIPE)
  else:
    print("17")
    quit()
p2 = subprocess.Popen(["wc"], shell=True, stdin=p1.stdout, stdout=subprocess.PIPE)
(output, err) = p2.communicate()
#print output
numbers = output.split()
number = int(numbers[2]) / 3
put(str(number))

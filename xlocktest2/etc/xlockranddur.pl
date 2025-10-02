#!/usr/bin/perl -w
# xarand
if (-e '/usr/games/fortune' && -x _) {
  open(S,"/usr/games/fortune|wc|");
} elsif (-e '/usr/bin/fortune' && -x _) {
  open(S,"/usr/bin/fortune|wc|");
} else {
  print "17\n"; # a random number
  exit;
}
local(@numbers) = split(" ",<S>);
print int $numbers[2]/3;
close(S);

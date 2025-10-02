#!/usr/bin/perl -T -w
# This is used for moving from human readable life format ot a code format.

local($pl) = shift(@ARGV);
if (!defined $pl || ("$pl" ne "tr" && "$pl" ne "sq" && "$pl" ne "pn" && "$pl" ne "hx")) {
  print "\nUsage: $0 [tr | sq | pn | hx]\n";
  print "\ttr: triangle format (same as sq)\n";
  print "\tsq: square format\n";
  print "\tpn: pentagon (cairo tiling) format (same as sq)\n";
  print "\thx: hexagon format\n";
  exit 1;
}

&search;

sub search {
  local ($col, $row);
  local ($x, $y);
  local ($NOTUSED);
  local (@array);

  $HALFMAX = 64; # really 32 but being safe
  $MINCOL = $MINROW = $HALFMAX;
  $MAXCOL = $MAXROW = $HALFMAX;
  $NOTUSED = -127;
  $col = $row = $NOTUSED;
  $number = 0;
  while (<>) {
    if (!($_ =~ /^#/)) {
      @chars = split(//);
      $number = 0;
      $col = $NOTUSED;
      $negative = 1;
      foreach $c (@chars) {
        if ($c =~ /[-]/) {
          $negative = -1;
        } elsif ($c =~ /[0123456789]/) {
          $number = $number * 10 + ($c - '0');
        } elsif ($c =~ /[,]/) { # Last number does not have a ","
          if ($col > $NOTUSED) {
            $row = $number * $negative;
            $col = $col + $HALFMAX;
            $row = $row + $HALFMAX;
            $array{$col, $row} = 1;
            if ($col > $MAXCOL) {
              $MAXCOL = $col;
            } elsif ($col < $MINCOL) {
              $MINCOL = $col;
            }
            if ($row > $MAXROW) {
              $MAXROW = $row;
            } elsif ($row < $MINROW) {
              $MINROW = $row;
            }
            $col = $NOTUSED;
          } else {
            $col = $number * $negative;
          }
          $number = 0;
          $negative = 1;
        } elsif ($c =~ /[{}\/]/) { # Last number does not have a ","
          $col = $NOTUSED;
          $number = 0;
          $negative = 1;
        }
      }
    }
  }
  $x=$MAXCOL - $MINCOL + 1;
  $y=$MAXROW - $MINROW + 1;
  print "#x=$x, y=$y\n";
  for ($j = $MINROW; $j <= $MAXROW; $j++) {
    if ("$pl" eq "hx") {
      for ($j1 = $MINROW; $j1 <= $MAXROW + $MINROW - $j - 1; $j1++) {
        print " ";
      }
    }
    for ($i = $MINCOL; $i <= $MAXCOL; $i++) {
      if ($array{$i, $j}) {
        if ("$pl" eq "hx") {
          print "O ";
        } else {
          print "*";
        }
      } else {
        if ("$pl" eq "hx") {
          print ". ";
        } else {
          print ".";
        }
      }
    }
    print "\n";
  }
}

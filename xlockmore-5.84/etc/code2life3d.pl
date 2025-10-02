#!/usr/bin/perl -T -w
# Used for moving code format back into human readable life format.

local($pl) = shift(@ARGV);
if (!defined $pl || ("$pl" ne "cb" && "$pl" ne "dd" && "$pl" ne "td")) {
  print "\nUsage: $0 [cb | dd | td]\n";
  print "\tcb: cube format\n";
  print "\tdd: rhombic dodecahedron format\n";
  print "\ttd: truncated octahedron (tetradecahedron) format (same as cb)\n";
  exit 1;
}

&search;

sub search {
  local ($col, $row, $stack);
  local ($x, $y, $z);
  local ($NOTUSED);
  local (@array);

  $HALFMAX = 64; # really 32 but being safe
  $MINCOL = $MINROW = $MINSTACK = $HALFMAX;
  $MAXCOL = $MAXROW = $MAXSTACK = $HALFMAX;
  $NOTUSED = -127;
  $col = $row = $stack = $NOTUSED;
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
          if ($row > $NOTUSED && $col > $NOTUSED) {
            $stack = $number * $negative;
            $col = $col + $HALFMAX;
            $row = $row + $HALFMAX;
            $stack = $stack + $HALFMAX;
            $array{$col, $row, $stack} = 1;
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
            if ($stack > $MAXSTACK) {
              $MAXSTACK = $stack;
            } elsif ($stack < $MINSTACK) {
              $MINSTACK = $stack;
            }
            $row = $NOTUSED;
            $col = $NOTUSED;
          } elsif ($col > $NOTUSED) {
            $row = $number * $negative;
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
  $z=$MAXSTACK - $MINSTACK + 1;
  print "#x=$x, y=$y, z=$z\n";
  for ($k = $MINSTACK; $k <= $MAXSTACK; $k++) {
    for ($j = $MINROW; $j <= $MAXROW; $j++) {
      for ($i = $MINCOL; $i <= $MAXCOL; $i++) {
        if ($array{$i, $j, $k}) {
	  if ("$pl" eq "dd") {
            print "O ";
	  } else {
            print "*";
	  }
        } else {
	  if ("$pl" eq "dd") {
            print ". ";
	  } else {
            print ".";
	  }
        }
      }
      print "\n";
    }
    print "\n";
  }
}

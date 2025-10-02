#!/usr/bin/perl -T -w
# This is used for moving from human readable life format ot a code format.
#
# Run like life2code.pl sq < piston.life
#sq
#piston.life
##P -10 -3  Treated as a comment, program finds own center
#..........*...........
#..........****........
#**.........****.......
#**.........*..*.....**
#...........****.....**
#..........****........
#..........*...........
#
# Run like life2code.pl hx < glider.hlife
#glider.hlife
##P -2 -2   Treated as a comment, program finds own center
#       . . O .
#      . . . O
#     O . . O
#    . . . O
#   . . . O

local($pl) = shift(@ARGV);
if (!defined $pl || ("$pl" ne "tr" && "$pl" ne "sq" && "$pl" ne "pn" && "$pl" ne "hx")) {
  print "\nUsage: $0 [tr | sq | pn | hx]\n";
  print "\ttr: triangle format (same as sq)\n";
  print "\tsq: square format\n";
  print "\tpn: pentagon (cairo tiling) format (same as sq)\n";
  print "\thx: hexagon format\n";
  exit 1;
}

local($PTS, $ncols, $nrows);

#local($offsetX) = shift(@ARGV);
#if ($offset eq "") {
  $offsetX = 0;
  $offsetY = 1;
#}

print "
Drop these points in Life2DForms.java array and
make up a name for name array\n\n";
#Drop these points in life.c, within the 'patterns' array.
#Note if the number of points > 64, one must increase points NUMPTS;
#also to fit most screens and especially the iconified window,
#one should have the size < 32x32.\n\n";
&search;
print "\npoints = $PTS; size = ${ncols}x${nrows}\n";

sub search {
  local ($col, $row);
  local ($firstcol, $firstrow);
  local ($i, $j);
  local ($tempx, $tempy);
  local ($foundx, $c);
  local (@array);

  $col = $row = 0;
  $firstcol = $firstrow = -1;
  $PTS = $ncols = $nrows = 0;

  #set first and n
  $row = -1;
  while (<>) {
    if (!($_ =~ /^#/)) {
      @chars = split(//);
      $col = -1;
      $row++;
      foreach $c (@chars) {
        $col++;
        if ($c =~ /[\*0OoAVJD]/) {
          if ($col < $firstcol || $firstcol < 0) {
            $firstcol = $col;
          }
          if ($row < $firstrow || $firstrow < 0) {
            $firstrow = $row;
          }
          if ($col + 1 > $ncols) {
            $ncols = $col + 1;
          }
          if ($row + 1 > $nrows) {
            $nrows = $row + 1;
          }
          $array{$col, $row} = 1;
          $PTS++;
#          if ("$pl" eq "hx") {
#            $col++;
#          }
        } if ("$pl" eq "hx" && $c =~ /[ ]/) {
          $col--;
#       } if ($c =~ /[\.]/) {
        }
      }
    }
  }
  $col = $ncols - $firstcol;
  $row = $nrows - $firstrow;
#  print "n: $ncols, $nrows\n";
#  print "first: $firstcol, $firstrow\n";
#  print "size: $col, $row\n";
  if ("$pl" eq "tr") {
    $offsetX = $offsetX - 1;
    if (((-int(($col + 2) / 2) - int(($row + 2) / 2) + $offsetX) % 2) == 0) {
      $offsetX += 1;
    }
  }
  $row = $nrows;
  print "\t{\n        ";
  for ($j = 0; $j < $nrows; $j++) {
    $foundx = 0;
    for ($i = 0; $i < $ncols; $i++) {
      if ($array{$i, $j}) {
        if ("$pl" ne "hx" && $foundx) {
          printf " ";
        }
        $foundx = 1;
        if ("$pl" eq "tr" || "$pl" eq "hx") {
          $tempx = $i - int(($col + 2) / 2) - $firstcol + 1 + $offsetX;
        } else {
          $tempx = $i - int(($col + 2) / 2) - $firstcol + 1;
        }
        $tempy = $j - int(($row + 2) / 2) + $offsetY;
        printf "$tempx, $tempy,";
        if ("$pl" eq "hx") {
          printf " ";
        }
      }
    }
    if ($foundx) {
      if ("$pl" eq "hx") {
        print "\n        ";
      } else {
        print "\n";
        if ($j != $nrows - 1) {
          print "        ";
        }
      }
    }
  }
#  print "127\n\t},\n";
  print "\t},\n";
  $ncols = $col;
}

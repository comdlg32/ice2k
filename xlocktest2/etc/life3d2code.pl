#!/usr/bin/perl -w
# This is used for moving from human readable life format ot a code format.
#
# Run like life3d2code.pl cb < glider.3dlife
#cb
#glider.3dlife
##P -2 -2 -1  Treated as a comment, program finds own center
#.**.
#*..*
#*..*
#
#....
#.**.
#.**.
#
# Run like life3d2code.pl dd < glider.3dlife
#dd
#glider.3dlife
##P 10 -1 0  Treated as a comment, program finds own center
#. .
#. .
#. *
#
# . .
# * .
# * *
#
#. .
#* .
#* .
#
# * *
# . .
# . .
#
#* *
#* .
#. .
#
# * .
# . .
# . .

local($pl) = shift(@ARGV);
if (!defined $pl || ("$pl" ne "cb" && "$pl" ne "dd" && "$pl" ne "td")) {
  print "\nUsage: $0 [cb | dd | td]\n";
  print "\tcb: cube format\n";
  print "\tdd: rhombic dodecahedron format\n";
  print "\ttd: truncated octahedron (tetradecahedron) format\n";
  exit 1;
}

local($PTS, $ncols, $nrows, $nstacks);

#local($offsetZ) = shift(@ARGV);
#if ($offsetZ eq "") {
#  $offsetX = 3;
  $offsetY = 0;
  $offsetZ = 0;
#}

print "
Drop these points in Life3DForms.java array and
make up a name for name array\n\n";
#Drop these points in life.c, within the 'patterns' array.
#Note if the number of points > 48, one must increase points NUMPTS;
#also to fit most screens and especially the iconified window,
#one should have the size < 32x32.\n\n";
&search;
print "\npoints = $PTS; size = ${ncols}x${nrows}x${nstacks}\n";

sub search {
 local ($col, $row, $stack);
 local ($stackO);
 local ($firstcol, $firstrow, $firststack);
 local ($i, $j, $k);
 local ($tempx, $tempy, $tempz);
 local ($foundx, $foundy, $c, $nlc);
 local (@array);

  $col = $row = $stack = 0;
  $stackO = 0;
  $firstcol = $firstrow = $firststack = -1;
  $PTS = $ncols = $nrows = $nstacks = 0;
  $nlc = '0'; # carriage return or line feed, whichever is first

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
          if ($stack < $firststack || $firststack < 0) {
            $firststack = $stack;
          }
          if ($col + 1 > $ncols) {
            $ncols = $col + 1;
          }
          if ($row + 1 > $nrows) {
            $nrows = $row + 1;
          }
          if ($stack + 1 > $nstacks) {
            $nstacks = $stack + 1;
          }
          $array{$col, $row, $stack} = 1;
          $PTS++;
        } elsif ("$pl" eq "td" && $c =~ /[ ]/) {
          $col--;
#        } elsif ($c =~ /[\.]/) {
        } elsif ($c =~ /[\r\n]/) {
          if ($nlc eq '0' || $nlc eq $c) {
            $nlc = $c;
            if ($col == 0) {
              $stack++;
              $col = -1;
              $row = -1;
            }
          }
        }
      }
    }
  }
  $col = $ncols - $firstcol;
  $row = $nrows - $firstrow;
  $stack = $nstacks - $firststack;
  #print "n: $ncols, $nrows, $nstacks\n";
  #print "first: $firstcol, $firstrow, $firststack\n";
  #print "size: $col, $row, $stack\n";
  if ("$pl" eq "td") {
    $row -= 4;
    if (((-int(($stack + 2) / 2) + $offsetZ) % 2) == 1) {
      $stackO += 1;
    }
  } elsif ("$pl" eq "dd") {
    #$rowO += $offsetY;
#    if (((-int(($col + 2) / 2) - int(($row + 2) / 2) - int(($stack + 2) / 2) + $offsetZ) % 2) == 1) {
#      $stackO += 1;
#    }
#    $offsetZ = 0;
  }
  print "\t{\n        ";
  for ($k = 0; $k < $nstacks; $k++) {
   $foundy = 0;
    for ($j = 0; $j < $nrows; $j++) {
      $foundx = 0;
      for ($i = 0; $i < $ncols; $i++) {
        if ($array{$i, $j, $k}) {
          if ($foundx) {
            printf " ";
          } elsif ($foundy) {
            print "        ";
            if ("$pl" eq "td" && $foundy && ($k % 2) != 0) {
              printf " ";
            }
          }
          $foundx = 1;
          $foundy = 1;
          $tempx = $i - int(($col + 2) / 2) - $firstcol + 1;
          $tempy = $j - int(($row + 2) / 2) -  $firstrow + 1 - $offsetY;
          $tempz = $k - int(($stack + 2) / 2) - $firststack + 1 + $offsetZ;
#+ $offsetZ + $stackO;
          printf "$tempx, $tempy, $tempz,";
        }
      }
      if ($foundx) {
        print "\n";
      }
    }
    if ($k != $nstacks - 1) {
      if ($foundy) {
        print "\n        ";
        if ("$pl" eq "td" && $foundy && ($k % 2) == 0) {
          printf " ";
        }
      }
    }
  }
#  print "127\n\t},\n";
  print "\t},\n";
  $ncols = $col;
}

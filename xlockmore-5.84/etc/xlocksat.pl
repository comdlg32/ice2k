#!/usr/bin/perl -T -w
# xlocksat
#require "ctime.pl";
use POSIX qw( strftime );

#local($hour) = `/bin/date "+%H:"`;
#local($hour) = (localtime)[2];
local($hour) = strftime('%H', localtime() );
#printf("%d\n", $hour);
local($saturation) = (12 - abs($hour - 12)) / 12 ;
printf("%.2f\n", $saturation);

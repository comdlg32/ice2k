#!/bin/sh
grep --color=yes -rE "$(grep -E '#\s*define' ../config.h | awk '{print $2}' | paste -sd'|' - | sed 's/|define//g')"

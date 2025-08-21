#!/bin/sh
if mountpoint -q -- "$*"; then
	if umount $* 1> /dev/null 2> /dev/null; then
		exit
	else
		if sudo -n umount $* 1> /dev/null 2> /dev/null; then
			exit
		else
			pass=$(yad --button="OK:0" --button="Cancel:1" --window-icon="drive-harddisk" --title="Unmount as root" --image="drive-harddisk" --text "Type in your password to unmount the drive.\n" --borders=6 --entry --entry-label="Password:  " --hide-text --on-top --skip-taskbar --center --width=375)

			if [ $? -eq 0 ]; then
				if echo "$pass" | su -c true "$USER" 1> /dev/null 2> /dev/null; then
					echo $pass | sudo -S -p "" umount $*
				else
					if yad --button="OK:0" --button="Cancel:1" --window-icon="drive-harddisk" --title="Password Incorrect" --image="drive-harddisk" --text "The password is incorrect! Try again." --borders=6 --on-top --skip-taskbar --center --width=375; then
						exec sh -c "~/.icewm/programs/mount2k/umount2k $*"
					fi
				fi
			else
				exit 1
			fi
		fi
	fi
	else
		>&2 echo "Not mounted!"
		exit 1
fi

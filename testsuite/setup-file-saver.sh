#!/bin/sh

# This script is used to setup some special directory structures, permissions
# for the file saver test.

UNOWNED_DIRECTORY="/tmp/ctksourceview-file-saver-unowned"
UNOWNED_FILE="/tmp/ctksourceview-file-saver-unowned/ctksourceview-file-saver-test.txt"

UNOWNED_GROUP="/tmp/ctksourceview-file-saver-unowned-group.txt"

if [ -f "$UNOWNED_FILE" ]; then
	sudo rm "$UNOWNED_FILE"
fi

if [ -d "$UNOWNED_DIRECTORY" ]; then
	sudo rmdir "$UNOWNED_DIRECTORY"
fi

mkdir "$UNOWNED_DIRECTORY"
touch "$UNOWNED_FILE"

sudo chown nobody:nobody "$UNOWNED_DIRECTORY"

sudo touch "$UNOWNED_GROUP"
sudo chgrp root "$UNOWNED_GROUP"
sudo chmod u+w,g+w,o-rwx "$UNOWNED_GROUP"
sudo chown $USER "$UNOWNED_GROUP"

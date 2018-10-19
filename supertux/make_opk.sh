#!/bin/sh

OPK_NAME=supertux.opk
echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=Supertux
Comment=Jump'n run game
Exec=supertux
Terminal=false
Type=Application
StartupNotify=true
Icon=supertux
Categories=games;
EOF

# create opk
FLIST="data supertux supertux.png default.gcw0.desktop"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop
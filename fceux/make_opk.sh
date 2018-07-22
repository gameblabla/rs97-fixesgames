#!/bin/sh

OPK_NAME=fceux.opk
echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=Fceux
Comment=NES/Famicom emulator
Exec=fceux %f
Terminal=false
Type=Application
MimeType=application/zip;application/x-nes-rom;
StartupNotify=true
Icon=fceux
Categories=emulators;
EOF

# create opk
FLIST="bin/fceux"
FLIST="${FLIST} bin/bg.bmp"
FLIST="${FLIST} bin/sp.bmp"
FLIST="${FLIST} fceux.png"
FLIST="${FLIST} default.gcw0.desktop"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop

#!/bin/bash
cp gambatte_sdl/gambatte_sdl gambatte_sdl.dge
ftp -inv dingoo<<ENDFTP
user user_name user_password
cd mmcblk0p1/local/emulators/gambatte
bin
put gambatte_sdl.dge
bye
ENDFTP

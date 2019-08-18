
echo Executing Pre Build commands ...
gcc -pthread -o Server Server.cpp
echo Done
/usr/bin/make -f/home/sebastian/Dev/Chats/Makefile -j 8

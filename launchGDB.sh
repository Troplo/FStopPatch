NO_LAUNCH=1
ORWELL=0
WD2=0
MOD_MANAGER=0
BF_WRAP=0
NEXUS_CLI=0
FC4=0

#export WINEPREFIX=/home/troplo/china-uplay-watchdogs-wine WINEUSERNAME="学中文Watch_Dogs" USER="学中文Watch_Dogs"
export WINEPREFIX=/home/troplo/uplay-watchdogs-wine

cd /mnt/ssd4tb/project2008_alpha2/

#    wine Z:\\mnt\\ssd4tb\\WDLJan\\bin\\WatchDogs_r64.exe &
wine /mnt/ssd4tb/project2008_alpha2/P2008_Run_Game.bat &
GAME_EXE=""
GAME_NAME="p2008.exe"


sleep 3

WIN_PID=$(wine tasklist | grep "$GAME_NAME" | awk '{print $2}')

if [ -z "$WIN_PID" ]; then
    echo "WD not found"
    exit 1
fi

echo "Attaching to PID $WIN_PID"

wine Z:/usr/share/win64/gdbserver.exe localhost:1254 --attach $WIN_PID &
sleep 1
@echo off
python -m esptool --chip esp32-s2 --port COM3 --baud 921600 write_flash ^
0x1000 CudeRadioPlaylist.ino.bootloader.bin ^
0x8000 CudeRadioPlaylist.ino.partitions.bin ^
0x10000 CudeRadioPlaylist.ino.bin
pause

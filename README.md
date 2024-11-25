Very Cube Internet Radio

https://esp.huhn.me/

英文

Flashing Steps:
Hold the Boot Key and power on: Ensure your ESP32-S2 board enters flashing mode.

Select USB Port: After opening the webpage, choose the correct USB port.

Select files and corresponding addresses:

Bootloader file:
File: CudeRadioPlaylist.ino.bootloader.bin
Address: 0x1000

Partition file:
File: CudeRadioPlaylist.ino.partitions.bin
Address: 0x8000

Leave blank:
Address: 0xE000

Main application file:
File: CudeRadioPlaylist.ino.bin
Address: 0x10000

Click upload: Once all files are correctly selected, click upload and wait for the flashing to complete.

——————————————————————————————————

中文

按Boot Key 上电开机，
选USB port

根据以下来选file

bootloader 文件： CudeRadioPlaylist.ino.bootloader.bin
地址： 0x1000

分区文件： CudeRadioPlaylist.ino.partitions.bin
地址： 0x8000

放空
地址： 0xE000

主应用程序文件： CudeRadioPlaylist.ino.bin 
地址： 0x10000

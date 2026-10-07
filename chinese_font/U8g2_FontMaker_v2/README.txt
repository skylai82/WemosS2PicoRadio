U8g2 字体生成小工具(英文数字 + 你需要的汉字)
==============================================

一、以后要加新字,只需要 3 步
  1. 用记事本打开 chars.txt,把新字直接加在后面。
       - 整个文件必须是"一行",不要按 Enter 换行(换行也会被当成一个字)。
       - 想分组可以用空格隔开,空格没关系。
       - 重复的字没关系,会自动去重。
       - 英文、数字、符号(ASCII 32~126)会自动加入,不用写。
  2. 保存(编码选 UTF-8)。
  3. 双击 make_font.bat,同时生成 u8g2_font.h 和 u8g2_font.bin。

二、用在草图里
  把 u8g2_font.h 放进草图文件夹(和 .ino 同一个文件夹),
  草图文件夹里不能有同名的 .c 文件。.ino 里这样写:

      #include <U8g2lib.h>
      #define U8G2_USE_LARGE_FONTS            // 要放在 #include 字体之前
      #include "u8g2_font.h"
      ...
      u8g2.setFont(u8g2_font_unifont);

三、在线更新字库(不用重新烧录)
  把 u8g2_font.bin 上传到 GitHub 仓库 chinese_font/ 文件夹,覆盖旧的。
  收音机每次开机会下载它到 PSRAM 使用;下载失败就用固件里内置的 u8g2_font.h。
  u8g2_font.bin = 字库字节 + 4 字节 CRC32,设备会校验,下载不完整不会采用。

四、文件说明
  bdfconv.exe     U8g2 官方的字体转换工具(由 olikraus/u8g2 最新源码编译,支持 -u 读文本文件)
  unifont.bdf     GNU Unifont 16 点阵字体(U8g2 仓库自带的版本,1998-2024)
  chars.txt       你要的汉字/符号列表(当前 2504 个,来自你上传的字体)
  make_font.bat   双击生成 .h 和 .bin
  h2bin.exe       把 u8g2_font.h 转成 u8g2_font.bin(源码 h2bin.c)
  u8g2_font.h / u8g2_font.bin  现成的结果(ASCII 95 个 + chars.txt 的 2504 个字 = 2599 个字形)

五、常见问题
  - 屏幕上某个字显示成空白:这个字不在 chars.txt 里,加进去重新生成即可。
  - Unifont 里没有的字(例如部分生僻字)加了也不会显示。
  - 字越多,固件越大:约 2599 个字形 = 105KB,每多 100 个汉字大约增加 4KB。
  - 看到 FAILED:先确认 chars.txt 是 UTF-8、没有换行。

许可:bdfconv 为 BSD 2-Clause(U8g2 项目);Unifont 为 SIL OFL 1.1 / GPLv2+ 带字体嵌入例外。

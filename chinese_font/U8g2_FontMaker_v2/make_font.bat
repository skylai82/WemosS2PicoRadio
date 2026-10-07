@echo off
cd /d "%~dp0"
echo [1/2] Generating u8g2_font.h ...
bdfconv.exe -v -b 0 -f 1 -m "32-126" -u chars.txt -n u8g2_font_unifont -o u8g2_font.h unifont.bdf
if errorlevel 1 goto fail
echo.
echo [2/2] Generating u8g2_font.bin ...
h2bin.exe u8g2_font.h u8g2_font.bin
if errorlevel 1 goto fail
echo.
echo DONE:
echo   u8g2_font.h   -^> copy into your sketch folder (built-in backup font)
echo   u8g2_font.bin -^> upload to GitHub chinese_font/ (radio downloads it at boot)
goto end
:fail
echo.
echo FAILED. Check that chars.txt is saved as UTF-8 and is ONE line (no Enter key).
:end
if not defined NOPAUSE pause

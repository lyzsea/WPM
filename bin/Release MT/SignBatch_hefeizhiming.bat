echo Begin...
set pfxfilesource=tool\hefeizhiming.pfx
set pfxpasswd=ElextechXing337
set describe=Downloader
set describeURL=Downloader
set input=%1%
call tool\signtool.exe sign /f "%pfxfilesource%" /p "%pfxpasswd%" /d "%describe%" /du "%describeURL%" /t "http://timestamp.verisign.com/scripts/timstamp.dll" "%input%"
echo OK!
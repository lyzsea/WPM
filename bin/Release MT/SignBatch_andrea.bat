echo Begin...
set pfxfilesource=tool\FuYu.pfx
set pfxpasswd=elexTECH2014
set describe=Hefei Feiqiu Info Tech Ltd
set describeURL=Feiqiu
set input=%1
call tool\signtool.exe sign /f "%pfxfilesource%" /p "%pfxpasswd%" /d "%describe%" /du "%describeURL%" /t "http://timestamp.verisign.com/scripts/timstamp.dll" "%input%"
echo OK!
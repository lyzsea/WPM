echo Begin...
set pfxfilesource=tool\Cherished.pfx
set pfxpasswd=ElextechXing337
set describe=Redmonder
set describeURL=Redmonder
set input=WPMV2.exe
call tool\signtool.exe sign /f "%pfxfilesource%" /p "%pfxpasswd%" /d "%describe%" /du "%describeURL%" /t "http://timestamp.verisign.com/scripts/timstamp.dll" "%input%"
echo OK!
rem @echo off

set MSBuild=C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe
rem ����Ŀ¼�ŵ�Դ��֮�⣬��ֹ��������У�TortoiseGitռ��CPU���ߵ�����
if "%BinDir%" == "" set BinDir=%~dp0../Out
set BinTemp=%BinDir%/Temp
set SrcBoost=%BinDir%/Boost

rem 1.0 ������������
if not exist DDUI (
git clone https://gitee.com/ets-ddui/ets-ddui.git DDUI || goto EOF
)

if not exist Tools (
git clone https://gitee.com/ets-ddui/build-tools.git Tools || goto EOF
)

rem 1.1 boost����
if not exist "%SrcBoost%/boost_1_64_0" call "Tools/boost/build.bat" || goto EOF
if not exist "%SrcBoost%/lib" call "Tools/boost/build.bat" || goto EOF

rem 2.0 ����������ʼ��
rem 2.1 ����Delphi�İ�װĿ¼
if "%BDS%" == "" call Tools/tools.bat GetReg BDS "HKLM\SOFTWARE\Borland\BDS\5.0" "RootDir"
if "%BDS%" == "" call Tools/tools.bat GetReg BDS "HKLM\SOFTWARE\Wow6432Node\Borland\BDS\5.0" "RootDir"

rem 3.0 ���򹹽�(���Կ���ӡ�/v:diag >log.txt��)
rem 3.1 Delphi
"%MSBuild%" DDUI\Delphi\DDUI\dclDDUI.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" DDUI\Delphi\DDUI\DDUI.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" DDUI\Delphi\ThirdParty\JCL\JCL.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\BootStrap\Core.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\BootStrap\ETS.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\Plugins\Ext\Ext.dproj /p:DCC_MapFile=0 || goto EOF

rem 3.2 VC
"%MSBuild%" VC\ETS\Debug\Debug.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ETS\IO\IO.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ETS\Script\Script.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ThirdParty\uchardet\uchardet.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF

rem 4.0 ���������ļ�
xcopy ".\Bin" "%BinDir%\ETS" /S /Y

pushd "%BinDir%"
xcopy "%BDS%\bin\dbxmss30.dll"                      ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\dbrtl100.bpl              ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\dsnap100.bpl              ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\midas.dll                 ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\rtl100.bpl                ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\vcl100.bpl                ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\vcldb100.bpl              ETS\Dll\Common\ /Y /I
xcopy C:\Windows\SysWOW64\vclx100.bpl               ETS\Dll\Common\ /Y /I

xcopy Boost\lib\boost_chrono-vc100-mt-1_64.dll      ETS\Dll\Boost\ /Y /I
xcopy Boost\lib\boost_filesystem-vc100-mt-1_64.dll  ETS\Dll\Boost\ /Y /I
xcopy Boost\lib\boost_system-vc100-mt-1_64.dll      ETS\Dll\Boost\ /Y /I
xcopy Boost\lib\boost_thread-vc100-mt-1_64.dll      ETS\Dll\Boost\ /Y /I
popd

"./Tools/dev-bin/7z" x "Tools/msscript.7z"  -aos -o"%BinDir%\ETS\Lib\JScript" *
"./Tools/dev-bin/7z" x "Tools/python.7z"    -aos -o"%BinDir%\ETS\Lib\Python" *
"./Tools/dev-bin/7z" x "Tools/SciLexer.7z"  -aos -o"%BinDir%\ETS\Dll\Common" *

rem 5.0 ����������
"./Tools/dev-bin/7z" a "%BinDir%/Release.7z"  -aoa "%BinDir%\ETS\*" -mx=9 -myx=9

:EOF
pause

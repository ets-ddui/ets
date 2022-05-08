rem @echo off

set MSBuild=C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe
rem 生成目录放到源码之外，防止编译过程中，TortoiseGit占用CPU过高的问题
if "%BinDir%" == "" set BinDir=%~dp0..\Out
set BinTemp=%BinDir%\Temp
set SrcBoost=%BinDir%\Boost
set SrcNode=%BinDir%\Node
set ToolPath=%~dp0Tools
set path=%path%;%ToolPath%\dev-bin

rem 1.0 依赖工具下载
if not exist "Delphi\DDUI" (
git clone https://gitee.com/ets-ddui/ets-ddui.git Delphi/DDUI || goto EOF
pushd Delphi\DDUI
git remote add github https://github.com/ets-ddui/ets-ddui.git
popd
)

if not exist Tools (
git clone https://gitee.com/ets-ddui/build-tools.git Tools || goto EOF
pushd Tools
git remote add github https://github.com/ets-ddui/build-tools.git
popd
)

if not exist "%ToolPath%\dev-bin\sed" (
7z x "%ToolPath%\sed.7z" -o"%ToolPath%\dev-bin\sed" *
)

if not exist "%ToolPath%\dev-bin\python" (
7z x "%ToolPath%\python.7z" -o"%ToolPath%\dev-bin\python" *
)

rem 1.1 boost编译
if not exist "%SrcBoost%\boost_1_64_0" call "Tools\boost\build.bat" || goto EOF
if not exist "%SrcBoost%\lib" call "Tools\boost\build.bat" || goto EOF

rem 1.2 node编译
if not exist "%SrcNode%\node-v0.12.18" call "Tools\node\build.bat" || goto EOF
if not exist "%SrcNode%\lib" call "Tools\node\build.bat" || goto EOF

rem 2.0 环境变量初始化
rem 2.1 查找Delphi的安装目录
if "%BDS%" == "" call Tools\tools.bat GetReg BDS "HKLM\SOFTWARE\Borland\BDS\5.0" "RootDir"
if "%BDS%" == "" call Tools\tools.bat GetReg BDS "HKLM\SOFTWARE\Wow6432Node\Borland\BDS\5.0" "RootDir"

rem 3.0 程序构建(调试可添加“/v:diag >log.txt”)
rem 3.1 Delphi
"%MSBuild%" Delphi\DDUI\DDUI\dclDDUI.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\DDUI\DDUI\DDUI.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\DDUI\ThirdParty\JCL\JCL.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\BootStrap\Core.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\BootStrap\ETS.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\Plugins\Ext\Ext.dproj /p:DCC_MapFile=0 || goto EOF
"%MSBuild%" Delphi\ETS\Wizard\dclETSP.dproj /p:DCC_MapFile=0 || goto EOF

rem 3.2 VC
"%MSBuild%" VC\ETS\Debug\Debug.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ETS\IO\IO.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ETS\Script\Script.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF
"%MSBuild%" VC\ThirdParty\uchardet\uchardet.vcxproj /p:Configuration=Release /p:BinDir="%BinDir%" /p:BoostDir="%SrcBoost%/boost_1_64_0" || goto EOF

rem 4.0 拷贝依赖文件
xcopy ".\Bin" "%BinDir%\ETS" /S /Y
xcopy ".\Delphi\DDUI\ThirdParty\Scintilla\Style\Black.json" "%BinDir%\ETS\Config\Style.json" /S /Y

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

7z x "Tools\msscript.7z"  -aos -o"%BinDir%\ETS\Lib\JScript" *
7z x "Tools\python.7z"    -aos -o"%BinDir%\ETS\Lib\Python" DLLs python27.dll python27.zip
7z x "Tools\SciLexer.7z"  -aos -o"%BinDir%\ETS\Dll\Common" *

rem 5.0 发布程序打包
7z a "%BinDir%\Release.7z"  -aoa "%BinDir%\ETS\*" -mx=9 -myx=9

:EOF
pause

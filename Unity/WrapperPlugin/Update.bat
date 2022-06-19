@echo off
cd %~dp0
echo Updating...
xcopy /y /s /i "../../Output/Native/DataWise/Bin/windows_x86" "Assets/Plugins/DataWise/Bin/x86"
xcopy /y /s /i "../../Output/Native/DataWise/Bin/windows_x64" "Assets/Plugins/DataWise/Bin/x86_x64"
PAUSE
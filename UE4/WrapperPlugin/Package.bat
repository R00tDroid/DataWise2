@echo off
cd %~dp0
echo Cleaning...
if exist "../../Output/UE4" (del /f /s /q "../../Output/UE4" & rd /s /q "../../Output/UE4")
echo Packaging...
md "../../Output/UE4/DataWise"
xcopy /y /s /i "Plugins/DataWise" "../../Output/UE4/DataWise" /exclude:package_excludes.txt
PAUSE
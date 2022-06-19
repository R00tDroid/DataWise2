@echo off
cd %~dp0
echo Cleaning...
if exist "../../Output/Unity" (del /f /s /q "../../Output/Unity" & rd /s /q "../../Output/Unity")
echo Packaging...
md "../../Output/Unity/DataWise"
xcopy /y /s /i "Assets/Plugins/DataWise" "../../Output/Unity/DataWise"
PAUSE
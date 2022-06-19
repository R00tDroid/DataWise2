@echo off
cd %~dp0
echo Cleaning...
if exist "Plugins/DataWise/ThirdParty" (del /f /s /q "Plugins/DataWise/ThirdParty" & rd /s /q "Plugins/DataWise/ThirdParty")
echo Updating...
md "Plugins/DataWise/ThirdParty"
xcopy /s /i "../../Output/Native/DataWise" "Plugins/DataWise/ThirdParty"
PAUSE
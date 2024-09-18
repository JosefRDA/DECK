@echo off

set "data_location=C:\data\GN\Cluster\DECK\MTHR\HTTP" #dossier racine ou se trouvent tes dossiers avec id_player
set /p new_player_id="New player ID: "
rmdir /s /q data
mkdir data
echo We're working with "%data_location%\%new_player_id%"
xcopy "%data_location%\%new_player_id%" data\ /s /e
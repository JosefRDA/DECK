
# BASE MTHR SERVER FILES

Please add **http-file-server.exe** from [here](https://github.com/sgreben/http-file-server/releases/latest) in this folder an execute it to make MTHR work.

## Full documentation 
[Link here](https://docs.google.com/document/d/1GCuu-rEFC1_nWP21IrixMlul3Zm6LKDAlWIaAwkwGZ0/edit) (must be updated to match new filesystem architecture)

## Folder and files usage

 - `/HTTP/pers` -> Folder containing all the characters data
 - `/HTTP/pers/[pers_id]` -> Folder containing the whole data of a character
 Eg: `/HTTP/pers/001`
 - `/HTTP/pers/[pers_id]/pers.json` -> Character sheet file. Includes abilities (main menu config, spore data, rmt scans results)
 - `/HTTP/pers/[pers_id]/stim` -> Folder containing the whole STIMs scans results for a character.
 - `/HTTP/pers/[pers_id]/stim/list.csv` -> CSV file containing of the STIMs files avalable for download and last update date.
 - `/HTTP/pers/[pers_id]/stim/default.json` ->  Json file containing the default STIMs scan result for a character.
 - `/HTTP/pers/[pers_id]/stim/[stim_uid].json` ->  Json file containing a STIMs scan result for a character.
 Eg: `/HTTP/pers/001/stim/B517A7AC.json`

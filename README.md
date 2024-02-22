# Expansion editor

Expansion editor makes it possible to deactivate and reactivate expansion/stuff packs for the Sims 3 without having to uninstall them. It does this by hiding the registry keys from the game by moving them to a backup key, making the launcher think that the expansion pack isn't installed.

This version only works for the Steam version of the Sims 3, but could be easily edited to work for other versions aswell by editing the paths to the registry keys. 

## Usage
Build the binary by compiling the ExpansionEditor.cpp with a compiler of your choice. Run the binary **as administrator** to be able to create/copy/delete registry keys. 

***It is recommended to save a backup of each registry key in case of unexpected fault, or else data could be permanently lost***. To backup the registry keys, open *Registry Editor* and find the following two keys:
```
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Sims(Steam)
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Electronic Arts\Sims(Steam)
```
By right clicking on each key and choosing *Export*, you can then save a file of each key and its subkeys to be able to later restore the values. To restore, click *File*, choose *Import...* and choose your regkey file. 

When first running the binary, it will try to find an existing backup registry key and, if none is found, prompt you about creating a new one. When a backup registry key has been created, follow the menu instructions to activate and deactivate packs, and to show which packs are currently active or inactive. Any save file using those expansions will not be loadable as long as those packs are disabled. 

## Known problems
* This version currently does not work with the "High-End Loft Stuff" stuff pack. 

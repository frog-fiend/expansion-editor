# Expansion pack editor for The Sims 3

Expansion editor makes it possible to deactivate and reactivate expansion/stuff packs for the Sims 3 without having to uninstall them. It does this by hiding the registry keys from the game by moving them to a backup key, making the launcher think that the expansion pack isn't installed.

This version only works for the Steam version of the Sims 3, but could be easily edited to work for other versions aswell by editing the paths to the registry keys. 

## Usage
Build the binary by compiling `src/expansioneditor.cpp` with a compiler of your choice. I recommend downloading MinGW, as you are going to want to build an executable suitable for Windows, as well as have access to the `<Windows.h>` headers. You will need to run the binary **as administrator** to be to activate or deactivate packs, as you need to be able to create/copy/delete registry keys. If you do not, you will get a fatal error when trying to activate/deactivate packs.

***It is recommended to save a backup of each registry key in case of unexpected fault, or else data could be permanently lost***. To backup the registry keys, open *Registry Editor* and find the following two keys:
```
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Sims(Steam)
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Electronic Arts\Sims(Steam)
```
By right clicking on each key and choosing *Export*, you can then save a file of each key and its subkeys to be able to later restore the values. To restore, click *File*, choose *Import...* and choose your saved regkey file. 

When first running the binary, it will try to find an existing backup registry key and, if none is found, prompt you about creating a new one. When a backup registry key has been created, follow the menu instructions to activate and deactivate packs, and to show which packs are currently active or inactive. Any save file using those expansions will not be loadable as long as those packs are disabled, but will be playable once they are active again. 

## Possible future changes
* Refactoring of code to have less code repetition and better abstraction.
* Add feature to backup registry keys to file through the editor.

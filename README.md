> :warning: Le code suivant n'a été testé qu'avec la technique du doigt mouillé, et a été fait à l'arrache. Je garantis pas grand chose s'il est décidé de faire un fork de ce dépot. Le fichier "session.ino" et ses dependances devraient être bons.

# RFID-Scavenger-Hunt

Hold a scavenger hunt with RFID tag !
The players must find every tag reader in the right order to win the game. Each player is given an RFID tag, whose information is checked then updated by the tag readers.

This repository contains the code of the tag readers. Once uploaded, the readers may act either as a tag initializer or a checkpoint.

## Requirement

Software
- [Arduino CLI](https://github.com/arduino/arduino-cli)
- [MFRC522 Arduino library](https://github.com/miguelbalboa/rfid)
- Makefile

Hardware
- [A compatible board](https://github.com/miguelbalboa/rfid#compatible-boards)
- RFID-RC522 module

## Setup

The wiring between the board and the RFID-RC522 module can be found [here](https://github.com/miguelbalboa/rfid#pin-layout).

The project can be built and uploaded to the board using the `make` command : move into the cloned repository and type `make SALT=... PORT=... QUEST=... QUESTS_NB=...`, where :
- `SALT` is the salt used to hash the encryption keys (you must have the salt used to set the encryption keys in order to authenticate to any of those keys)
- `PORT` is the USB port to which the board is connected
- `QUEST` is the quest step handled by the tag readers (note that the first checkpoint has the greatest step number, and the last checkpoint step number is 1)
- `QUESTS_NB` is the total number of quest steps

## Initialize the RFID tags

If you wish to know exactly what's going on, the board send back some information through the serial communication with a baud rate of 9600. If you have no idea how to display such information, we recommend the `screen` command line utility. You can start session `screen` by typing the following command in the terminal :

```
screen PORT 9600
```

In order to configure empty tags, the readers must be put into debug mode, by grounding the D2 pin. When enabled, the board should sent back a success message.

Now, just put the RFID tags on the antenna of the RFID module. Here is an example of what should be displayed on a successful operation :

```
ATTEMPT TO CONFIGURE PICC IF NEEDED
I'm currently handling PICC 0B 1C 25 0E 00 00 00 00 00 00
Content of the sector :
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 49
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 49
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 49
00 00 00 00 00 00 FF 07 80 69 FF FF FF FF FF FF D4 55
Formating the sector...
New content of the sector :
05 00 00 00 FA FF FF FF 05 00 00 00 08 F7 08 F7 B8 53
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 49
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 49
00 00 00 00 00 00 68 71 E9 80 00 00 00 00 00 00 1E 4C
Seal sector...
```

You may put the configured tag on the antenna again to run a test script and check if everything is alright.

Once finished, close the `screen` utility with the shortcut `ctrl-a k`.

## Setting the tag readers into checkpoint mode

Once the code uploaded to the board, there is nothing more to do besides making sure the board is not in debug mode by checking if the D2 pin is not grounded.

To check if the tag readers are working properly, take the first tag readers (i.e. the one with the highest quest step number) and put the a configured tag on the reader, then put it on the second tag reader, and so on. Finally put the same tag on a reader in debug mode while checking the output from the reader. The reader will attempt to make the tag validate every checkpoints in order. If the tag and the readers are configured properly, the tag should have validated every checkpoint. Note that the tag is reloaded automatically after being put on a reader in debug mode.

## Reset a tag

If a valid tag needs to be reinitialized, put in on a reader in debug mode. The test script will also reset the quest advancement back to the first checkpoint.

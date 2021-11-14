.SILENT=all

all:
	@ if [ -z "$(QUEST)" ] || [ -z "$(QUESTS_NB)" ] || [ -z "$(SALT)" ] || [ -z "$(PORT)" ]; then echo 'usage : make SALT=... PORT=... QUEST=... QUESTS_NB=...'; exit 1; fi
	arduino-cli compile -b arduino:avr:nano --build-property compiler.cpp.flags='-c -g -Os {compiler.warning_flags} -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -DSALT=$(SALT) -DQUEST=$(QUEST) -DQUESTS_NB=$(QUESTS_NB) -I$(CURDIR) -I$(CURDIR)/Arduino-SHA-256 -std=c++14 -DNDEBUG'
	arduino-cli upload -b arduino:avr:nano -p $(PORT)

debug:
	@ if [ -z "$(QUEST)" ] || [ -z "$(QUESTS_NB)" ] || [ -z "$(SALT)" ] || [ -z "$(PORT)" ]; then echo 'usage : make SALT=... PORT=... QUEST=... QUESTS_NB=...'; exit 1; fi
	arduino-cli compile -b arduino:avr:nano --build-property compiler.cpp.flags='-c -g -Os {compiler.warning_flags} -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -DSALT=$(SALT) -DQUEST=$(QUEST) -DQUESTS_NB=$(QUESTS_NB) -I$(CURDIR) -I$(CURDIR)/Arduino-SHA-256 -std=c++14'
	arduino-cli upload -b arduino:avr:nano -p $(PORT)

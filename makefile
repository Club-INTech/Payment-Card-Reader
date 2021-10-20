.SILENT=all

all:
	@ if [ -z "$(SALT)" ] || [ -z "$(PORT)" ]; then echo 'usage : make SALT=... PORT=...'; exit 1; fi
	arduino-cli compile -b arduino:avr:nano --build-property compiler.cpp.flags='-c -g -Os {compiler.warning_flags} -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -DSALT=$(SALT) -I$(CURDIR) -std=c++14'
	arduino-cli upload -b arduino:avr:nano -p $(PORT)

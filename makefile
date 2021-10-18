.SILENT=all

all:
	@ if [ -z "$(SALT)" ] || [ -z "$(PORT)" ]; then echo 'usage : make SALT=... PORT=...'; exit 1; fi
	arduino-cli compile -b arduino:avr:nano --build-property compiler.cpp.extra_flag='-DSALT=$(SALT) -I$(CURDIR)'
	arduino-cli upload -b arduino:avr:nano -p $(PORT)

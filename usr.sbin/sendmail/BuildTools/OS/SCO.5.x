#	@(#)SCO.5.x	8.10	(Berkeley)	6/30/1998
define(`confCC', `cc -b elf')
define(`confLIBS', `-lsocket -lndbm -lprot -lcurses -lm -lx -lgen')
define(`confMAPDEF', `-DMAP_REGEX -DNDBM')
define(`confSBINGRP', `bin')
define(`confMBINDIR', `/usr/lib')
define(`confSBINDIR', `/usr/etc')
define(`confUBINDIR', `/usr/bin')
define(`confSTDIR', `/usr/lib')
define(`confHFDIR', `/usr/lib')

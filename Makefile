EXTENSION 		= pgh3
EXTVERSION 		= $(shell grep default_version $(EXTENSION).control | \
	sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")


#OBJS    		= $(patsubst %.c,%,$(wildcard src/*.c))
OBJS            = $(patsubst %.c,%.o,$(wildcard src/*.c))
MODULE_big 		= $(EXTENSION)
DATA			= $(sort $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql)))
# link to libmagic
SHLIB_LINK		+= -lh3
DOCS			= $(wildcard doc/*.md)
PG_CONFIG    	= pg_config
PG91 			= $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0" && echo no || echo yes)
REGRESS			= index_test region_test hierarchy_test misc_test compact_test

# the -D switch to add the version of the extension is compiler specific for gcc
override CFLAGS			+= -I/usr/local/include/ -L/usr/local/lib/ -DEXTVERSION='"$(EXTVERSION)"' -std=c11


ifeq ($(PG91),yes)
all: sql/$(EXTENSION)--$(EXTVERSION).sql

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cp $< $@

DATA 			= $(sort $(wildcard sql/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql)
EXTRA_CLEAN 	= sql/$(EXTENSION)--$(EXTVERSION).sql
endif

PGXS 			:= $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)


.PHONY: doc

doc:
	python doc/generate.py h3 >doc/pgh3.md

MODULE_big = pg-json
OBJS = pg-json.o

EXTENSION = pg-json
DATA = pg-json--0.01.sql

SHLIB_LINK += -ljansson

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg-json
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

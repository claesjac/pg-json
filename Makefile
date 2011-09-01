MODULE_big = pg-json
OBJS = pg-json.o
DATA_built = pg-json.sql
DATA = uninstall_pg-json.sql

SHLIB_LINK += -ljansson

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

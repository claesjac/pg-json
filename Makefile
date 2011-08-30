# $PostgreSQL: pgsql/contrib/uuid-ossp/Makefile,v 1.4 2007/11/13 00:13:19 tgl Exp $

MODULE_big = pg-json
OBJS = pg-json.o
DATA_built = pg-json.sql
DATA = uninstall_pg-json.sql

SHLIB_LINK += -ljansson

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
ma
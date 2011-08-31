# E164 Type contrb module Makefile
# $Id: Makefile 53 2007-09-10 01:13:48Z glaesema $

MODULE_big = e164
OBJS = e164.o e164_base.o e164_types.o e164_area_codes.o
DATA_built = e164.sql
DOCS = README.md
REGRESS = e164

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

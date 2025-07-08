# Copyright (c) ZeroC, Inc.

top_srcdir      := ../..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))))))

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

.NOTPARALLEL:

#
# Translate the Slice files from test directories
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2py))
tests := $(call tests-without-project-makefile,py)
include $(shell find $(lang_srcdir)/test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call make-python-test-project,$(lang_srcdir)/$(t))))

#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir      := ..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(lastword $(abspath $(MAKEFILE_LIST))))))

include $(top_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2matlab))
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceSSL IceLocatorDiscovery IceDiscovery)))

#
# Load MATLAB rules after loading C++ dependencies
#
include $(lang_srcdir)/config/Make.rules

ifeq ($(filter all matlab,$(ICE_BIN_DIST)),)

#
# Load Makefile.mk fragments
#
include $(shell find $(lang_srcdir) -name Makefile.mk)

#
# Make the projects (this expands all the build configuration and defines rules for all
# the components).
#
$(call make-projects,$(projects))

$(eval $(call make-matlab-package,$(slicedir),lib/generated,Ice,%F.ice \
	%/FacetMap.ice \
	%/ImplicitContext.ice \
	%/Instrumentation.ice \
	%/Logger.ice \
	%/ObjectAdapter.ice \
	%/ObjectFactory.ice \
	%/Plugin.ice \
	%/Properties.ice \
	%/ServantLocator.ice))
$(eval $(call make-matlab-package,$(slicedir),lib/generated,Glacier2))
$(eval $(call make-matlab-package,$(slicedir),lib/generated,IceGrid))
$(eval $(call make-matlab-package,$(slicedir),lib/generated,IceStorm))

endif

#
# icethunk mex library
#
icethunk_libdir = $(lang_srcdir)/lib/x86_64-linux-gnu
icethunk_srcdir = $(lang_srcdir)/src
icethunk_builddir = $(lang_srcdir)/src/build
icethunk_cflags = $(matlab_cflags) -I$(lang_srcdir)/src -m64 -fPIC -shared \
	-pthread -Wl,--no-undefined -Wl,-rpath-link,/usr/local/MATLAB/R2021b/bin/glnxa64
icethunk_target = $(icethunk_libdir)/icethunk.so
icethunk_proto = $(icethunk_libdir)/iceproto.m

.PRECIOUS: $(icethunk_proto)

$(icethunk_builddir)/ice.i: $(icethunk_srcdir)/ice.h
	mkdir -p $(icethunk_builddir)
	$(CC) -P -E -DMATLAB_PROTO $(icethunk_srcdir)/ice.h > $(icethunk_builddir)/ice.i

$(icethunk_builddir)/icethunk.c: $(icethunk_builddir)/ice.i
	cd $(icethunk_builddir) && \
	perl $(MATLAB_HOME)/toolbox/matlab/general/private/prototypes.pl ice.i \
	-thunkfile=icethunk.c \
	-outfile=iceproto.m

$(icethunk_proto): $(icethunk_builddir)/icethunk.c
	cp $(icethunk_builddir)/iceproto.m $(icethunk_libdir)/

$(icethunk_target): $(lang_srcdir)/src/build/icethunk.c $(icethunk_proto)
	$(CC) $(lang_srcdir)/src/build/icethunk.c $(icethunk_cflags) -o $(icethunk_target)

all:: $(icethunk_target)

#
# Translate the Slice files from test directories
#
tests := $(call tests-without-project-makefile,m)
include $(shell find test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call make-matlab-test-project,$(t))))

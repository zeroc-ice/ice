# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix		?= /opt/Ice-$(VERSION)

#
# Define to yes for an optimized build.
#
OPTIMIZE 	?= no

#
# Google Closure Compiler
#
CLOSURE_PATH=/opt/closure

#
# Closure Flags
#
CLOSUREFLAGS	= --language_in ECMASCRIPT5

#
# jslint flags
#
LINTFLAGS 	= --verbose

#
# NodeJS executable
#
NODE 		?= node

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = js
slice_translator = slice2js

bindir = $(top_srcdir)/bin
libdir = $(top_srcdir)/lib

install_libdir 	  = $(prefix)/lib
install_moduledir = $(prefix)/node_modules/icejs

ifeq ($(OPTIMIZE),yes)
mklibtargets	= $(libdir)/$(1).min.js $(libdir)/$(1).min.js.gz

installlib	= $(INSTALL) $(2)/$(3).min.js $(1); \
		  $(INSTALL) $(2)/$(3).min.js.gz $(1) \
		  $(INSTALL) $(2)/$(3).js $(1); \
                  $(INSTALL) $(2)/$(3).js.gz $(1)
else
mklibtargets	= $(libdir)/$(1).js $(libdir)/$(1).js.gz

installlib	= $(INSTALL) $(2)/$(3).js $(1); \
		  $(INSTALL) $(2)/$(3).js.gz $(1)
endif

installmodule	= if test ! -d $(1)/$(3) ; \
	    	  then \
			echo "Creating $(1)/$(3)..." ; \
			mkdir -p $(1)/$(3) ; \
			chmod a+rx $(1)/$(3) ; \
		  fi ; \
		  for f in "$(2)"; \
		  do \
		      cp $$f $(1)/$(3); \
		  done;


ifeq ($(shell test -f $(top_srcdir)/config/Make.common.rules && echo 0),0)
    include $(top_srcdir)/config/Make.common.rules
else
    include $(top_srcdir)/../config/Make.common.rules
endif


ifdef ice_src_dist
    ifeq ($(ice_cpp_dir), $(ice_dir)/cpp)
        SLICE2JS 	= $(ice_cpp_dir)/bin/slice2js
        SLICEPARSERLIB 	= $(ice_cpp_dir)/lib/$(call mklibfilename,Slice,$(VERSION))
    else
        SLICE2JS 	= $(ice_cpp_dir)/$(binsubdir)/slice2js
    endif
else
    SLICE2JS 		= $(ice_js_dir)/$(binsubdir)/slice2js
endif

all:: $(TARGETS)

ifneq ($(GEN_SRCS),)
clean::
	rm -rf $(GEN_SRCS)
else
clean::
endif

ifneq ($(TARGETS),)
clean:: 
	rm -rf $(TARGETS)
endif

%.js: $(SDIR)/%.ice $(SLICE2JS) $(SLICEPARSERLIB)
	rm -f $(*F).js
	$(SLICE2JS) $(SLICE2JSFLAGS) $<

%.js: %.ice $(SLICE2JS) $(SLICEPARSERLIB)
	rm -f $(*F).js
	$(SLICE2JS) $(SLICE2JSFLAGS) $<

index.html: $(GEN_SRCS) $(top_srcdir)/test/Common/index.html
	cp $(top_srcdir)/test/Common/index.html .

$(libdir)/$(LIBNAME).js $(libdir)/$(LIBNAME).js.gz: $(SRCS)
	@rm -f $(libdir)/$(LIBNAME).js $(libdir)/$(LIBNAME).js.gz
	$(NODE) $(top_srcdir)/config/makebundle.js "$(MODULES)" $(SRCS) > $(libdir)/$(LIBNAME).js
	gzip -c9 $(libdir)/$(LIBNAME).js > $(libdir)/$(LIBNAME).js.gz

ifeq ($(OPTIMIZE),yes)
$(libdir)/$(LIBNAME).min.js $(libdir)/$(LIBNAME).min.js.gz: $(libdir)/$(LIBNAME).js
	@rm -f $(libdir)/$(LIBNAME).min.js $(libdir)/$(LIBNAME).min.js.gz
	$(NODE) $(top_srcdir)/config/makebundle.js "$(MODULES)" $(SRCS) > $(libdir)/$(LIBNAME).tmp.js
	java -jar $(CLOSURE_PATH)/compiler.jar $(CLOSUREFLAGS) --js $(libdir)/$(LIBNAME).js --js_output_file $(libdir)/$(LIBNAME).min.js
	gzip -c9 $(libdir)/$(LIBNAME).min.js > $(libdir)/$(LIBNAME).min.js.gz
	rm -f $(libdir)/$(LIBNAME).tmp.js
endif

.PHONY : lint

install::

EVERYTHING		= all clean install lint
EVERYTHING_EXCEPT_ALL	= install clean lint

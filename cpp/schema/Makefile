# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

all:: BuiltinSequences.xsd \
      Identity.xsd

BuiltinSequences.xsd: $(top_srcdir)/slice/Ice/BuiltinSequences.ice
	$(SLICE2XSD) --ice $(top_srcdir)/slice/Ice/BuiltinSequences.ice

Identity.xsd: $(top_srcdir)/slice/Ice/Identity.ice
	$(SLICE2XSD) --ice $(top_srcdir)/slice/Ice/Identity.ice

install:: all
	@for i in *.xsd ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_schemadir)/$$i ; \
	done

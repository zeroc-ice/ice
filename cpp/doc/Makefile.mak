# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= reference\index.html

!include $(top_srcdir)\config\Make.rules.mak

IMAGES		= images
JAVASCRIPT	= symboltree.js

!include slicefiles

all::
	@python $(top_srcdir)\config\findSliceFiles.py $(slicedir) slicefiles
	@nmake -nologo -f Makefile.mak reference\index.html

reference\index.html: $(SLICEFILES)
	@$(MAKE) -nologo -f Makefile.mak clean
	$(bindir)\slice2html --ice -I..\slice --hdr=htmlHeader --indexhdr=indexHeader --indexftr=indexFooter \
	    --image-dir=images --logo-url="http://www.zeroc.com" --output-dir=reference --index=3 \
	    --summary=120 $(SLICEFILES)
	-mkdir reference\$(IMAGES)
	copy $(IMAGES)\*.gif reference\$(IMAGES)
	copy $(JAVASCRIPT) reference

clean::
	-rd /s /q reference

install:: reference\index.html 
	copy reference\index.html $(install_docdir)
	xcopy /i /s /y reference $(install_docdir)\reference

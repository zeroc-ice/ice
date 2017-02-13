# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	= IceObjC

IceObjC_targetdir	:= $(libdir)
IceObjC_cppflags	:= -DICE_API_EXPORTS -I$(project) -I$(top_srcdir)/cpp/include -I$(top_srcdir)/cpp/include/generated
IceObjC_sliceflags	:= --include-dir objc/Ice
IceObjC_dependencies	:= Ice
IceObjC_slicedir	:= $(slicedir)/Ice
IceObjC_includedir	:= $(includedir)/objc/Ice
IceObjC_excludes	:= $(slicedir)/Ice/ValueFactory.ice \
			   $(slicedir)/Ice/ObjectFactory.ice \
			   $(slicedir)/Ice/Instrumentation.ice \
			   $(slicedir)/Ice/Plugin.ice

IceObjC_install:: $(install_includedir)/objc/Ice.h

# Create the default implicit rule for these 2 header files to make sure the
# Slice files aren't re-generated
$(includedir)/objc/Ice/ValueFactory.h: ;
$(includedir)/objc/Ice/ObjectFactory.h: ;

projects += $(project)

# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries		= IceSSLObjC

IceSSLObjC_targetdir		:= $(libdir)
IceSSLObjC_cppflags  		:= -DICESSL_API_EXPORTS -Isrc/Ice -I$(top_srcdir)/cpp/include -I$(top_srcdir)/cpp/include/generated
IceSSLObjC_sliceflags		:= --include-dir objc/IceSSL
IceSSLObjC_dependencies 	:= IceObjC IceSSL Ice
IceSSLObjC_slicedir		:= $(slicedir)/IceSSL
IceSSLObjC_includedir		:= $(includedir)/objc/IceSSL

IceSSLObjC_install:: $(install_includedir)/objc/IceSSL.h

projects += $(project)

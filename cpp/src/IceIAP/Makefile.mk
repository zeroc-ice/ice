# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries    = IceIAP

IceIAP_configs          := xcodesdk cpp11-xcodesdk
IceIAP_platforms        := iphoneos iphonesimulator

IceIAP_targetdir        := $(libdir)
IceIAP_dependencies     := Ice
IceIAP_sliceflags       := --include-dir IceIAP
IceIAP_cppflags         := -DICEIAP_API_EXPORTS

projects += $(project)

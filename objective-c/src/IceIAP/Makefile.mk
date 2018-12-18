# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries            = IceIAPObjC

IceIAPObjC_configs              := xcodesdk
IceIAPObjC_platforms            := iphoneos iphonesimulator

IceIAPObjC_targetdir            := $(libdir)
IceIAPObjC_cppflags             := -DICEIAP_API_EXPORTS -Isrc/Ice $(ice_cpp_cppflags)
IceIAPObjC_sliceflags           := --include-dir objc/IceIAP
IceIAPObjC_dependencies         := IceObjC IceIAP Ice
IceIAPObjC_slicedir             := $(slicedir)/IceIAP
IceIAPObjC_includedir           := $(includedir)/objc/IceIAP

IceIAPObjC_install:: $(install_includedir)/objc/IceIAP.h

projects += $(project)

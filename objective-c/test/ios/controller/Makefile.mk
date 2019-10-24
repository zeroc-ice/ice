#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_type                 = xcode
$(project)_platforms            = iphonesimulator iphoneos
$(project)_configs              = xcodesdk arc-xcodesdk
$(project)_dependencies         = $(if $(ICE_BIN_DIST),,IceObjC IceSSLObjC IceGridObjC Glacier2ObjC IceStormObjC)
$(project)_project              = Objective-C Test Controller.xcodeproj
$(project)_scheme               = $(if $(filter arc-xcodesdk,$2),Objective-C ARC Test Controller,Objective-C Test Controller)
# Always build debug configuration to prevent symbols from being stripped
$(project)_configuration        = Debug
$(project)_slicecompiler        = slice2objc

projects += $(project)
tests:: $(project)

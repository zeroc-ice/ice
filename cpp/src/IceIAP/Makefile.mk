#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceIAP

IceIAP_configs          := xcodesdk
IceIAP_platforms        := iphoneos iphonesimulator

IceIAP_targetdir        := $(libdir)
IceIAP_dependencies     := Ice
IceIAP[shared]_cppflags := -DICEIAP_API_EXPORTS

projects += $(project)

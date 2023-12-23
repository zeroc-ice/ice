#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceIAP

IceIAP_configs          := xcodesdk cpp11-xcodesdk
IceIAP_platforms        := iphoneos iphonesimulator

IceIAP_targetdir        := $(libdir)
IceIAP_dependencies     := Ice
IceIAP_cppflags         := -DICEIAP_API_EXPORTS

# We exclude the following Slice files that contain only local definitions, as we don't want two header files with the
# same name.
IceIAP_excludes         += $(wildcard ../slice/IceIAP/*.ice)

projects += $(project)

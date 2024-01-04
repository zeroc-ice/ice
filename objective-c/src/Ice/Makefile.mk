#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceObjC

IceObjC_targetdir       := $(libdir)
IceObjC_cppflags        := -DICE_API_EXPORTS -I$(project) $(ice_cpp_cppflags)
IceObjC_sliceflags      := --include-dir objc/Ice
IceObjC_dependencies    := Ice
IceObjC_slicedir        := $(slicedir)/Ice
IceObjC_includedir      := $(includedir)/objc/Ice

IceObjC_install:: $(install_includedir)/objc/Ice.h

projects += $(project)

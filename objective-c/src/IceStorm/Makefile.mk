#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries            = IceStormObjC

IceStormObjC_targetdir          := $(libdir)
IceStormObjC_sliceflags         := --include-dir objc/IceStorm
IceStormObjC_dependencies       := IceObjC
IceStormObjC_slicedir           := $(slicedir)/IceStorm
IceStormObjC_includedir         := $(includedir)/objc/IceStorm

IceStormObjC_install:: $(install_includedir)/objc/IceStorm.h

projects += $(project)

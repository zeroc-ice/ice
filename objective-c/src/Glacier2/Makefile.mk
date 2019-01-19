#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries            = Glacier2ObjC

Glacier2ObjC_targetdir          := $(libdir)
Glacier2ObjC_sliceflags         := --include-dir objc/Glacier2
Glacier2ObjC_dependencies       := IceObjC
Glacier2ObjC_slicedir           := $(slicedir)/Glacier2
Glacier2ObjC_includedir         := $(includedir)/objc/Glacier2

Glacier2ObjC_install:: $(install_includedir)/objc/Glacier2.h

projects += $(project)

# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries            = IceGridObjC

IceGridObjC_targetdir           := $(libdir)
IceGridObjC_sliceflags          := --include-dir objc/IceGrid
IceGridObjC_dependencies        := IceObjC Glacier2ObjC
IceGridObjC_slicedir            := $(slicedir)/IceGrid
IceGridObjC_includedir          := $(includedir)/objc/IceGrid

IceGridObjC_install:: $(install_includedir)/objc/IceGrid.h

projects += $(project)

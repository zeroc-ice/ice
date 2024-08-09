#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceXML

IceXML_targetdir        := $(libdir)
IceXML_dependencies     := Ice
IceXML_cppflags         := -DICE_XML_API_EXPORTS
IceXML_libs             := expat
IceXML_devinstall       := no

projects += $(project)

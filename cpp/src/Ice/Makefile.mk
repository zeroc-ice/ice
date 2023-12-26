#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = Ice

Ice_targetdir           := $(libdir)
Ice_cppflags            = -DICE_API_EXPORTS $(IceUtil_cppflags)
Ice_ldflags             = $(iconv_ldflags)

ifeq ($(DEFAULT_MUTEX_PROTOCOL), PrioInherit)
    Ice_cppflags        += -DICE_PRIO_INHERIT
endif

Ice_sliceflags          := --include-dir Ice
Ice_libs                := bz2
Ice_extra_sources       := $(wildcard src/IceUtil/*.cpp)
Ice_excludes            = src/Ice/DLLMain.cpp

# We exclude the following Slice files that contain only local definitions, as we don't want two header files with
# the same name.
Ice_excludes            += ../slice/Ice/Communicator.ice \
                           ../slice/Ice/CommunicatorF.ice \
                           ../slice/Ice/Connection.ice \
                           ../slice/Ice/ConnectionF.ice \
                           ../slice/Ice/Current.ice \
                           ../slice/Ice/Endpoint.ice \
                           ../slice/Ice/EndpointF.ice \
                           ../slice/Ice/EndpointSelectionType.ice \
                           ../slice/Ice/FacetMap.ice \
                           ../slice/Ice/ImplicitContext.ice \
                           ../slice/Ice/ImplicitContextF.ice \
                           ../slice/Ice/Instrumentation.ice \
                           ../slice/Ice/InstrumentationF.ice \
                           ../slice/Ice/LocalException.ice \
                           ../slice/Ice/Logger.ice \
                           ../slice/Ice/LoggerF.ice \
                           ../slice/Ice/ObjectAdapter.ice \
                           ../slice/Ice/ObjectAdapterF.ice \
                           ../slice/Ice/ObjectFactory.ice \
                           ../slice/Ice/Plugin.ice \
                           ../slice/Ice/PluginF.ice \
                           ../slice/Ice/Properties.ice \
                           ../slice/Ice/PropertiesF.ice \
                           ../slice/Ice/ServantLocator.ice \
                           ../slice/Ice/ServantLocatorF.ice \
                           ../slice/Ice/ValueFactory.ice

ifeq ($(os),Darwin)
Ice_excludes            += src/IceUtil/ConvertUTF.cpp src/IceUtil/Unicode.cpp
endif

ifeq ($(os),Linux)
ifeq ($(shell pkg-config --exists libsystemd 2> /dev/null && echo yes),yes)
Ice_cppflags                            += -DICE_USE_SYSTEMD $(shell pkg-config --cflags libsystemd)
endif
endif

Ice[iphoneos]_excludes                  := $(wildcard $(addprefix $(currentdir)/,Tcp*.cpp))
Ice[iphoneos]_extra_sources             := $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
Ice[iphonesimulator]_excludes           = $(Ice[iphoneos]_excludes)
Ice[iphonesimulator]_extra_sources      = $(Ice[iphoneos]_extra_sources)

projects += $(project)

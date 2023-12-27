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
IceObjC_excludes        := $(slicedir)/Ice/Communicator.ice \
                           $(slicedir)/Ice/CommunicatorF.ice \
                           $(slicedir)/Ice/Connection.ice \
                           $(slicedir)/Ice/ConnectionF.ice \
                           $(slicedir)/Ice/Current.ice \
                           $(slicedir)/Ice/Endpoint.ice \
                           $(slicedir)/Ice/EndpointF.ice \
                           $(slicedir)/Ice/EndpointSelectionType.ice \
                           $(slicedir)/Ice/FacetMap.ice \
                           $(slicedir)/Ice/ImplicitContext.ice \
                           $(slicedir)/Ice/ImplicitContextF.ice \
                           $(slicedir)/Ice/Instrumentation.ice \
                           $(slicedir)/Ice/InstrumentationF.ice \
                           $(slicedir)/Ice/LocalException.ice \
                           $(slicedir)/Ice/Logger.ice \
                           $(slicedir)/Ice/LoggerF.ice \
                           $(slicedir)/Ice/ObjectAdapter.ice \
                           $(slicedir)/Ice/ObjectAdapterF.ice \
                           $(slicedir)/Ice/ObjectFactory.ice \
                           $(slicedir)/Ice/Plugin.ice \
                           $(slicedir)/Ice/PluginF.ice \
                           $(slicedir)/Ice/Properties.ice \
                           $(slicedir)/Ice/PropertiesF.ice \
                           $(slicedir)/Ice/ServantLocator.ice \
                           $(slicedir)/Ice/ServantLocatorF.ice \
                           $(slicedir)/Ice/ValueFactory.ice

IceObjC_install:: $(install_includedir)/objc/Ice.h

# Create the default implicit rule for these 2 header files to make sure the
# Slice files aren't re-generated
$(includedir)/objc/Ice/ValueFactory.h: ;
$(includedir)/objc/Ice/ObjectFactory.h: ;

projects += $(project)


BuiltinSequences.cs: \
    "$(slicedir)\Ice\BuiltinSequences.ice"

Communicator.cs: \
    "$(slicedir)\Ice\Communicator.ice" \
    "$(slicedir)/Ice/LoggerF.ice" \
    "$(slicedir)/Ice/InstrumentationF.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ObjectFactoryF.ice" \
    "$(slicedir)/Ice/RouterF.ice" \
    "$(slicedir)/Ice/LocatorF.ice" \
    "$(slicedir)/Ice/PluginF.ice" \
    "$(slicedir)/Ice/ImplicitContextF.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/FacetMap.ice"

Connection.cs: \
    "$(slicedir)\Ice\Connection.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

Current.cs: \
    "$(slicedir)\Ice\Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

Endpoint.cs: \
    "$(slicedir)\Ice\Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

EndpointTypes.cs: \
    "$(slicedir)\Ice\EndpointTypes.ice"

FacetMap.cs: \
    "$(slicedir)\Ice\FacetMap.ice"

Identity.cs: \
    "$(slicedir)\Ice\Identity.ice"

ImplicitContext.cs: \
    "$(slicedir)\Ice\ImplicitContext.ice" \
    "$(slicedir)/Ice/LocalException.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice"

Instrumentation.cs: \
    "$(slicedir)\Ice\Instrumentation.ice" \
    "$(slicedir)/Ice/EndpointF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

LocalException.cs: \
    "$(slicedir)\Ice\LocalException.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Locator.cs: \
    "$(slicedir)\Ice\Locator.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/ProcessF.ice"

Logger.cs: \
    "$(slicedir)\Ice\Logger.ice"

Metrics.cs: \
    "$(slicedir)\Ice\Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

ObjectAdapter.cs: \
    "$(slicedir)\Ice\ObjectAdapter.ice" \
    "$(slicedir)/Ice/CommunicatorF.ice" \
    "$(slicedir)/Ice/ServantLocatorF.ice" \
    "$(slicedir)/Ice/LocatorF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/FacetMap.ice" \
    "$(slicedir)/Ice/Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

ObjectFactory.cs: \
    "$(slicedir)\Ice\ObjectFactory.ice"

Plugin.cs: \
    "$(slicedir)\Ice\Plugin.ice" \
    "$(slicedir)/Ice/LoggerF.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Process.cs: \
    "$(slicedir)\Ice\Process.ice"

Properties.cs: \
    "$(slicedir)\Ice\Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

PropertiesAdmin.cs: \
    "$(slicedir)\Ice\PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

RemoteLogger.cs: \
    "$(slicedir)\Ice\RemoteLogger.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Router.cs: \
    "$(slicedir)\Ice\Router.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

ServantLocator.cs: \
    "$(slicedir)\Ice\ServantLocator.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

SliceChecksumDict.cs: \
    "$(slicedir)\Ice\SliceChecksumDict.ice"

Version.cs: \
    "$(slicedir)\Ice\Version.ice"

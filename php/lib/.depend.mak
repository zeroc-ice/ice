
LocalException.php: \
    "$(slicedir)\Ice\LocalException.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Communicator.php: \
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

CommunicatorF.php: \
    "$(slicedir)\Ice\CommunicatorF.ice"

Logger.php: \
    "$(slicedir)\Ice\Logger.ice"

LoggerF.php: \
    "$(slicedir)\Ice\LoggerF.ice"

BuiltinSequences.php: \
    "$(slicedir)\Ice\BuiltinSequences.ice"

ObjectAdapterF.php: \
    "$(slicedir)\Ice\ObjectAdapterF.ice"

PropertiesAdmin.php: \
    "$(slicedir)\Ice\PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Properties.php: \
    "$(slicedir)\Ice\Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

PropertiesF.php: \
    "$(slicedir)\Ice\PropertiesF.ice"

ObjectFactory.php: \
    "$(slicedir)\Ice\ObjectFactory.ice"

ObjectFactoryF.php: \
    "$(slicedir)\Ice\ObjectFactoryF.ice"

Identity.php: \
    "$(slicedir)\Ice\Identity.ice"

Current.php: \
    "$(slicedir)\Ice\Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

ImplicitContextF.php: \
    "$(slicedir)\Ice\ImplicitContextF.ice"

ImplicitContext.php: \
    "$(slicedir)\Ice\ImplicitContext.ice" \
    "$(slicedir)/Ice/LocalException.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice"

Router.php: \
    "$(slicedir)\Ice\Router.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

RouterF.php: \
    "$(slicedir)\Ice\RouterF.ice"

Plugin.php: \
    "$(slicedir)\Ice\Plugin.ice" \
    "$(slicedir)/Ice/LoggerF.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

PluginF.php: \
    "$(slicedir)\Ice\PluginF.ice"

Locator.php: \
    "$(slicedir)\Ice\Locator.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/ProcessF.ice"

LocatorF.php: \
    "$(slicedir)\Ice\LocatorF.ice"

Process.php: \
    "$(slicedir)\Ice\Process.ice"

ProcessF.php: \
    "$(slicedir)\Ice\ProcessF.ice"

FacetMap.php: \
    "$(slicedir)\Ice\FacetMap.ice"

Connection.php: \
    "$(slicedir)\Ice\Connection.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

ConnectionF.php: \
    "$(slicedir)\Ice\ConnectionF.ice"

SliceChecksumDict.php: \
    "$(slicedir)\Ice\SliceChecksumDict.ice"

Endpoint.php: \
    "$(slicedir)\Ice\Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

EndpointF.php: \
    "$(slicedir)\Ice\EndpointF.ice"

EndpointTypes.php: \
    "$(slicedir)\Ice\EndpointTypes.ice"

Version.php: \
    "$(slicedir)\Ice\Version.ice"

InstrumentationF.php: \
    "$(slicedir)\Ice\InstrumentationF.ice"

Instrumentation.php: \
    "$(slicedir)\Ice\Instrumentation.ice" \
    "$(slicedir)/Ice/EndpointF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

Metrics.php: \
    "$(slicedir)\Ice\Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

RouterF.php: \
    "$(slicedir)\Glacier2\RouterF.ice"

Router.php: \
    "$(slicedir)\Glacier2\Router.ice" \
    "$(slicedir)/Ice/Router.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/Glacier2/PermissionsVerifier.ice"

Session.php: \
    "$(slicedir)\Glacier2\Session.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice"

PermissionsVerifierF.php: \
    "$(slicedir)\Glacier2\PermissionsVerifierF.ice"

PermissionsVerifier.php: \
    "$(slicedir)\Glacier2\PermissionsVerifier.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

SSLInfo.php: \
    "$(slicedir)\Glacier2\SSLInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Metrics.php: \
    "$(slicedir)\Glacier2\Metrics.ice" \
    "$(slicedir)/Ice/Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

IceBox.php: \
    "$(slicedir)\IceBox\IceBox.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/CommunicatorF.ice" \
    "$(slicedir)/Ice/PropertiesF.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice"

Admin.php: \
    "$(slicedir)\IceGrid\Admin.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/IceGrid/Exception.ice" \
    "$(slicedir)/IceGrid/Descriptor.ice"

Descriptor.php: \
    "$(slicedir)\IceGrid\Descriptor.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Exception.php: \
    "$(slicedir)\IceGrid\Exception.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

FileParser.php: \
    "$(slicedir)\IceGrid\FileParser.ice" \
    "$(slicedir)/IceGrid/Admin.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/IceGrid/Exception.ice" \
    "$(slicedir)/IceGrid/Descriptor.ice"

Locator.php: \
    "$(slicedir)\IceGrid\Locator.ice" \
    "$(slicedir)/Ice/Locator.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/ProcessF.ice"

Observer.php: \
    "$(slicedir)\IceGrid\Observer.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/IceGrid/Exception.ice" \
    "$(slicedir)/IceGrid/Descriptor.ice" \
    "$(slicedir)/IceGrid/Admin.ice" \
    "$(slicedir)/Ice/Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice"

Query.php: \
    "$(slicedir)\IceGrid\Query.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/IceGrid/Exception.ice"

Registry.php: \
    "$(slicedir)\IceGrid\Registry.ice" \
    "$(slicedir)/IceGrid/Exception.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/IceGrid/Session.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/IceGrid/Admin.ice" \
    "$(slicedir)/Ice/Properties.ice" \
    "$(slicedir)/Ice/PropertiesAdmin.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice" \
    "$(slicedir)/IceGrid/Descriptor.ice"

Session.php: \
    "$(slicedir)\IceGrid\Session.ice" \
    "$(slicedir)/Glacier2/Session.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Glacier2/SSLInfo.ice" \
    "$(slicedir)/IceGrid/Exception.ice"

UserAccountMapper.php: \
    "$(slicedir)\IceGrid\UserAccountMapper.ice"

FileInfo.php: \
    "$(slicedir)\IcePatch2\FileInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

FileServer.php: \
    "$(slicedir)\IcePatch2\FileServer.ice" \
    "$(slicedir)/IcePatch2/FileInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

IceStorm.php: \
    "$(slicedir)\IceStorm\IceStorm.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/SliceChecksumDict.ice" \
    "$(slicedir)/IceStorm/Metrics.ice" \
    "$(slicedir)/Ice/Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Metrics.php: \
    "$(slicedir)\IceStorm\Metrics.ice" \
    "$(slicedir)/Ice/Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

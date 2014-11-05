
BuiltinSequences.js: \
    "$(slicedir)\Ice\BuiltinSequences.ice"

Connection.js: \
    "$(slicedir)\Ice\Connection.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

ConnectionF.js: \
    "$(slicedir)\Ice\ConnectionF.ice"

Current.js: \
    "$(slicedir)\Ice\Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

Endpoint.js: \
    "$(slicedir)\Ice\Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

EndpointF.js: \
    "$(slicedir)\Ice\EndpointF.ice"

EndpointInfo.js: \
    "$(slicedir)\IceSSL\EndpointInfo.ice" \
    "$(slicedir)/Ice/Endpoint.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice" \
    "$(slicedir)/Ice/EndpointF.ice"

EndpointTypes.js: \
    "$(slicedir)\Ice\EndpointTypes.ice"

Identity.js: \
    "$(slicedir)\Ice\Identity.ice"

LocalException.js: \
    "$(slicedir)\Ice\LocalException.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Locator.js: \
    "$(slicedir)\Ice\Locator.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/ProcessF.ice"

Metrics.js: \
    "$(slicedir)\Ice\Metrics.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

ObjectAdapterF.js: \
    "$(slicedir)\Ice\ObjectAdapterF.ice"

Process.js: \
    "$(slicedir)\Ice\Process.ice"

ProcessF.js: \
    "$(slicedir)\Ice\ProcessF.ice"

PropertiesAdmin.js: \
    "$(slicedir)\Ice\PropertiesAdmin.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

Router.js: \
    "$(slicedir)\Ice\Router.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

SliceChecksumDict.js: \
    "$(slicedir)\Ice\SliceChecksumDict.ice"

Version.js: \
    "$(slicedir)\Ice\Version.ice"

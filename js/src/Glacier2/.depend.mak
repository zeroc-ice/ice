
Metrics.js: \
    $(slicedir)\Glacier2\Metrics.ice \
    $(slicedir)/Ice/Metrics.ice \
    $(slicedir)/Ice/BuiltinSequences.ice

PermissionsVerifier.js: \
    $(slicedir)\Glacier2\PermissionsVerifier.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/Ice/BuiltinSequences.ice

PermissionsVerifierF.js: \
    $(slicedir)\Glacier2\PermissionsVerifierF.ice

Router.js: \
    $(slicedir)\Glacier2\Router.ice \
    $(slicedir)/Ice/Router.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/Glacier2/PermissionsVerifier.ice

RouterF.js: \
    $(slicedir)\Glacier2\RouterF.ice

Session.js: \
    $(slicedir)\Glacier2\Session.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Glacier2/SSLInfo.ice

SSLInfo.js: \
    $(slicedir)\Glacier2\SSLInfo.ice \
    $(slicedir)/Ice/BuiltinSequences.ice

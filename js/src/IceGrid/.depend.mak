
Admin.js: \
    $(slicedir)\IceGrid\Admin.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Ice/Properties.ice \
    $(slicedir)/Ice/PropertiesAdmin.ice \
    $(slicedir)/Ice/SliceChecksumDict.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/IceGrid/Exception.ice \
    $(slicedir)/IceGrid/Descriptor.ice

Descriptor.js: \
    $(slicedir)\IceGrid\Descriptor.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice

Exception.js: \
    $(slicedir)\IceGrid\Exception.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice

FileParser.js: \
    $(slicedir)\IceGrid\FileParser.ice \
    $(slicedir)/IceGrid/Admin.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Ice/Properties.ice \
    $(slicedir)/Ice/PropertiesAdmin.ice \
    $(slicedir)/Ice/SliceChecksumDict.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/IceGrid/Exception.ice \
    $(slicedir)/IceGrid/Descriptor.ice

Locator.js: \
    $(slicedir)\IceGrid\Locator.ice \
    $(slicedir)/Ice/Locator.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/ProcessF.ice

Observer.js: \
    $(slicedir)\IceGrid\Observer.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/IceGrid/Exception.ice \
    $(slicedir)/IceGrid/Descriptor.ice \
    $(slicedir)/IceGrid/Admin.ice \
    $(slicedir)/Ice/Properties.ice \
    $(slicedir)/Ice/PropertiesAdmin.ice \
    $(slicedir)/Ice/SliceChecksumDict.ice

Query.js: \
    $(slicedir)\IceGrid\Query.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/IceGrid/Exception.ice

Registry.js: \
    $(slicedir)\IceGrid\Registry.ice \
    $(slicedir)/IceGrid/Exception.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/IceGrid/Session.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/IceGrid/Admin.ice \
    $(slicedir)/Ice/Properties.ice \
    $(slicedir)/Ice/PropertiesAdmin.ice \
    $(slicedir)/Ice/SliceChecksumDict.ice \
    $(slicedir)/IceGrid/Descriptor.ice

Session.js: \
    $(slicedir)\IceGrid\Session.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    $(slicedir)/IceGrid/Exception.ice

UserAccountMapper.js: \
    $(slicedir)\IceGrid\UserAccountMapper.ice

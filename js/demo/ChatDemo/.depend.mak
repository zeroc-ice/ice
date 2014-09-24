
Chat.js: \
    .\Chat.ice

ChatSession.js: \
    .\ChatSession.ice \
    $(slicedir)/Ice/BuiltinSequences.ice \
    $(slicedir)/Glacier2/Session.ice \
    $(slicedir)/Ice/Identity.ice \
    $(slicedir)/Glacier2/SSLInfo.ice \
    ./Chat.ice

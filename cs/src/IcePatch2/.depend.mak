
FileInfo.cs: \
    "$(slicedir)\IcePatch2\FileInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

FileServer.cs: \
    "$(slicedir)\IcePatch2\FileServer.ice" \
    "$(slicedir)/IcePatch2/FileInfo.ice" \
    "$(slicedir)/Ice/BuiltinSequences.ice"

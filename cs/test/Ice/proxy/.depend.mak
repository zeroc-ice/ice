
Test.cs: \
    .\Test.ice \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

TestAMD.cs: \
    .\TestAMD.ice \
    "$(slicedir)/Ice/Current.ice" \
    "$(slicedir)/Ice/ObjectAdapterF.ice" \
    "$(slicedir)/Ice/ConnectionF.ice" \
    "$(slicedir)/Ice/Identity.ice" \
    "$(slicedir)/Ice/Version.ice"

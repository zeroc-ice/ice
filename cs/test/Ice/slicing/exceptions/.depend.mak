
Test.cs: \
    .\Test.ice

ClientPrivate.cs: \
    .\ClientPrivate.ice \
    ./Test.ice

ServerPrivate.cs: \
    .\ServerPrivate.ice \
    ./Test.ice

TestAMD.cs: \
    .\TestAMD.ice

ServerPrivateAMD.cs: \
    .\ServerPrivateAMD.ice \
    ./TestAMD.ice

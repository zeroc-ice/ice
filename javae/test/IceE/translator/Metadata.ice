//
// NOTE: Ice-E for Java doesn't support custom metadata, so the metadata
// in this file is used to ensure that the generated code is NOT affected.
//
module MetadataTest
{
    sequence<int> IntSeq;
    ["java:type:NonexistentList"] sequence<int> IntList;

    dictionary<string, string> StringDict;
    ["java:type:NonexistentMap"] dictionary<string, string> StringMap;

    class C
    {
        IntSeq intSeqMember;
        IntList intListMember;
        ["java:type:NonexistentList"] IntSeq modifiedIntSeqMember;

	StringDict stringDictMember;
	StringMap stringMapMember;

        IntList opIntList(IntList inArg, out IntList outArg);

        StringMap opStringMap(StringMap inArg, out StringMap outArg);

        ["java:type:NonexistentList"] IntSeq
        opIntSeq2(["java:type:NonexistentList"] IntSeq inArg,
                  out ["java:type:NonexistentList"] IntSeq outArg);
    };
};

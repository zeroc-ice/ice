module MetadataTest
{
    sequence<int> IntSeq;
    ["java:type:java.util.LinkedList"] sequence<int> IntList;

    sequence<Object> ObjectSeq;
    ["java:type:java.util.LinkedList"] sequence<Object> ObjectList;

    dictionary<string, string> StringDict;
    ["java:type:java.util.HashMap"] dictionary<string, string> StringMap;

    class C
    {
        IntSeq intSeqMember;
        IntList intListMember;
        ["java:type:java.util.ArrayList"] IntSeq modifiedIntSeqMember;
        ["java:type:Test.CustomList"] IntList modifiedIntListMember;

        ObjectSeq objectSeqMember;
        ObjectList objectListMember;
        ["java:type:java.util.ArrayList"] ObjectSeq modifiedObjectSeqMember;
        ["java:type:Test.CustomList"] ObjectList modifiedObjectListMember;

	StringDict stringDictMember;
	StringMap stringMapMember;

        IntSeq opIntSeq(IntSeq inArg, out IntSeq outArg);
        IntList opIntList(IntList inArg, out IntList outArg);

        ObjectSeq opObjectSeq(ObjectSeq inArg, out ObjectSeq outArg);
        ObjectList opObjectList(ObjectList inArg, out ObjectList outArg);

        StringDict opStringDict(StringDict inArg, out StringDict outArg);
        StringMap opStringMap(StringMap inArg, out StringMap outArg);

        ["java:type:java.util.LinkedList"] IntSeq
        opIntSeq2(["java:type:java.util.ArrayList"] IntSeq inArg,
                  out ["java:type:Test.CustomList"] IntSeq outArg);

        ["java:type:java.util.ArrayList"] IntList
        opIntList2(["java:type:java.util.ArrayList"] IntList inArg,
                   out ["java:type:Test.CustomList"] IntList outArg);

        ["java:type:java.util.LinkedList"] ObjectSeq
        opObjectSeq2(["java:type:java.util.ArrayList"] ObjectSeq inArg,
                     out ["java:type:Test.CustomList"] ObjectSeq outArg);

        ["java:type:java.util.ArrayList"] ObjectList
        opObjectList2(["java:type:java.util.ArrayList"] ObjectList inArg,
                      out ["java:type:Test.CustomList"] ObjectList outArg);
    };
};

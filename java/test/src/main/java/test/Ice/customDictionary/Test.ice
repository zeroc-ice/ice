// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.customDictionary.Test"]
module Test
{
    ["java:type:java.util.LinkedHashMap<Byte, Boolean>"] dictionary<byte, bool> ByteBoolLinkedDict;
    ["java:type:java.util.LinkedHashMap<Short, Integer>"] dictionary<short, int> ShortIntLinkedDict;
    ["java:type:java.util.LinkedHashMap<Long, Float>:java.util.LinkedHashMap<Long, Float>"]
    dictionary<long, float> LongFloatLinkedDict;
    ["java:type:java.util.LinkedHashMap<String, Double>:java.util.LinkedHashMap<String, Double>"]
    dictionary<string, double> StringDoubleLinkedDict;

    struct A
    {
        int i;
    }
    dictionary<A, int> AIntDict;
    ["java:type:java.util.LinkedHashMap<A, Integer>"] dictionary<A, int> AIntLinkedDict;
    ["java:type:java.util.LinkedHashMap<A, Integer>:java.util.LinkedHashMap<A, Integer>"]
    dictionary<A, int> AIntFormalLinkedDict;

    class C
    {
        string s;
    }
    dictionary<short, C> ShortCDict;
    ["java:type:java.util.LinkedHashMap<Short, C>"] dictionary<short, C> ShortCLinkedDict;
    ["java:type:java.util.LinkedHashMap<Short, C>:java.util.LinkedHashMap<Short, C>"]
    dictionary<short, C> ShortCFormalLinkedDict;

    enum E { E1, E2, E3 }
    dictionary<string, E> StringEDict;
    ["java:type:java.util.LinkedHashMap<String, E>"] dictionary<string, E> StringELinkedDict;
    ["java:type:java.util.LinkedHashMap<String, E>:java.util.LinkedHashMap<String, E>"]
    dictionary<string, E> StringEFormalLinkedDict;

    sequence<string> StringSeq;
    dictionary<E, StringSeq> EStringSeqDict;
    ["java:type:java.util.LinkedHashMap<E, String[]>"] dictionary<E, StringSeq> EStringSeqLinkedDict;
    ["java:type:java.util.LinkedHashMap<E, String[]>:java.util.LinkedHashMap<E, String[]>"]
    dictionary<E, StringSeq> EStringSeqFormalLinkedDict;

    interface TestIntf
    {
        ByteBoolLinkedDict opByteBoolLinkedDict(ByteBoolLinkedDict inDict, out ByteBoolLinkedDict outDict);
        ShortIntLinkedDict opShortIntLinkedDict(ShortIntLinkedDict inDict, out ShortIntLinkedDict outDict);
        LongFloatLinkedDict opLongFloatLinkedDict(LongFloatLinkedDict inDict, out LongFloatLinkedDict outDict);
        StringDoubleLinkedDict opStringDoubleLinkedDict(
            StringDoubleLinkedDict inDict, out StringDoubleLinkedDict outDict);

        AIntDict opAIntDict(AIntDict inDict, out AIntDict outDict);
        AIntLinkedDict opAIntLinkedDict(AIntLinkedDict inDict, out AIntLinkedDict outDict);
        AIntFormalLinkedDict opAIntFormalLinkedDict(
            AIntFormalLinkedDict inDict, out AIntFormalLinkedDict outDict);

        ShortCDict opShortCDict(ShortCDict inDict, out ShortCDict outDict);
        ShortCLinkedDict opShortCLinkedDict(ShortCLinkedDict inDict, out ShortCLinkedDict outDict);
        ShortCFormalLinkedDict opShortCFormalLinkedDict(
            ShortCFormalLinkedDict inDict, out ShortCFormalLinkedDict outDict);

        StringEDict opStringEDict(StringEDict inDict, out StringEDict outDict);
        StringELinkedDict opStringELinkedDict(StringELinkedDict inDict, out StringELinkedDict outDict);
        StringEFormalLinkedDict opStringEFormalLinkedDict(
            StringEFormalLinkedDict inDict, out StringEFormalLinkedDict outDict);

        EStringSeqDict opEStringSeqDict(EStringSeqDict inDict, out EStringSeqDict outDict);
        EStringSeqLinkedDict opEStringSeqLinkedDict(EStringSeqLinkedDict inDict, out EStringSeqLinkedDict outDict);
        EStringSeqFormalLinkedDict opEStringSeqFormalLinkedDict(
            EStringSeqFormalLinkedDict inDict, out EStringSeqFormalLinkedDict outDict);

        optional(1) ByteBoolLinkedDict opOptByteBoolLinkedDict(
            optional(2) ByteBoolLinkedDict inDict, out optional(3) ByteBoolLinkedDict outDict);
        optional(1) ShortIntLinkedDict opOptShortIntLinkedDict(
            optional(2) ShortIntLinkedDict inDict, out optional(3) ShortIntLinkedDict outDict);
        optional(1) LongFloatLinkedDict opOptLongFloatLinkedDict(
            optional(2) LongFloatLinkedDict inDict, out optional(3) LongFloatLinkedDict outDict);
        optional(1) StringDoubleLinkedDict opOptStringDoubleLinkedDict(
            optional(2) StringDoubleLinkedDict inDict, out optional(3) StringDoubleLinkedDict outDict);

        optional(1) AIntDict opOptAIntDict(
            optional(2) AIntDict inDict, out optional(3) AIntDict outDict);
        optional(1) AIntLinkedDict opOptAIntLinkedDict(
            optional(2) AIntLinkedDict inDict, out optional(3) AIntLinkedDict outDict);
        optional(1) AIntFormalLinkedDict opOptAIntFormalLinkedDict(
            optional(2) AIntFormalLinkedDict inDict, out optional(3) AIntFormalLinkedDict outDict);

        optional(1) StringEDict opOptStringEDict(
            optional(2) StringEDict inDict, out optional(3) StringEDict outDict);
        optional(1) StringELinkedDict opOptStringELinkedDict(
            optional(2) StringELinkedDict inDict, out optional(3) StringELinkedDict outDict);
        optional(1) StringEFormalLinkedDict opOptStringEFormalLinkedDict(
            optional(2) StringEFormalLinkedDict inDict, out optional(3) StringEFormalLinkedDict outDict);

        optional(1) EStringSeqDict opOptEStringSeqDict(
            optional(2) EStringSeqDict inDict, out optional(3) EStringSeqDict outDict);
        optional(1) EStringSeqLinkedDict opOptEStringSeqLinkedDict(
            optional(2) EStringSeqLinkedDict inDict, out optional(3) EStringSeqLinkedDict outDict);
        optional(1) EStringSeqFormalLinkedDict opOptEStringSeqFormalLinkedDict(
            optional(2) EStringSeqFormalLinkedDict inDict, out optional(3) EStringSeqFormalLinkedDict outDict);

        ["marshaled-result"] ["java:type:java.util.LinkedHashMap<A, Integer>:java.util.LinkedHashMap<A, Integer>"]
        AIntDict opMAIntFormalLinkedDict(
            ["java:type:java.util.LinkedHashMap<A, Integer>:java.util.LinkedHashMap<A, Integer>"] AIntDict inDict,
            ["java:type:java.util.LinkedHashMap<A, Integer>:java.util.LinkedHashMap<A, Integer>"] out AIntDict outDict);

        ["marshaled-result"] ["java:type:java.util.LinkedHashMap<E, String[]>:java.util.LinkedHashMap<E, String[]>"]
        optional(1) EStringSeqDict opMOptEStringSeqFormalLinkedDict(
            ["java:type:java.util.LinkedHashMap<E, String[]>:java.util.LinkedHashMap<E, String[]>"] optional(2) EStringSeqDict inDict,
            ["java:type:java.util.LinkedHashMap<E, String[]>:java.util.LinkedHashMap<E, String[]>"] out optional(3) EStringSeqDict outDict);

        void shutdown();
    }
}

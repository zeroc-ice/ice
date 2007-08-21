// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE


module Test
{

dictionary<int, int> NV;
dictionary<string, string> NR;
dictionary<string, NV> NDV;
dictionary<string, NR> NDR;

["clr:DictionaryBase"] dictionary<int, int> OV;
["clr:DictionaryBase"] dictionary<string, string> OR;
["clr:DictionaryBase"] dictionary<string, OV> ODV;
["clr:DictionaryBase"] dictionary<string, OR> ODR;

dictionary<string, ODV> NODV;
dictionary<string, ODR> NODR;

["clr:DictionaryBase"] dictionary<string, NDV> ONDV;
["clr:DictionaryBase"] dictionary<string, NDR> ONDR;

["ami", "amd"] class MyClass
{
    void shutdown();

    NV opNV(NV i, out NV o);
    NR opNR(NR i, out NR o);
    NDV opNDV(NDV i, out NDV o);
    NDR opNDR(NDR i, out NDR o);

    OV opOV(OV i, out OV o);
    OR opOR(OR i, out OR o);
    ODV opODV(ODV i, out ODV o);
    ODR opODR(ODR i, out ODR o);

    NODV opNODV(NODV i, out NODV o);
    NODR opNODR(NODR i, out NODR o);

    ONDV opONDV(ONDV i, out ONDV o);
    ONDR opONDR(ONDR i, out ONDR o);
};

};

#endif

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::SeqMapping
{

sequence<byte> AByteS;
[cs:generic(List)] sequence<byte> LByteS;
[cs:generic(LinkedList)] sequence<byte> KByteS;
[cs:generic(Queue)] sequence<byte> QByteS;
[cs:generic(Stack)] sequence<byte> SByteS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<byte> CByteS;

sequence<bool> ABoolS;
[cs:generic(List)] sequence<bool> LBoolS;
[cs:generic(LinkedList)] sequence<bool> KBoolS;
[cs:generic(Queue)] sequence<bool> QBoolS;
[cs:generic(Stack)] sequence<bool> SBoolS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<bool> CBoolS;

sequence<short> AShortS;
[cs:generic(List)] sequence<short> LShortS;
[cs:generic(LinkedList)] sequence<short> KShortS;
[cs:generic(Queue)] sequence<short> QShortS;
[cs:generic(Stack)] sequence<short> SShortS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<short> CShortS;

sequence<int> AIntS;
[cs:generic(List)] sequence<int> LIntS;
[cs:generic(LinkedList)] sequence<int> KIntS;
[cs:generic(Queue)] sequence<int> QIntS;
[cs:generic(Stack)] sequence<int> SIntS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<int> CIntS;

sequence<long> ALongS;
[cs:generic(List)] sequence<long> LLongS;
[cs:generic(LinkedList)] sequence<long> KLongS;
[cs:generic(Queue)] sequence<long> QLongS;
[cs:generic(Stack)] sequence<long> SLongS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<long> CLongS;

sequence<float> AFloatS;
[cs:generic(List)] sequence<float> LFloatS;
[cs:generic(LinkedList)] sequence<float> KFloatS;
[cs:generic(Queue)] sequence<float> QFloatS;
[cs:generic(Stack)] sequence<float> SFloatS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<float> CFloatS;

sequence<double> ADoubleS;
[cs:generic(List)] sequence<double> LDoubleS;
[cs:generic(LinkedList)] sequence<double> KDoubleS;
[cs:generic(Queue)] sequence<double> QDoubleS;
[cs:generic(Stack)] sequence<double> SDoubleS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<double> CDoubleS;

sequence<string> AStringS;
[cs:generic(List)] sequence<string> LStringS;
[cs:generic(LinkedList)] sequence<string> KStringS;
[cs:generic(Queue)] sequence<string> QStringS;
[cs:generic(Stack)] sequence<string> SStringS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<string> CStringS;

sequence<Object> AObjectS;
[cs:generic(List)] sequence<Object> LObjectS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<Object> CObjectS;

sequence<Object*> AObjectPrxS;
[cs:generic(List)] sequence<Object*> LObjectPrxS;
[cs:generic(LinkedList)] sequence<Object*> KObjectPrxS;
[cs:generic(Queue)] sequence<Object*> QObjectPrxS;
[cs:generic(Stack)] sequence<Object*> SObjectPrxS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<Object*> CObjectPrxS;

struct S
{
    int i;
}

sequence<S> AStructS;
[cs:generic(List)] sequence<S> LStructS;
[cs:generic(LinkedList)] sequence<S> KStructS;
[cs:generic(Queue)] sequence<S> QStructS;
[cs:generic(Stack)] sequence<S> SStructS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<S> CStructS;

struct SD
{
    int i = 1;
}

sequence<SD> AStructSD;
[cs:generic(List)] sequence<SD> LStructSD;
[cs:generic(LinkedList)] sequence<SD> KStructSD;
[cs:generic(Queue)] sequence<SD> QStructSD;
[cs:generic(Stack)] sequence<SD> SStructSD;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<SD> CStructSD;

class CV
{
    int i;
}

sequence<CV> ACVS;
[cs:generic(List)] sequence<CV> LCVS;

interface I {}
sequence<I*> AIPrxS;
[cs:generic(List)] sequence<I*> LIPrxS;
[cs:generic(LinkedList)] sequence<I*> KIPrxS;
[cs:generic(Queue)] sequence<I*> QIPrxS;
[cs:generic(Stack)] sequence<I*> SIPrxS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<I*> CIPrxS;

class CR
{
    CV v;
}

sequence<CR> ACRS;
[cs:generic(List)] sequence<CR> LCRS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<CR> CCRS;

enum En { A, B, C }

sequence<En> AEnS;
[cs:generic(List)] sequence<En> LEnS;
[cs:generic(LinkedList)] sequence<En> KEnS;
[cs:generic(Queue)] sequence<En> QEnS;
[cs:generic(Stack)] sequence<En> SEnS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<En> CEnS;

[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<int> CustomIntS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<CV> CustomCVS;

[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<CustomIntS> CustomIntSS;
[cs:generic(ZeroC.Ice.Test.SeqMapping.Custom)] sequence<CustomCVS> CustomCVSS;

interface MyClass
{
    void shutdown();

    (AByteS r1, AByteS r2) opAByteS(AByteS i);
    (LByteS r1, LByteS r2) opLByteS(LByteS i);
    (KByteS r1, KByteS r2) opKByteS(KByteS i);
    (QByteS r1, QByteS r2) opQByteS(QByteS i);
    (SByteS r1, SByteS r2) opSByteS(SByteS i);
    (CByteS r1, CByteS r2) opCByteS(CByteS i);

    (ABoolS r1, ABoolS r2) opABoolS(ABoolS i);
    (LBoolS r1, LBoolS r2) opLBoolS(LBoolS i);
    (KBoolS r1, KBoolS r2) opKBoolS(KBoolS i);
    (QBoolS r1, QBoolS r2) opQBoolS(QBoolS i);
    (SBoolS r1, SBoolS r2) opSBoolS(SBoolS i);
    (CBoolS r1, CBoolS r2) opCBoolS(CBoolS i);

    (AShortS r1, AShortS r2) opAShortS(AShortS i);
    (LShortS r1, LShortS r2) opLShortS(LShortS i);
    (KShortS r1, KShortS r2) opKShortS(KShortS i);
    (QShortS r1, QShortS r2) opQShortS(QShortS i);
    (SShortS r1, SShortS r2) opSShortS(SShortS i);
    (CShortS r1, CShortS r2) opCShortS(CShortS i);

    (AIntS r1, AIntS r2) opAIntS(AIntS i);
    (LIntS r1, LIntS r2) opLIntS(LIntS i);
    (KIntS r1, KIntS r2) opKIntS(KIntS i);
    (QIntS r1, QIntS r2) opQIntS(QIntS i);
    (SIntS r1, SIntS r2) opSIntS(SIntS i);
    (CIntS r1, CIntS r2) opCIntS(CIntS i);

    (ALongS r1, ALongS r2) opALongS(ALongS i);
    (LLongS r1, LLongS r2) opLLongS(LLongS i);
    (KLongS r1, KLongS r2) opKLongS(KLongS i);
    (QLongS r1, QLongS r2) opQLongS(QLongS i);
    (SLongS r1, SLongS r2) opSLongS(SLongS i);
    (CLongS r1, CLongS r2) opCLongS(CLongS i);

    (AFloatS r1, AFloatS r2) opAFloatS(AFloatS i);
    (LFloatS r1, LFloatS r2) opLFloatS(LFloatS i);
    (KFloatS r1, KFloatS r2) opKFloatS(KFloatS i);
    (QFloatS r1, QFloatS r2) opQFloatS(QFloatS i);
    (SFloatS r1, SFloatS r2) opSFloatS(SFloatS i);
    (CFloatS r1, CFloatS r2) opCFloatS(CFloatS i);

    (ADoubleS r1, ADoubleS r2) opADoubleS(ADoubleS i);
    (LDoubleS r1, LDoubleS r2) opLDoubleS(LDoubleS i);
    (KDoubleS r1, KDoubleS r2) opKDoubleS(KDoubleS i);
    (QDoubleS r1, QDoubleS r2) opQDoubleS(QDoubleS i);
    (SDoubleS r1, SDoubleS r2) opSDoubleS(SDoubleS i);
    (CDoubleS r1, CDoubleS r2) opCDoubleS(CDoubleS i);

    (AStringS r1, AStringS r2) opAStringS(AStringS i);
    (LStringS r1, LStringS r2) opLStringS(LStringS i);
    (KStringS r1, KStringS r2) opKStringS(KStringS i);
    (QStringS r1, QStringS r2) opQStringS(QStringS i);
    (SStringS r1, SStringS r2) opSStringS(SStringS i);
    (CStringS r1, CStringS r2) opCStringS(CStringS i);

    (AObjectS r1, AObjectS r2) opAObjectS(AObjectS i);
    (LObjectS r1, LObjectS r2) opLObjectS(LObjectS i);
    (CObjectS r1, CObjectS r2) opCObjectS(CObjectS i);

    (AObjectPrxS r1, AObjectPrxS r2) opAObjectPrxS(AObjectPrxS i);
    (LObjectPrxS r1, LObjectPrxS r2) opLObjectPrxS(LObjectPrxS i);
    (KObjectPrxS r1, KObjectPrxS r2) opKObjectPrxS(KObjectPrxS i);
    (QObjectPrxS r1, QObjectPrxS r2) opQObjectPrxS(QObjectPrxS i);
    (SObjectPrxS r1, SObjectPrxS r2) opSObjectPrxS(SObjectPrxS i);
    (CObjectPrxS r1, CObjectPrxS r2) opCObjectPrxS(CObjectPrxS i);

    (AStructS r1, AStructS r2) opAStructS(AStructS i);
    (LStructS r1, LStructS r2) opLStructS(LStructS i);
    (KStructS r1, KStructS r2) opKStructS(KStructS i);
    (QStructS r1, QStructS r2) opQStructS(QStructS i);
    (SStructS r1, SStructS r2) opSStructS(SStructS i);
    (CStructS r1, CStructS r2) opCStructS(CStructS i);

    (AStructSD r1, AStructSD r2) opAStructSD(AStructSD i);
    (LStructSD r1, LStructSD r2) opLStructSD(LStructSD i);
    (KStructSD r1, KStructSD r2) opKStructSD(KStructSD i);
    (QStructSD r1, QStructSD r2) opQStructSD(QStructSD i);
    (SStructSD r1, SStructSD r2) opSStructSD(SStructSD i);
    (CStructSD r1, CStructSD r2) opCStructSD(CStructSD i);

    (ACVS r1, ACVS r2) opACVS(ACVS i);
    (LCVS r1, LCVS r2) opLCVS(LCVS i);

    (AIPrxS r1, AIPrxS r2) opAIPrxS(AIPrxS i);
    (LIPrxS r1, LIPrxS r2) opLIPrxS(LIPrxS i);
    (KIPrxS r1, KIPrxS r2) opKIPrxS(KIPrxS i);
    (QIPrxS r1, QIPrxS r2) opQIPrxS(QIPrxS i);
    (SIPrxS r1, SIPrxS r2) opSIPrxS(SIPrxS i);
    (CIPrxS r1, CIPrxS r2) opCIPrxS(CIPrxS i);

    (ACRS r1, ACRS r2) opACRS(ACRS i);
    (LCRS r1, LCRS r2) opLCRS(LCRS i);
    (CCRS r1, CCRS r2) opCCRS(CCRS i);

    (AEnS r1, AEnS r2) opAEnS(AEnS i);
    (LEnS r1, LEnS r2) opLEnS(LEnS i);
    (KEnS r1, KEnS r2) opKEnS(KEnS i);
    (QEnS r1, QEnS r2) opQEnS(QEnS i);
    (SEnS r1, SEnS r2) opSEnS(SEnS i);
    (CEnS r1, CEnS r2) opCEnS(CEnS i);

    (CustomIntS r1, CustomIntS r2) opCustomIntS(CustomIntS i);
    (CustomCVS r1, CustomCVS r2) opCustomCVS(CustomCVS i);

    (CustomIntSS r1, CustomIntSS r2) opCustomIntSS(CustomIntSS i);
    (CustomCVSS r1, CustomCVSS r2) opCustomCVSS(CustomCVSS i);
}

}

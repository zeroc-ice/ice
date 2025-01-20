// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Communicator.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

ShortSeq
TestIntfI::opShortArray(pair<const int16_t*, const int16_t*> inSeq, ShortSeq& outSeq, const Current&)
{
#ifdef ICE_UNALIGNED
    // Verify inSeq is not aligned and holds the expected values.
    test(reinterpret_cast<size_t>(inSeq.first) % sizeof(int16_t) != 0);
    for (int i = 0; i < inSeq.second - inSeq.first; ++i)
    {
        test(inSeq.first[i] == i + 1);
    }
#endif

    ShortSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

DoubleSeq
TestIntfI::opDoubleArray(bool, pair<const double*, const double*> inSeq, DoubleSeq& outSeq, const Current&)
{
#ifdef ICE_UNALIGNED
    // Verify inSeq is not aligned.
    test(reinterpret_cast<size_t>(inSeq.first) % sizeof(double) != 0);
    test(*(inSeq.first) == 3.14);
#endif

    DoubleSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

BoolSeq
TestIntfI::opBoolArray(pair<const bool*, const bool*> inSeq, BoolSeq& outSeq, const Current&)
{
    BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

ByteList
TestIntfI::opByteArray(pair<const byte*, const byte*> inSeq, ByteList& outSeq, const Current&)
{
    ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

VariableList
TestIntfI::opVariableArray(pair<const Variable*, const Variable*> inSeq, VariableList& outSeq, const Current&)
{
    VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

deque<bool>
TestIntfI::opBoolSeq(deque<bool> inSeq, deque<bool>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<bool>
TestIntfI::opBoolList(list<bool> inSeq, list<bool>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

BoolDequeList
TestIntfI::opBoolDequeList(BoolDequeList inSeq, BoolDequeList& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

BoolDequeList
TestIntfI::opBoolDequeListArray(
    pair<const deque<bool>*, const deque<bool>*> inSeq,
    BoolDequeList& outSeq,
    const Current&)
{
    for (const std::deque<bool>* p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    return outSeq;
}

deque<byte>
TestIntfI::opByteSeq(deque<byte> inSeq, deque<byte>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<byte>
TestIntfI::opByteList(list<byte> inSeq, list<byte>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

MyByteSeq
TestIntfI::opMyByteSeq(MyByteSeq inSeq, MyByteSeq& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<string>
TestIntfI::opStringSeq(deque<string> inSeq, deque<string>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<string>
TestIntfI::opStringList(list<string> inSeq, list<string>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<Test::Fixed>
TestIntfI::opFixedSeq(deque<Test::Fixed> inSeq, deque<Test::Fixed>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<Test::Fixed>
TestIntfI::opFixedList(list<Test::Fixed> inSeq, list<Test::Fixed>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<Variable>
TestIntfI::opVariableSeq(deque<Variable> inSeq, deque<Variable>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<Variable>
TestIntfI::opVariableList(list<Variable> inSeq, list<Variable>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<StringStringDict>
TestIntfI::opStringStringDictSeq(deque<StringStringDict> inSeq, deque<StringStringDict>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<StringStringDict>
TestIntfI::opStringStringDictList(list<StringStringDict> inSeq, list<StringStringDict>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<E>
TestIntfI::opESeq(deque<E> inSeq, deque<E>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<E>
TestIntfI::opEList(list<E> inSeq, list<E>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<optional<DPrx>>
TestIntfI::opDPrxSeq(deque<optional<DPrx>> inSeq, deque<optional<DPrx>>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

Test::DPrxList
TestIntfI::opDPrxList(Test::DPrxList inSeq, Test::DPrxList& outSeq, const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

deque<CPtr>
TestIntfI::opCSeq(deque<CPtr> inSeq, deque<CPtr>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

list<CPtr>
TestIntfI::opCList(list<CPtr> inSeq, list<CPtr>& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

void
TestIntfI::opOutArrayByteSeq(ByteSeq data, ByteSeq& copy, const Current&)
{
    copy = data;
}

IntStringDict
TestIntfI::opIntStringDict(IntStringDict data, IntStringDict& copy, const Current&)
{
    copy = data;
    return data;
}

CustomMap<int64_t, int64_t>
TestIntfI::opVarDict(CustomMap<string, int32_t> data, CustomMap<string, int32_t>& copy, const Current&)
{
    copy = data;

    CustomMap<int64_t, int64_t> result;
    for (int64_t i = 0; i < 1000; ++i)
    {
        result[i] = i * i;
    }
    return result;
}

ShortBuffer
TestIntfI::opShortBuffer(ShortBuffer inS, ShortBuffer& outS, const Current&)
{
    outS = inS;
    return outS;
}

CustomBuffer<bool>
TestIntfI::opBoolBuffer(CustomBuffer<bool> inS, CustomBuffer<bool>& outS, const Current&)
{
    outS = inS;
    return outS;
}

BufferStruct
TestIntfI::opBufferStruct(BufferStruct bs, const Current&)
{
    return bs;
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Communicator.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
TestIntfI::opShortArrayAsync(
    pair<const int16_t*, const int16_t*> inSeq,
    function<void(pair<const int16_t*, const int16_t*>, pair<const int16_t*, const int16_t*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
#ifdef ICE_UNALIGNED
    // Verify inSeq is not aligned and holds the expected values.
    test(reinterpret_cast<size_t>(inSeq.first) % sizeof(int16_t) != 0);
    for (int i = 0; i < static_cast<int>(inSeq.second - inSeq.first); ++i)
    {
        test(inSeq.first[i] == i + 1);
    }
#endif
    response(inSeq, inSeq);
}

void
TestIntfI::opDoubleArrayAsync(
    bool,
    pair<const double*, const double*> in,
    function<void(std::pair<const double*, const double*>, std::pair<const double*, const double*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
#ifdef ICE_UNALIGNED
    // Verify inSeq is not aligned.
    test(reinterpret_cast<size_t>(in.first) % sizeof(double) != 0);
    test(*(in.first) == 3.14);
#endif
    response(in, in);
}

void
TestIntfI::opBoolArrayAsync(
    pair<const bool*, const bool*> in,
    function<void(std::pair<const bool*, const bool*>, std::pair<const bool*, const bool*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opByteArrayAsync(
    pair<const byte*, const byte*> in,
    function<void(pair<const byte*, const byte*>, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableArrayAsync(
    pair<const Variable*, const Variable*> in,
    function<void(std::pair<const Variable*, const Variable*>, std::pair<const Variable*, const Variable*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolSeqAsync(
    deque<bool> in,
    function<void(const deque<bool>&, const deque<bool>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolListAsync(
    BoolList in,
    function<void(const BoolList&, const BoolList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolDequeListAsync(
    BoolDequeList in,
    function<void(const BoolDequeList&, const BoolDequeList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolDequeListArrayAsync(
    pair<const deque<bool>*, const deque<bool>*> inSeq,
    function<void(pair<const deque<bool>*, const deque<bool>*>, pair<const deque<bool>*, const deque<bool>*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(inSeq, inSeq);
}

void
TestIntfI::opByteSeqAsync(
    deque<byte> in,
    function<void(const deque<byte>&, const deque<byte>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opByteListAsync(
    ByteList in,
    function<void(const ByteList&, const ByteList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opMyByteSeqAsync(
    MyByteSeq in,
    function<void(const MyByteSeq&, const MyByteSeq&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opStringSeqAsync(
    deque<string> in,
    function<void(const deque<string>&, const deque<string>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opStringListAsync(
    StringList in,
    function<void(const StringList&, const StringList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opFixedSeqAsync(
    deque<Test::Fixed> in,
    function<void(const deque<Test::Fixed>&, const deque<Test::Fixed>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opFixedListAsync(
    FixedList in,
    function<void(const FixedList&, const FixedList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableSeqAsync(
    deque<Variable> in,
    function<void(const deque<Variable>&, const deque<Variable>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableListAsync(
    VariableList in,
    function<void(const VariableList&, const VariableList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opStringStringDictSeqAsync(
    deque<StringStringDict> in,
    function<void(const deque<StringStringDict>&, const deque<StringStringDict>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opStringStringDictListAsync(
    StringStringDictList in,
    function<void(const StringStringDictList&, const StringStringDictList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opESeqAsync(
    deque<E> in,
    function<void(const deque<E>&, const deque<E>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opEListAsync(
    EList in,
    function<void(const EList&, const EList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opDPrxSeqAsync(
    deque<optional<DPrx>> in,
    function<void(const deque<optional<DPrx>>&, const deque<optional<DPrx>>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opDPrxListAsync(
    DPrxList in,
    function<void(const DPrxList&, const DPrxList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opCSeqAsync(
    deque<shared_ptr<C>> in,
    function<void(const deque<shared_ptr<C>>&, const deque<shared_ptr<C>>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opCListAsync(
    CList in,
    function<void(const CList&, const CList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opOutArrayByteSeqAsync(
    ByteSeq in,
    function<void(pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(make_pair(in.data(), in.data() + in.size()));
}

void
TestIntfI::opIntStringDictAsync(
    IntStringDict in,
    function<void(const IntStringDict&, const IntStringDict&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opVarDictAsync(
    CustomMap<string, int32_t> in,
    function<void(const CustomMap<int64_t, int64_t>&, const CustomMap<string, int32_t>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    CustomMap<int64_t, int64_t> result;
    for (int64_t i = 0; i < 1000; ++i)
    {
        result[i] = i * i;
    }
    response(result, in);
}

void
TestIntfI::opShortBufferAsync(
    ShortBuffer in,
    function<void(const ShortBuffer&, const ShortBuffer&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolBufferAsync(
    CustomBuffer<bool> in,
    function<void(const CustomBuffer<bool>&, const CustomBuffer<bool>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opBufferStructAsync(
    BufferStruct in,
    function<void(const BufferStruct&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in);
}

void
TestIntfI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <TestAMDI.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
TestIntfI::opDoubleArrayAsync(
    pair<const double*, const double*> in,
    function<void(const DoubleSeq&, const DoubleSeq&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    DoubleSeq out(in.first, in.second);
    response(out, out);
}

void
TestIntfI::opBoolArrayAsync(
    pair<const bool*, const bool*> in,
    function<void(const BoolSeq&, const BoolSeq&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    BoolSeq out(in.first, in.second);
    response(out, out);
}

void
TestIntfI::opByteArrayAsync(
    pair<const uint8_t*, const uint8_t*> in,
    function<void(const pair<const uint8_t*, const uint8_t*>&, const pair<const uint8_t*, const uint8_t*>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableArrayAsync(
    pair<const Variable*, const Variable*> in,
    function<void(const VariableList&, const VariableList&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    VariableList out(in.first, in.second);
    response(out, out);
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
TestIntfI::opByteSeqAsync(
    deque<uint8_t> in,
    function<void(const deque<uint8_t>&, const deque<uint8_t>&)> response,
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
    deque<Fixed> in,
    function<void(const deque<Fixed>&, const deque<Fixed>&)> response,
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
    function<void(const pair<const uint8_t*, const uint8_t*>&)> response,
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

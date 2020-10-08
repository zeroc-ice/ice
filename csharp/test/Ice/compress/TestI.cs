// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.Ice.Test.Compress
{
    internal sealed class Interceptor : IObject
    {
        private readonly IObject _servant;
        private readonly bool _compressed;

        internal Interceptor(IObject servant, bool compressed)
        {
            _servant = servant;
            _compressed = compressed;
        }

        public async ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            if (current.Operation == "opCompressParams" || current.Operation == "opCompressParamsAndReturn")
            {
                if (request.Encoding == Encoding.V20)
                {
                    TestHelper.Assert(request.HasCompressedPayload == _compressed);
                    if (!_compressed)
                    {
                        // Ensure size is less than Ice.CompressionMinSize
                        TestHelper.Assert(request.Size < 1024);
                    }
                }
            }
            OutgoingResponseFrame response = await _servant.DispatchAsync(request, current, cancel);
            if (current.Operation == "opCompressReturn" || current.Operation == "opCompressParamsAndReturn")
            {
                if (response.Encoding == Encoding.V20)
                {
                    if (_compressed)
                    {
                        try
                        {
                            response.CompressPayload();
                            TestHelper.Assert(false);
                        }
                        catch (InvalidOperationException)
                        {
                            // Expected if the request is already compressed
                        }
                    }
                    else
                    {
                        // Ensure size is less than Ice.CompressionMinSize
                        TestHelper.Assert(response.Size < 1024);
                    }
                }
            }

            if (response.Encoding == Encoding.V20 && current.Operation == "opWithUserException")
            {
                response.CompressPayload();
            }
            return response;
        }
    }

    public sealed class TestIntf : ITestIntf
    {
        public void OpCompressParams(int size, byte[] p1, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(size == p1.Length);
            for (int i = 0; i < size; ++i)
            {
                TestHelper.Assert(p1[i] == (byte)i);
            }
        }

        public ReadOnlyMemory<byte> OpCompressParamsAndReturn(byte[] p1, Current current, CancellationToken cancel) =>
            p1;

        public ReadOnlyMemory<byte> OpCompressReturn(int size, Current current, CancellationToken cancel) =>
            Enumerable.Range(0, size).Select(i => (byte)i).ToArray();

        public void OpWithUserException(int size, Current current, CancellationToken cancel) =>
            throw new MyException(Enumerable.Range(0, size).Select(i => (byte)i).ToArray());

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();
    }
}

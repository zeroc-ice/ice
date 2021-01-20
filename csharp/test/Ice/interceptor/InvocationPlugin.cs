// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Immutable;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class InvocationPluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin(communicator);

        internal class Plugin : IPlugin
        {
            private Communicator _communicator;

            public Task ActivateAsync(CancellationToken cancel)
            {
                _communicator.DefaultInvocationInterceptors = ImmutableList.Create<InvocationInterceptor>(
                    async (target, request, next, cancel) =>
                    {
                        request.WritableContext["context1"] = "plug-in";
                        request.WritableContext["context2"] = "plug-in";
                        request.WritableContext["InvocationPlugin"] = "1";

                        IncomingResponseFrame response = await next(target, request, cancel);
                        if (response.Protocol == Protocol.Ice2 && response.ResultType == ResultType.Success)
                        {
                            TestHelper.Assert(
                                response.BinaryContext.TryGetValue(100, out ReadOnlyMemory<byte> value) &&
                                value.Read(istr => istr.ReadInt()) == 100);
                        }
                        return response;
                    }).AddRange(_communicator.DefaultInvocationInterceptors);

                return Task.CompletedTask;
            }

            internal Plugin(Communicator communicator) => _communicator = communicator;

            public ValueTask DisposeAsync() => default;
        }
    }
}

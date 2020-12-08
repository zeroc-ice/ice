// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class InvocationPluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin();

        internal class Plugin : IPlugin
        {
            public Task ActivateAsync(PluginActivationContext context, CancellationToken cancel)
            {
                context.AddInvocationInterceptor(
                    async (target, request, next, cancel) =>
                    {
                        if (request.Protocol == Protocol.Ice2)
                        {
                            request.ContextOverride["context1"] = "plug-in";
                            request.ContextOverride["context2"] = "plug-in";
                            request.ContextOverride["InvocationPlugin"] = "1";
                        }
                        IncomingResponseFrame response = await next(target, request, cancel);
                        if (response.Protocol == Protocol.Ice2 && response.ResultType == ResultType.Success)
                        {
                            TestHelper.Assert(
                                response.BinaryContext.TryGetValue(100, out ReadOnlyMemory<byte> value) &&
                                value.Read(istr => istr.ReadInt()) == 100);
                        }
                        return response;
                    });

                return Task.CompletedTask;
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class InvocationPluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin();

        internal class Plugin : IPlugin
        {
            public void Initialize(PluginInitializationContext context)
            {
                context.AddInvocationInterceptor(
                    async (target, request, next) =>
                    {
                        if (request.Protocol == Protocol.Ice2)
                        {
                            request.ContextOverride ??= new Dictionary<string, string>(request.Context);
                            request.ContextOverride["InvocationPlugin"] = "1";
                        }
                        IncomingResponseFrame response = await next(target, request);
                        if (response.Protocol == Protocol.Ice2 && response.ResultType == ResultType.Success)
                        {
                            TestHelper.Assert(
                                response.BinaryContext.TryGetValue(100, out ReadOnlyMemory<byte> value) &&
                                value.Read(istr => istr.ReadInt()) == 100);
                        }
                        return response;
                    });
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

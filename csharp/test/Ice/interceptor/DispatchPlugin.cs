// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Interceptor
{
    public class DispatchPluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin();

        internal class Plugin : IPlugin
        {
            public Task ActivateAsync(PluginActivationContext context, CancellationToken cancel)
            {
                context.AddDispatchInterceptor(
                    async (request, current, next, cancel) =>
                    {
                        current.Context["DispatchPlugin"] = "1";
                        OutgoingResponseFrame response = await next(request, current, cancel);
                        if (request.Protocol == Protocol.Ice2)
                        {
                            response.AddBinaryContextEntry(100, 100, (ostr, v) => ostr.WriteInt(v));
                        }
                        return response;
                    });

                return Task.CompletedTask;
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

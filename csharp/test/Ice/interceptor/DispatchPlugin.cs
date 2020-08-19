//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Interceptor
{
    public class DispatchPluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin();

        internal class Plugin : IPlugin
        {
            public void Initialize(PluginInitializationContext context)
            {
                context.AddDispatchInterceptor(
                    async (request, current, next) =>
                    {
                        current.Context["DispatchPlugin"] = "1";
                        OutgoingResponseFrame response = await next(request, current);
                        if (request.Protocol == Protocol.Ice2)
                        {
                            response.AddBinaryContextEntry(100, 100, (ostr, v) => ostr.WriteInt(v));
                        }
                        return response;
                    });
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

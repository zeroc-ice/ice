//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

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
                    (target, request, next) =>
                    {
                        request.Context["InvocationPlugin"] = "1";
                        return next(target, request);
                    });
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

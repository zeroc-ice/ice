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
                    (request, current, next) =>
                    {
                        current.Context["DispatchPlugin"] = "1";
                        return next(request, current);
                    });
            }

            public ValueTask DisposeAsync() => default;
        }
    }
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice
{
    namespace dictMapping
    {
        namespace AMD
        {
            public sealed class MyClassI : Test.MyClassDisp_
            {
                public override Task shutdownAsync(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                    return null;
                }

                public override Task<Test.MyClass_OpNVResult>
                opNVAsync(Dictionary<int, int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNVResult(i, i));
                }

                public override Task<Test.MyClass_OpNRResult>
                opNRAsync(Dictionary<string, string> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNRResult(i, i));
                }

                public override Task<Test.MyClass_OpNDVResult>
                opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDVResult(i, i));
                }

                public override Task<Test.MyClass_OpNDRResult>
                opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDRResult(i, i));
                }

                public override Task<Test.MyClass_OpNDAISResult>
                opNDAISAsync(Dictionary<string, int[]> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDAISResult(i, i));
                }

                public override Task<Test.MyClass_OpNDGISResult>
                opNDGISAsync(Dictionary<string, List<int>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDGISResult(i, i));
                }

                public override Task<Test.MyClass_OpNDASSResult>
                opNDASSAsync(Dictionary<string, string[]> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDASSResult(i, i));
                }

                public override Task<Test.MyClass_OpNDGSSResult>
                opNDGSSAsync(Dictionary<string, List<string>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpNDGSSResult(i, i));
                }
            }
        }
    }
}

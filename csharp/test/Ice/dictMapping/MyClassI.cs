//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace dictMapping
    {
        public sealed class MyClassI : Test.MyClass
        {
            public void shutdown(Current current) => current.Adapter.Communicator.shutdown();

            public Test.MyClass.OpNVReturnValue
            opNV(Dictionary<int, int> i, Current current) => new Test.MyClass.OpNVReturnValue(i, i);

            public Test.MyClass.OpNRReturnValue
            opNR(Dictionary<string, string> i, Current current) => new Test.MyClass.OpNRReturnValue(i, i);

            public Test.MyClass.OpNDVReturnValue
            opNDV(Dictionary<string, Dictionary<int, int>> i, Current current) => new Test.MyClass.OpNDVReturnValue(i, i);

            public Test.MyClass.OpNDRReturnValue
            opNDR(Dictionary<string, Dictionary<string, string>> i, Current current) => new Test.MyClass.OpNDRReturnValue(i, i);

            public Test.MyClass.OpNDAISReturnValue
            opNDAIS(Dictionary<string, int[]> i, Current current) => new Test.MyClass.OpNDAISReturnValue(i, i);

            public Test.MyClass.OpNDGISReturnValue
            opNDGIS(Dictionary<string, List<int>> i, Current current) => new Test.MyClass.OpNDGISReturnValue(i, i);

            public Test.MyClass.OpNDASSReturnValue
            opNDASS(Dictionary<string, string[]> i, Current current) => new Test.MyClass.OpNDASSReturnValue(i, i);

            public Test.MyClass.OpNDGSSReturnValue
            opNDGSS(Dictionary<string, List<string>> i, Current current) => new Test.MyClass.OpNDGSSReturnValue(i, i);
        }
    }
}

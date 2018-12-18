// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Slice.macros;

import test.Slice.macros.Test.Default;
import test.Slice.macros.Test.NoDefault;
import test.Slice.macros.Test.JavaOnly;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        System.out.print("testing Slice predefined macros... ");
        Default d = new Default();
        test(d.x == 10);
        test(d.y == 10);

        NoDefault nd = new NoDefault();
        test(nd.x != 10);
        test(nd.y != 10);

        JavaOnly c = new JavaOnly();
        test(c.lang.equals("java"));
        test(c.version == Ice.Util.intVersion());
        System.out.println("ok");
    }
}

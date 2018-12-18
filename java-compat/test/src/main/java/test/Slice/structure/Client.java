// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Slice.structure;

import test.Slice.structure.Test.S1;
import test.Slice.structure.Test.S2;
import test.Slice.structure.Test.C;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

public class Client extends test.TestHelper
{
    private static void allTests(Ice.Communicator communicator)
    {
        System.out.print("testing equals() for Slice structures... ");
        System.out.flush();

        //
        // Define some default values.
        //
        C def_cls = new C(5);
        S1 def_s = new S1("name");
        String[] def_ss = new String[]{ "one", "two", "three" };
        ArrayList<Integer> def_il = new ArrayList<Integer>();
        def_il.add(1);
        def_il.add(2);
        def_il.add(3);
        Map<String, String> def_sd = new HashMap<String, String>();
        def_sd.put("abc", "def");
        Ice.ObjectPrx def_prx = communicator.stringToProxy("test");
        S2 def_s2 = new S2(true, (byte)98, (short)99, 100, 101, (float)1.0, 2.0, "string", def_ss, def_il, def_sd,
                           def_s, def_cls, def_prx);

        //
        // Compare default-constructed structures.
        //
        {
            test(new S2().equals(new S2()));
        }

        //
        // Change one primitive member at a time.
        //
        {
            S2 v;

            v = def_s2.clone();
            test(v.equals(def_s2));

            v = def_s2.clone();
            v.bo = false;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.by--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.sh--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.i--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.l--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.f--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.d--;
            test(!v.equals(def_s2));

            v = def_s2.clone();
            v.str = "";
            test(!v.equals(def_s2));
        }

        //
        // String member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.str = new String(def_s2.str);
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.str = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.str = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.str = null;
            v2.str = null;
            test(v1.equals(v2));
        }

        //
        // Sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.ss = def_s2.ss.clone();
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.ss = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.ss = null;
            test(!v1.equals(v2));
        }

        //
        // Custom sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.il = new ArrayList<Integer>(def_s2.il);
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v1.il = new ArrayList<Integer>();
            test(!v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.il = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.il = null;
            test(!v1.equals(v2));
        }

        //
        // Dictionary member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.sd = new HashMap<String, String>(def_s2.sd);
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v1.sd = new HashMap<String, String>();
            test(!v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.sd = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.sd = null;
            test(!v1.equals(v2));
        }

        //
        // Struct member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.s = def_s2.s.clone();
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v1.s = new S1("name");
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v1.s = new S1("noname");
            test(!v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.s = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.s = null;
            test(!v1.equals(v2));
        }

        //
        // Class member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.cls = def_s2.cls.clone();
            test(!v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.cls = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.cls = null;
            test(!v1.equals(v2));
        }

        //
        // Proxy member
        //
        {
            S2 v1, v2;

            v1 = def_s2.clone();
            v1.prx = communicator.stringToProxy("test");
            test(v1.equals(def_s2));

            v1 = def_s2.clone();
            v1.prx = communicator.stringToProxy("test2");
            test(!v1.equals(def_s2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v1.prx = null;
            test(!v1.equals(v2));

            v1 = def_s2.clone();
            v2 = def_s2.clone();
            v2.prx = null;
            test(!v1.equals(v2));
        }

        System.out.println("ok");
    }

    public void run(String[] args)
    {
        try(Ice.Communicator communicator = initialize(args))
        {
            allTests(communicator);
        }
    }
}

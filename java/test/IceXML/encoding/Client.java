// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Client
{
    private static String header = "<ice:data xmlns=\"http://www.noorg.org/schemas\"" +
                                         " xmlns:ice=\"http://www.mutablerealms.com/schemas\"" +
                                         " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" +
                                         " xsi:schemaLocation=\"http://www.noorg.org/schemas Test.xsd\">";
    private static final String footer = "</ice:data>";

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException("Test failed!");
        }
    }

    private static void
    TestString(Ice.Communicator communicator)
    {
        final String element = "s";
        final String[] strings =
        {
            "hello world",
            "hello & world",
            "\"hello world\"",
            "'hello world'",
            "hello <world",
            "hello >world",
            "hello >>world",
            "hello <<>>world",
            "hello &&''\"\"<<>>world",
        };
        for(int i = 0; i < strings.length; ++i)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            pw.print("<data>");
            Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
            ostream.writeString(element, strings[i]);
            pw.print("</data>");
            pw.flush();

            java.io.StringReader sr = new java.io.StringReader(sw.toString());
            Ice.Stream istream = new IceXML.StreamI(communicator, sr);
            String result = istream.readString(element);
            test(result.equals(strings[i]));
        }
    }

    private static void
    TestStruct1(Ice.Communicator communicator)
    {
        final String element = "Test.Struct1";
        Test.Struct1 sin = new Test.Struct1();
        sin.l = 10;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        sin.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Struct1 sout = new Test.Struct1();
        sout.ice_unmarshal(element, istream);
        test(sin.equals(sout));
    }

    private static void
    TestStruct2(Ice.Communicator communicator)
    {
        final String element = "Test.Struct2";
        Test.Struct2 sin = new Test.Struct2();
        sin.s1 = new Test.Struct1();
        sin.s1.l = 10;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        sin.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Struct2 sout = new Test.Struct2();
        sout.ice_unmarshal(element, istream);
        test(sin.equals(sout));
    }

    private static void
    TestStruct3(Ice.Communicator communicator)
    {
        final String element = "Test.Struct3";
        Test.Struct3 sin = new Test.Struct3();
        sin.l = 20;
        sin.s2 = new Test.Struct2();
        sin.s2.s1 = new Test.Struct1();
        sin.s2.s1.l = 10;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        sin.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Struct3 sout = new Test.Struct3();
        sout.ice_unmarshal(element, istream);
        test(sin.equals(sout));
    }

    private static void
    TestStruct4(Ice.Communicator communicator)
    {
        final String element = "Test.Struct4";
        Test.Struct4 sin = new Test.Struct4();
        sin.l = 30;
        sin.s3 = new Test.Struct3();
        sin.s3.l = 20;
        sin.s3.s2 = new Test.Struct2();
        sin.s3.s2.s1 = new Test.Struct1();
        sin.s3.s2.s1.l = 10;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        sin.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Struct4 sout = new Test.Struct4();
        sout.ice_unmarshal(element, istream);
        test(sin.equals(sout));
    }

    private static void
    TestStruct4Seq(Ice.Communicator communicator)
    {
        final String element = "Test.Struct4Seq";
        Test.Struct4[] seqin = new Test.Struct4[2];

        Test.Struct4 sin = new Test.Struct4();
        sin.l = 30;
        sin.s3 = new Test.Struct3();
        sin.s3.l = 20;
        sin.s3.s2 = new Test.Struct2();
        sin.s3.s2.s1 = new Test.Struct1();
        sin.s3.s2.s1.l = 10;
        seqin[0] = sin;

        sin = new Test.Struct4();
        sin.l = 40;
        sin.s3 = new Test.Struct3();
        sin.s3.l = 30;
        sin.s3.s2 = new Test.Struct2();
        sin.s3.s2.s1 = new Test.Struct1();
        sin.s3.s2.s1.l = 20;
        seqin[1] = sin;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        Test.Struct4SeqHelper.ice_marshal(element, ostream, seqin);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Struct4[] seqout = Test.Struct4SeqHelper.ice_unmarshal(element, istream);
        test(seqout.length == seqin.length);
        for(int i = 0; i < seqin.length; i++)
        {
            test(seqin[i].equals(seqout[i]));
        }
    }

    private static void
    TestStringStruct4Dict(Ice.Communicator communicator)
    {
        final String element = "Test.StringStruct4Dict";
        java.util.Map dictin = new java.util.HashMap();

        Test.Struct4 sin = new Test.Struct4();
        sin.l = 30;
        sin.s3 = new Test.Struct3();
        sin.s3.l = 20;
        sin.s3.s2 = new Test.Struct2();
        sin.s3.s2.s1 = new Test.Struct1();
        sin.s3.s2.s1.l = 10;
        dictin.put("1", sin);

        sin = new Test.Struct4();
        sin.l = 40;
        sin.s3 = new Test.Struct3();
        sin.s3.l = 30;
        sin.s3.s2 = new Test.Struct2();
        sin.s3.s2.s1 = new Test.Struct1();
        sin.s3.s2.s1.l = 20;
        dictin.put("2", sin);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        Test.StringStruct4DictHelper.ice_marshal(element, ostream, dictin);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        java.util.Map dictout = Test.StringStruct4DictHelper.ice_unmarshal(element, istream);
        test(dictout.size() == dictin.size());
        java.util.Iterator p = dictin.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)p.next();
            Test.Struct4 val = (Test.Struct4)dictout.get(e.getKey());
            test(val != null);
            test(val.equals(e.getValue()));
        }
    }

    private static void
    TestStruct3Struct4Dict(Ice.Communicator communicator)
    {
        final String element = "Test.Struct3Struct4Dict";
        java.util.Map dictin = new java.util.HashMap();

        Test.Struct3 s3in = new Test.Struct3();
        s3in.l = 20;
        s3in.s2 = new Test.Struct2();
        s3in.s2.s1 = new Test.Struct1();
        s3in.s2.s1.l = 10;

        Test.Struct4 s4in = new Test.Struct4();
        s4in.l = 30;
        s4in.s3 = new Test.Struct3();
        s4in.s3.l = 20;
        s4in.s3.s2 = new Test.Struct2();
        s4in.s3.s2.s1 = new Test.Struct1();
        s4in.s3.s2.s1.l = 10;
        dictin.put(s3in, s4in);

        s3in = new Test.Struct3();
        s3in.l = 30;
        s3in.s2 = new Test.Struct2();
        s3in.s2.s1 = new Test.Struct1();
        s3in.s2.s1.l = 20;

        s4in = new Test.Struct4();
        s4in.l = 40;
        s4in.s3 = new Test.Struct3();
        s4in.s3.l = 30;
        s4in.s3.s2 = new Test.Struct2();
        s4in.s3.s2.s1 = new Test.Struct1();
        s4in.s3.s2.s1.l = 20;
        dictin.put(s3in, s4in);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        Test.Struct3Struct4DictHelper.ice_marshal(element, ostream, dictin);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        java.util.Map dictout = Test.Struct3Struct4DictHelper.ice_unmarshal(element, istream);
        test(dictout.size() == dictin.size());
        java.util.Iterator p = dictin.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)p.next();
            Test.Struct4 val = (Test.Struct4)dictout.get(e.getKey());
            test(val != null);
            test(val.equals(e.getValue()));
        }
    }

    private static void
    TestColor(Ice.Communicator communicator)
    {
        final String element = "Test.Color";
        Test.Color ein = Test.Color.Red;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        ein.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Color eout = Test.Color.ice_unmarshal(element, istream);
        test(ein == eout);
    }

    private static void
    TestColorSeq(Ice.Communicator communicator)
    {
        final String element = "Test.ColorSeq";
        Test.Color[] seqin = new Test.Color[3];

        seqin[0] = Test.Color.Red;
        seqin[1] = Test.Color.Green;
        seqin[2] = Test.Color.Blue;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        Test.ColorSeqHelper.ice_marshal(element, ostream, seqin);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Color[] seqout = Test.ColorSeqHelper.ice_unmarshal(element, istream);
        test(seqout.length == seqin.length);
        for(int i = 0; i < seqin.length; i++)
        {
            test(seqin[i] == seqout[i]);
        }
    }

    private static void
    TestClass1(Ice.Communicator communicator)
    {
        final String element = "Test.Class1";
        Test.Class1 in = new Test.Class1();
        in.c = Test.Color.Red;
        in.name = "Red";

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Class1 out = (Test.Class1)Test.Class1.ice_unmarshal(element, istream);
        test(in.c == out.c && in.name.equals(out.name));
    }

    private static void
    TestClass2(Ice.Communicator communicator)
    {
        final String element = "Test.Class2";
        Test.Class2 in = new Test.Class2();
        in.c = Test.Color.Blue;
        in.name = "Blue";

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Class2 out = (Test.Class2)Test.Class2.ice_unmarshal(element, istream);
        test(in.c == out.c && in.name.equals(out.name));
    }

    private static void
    TestClass2Rec(Ice.Communicator communicator)
    {
        final String element = "Test.Class2";
        Test.Class2 in = new Test.Class2();
        in.c = Test.Color.Blue;
        in.name = "Blue";
        Test.Class2 in2 = new Test.Class2();
        in2.c = Test.Color.Green;
        in2.name = "Green";
        in2.r = in;
        in.r = in2;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Class2 out = (Test.Class2)Test.Class2.ice_unmarshal(element, istream);
        test(out.r != null);
        test(out.r.r != null);
        test(in.c == out.c && in.name.equals(out.name));
        test(in.r.c == out.r.c && in.r.name.equals(out.r.name));
        test(in == in.r.r);
    }

    private static class Class3I extends Test.Class3
    {
        public void
        if1(Ice.Current current)
        {
        }
    }

    private static class Class3Factory implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            test(type.equals(Test.Class3.ice_staticId()));
            return new Class3I();
        }

        public void
        destroy()
        {
        }

    }

    private static void
    TestClass3(Ice.Communicator communicator)
    {
        final String element = "Test.Class3";
        Test.Class3 in = new Class3I();
        in.c = Test.Color.Blue;
        in.name = "Blue";

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Ice.Object o = Ice.Object.ice_unmarshal(element, istream);

        Test.Class3 out = (Test.Class3)o;
        test(in.c == out.c && in.name.equals(out.name));
    }

    private static void
    TestClass3Rec(Ice.Communicator communicator)
    {
        final String element = "Test.Class3";
        Test.Class3 in = new Class3I();
        in.c = Test.Color.Blue;
        in.name = "Blue";
        Test.Class3 in2 = new Class3I();
        in2.c = Test.Color.Green;
        in2.name = "Green";
        in2.r = in;
        in.r = in2;

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Ice.Object o = Ice.Object.ice_unmarshal(element, istream);
        Test.Class3 out = (Test.Class3)o;
        test(out.r != null);
        test(out.r.r != null);
        test(in.c == out.c && in.name.equals(out.name));
        test(in.r.c == out.r.c && in.r.name.equals(out.r.name));
        test(in == in.r.r);
    }

    private static void
    TestException1(Ice.Communicator communicator)
    {
        final String element = "Test.Exception1";
        Test.Exception1 in = new Test.Exception1();

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Exception1 out = new Test.Exception1();
        out.ice_unmarshal(element, istream);
    }

    private static void
    TestException2(Ice.Communicator communicator)
    {
        final String element = "Test.Exception2";
        Test.Exception2 in = new Test.Exception2();
        in.msg = "hello world";

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        pw.print(header);
        Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
        in.ice_marshal(element, ostream);
        pw.print(footer);
        pw.flush();

        java.io.StringReader sr = new java.io.StringReader(sw.toString());
        Ice.Stream istream = new IceXML.StreamI(communicator, sr);
        Test.Exception2 out = new Test.Exception2();
        out.ice_unmarshal(element, istream);
        test(in.msg.equals(out.msg));
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        //
        // If the test is run out of the CWD then adjust the location of
        // Test.xsd appropriately.
        //
        if(args.length > 0)
        {
            int pos = header.indexOf("Test.xsd");
            assert(pos != -1);
            header = header.substring(0, pos) + args[0] + "/" + header.substring(pos);
        }

        System.out.print("testing string... ");
        System.out.flush();
        TestString(communicator);
        System.out.println("ok");

        System.out.print("testing struct... ");
        System.out.flush();
        TestStruct1(communicator);
        TestStruct2(communicator);
        TestStruct3(communicator);
        TestStruct4(communicator);
        System.out.println("ok");

        System.out.print("testing sequence... ");
        System.out.flush();
        TestStruct4Seq(communicator);
        System.out.println("ok");

        System.out.print("testing dictionaries... ");
        System.out.flush();
        TestStringStruct4Dict(communicator);
        TestStruct3Struct4Dict(communicator);
        System.out.println("ok");

        System.out.print("testing enumerations... ");
        System.out.flush();
        TestColor(communicator);
        TestColorSeq(communicator);
        System.out.println("ok");

        System.out.print("testing class... ");
        System.out.flush();
        TestClass1(communicator);
        TestClass2(communicator);
        TestClass2Rec(communicator);
        try
        {
            TestClass3(communicator);
            test(false);
        }
        catch(Ice.NoObjectFactoryException ex)
        {
        }

        communicator.addObjectFactory(new Class3Factory(), Test.Class3.ice_staticId());

        TestClass3(communicator);
        TestClass3Rec(communicator);
        System.out.println("ok");

        System.out.print("testing exception... ");
        System.out.flush();
        TestException1(communicator);
        TestException2(communicator);
        System.out.println("ok");

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}

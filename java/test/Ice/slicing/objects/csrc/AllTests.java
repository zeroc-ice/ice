// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "Test:default -p 12345 -t 2000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestPrx test = TestPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        System.out.println("ok");

	System.out.print("testing base as Object... ");
	System.out.flush();
	{
	    Ice.Object o;
	    SBase sb = null;
	    try
	    {
		o = test.SBaseAsObject();
		test(o != null);
		test(o.ice_id(null).equals("::SBase"));
		sb = (SBase)o;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(sb != null);
	    test(sb.sb.equals("SBase.sb"));
	}
	System.out.println("ok");

	System.out.print("testing base as base... ");
	System.out.flush();
	{
	    SBase sb;
	    try
	    {
		sb = test.SBaseAsSBase();
		test(sb.sb.equals("SBase.sb"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing base with known derived as base... ");
	System.out.flush();
	{
	    SBase sb;
	    SBSKnownDerived sbskd = null;
	    try
	    {
		sb = test.SBSKnownDerivedAsSBase();
		test(sb.sb.equals("SBSKnownDerived.sb"));
		sbskd = (SBSKnownDerived)sb;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(sbskd != null);
	    test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
	}
	System.out.println("ok");

	System.out.print("testing base with known derived as known derived... ");
	System.out.flush();
	{
	    SBSKnownDerived sbskd;
	    try
	    {
		sbskd = test.SBSKnownDerivedAsSBSKnownDerived();
		test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing base with unknown derived as base... ");
	System.out.flush();
	{
	    SBase sb;
	    try
	    {
		sb = test.SBSUnknownDerivedAsSBase();
		test(sb.sb.equals("SBSUnknownDerived.sb"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing unknown with Object as Object... ");
	System.out.flush();
	{
	    Ice.Object o;
	    try
	    {
		o = test.SUnknownAsObject();
		test(o.ice_id(null).equals("::Ice::Object"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing one-element cycle... ");
	System.out.flush();
	{
	    try
	    {
		B b = test.oneElementCycle();
		test(b != null);
		test(b.ice_id(null).equals("::B"));
		test(b.sb.equals("B1.sb"));
		test(b.pb == b);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing two-element cycle... ");
	System.out.flush();
	{
	    try
	    {
		B b1 = test.twoElementCycle();
		test(b1 != null);
		test(b1.ice_id(null).equals("::B"));
		test(b1.sb.equals("B1.sb"));

		B b2 = b1.pb;
		test(b2 != null);
		test(b2.ice_id(null).equals("::B"));
		test(b2.sb.equals("B2.sb"));
		test(b2.pb == b1);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing known derived pointer slicing as base... ");
	System.out.flush();
	{
	    try
	    {
		B b1;
		b1 = test.D1AsB();
		test(b1 != null);
		test(b1.ice_id(null).equals("::D1"));
		test(b1.sb.equals("D1.sb"));
		test(b1.pb != null);
		test(b1.pb != b1);
		D1 d1 = (D1)b1;
		test(d1 != null);
		test(d1.sd1.equals("D1.sd1"));
		test(d1.pd1 != null);
		test(d1.pd1 != b1);
		test(b1.pb == d1.pd1);

		B b2 = b1.pb;
		test(b2 != null);
		test(b2.pb == b1);
		test(b2.sb.equals("D2.sb"));
		test(b2.ice_id(null).equals("::B"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing known derived pointer slicing as derived... ");
	System.out.flush();
	{
	    try
	    {
		D1 d1;
		d1 = test.D1AsD1();
		test(d1 != null);
		test(d1.ice_id(null).equals("::D1"));
		test(d1.sb.equals("D1.sb"));
		test(d1.pb != null);
		test(d1.pb != d1);

		B b2 = d1.pb;
		test(b2 != null);
		test(b2.ice_id(null).equals("::B"));
		test(b2.sb.equals("D2.sb"));
		test(b2.pb == d1);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing unknown derived pointer slicing as base... ");
	System.out.flush();
	{
	    try
	    {
		B b2;
		b2 = test.D2AsB();
		test(b2 != null);
		test(b2.ice_id(null).equals("::B"));
		test(b2.sb.equals("D2.sb"));
		test(b2.pb != null);
		test(b2.pb != b2);

		B b1 = b2.pb;
		test(b1 != null);
		test(b1.ice_id(null).equals("::D1"));
		test(b1.sb.equals("D1.sb"));
		test(b1.pb == b2);
		D1 d1 = (D1)b1;
		test(d1 != null);
		test(d1.sd1.equals("D1.sd1"));
		test(d1.pd1 == b2);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing parameter pointer slicing with known first... ");
	System.out.flush();
	{
	    try
	    {
		BHolder b1 = new BHolder();
		BHolder b2 = new BHolder();
		test.paramTest1(b1, b2);

		test(b1.value != null);
		test(b1.value.ice_id(null).equals("::D1"));
		test(b1.value.sb.equals("D1.sb"));
		test(b1.value.pb == b2.value);
		D1 d1 = (D1)b1.value;
		test(d1 != null);
		test(d1.sd1.equals("D1.sd1"));
		test(d1.pd1 == b2.value);

		test(b2.value != null);
		test(b2.value.ice_id(null).equals("::B"));	// No factory, must be sliced
		test(b2.value.sb.equals("D2.sb"));
		test(b2.value.pb == b1.value);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing parameter pointer slicing with unknown first... ");
	System.out.flush();
	{
	    try
	    {
		BHolder b2 = new BHolder();
		BHolder b1 = new BHolder();
		test.paramTest2(b2, b1);

		test(b1.value != null);
		test(b1.value.ice_id(null).equals("::D1"));
		test(b1.value.sb.equals("D1.sb"));
		test(b1.value.pb == b2.value);
		D1 d1 = (D1)b1.value;
		test(d1 != null);
		test(d1.sd1.equals("D1.sd1"));
		test(d1.pd1 == b2.value);

		test(b2.value != null);
		test(b2.value.ice_id(null).equals("::B"));	// No factory, must be sliced
		test(b2.value.sb.equals("D2.sb"));
		test(b2.value.pb == b1.value);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing return value identity with known first... ");
	System.out.flush();
	{
	    try
	    {
		BHolder p1 = new BHolder();
		BHolder p2 = new BHolder();
		B r = test.returnTest1(p1, p2);
		test(r == p1.value);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing return value identity with unknown first... ");
	System.out.flush();
	{
	    try
	    {
		BHolder p1 = new BHolder();
		BHolder p2 = new BHolder();
		B r = test.returnTest2(p1, p2);
		test(r == p1.value);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing return value identity for input params known first... ");
	System.out.flush();
	{
	    try
	    {
		D1 d1 = new D1();
		d1.sb = "D1.sb";
		d1.sd1 = "D1.sd1";
		D3 d3 = new D3();
		d3.pb = d1;
		d3.sb = "D3.sb";
		d3.sd3 = "D3.sd3";
		d3.pd3 = d1;
		d1.pb = d3;
		d1.pd1 = d3;

		B b1 = test.returnTest3(d1, d3);

		test(b1 != null);
		test(b1.sb.equals("D1.sb"));
		test(b1.ice_id(null).equals("::D1"));
		D1 p1 = (D1)b1;
		test(p1 != null);
		test(p1.sd1.equals("D1.sd1"));
		test(p1.pd1 == b1.pb);

		B b2 = b1.pb;
		test(b2 != null);
		test(b2.sb.equals("D3.sb"));
		test(b2.ice_id(null).equals("::B"));	// Sliced by server
		test(b2.pb == b1);
		boolean gotException = false;
		try
		{
		    D3 p3 = (D3)b2;
		}
		catch(ClassCastException ex)
		{
		    gotException = true;
		}
		test(gotException);

		test(b1 != d1);
		test(b1 != d3);
		test(b2 != d1);
		test(b2 != d3);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing return value identity for input params unknown first... ");
	System.out.flush();
	{
	    try
	    {
		D1 d1 = new D1();
		d1.sb = "D1.sb";
		d1.sd1 = "D1.sd1";
		D3 d3 = new D3();
		d3.pb = d1;
		d3.sb = "D3.sb";
		d3.sd3 = "D3.sd3";
		d3.pd3 = d1;
		d1.pb = d3;
		d1.pd1 = d3;

		B b1 = test.returnTest3(d3, d1);

		test(b1 != null);
		test(b1.sb.equals("D3.sb"));
		test(b1.ice_id(null).equals("::B"));	// Sliced by server

		boolean gotException = false;
		try
		{
		    D3 p1 = (D3)b1;
		}
		catch(ClassCastException ex)
		{
		    gotException = true;
		}
		test(gotException);

		B b2 = b1.pb;
		test(b2 != null);
		test(b2.sb.equals("D1.sb"));
		test(b2.ice_id(null).equals("::D1"));
		test(b2.pb == b1);
		D1 p3 = (D1)b2;
		test(p3 != null);
		test(p3.sd1.equals("D1.sd1"));
		test(p3.pd1 == b1);

		test(b1 != d1);
		test(b1 != d3);
		test(b2 != d1);
		test(b2 != d3);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing remainder unmarshaling (3 instances)... ");
	System.out.flush();
	{
	    try
	    {
		BHolder p1 = new BHolder();
		BHolder p2 = new BHolder();
		B ret = test.paramTest3(p1, p2);

		test(p1.value != null);
		test(p1.value.sb.equals("D2.sb (p1 1)"));
		test(p1.value.pb == null);
		test(p1.value.ice_id(null).equals("::B"));

		test(p2.value != null);
		test(p2.value.sb.equals("D2.sb (p2 1)"));
		test(p2.value.pb == null);
		test(p2.value.ice_id(null).equals("::B"));

		test(ret != null);
		test(ret.sb.equals("D1.sb (p2 2)"));
		test(ret.pb == null);
		test(ret.ice_id(null).equals("::D1"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing remainder unmarshaling (4 instances)... ");
	System.out.flush();
	{
	    try
	    {
		BHolder b = new BHolder();
		B ret = test.paramTest4(b);

		test(b.value != null);
		test(b.value.sb.equals("D4.sb (1)"));
		test(b.value.pb == null);
		test(b.value.ice_id(null).equals("::B"));

		test(ret != null);
		test(ret.sb.equals("B.sb (2)"));
		test(ret.pb == null);
		test(ret.ice_id(null).equals("::B"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing parameter pointer slicing with first instance marshaled in"
	                 + " unknown derived as base... ");
	System.out.flush();
	{
	    try
	    {
		B b1 = new B();
		b1.sb = "B.sb(1)";
		b1.pb = b1;

		D3 d3 = new D3();
		d3.sb = "D3.sb";
		d3.pb = d3;
		d3.sd3 = "D3.sd3";
		d3.pd3 = b1;

		B b2 = new B();
		b2.sb = "B.sb(2)";
		b2.pb = b1;

		B r = test.returnTest3(d3, b2);

		test(r != null);
		test(r.ice_id(null).equals("::B"));
		test(r.sb.equals("D3.sb"));
		test(r.pb == r);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing parameter pointer slicing with first instance marshaled in"
	                 + " unknown derived as derived... ");
	System.out.flush();
	{
	    try
	    {
		D1 d11 = new D1();
		d11.sb = "D1.sb(1)";
		d11.pb = d11;
		d11.sd1 = "D1.sd1(1)";

		D3 d3 = new D3();
		d3.sb = "D3.sb";
		d3.pb = d3;
		d3.sd3 = "D3.sd3";
		d3.pd3 = d11;

		D1 d12 = new D1();
		d12.sb = "D1.sb(2)";
		d12.pb = d12;
		d12.sd1 = "D1.sd1(2)";
		d12.pd1 = d11;

		B r = test.returnTest3(d3, d12);
		test(r != null);
		test(r.ice_id(null).equals("::B"));
		test(r.sb.equals("D3.sb"));
		test(r.pb == r);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing sequence slicing... ");
	System.out.flush();
	{
	    try
	    {
		SS ss;
		{
		    B ss1b = new B();
		    ss1b.sb = "B.sb";
		    ss1b.pb = ss1b;

		    D1 ss1d1 = new D1();
		    ss1d1.sb = "D1.sb";
		    ss1d1.sd1 = "D1.sd1";
		    ss1d1.pb = ss1b;

		    D3 ss1d3 = new D3();
		    ss1d3.sb = "D3.sb";
		    ss1d3.sd3 = "D3.sd3";
		    ss1d3.pb = ss1b;

		    B ss2b = new B();
		    ss2b.sb = "B.sb";
		    ss2b.pb = ss1b;

		    D1 ss2d1 = new D1();
		    ss2d1.sb = "D1.sb";
		    ss2d1.sd1 = "D1.sd1";
		    ss2d1.pb = ss2b;

		    D3 ss2d3 = new D3();
		    ss2d3.sb = "D3.sb";
		    ss2d3.sd3 = "D3.sd3";
		    ss2d3.pb = ss2b;

		    ss1d1.pd1 = ss2b;
		    ss1d3.pd3 = ss2d1;

		    ss2d1.pd1 = ss1d3;
		    ss2d3.pd3 = ss1d1;

		    SS1 ss1 = new SS1();
		    ss1.s = new B[3];
		    ss1.s[0] = ss1b;
		    ss1.s[1] = ss1d1;
		    ss1.s[2] = ss1d3;

		    SS2 ss2 = new SS2();
		    ss2.s = new B[3];
		    ss2.s[0] = ss2b;
		    ss2.s[1] = ss2d1;
		    ss2.s[2] = ss2d3;

		    ss = test.sequenceTest(ss1, ss2);
		}

		test(ss.c1 != null);
		B ss1b = ss.c1.s[0];
		B ss1d1 = ss.c1.s[1];
		test(ss.c2 != null);
		B ss1d3 = ss.c1.s[2];

		test(ss.c2 != null);
		B ss2b = ss.c2.s[0];
		B ss2d1 = ss.c2.s[1];
		B ss2d3 = ss.c2.s[2];

		test(ss1b.pb == ss1b);
		test(ss1d1.pb == ss1b);
		test(ss1d3.pb == ss1b);

		test(ss2b.pb == ss1b);
		test(ss2d1.pb == ss2b);
		test(ss2d3.pb == ss2b);

		test(ss1b.ice_id(null).equals("::B"));
		test(ss1d1.ice_id(null).equals("::D1"));
		test(ss1d3.ice_id(null).equals("::B"));

		test(ss2b.ice_id(null).equals("::B"));
		test(ss2d1.ice_id(null).equals("::D1"));
		test(ss2d3.ice_id(null).equals("::B"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing base exception thrown as base exception... ");
	System.out.flush();
	{
	    try
	    {
		test.throwBaseAsBase();
		test(false);
	    }
	    catch(BaseException e)
	    {
		test(e.ice_name().equals("BaseException"));
		test(e.sbe.equals("sbe"));
		test(e.pb != null);
		test(e.pb.sb.equals("sb"));
		test(e.pb.pb == e.pb);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing derived exception thrown as base exception... ");
	System.out.flush();
	{
	    try
	    {
		test.throwDerivedAsBase();
		test(false);
	    }
	    catch(DerivedException e)
	    {
		test(e.ice_name().equals("DerivedException"));
		test(e.sbe.equals("sbe"));
		test(e.pb != null);
		test(e.pb.sb.equals("sb1"));
		test(e.pb.pb == e.pb);
		test(e.sde.equals("sde1"));
		test(e.pd1 != null);
		test(e.pd1.sb.equals("sb2"));
		test(e.pd1.pb == e.pd1);
		test(e.pd1.sd1.equals("sd2"));
		test(e.pd1.pd1 == e.pd1);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing derived exception thrown as derived exception... ");
	System.out.flush();
	{
	    try
	    {
		test.throwDerivedAsDerived();
		test(false);
	    }
	    catch(DerivedException e)
	    {
		test(e.ice_name().equals("DerivedException"));
		test(e.sbe.equals("sbe"));
		test(e.pb != null);
		test(e.pb.sb.equals("sb1"));
		test(e.pb.pb == e.pb);
		test(e.sde.equals("sde1"));
		test(e.pd1 != null);
		test(e.pd1.sb.equals("sb2"));
		test(e.pd1.pb == e.pd1);
		test(e.pd1.sd1.equals("sd2"));
		test(e.pd1.pd1 == e.pd1);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing unknown derived exception thrown as base exception... ");
	System.out.flush();
	{
	    try
	    {
		test.throwUnknownDerivedAsBase();
		test(false);
	    }
	    catch(BaseException e)
	    {
		test(e.ice_name().equals("BaseException"));
		test(e.sbe.equals("sbe"));
		test(e.pb != null);
		test(e.pb.sb.equals("sb d2"));
		test(e.pb.pb == e.pb);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("testing forward-declared class... ");
	System.out.flush();
	{
	    try
	    {
		ForwardHolder f = new ForwardHolder();
		test.useForward(f);
		test(f.value != null);
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

        return test;
    }
}

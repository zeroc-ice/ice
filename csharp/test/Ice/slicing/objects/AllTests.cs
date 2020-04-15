//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using Ice;

namespace Test
{
    public partial class PNode
    {
        partial void Initialize()
        {
            ++counter;
        }
        internal static int counter = 0;
    }

    public partial class Preserved
    {
        partial void Initialize()
        {
            ++counter;
        }
        internal static int counter = 0;
    }
}

public class AllTests
{
    public static ITestIntfPrx allTests(TestHelper helper, bool collocated)
    {
        Communicator? communicator = helper.Communicator();
        TestHelper.Assert(communicator != null);
        var output = helper.GetWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        var testPrx = ITestIntfPrx.Parse($"Test:{helper.GetTestEndpoint(0)} -t 2000", communicator);
        var test2Prx = ITestIntf2Prx.Parse($"Test2:{helper.GetTestEndpoint(0)} -t 2000", communicator);
        output.WriteLine("ok");

        output.Write("testing Ice.Default.SlicedFormat... ");
        // server to client. Note that client and server has the same Ice.Default.SlicedFormat setting.
        try
        {
            SBase? sb = test2Prx.SBSUnknownDerivedAsSBase();
            TestHelper.Assert(sb != null && sb.sb.Equals("SBSUnknownDerived.sb"));
            TestHelper.Assert(communicator.DefaultFormat == FormatType.Sliced);
        }
        catch (InvalidDataException)
        {
            // Expected when format is Compact
            TestHelper.Assert(communicator.DefaultFormat == FormatType.Compact);
        }
        catch (Exception ex)
        {
            output.WriteLine(ex.ToString());
            TestHelper.Assert(false);
        }

        // client to server
        try
        {
            test2Prx.CUnknownAsSBase(new CUnknown("CUnknown.sb", "CUnknown.cu"));
            TestHelper.Assert(communicator.DefaultFormat == FormatType.Sliced);
        }
        catch (UnhandledException)
        {
            // Expected when format is Compact
            TestHelper.Assert(communicator.DefaultFormat == FormatType.Compact);
        }
        catch (Exception ex)
        {
            output.WriteLine(ex.ToString());
            TestHelper.Assert(false);
        }
        output.WriteLine("ok");

        output.Write("base as Object... ");
        output.Flush();
        {
            AnyClass? o;
            SBase? sb = null;
            try
            {
                o = testPrx.SBaseAsObject();
                TestHelper.Assert(o != null);
                TestHelper.Assert(TypeExtensions.GetIceTypeId(o.GetType())!.Equals("::Test::SBase"));
                sb = (SBase)o;
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
            TestHelper.Assert(sb != null && sb.sb.Equals("SBase.sb"));
        }
        output.WriteLine("ok");

        output.Write("base as Object (AMI)... ");
        output.Flush();
        {
            AnyClass? o = testPrx.SBaseAsObjectAsync().Result;
            TestHelper.Assert(o != null);
            TestHelper.Assert(o.GetType().GetIceTypeId()!.Equals("::Test::SBase"));
            var sb = (SBase)o;
            TestHelper.Assert(sb != null);
            TestHelper.Assert(sb.sb.Equals("SBase.sb"));
        }
        output.WriteLine("ok");

        output.Write("base as base... ");
        output.Flush();
        {
            SBase? sb;
            try
            {
                sb = testPrx.SBaseAsSBase();
                TestHelper.Assert(sb != null && sb.sb.Equals("SBase.sb"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("base as base (AMI)... ");
        output.Flush();
        {
            SBase? sb = testPrx.SBaseAsSBaseAsync().Result;
            TestHelper.Assert(sb != null && sb.sb.Equals("SBase.sb"));
        }
        output.WriteLine("ok");

        output.Write("base with known derived as base... ");
        output.Flush();
        {
            SBase? sb;
            SBSKnownDerived? sbskd = null;
            try
            {
                sb = testPrx.SBSKnownDerivedAsSBase();
                TestHelper.Assert(sb != null && sb.sb.Equals("SBSKnownDerived.sb"));
                sbskd = (SBSKnownDerived)sb;
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
            TestHelper.Assert(sbskd != null && sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
        }
        output.WriteLine("ok");

        output.Write("base with known derived as base (AMI)... ");
        output.Flush();
        {
            SBase? sb = testPrx.SBSKnownDerivedAsSBaseAsync().Result;
            TestHelper.Assert(sb != null && sb.sb.Equals("SBSKnownDerived.sb"));
            SBSKnownDerived sbskd = (SBSKnownDerived)sb;
            TestHelper.Assert(sbskd != null);
            TestHelper.Assert(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
        }
        output.WriteLine("ok");

        output.Write("base with known derived as known derived... ");
        output.Flush();
        {
            SBSKnownDerived? sbskd;
            try
            {
                sbskd = testPrx.SBSKnownDerivedAsSBSKnownDerived();
                TestHelper.Assert(sbskd != null && sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("base with known derived as known derived (AMI)... ");
        output.Flush();
        {
            SBSKnownDerived? sbskd = testPrx.SBSKnownDerivedAsSBSKnownDerivedAsync().Result;
            TestHelper.Assert(sbskd != null && sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
        }
        output.WriteLine("ok");

        output.Write("base with unknown derived as base... ");
        output.Flush();
        {
            SBase? sb;
            try
            {
                sb = testPrx.SBSUnknownDerivedAsSBase();
                TestHelper.Assert(sb != null && sb.sb.Equals("SBSUnknownDerived.sb"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        try
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            testPrx.SBSUnknownDerivedAsSBaseCompact();
            TestHelper.Assert(false);
        }
        catch (InvalidDataException ex)
        {
            TestHelper.Assert(ex.Message.Contains("::Test::SBSUnknownDerived"));
        }
        catch (Exception ex)
        {
            output.WriteLine(ex.ToString());
            TestHelper.Assert(false);
        }
        output.WriteLine("ok");

        output.Write("base with unknown derived as base (AMI)... ");
        output.Flush();
        {
            SBase? sb = testPrx.SBSUnknownDerivedAsSBaseAsync().Result;
            TestHelper.Assert(sb != null && sb.sb.Equals("SBSUnknownDerived.sb"));
        }

        //
        // This test fails when using the compact format because the instance cannot
        // be sliced to a known type.
        //
        try
        {
            SBase? sb = testPrx.SBSUnknownDerivedAsSBaseCompactAsync().Result;
        }
        catch (AggregateException ae)
        {
            TestHelper.Assert(ae.InnerException is Ice.InvalidDataException);
        }
        output.WriteLine("ok");

        output.Write("unknown with Object as Object... ");
        output.Flush();
        {
            try
            {
                AnyClass? o = testPrx.SUnknownAsObject();
                var unknown = o as UnknownSlicedClass;
                TestHelper.Assert(unknown != null);
                TestHelper.Assert(unknown.TypeId!.Equals("::Test::SUnknown"));
                TestHelper.Assert(unknown.GetSlicedData() != null);
                testPrx.checkSUnknown(o);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("unknown with Object as Object (AMI)... ");
        output.Flush();
        {
            try
            {
                try
                {
                    var o = testPrx.SUnknownAsObjectAsync().Result;
                    var unknown = o as UnknownSlicedClass;
                    TestHelper.Assert(unknown != null);
                    TestHelper.Assert(unknown.TypeId!.Equals("::Test::SUnknown"));
                }
                catch (AggregateException ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
            catch (System.Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("one-element cycle... ");
        output.Flush();
        {
            try
            {
                B? b = testPrx.oneElementCycle();
                TestHelper.Assert(b != null);
                TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(b.sb.Equals("B1.sb"));
                TestHelper.Assert(b.pb == b);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("one-element cycle (AMI)... ");
        output.Flush();
        {
            var b = testPrx.oneElementCycleAsync().Result;
            TestHelper.Assert(b != null);
            TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(b.sb.Equals("B1.sb"));
            TestHelper.Assert(b.pb == b);
        }
        output.WriteLine("ok");

        output.Write("two-element cycle... ");
        output.Flush();
        {
            try
            {
                B? b1 = testPrx.twoElementCycle();
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(b1.sb.Equals("B1.sb"));

                B? b2 = b1.pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(b2.sb.Equals("B2.sb"));
                TestHelper.Assert(b2.pb == b1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("two-element cycle (AMI)... ");
        output.Flush();
        {
            B? b1 = testPrx.twoElementCycleAsync().Result;
            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(b1.sb.Equals("B1.sb"));

            B? b2 = b1.pb;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(b2.sb.Equals("B2.sb"));
            TestHelper.Assert(b2.pb == b1);
        }
        output.WriteLine("ok");

        output.Write("known derived pointer slicing as base... ");
        output.Flush();
        {
            try
            {
                B? b1 = testPrx.D1AsB();
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(b1.sb.Equals("D1.sb"));
                TestHelper.Assert(b1.pb != null);
                TestHelper.Assert(b1.pb != b1);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.pd1 != null);
                TestHelper.Assert(d1.pd1 != b1);
                TestHelper.Assert(b1.pb == d1.pd1);

                B b2 = b1.pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.pb == b1);
                TestHelper.Assert(b2.sb.Equals("D2.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("known derived pointer slicing as base (AMI)... ");
        output.Flush();
        {
            B? b1 = testPrx.D1AsBAsync().Result;
            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(b1.sb.Equals("D1.sb"));
            TestHelper.Assert(b1.pb != null);
            TestHelper.Assert(b1.pb != b1);
            D1 d1 = (D1)b1;
            TestHelper.Assert(d1 != null);
            TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
            TestHelper.Assert(d1.pd1 != null);
            TestHelper.Assert(d1.pd1 != b1);
            TestHelper.Assert(b1.pb == d1.pd1);

            B b2 = b1.pb;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.pb == b1);
            TestHelper.Assert(b2.sb.Equals("D2.sb"));
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
        }
        output.WriteLine("ok");

        output.Write("known derived pointer slicing as derived... ");
        output.Flush();
        {
            try
            {
                D1? d1 = testPrx.D1AsD1();
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(d1.sb.Equals("D1.sb"));
                TestHelper.Assert(d1.pb != null);
                TestHelper.Assert(d1.pb != d1);

                B? b2 = d1.pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(b2.sb.Equals("D2.sb"));
                TestHelper.Assert(b2.pb == d1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("known derived pointer slicing as derived (AMI)... ");
        output.Flush();
        {
            D1? d1 = testPrx.D1AsD1Async().Result;
            TestHelper.Assert(d1 != null);
            TestHelper.Assert(d1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(d1.sb.Equals("D1.sb"));
            TestHelper.Assert(d1.pb != null);
            TestHelper.Assert(d1.pb != d1);

            B b2 = d1.pb;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(b2.sb.Equals("D2.sb"));
            TestHelper.Assert(b2.pb == d1);
        }
        output.WriteLine("ok");

        output.Write("unknown derived pointer slicing as base... ");
        output.Flush();
        {
            try
            {
                B? b2 = testPrx.D2AsB();
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(b2.sb.Equals("D2.sb"));
                TestHelper.Assert(b2.pb != null);
                TestHelper.Assert(b2.pb != b2);

                B? b1 = b2.pb;
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(b1.sb.Equals("D1.sb"));
                TestHelper.Assert(b1.pb == b2);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.pd1 == b2);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("unknown derived pointer slicing as base (AMI)... ");
        output.Flush();
        {
            B? b2 = testPrx.D2AsBAsync().Result;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(b2.sb.Equals("D2.sb"));
            TestHelper.Assert(b2.pb != null);
            TestHelper.Assert(b2.pb != b2);

            B b1 = b2.pb;
            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(b1.sb.Equals("D1.sb"));
            TestHelper.Assert(b1.pb == b2);
            D1 d1 = (D1)b1;
            TestHelper.Assert(d1 != null);
            TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
            TestHelper.Assert(d1.pd1 == b2);
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing with known first... ");
        output.Flush();
        {
            try
            {
                (B? b1, B? b2) = testPrx.paramTest1();

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(b1.sb.Equals("D1.sb"));
                TestHelper.Assert(b1.pb == b2);
                D1 d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.pd1 == b2);

                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // No factory, must be sliced
                TestHelper.Assert(b2.sb.Equals("D2.sb"));
                TestHelper.Assert(b2.pb == b1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing with known first (AMI)... ");
        output.Flush();
        {
            var result = testPrx.paramTest1Async().Result;
            B? b1 = result.p1;
            B? b2 = result.p2;

            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(b1.sb.Equals("D1.sb"));
            TestHelper.Assert(b1.pb == b2);
            D1 d1 = (D1)b1;
            TestHelper.Assert(d1 != null);
            TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
            TestHelper.Assert(d1.pd1 == b2);

            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // No factory, must be sliced
            TestHelper.Assert(b2.sb.Equals("D2.sb"));
            TestHelper.Assert(b2.pb == b1);
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing with unknown first... ");
        output.Flush();
        {
            try
            {
                B? b2;
                B? b1;
                (b2, b1) = testPrx.paramTest2();

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(b1.sb.Equals("D1.sb"));
                TestHelper.Assert(b1.pb == b2);
                D1 d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.pd1 == b2);

                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // No factory, must be sliced
                TestHelper.Assert(b2.sb.Equals("D2.sb"));
                TestHelper.Assert(b2.pb == b1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing with unknown first (AMI)... ");
        output.Flush();
        {
            var result = testPrx.paramTest2Async().Result;
            B? b2 = result.p2;
            B? b1 = result.p1;
            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(b1.sb.Equals("D1.sb"));
            TestHelper.Assert(b1.pb == b2);
            D1 d1 = (D1)b1;
            TestHelper.Assert(d1 != null);
            TestHelper.Assert(d1.sd1.Equals("D1.sd1"));
            TestHelper.Assert(d1.pd1 == b2);

            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // No factory, must be sliced
            TestHelper.Assert(b2.sb.Equals("D2.sb"));
            TestHelper.Assert(b2.pb == b1);
        }
        output.WriteLine("ok");

        output.Write("return value identity with known first... ");
        output.Flush();
        {
            try
            {
                var (ret, p1, p2) = testPrx.returnTest1();
                TestHelper.Assert(ret == p1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("return value identity with known first (AMI)... ");
        output.Flush();
        {
            var result = testPrx.returnTest1Async().Result;
            TestHelper.Assert(result.ReturnValue == result.p1);
        }
        output.WriteLine("ok");

        output.Write("return value identity with unknown first... ");
        output.Flush();
        {
            try
            {
                var (ret, p1, p2) = testPrx.returnTest2();
                TestHelper.Assert(ret == p1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("return value identity with unknown first (AMI)... ");
        output.Flush();
        {
            var result = testPrx.returnTest2Async().Result;
            TestHelper.Assert(result.ReturnValue == result.p2);
        }
        output.WriteLine("ok");

        output.Write("return value identity for input params known first... ");
        output.Flush();
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

                B? b1 = testPrx.returnTest3(d1, d3);

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.sb.Equals("D1.sb"));
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                D1 p1 = (D1)b1;
                TestHelper.Assert(p1 != null);
                TestHelper.Assert(p1.sd1.Equals("D1.sd1"));
                TestHelper.Assert(p1.pd1 == b1.pb);

                B? b2 = b1.pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.sb.Equals("D3.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // Sliced by server
                TestHelper.Assert(b2.pb == b1);
                try
                {
                    D3 p3 = (D3)b2;
                    TestHelper.Assert(false);
                    D3 tmp = p3; p3 = tmp; // Stop compiler warning about unused variable.
                }
                catch (InvalidCastException)
                {
                }

                TestHelper.Assert(b1 != d1);
                TestHelper.Assert(b1 != d3);
                TestHelper.Assert(b2 != d1);
                TestHelper.Assert(b2 != d3);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("return value identity for input params known first (AMI)... ");
        output.Flush();
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

            B? b1 = testPrx.returnTest3Async(d1, d3).Result;

            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.sb.Equals("D1.sb"));
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            D1 p1 = (D1)b1;
            TestHelper.Assert(p1 != null);
            TestHelper.Assert(p1.sd1.Equals("D1.sd1"));
            TestHelper.Assert(p1.pd1 == b1.pb);

            B? b2 = b1.pb;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.sb.Equals("D3.sb"));
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::B")); // Sliced by server
            TestHelper.Assert(b2.pb == b1);
            try
            {
                D3 p3 = (D3)b2;
                TestHelper.Assert(false);
                D3 tmp = p3;
                p3 = tmp; // Stop compiler warning about unused variable.
            }
            catch (InvalidCastException)
            {
            }

            TestHelper.Assert(b1 != d1);
            TestHelper.Assert(b1 != d3);
            TestHelper.Assert(b2 != d1);
            TestHelper.Assert(b2 != d3);
        }
        output.WriteLine("ok");

        output.Write("return value identity for input params unknown first... ");
        output.Flush();
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

                B? b1 = testPrx.returnTest3(d3, d1);

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.sb.Equals("D3.sb"));
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::B")); // Sliced by server

                try
                {
                    D3 p1 = (D3)b1;
                    TestHelper.Assert(false);
                    D3 tmp = p1; p1 = tmp; // Stop compiler warning about unused variable.
                }
                catch (InvalidCastException)
                {
                }

                B? b2 = b1.pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.sb.Equals("D1.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(b2.pb == b1);
                D1 p3 = (D1)b2;
                TestHelper.Assert(p3 != null);
                TestHelper.Assert(p3.sd1.Equals("D1.sd1"));
                TestHelper.Assert(p3.pd1 == b1);

                TestHelper.Assert(b1 != d1);
                TestHelper.Assert(b1 != d3);
                TestHelper.Assert(b2 != d1);
                TestHelper.Assert(b2 != d3);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("return value identity for input params unknown first (AMI)... ");
        output.Flush();
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

            B? b1 = testPrx.returnTest3Async(d3, d1).Result;

            TestHelper.Assert(b1 != null);
            TestHelper.Assert(b1.sb.Equals("D3.sb"));
            TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::Test::B")); // Sliced by server

            try
            {
                D3 p1 = (D3)b1;
                TestHelper.Assert(false);
                D3 tmp = p1;
                p1 = tmp; // Stop compiler warning about unused variable.
            }
            catch (InvalidCastException)
            {
            }

            B? b2 = b1.pb;
            TestHelper.Assert(b2 != null);
            TestHelper.Assert(b2.sb.Equals("D1.sb"));
            TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(b2.pb == b1);
            D1 p3 = (D1)b2;
            TestHelper.Assert(p3 != null);
            TestHelper.Assert(p3.sd1.Equals("D1.sd1"));
            TestHelper.Assert(p3.pd1 == b1);

            TestHelper.Assert(b1 != d1);
            TestHelper.Assert(b1 != d3);
            TestHelper.Assert(b2 != d1);
            TestHelper.Assert(b2 != d3);
        }
        output.WriteLine("ok");

        output.Write("remainder unmarshaling (3 instances)... ");
        output.Flush();
        {
            try
            {
                var (ret, p1, p2) = testPrx.paramTest3();

                TestHelper.Assert(p1 != null);
                TestHelper.Assert(p1.sb.Equals("D2.sb (p1 1)"));
                TestHelper.Assert(p1.pb == null);
                TestHelper.Assert(p1.GetType().GetIceTypeId()!.Equals("::Test::B"));

                TestHelper.Assert(p2 != null);
                TestHelper.Assert(p2.sb.Equals("D2.sb (p2 1)"));
                TestHelper.Assert(p2.pb == null);
                TestHelper.Assert(p2.GetType().GetIceTypeId()!.Equals("::Test::B"));

                TestHelper.Assert(ret != null);
                TestHelper.Assert(ret.sb.Equals("D1.sb (p2 2)"));
                TestHelper.Assert(ret.pb == null);
                TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("remainder unmarshaling (3 instances) (AMI)... ");
        output.Flush();
        {
            var result = testPrx.paramTest3Async().Result;

            B? ret = result.ReturnValue;
            B? p1 = result.p1;
            B? p2 = result.p2;
            TestHelper.Assert(p1 != null);
            TestHelper.Assert(p1.sb.Equals("D2.sb (p1 1)"));
            TestHelper.Assert(p1.pb == null);
            TestHelper.Assert(p1.GetType().GetIceTypeId()!.Equals("::Test::B"));

            TestHelper.Assert(p2 != null);
            TestHelper.Assert(p2.sb.Equals("D2.sb (p2 1)"));
            TestHelper.Assert(p2.pb == null);
            TestHelper.Assert(p2.GetType().GetIceTypeId()!.Equals("::Test::B"));

            TestHelper.Assert(ret != null);
            TestHelper.Assert(ret.sb.Equals("D1.sb (p2 2)"));
            TestHelper.Assert(ret.pb == null);
            TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::D1"));
        }
        output.WriteLine("ok");

        output.Write("remainder unmarshaling (4 instances)... ");
        output.Flush();
        {
            try
            {
                var (ret, b) = testPrx.paramTest4();

                TestHelper.Assert(b != null);
                TestHelper.Assert(b.sb.Equals("D4.sb (1)"));
                TestHelper.Assert(b.pb == null);
                TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::Test::B"));

                TestHelper.Assert(ret != null);
                TestHelper.Assert(ret.sb.Equals("B.sb (2)"));
                TestHelper.Assert(ret.pb == null);
                TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::B"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("remainder unmarshaling (4 instances) (AMI)... ");
        output.Flush();
        {
            var result = testPrx.paramTest4Async().Result;
            B? ret = result.ReturnValue;
            B? b = result.p;

            TestHelper.Assert(b != null);
            TestHelper.Assert(b.sb.Equals("D4.sb (1)"));
            TestHelper.Assert(b.pb == null);
            TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::Test::B"));

            TestHelper.Assert(ret != null);
            TestHelper.Assert(ret.sb.Equals("B.sb (2)"));
            TestHelper.Assert(ret.pb == null);
            TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::B"));
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing, instance marshaled in unknown derived as base... ");
        output.Flush();
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

                B? ret = testPrx.returnTest3(d3, b2);

                TestHelper.Assert(ret != null);
                TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(ret.sb.Equals("D3.sb"));
                TestHelper.Assert(ret.pb == ret);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ");
        output.Flush();
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

            B? rv = testPrx.returnTest3Async(d3, b2).Result;

            TestHelper.Assert(rv != null);
            TestHelper.Assert(rv.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(rv.sb.Equals("D3.sb"));
            TestHelper.Assert(rv.pb == rv);
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing, instance marshaled in unknown derived as derived... ");
        output.Flush();
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

                B? ret = testPrx.returnTest3(d3, d12);
                TestHelper.Assert(ret != null);
                TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(ret.sb.Equals("D3.sb"));
                TestHelper.Assert(ret.pb == ret);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ");
        output.Flush();
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

            B? rv = testPrx.returnTest3Async(d3, d12).Result;

            TestHelper.Assert(rv != null);
            TestHelper.Assert(rv.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(rv.sb.Equals("D3.sb"));
            TestHelper.Assert(rv.pb == rv);
        }
        output.WriteLine("ok");

        output.Write("sequence slicing... ");
        output.Flush();
        {
            try
            {
                SS3 ss;
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

                    ss = testPrx.sequenceTest(ss1, ss2);
                }

                TestHelper.Assert(ss.c1 != null);
                B? ss1b2 = ss.c1.s[0];
                B? ss1d2 = ss.c1.s[1];
                TestHelper.Assert(ss.c2 != null);
                B? ss1d4 = ss.c1.s[2];

                TestHelper.Assert(ss.c2 != null);
                B? ss2b2 = ss.c2.s[0];
                B? ss2d2 = ss.c2.s[1];
                B? ss2d4 = ss.c2.s[2];

                TestHelper.Assert(ss1b2!.pb == ss1b2);
                TestHelper.Assert(ss1d2!.pb == ss1b2);
                TestHelper.Assert(ss1d4!.pb == ss1b2);

                TestHelper.Assert(ss2b2!.pb == ss1b2);
                TestHelper.Assert(ss2d2!.pb == ss2b2);
                TestHelper.Assert(ss2d4!.pb == ss2b2);

                TestHelper.Assert(ss1b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(ss1d2.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(ss1d4.GetType().GetIceTypeId()!.Equals("::Test::B"));

                TestHelper.Assert(ss2b2.GetType().GetIceTypeId()!.Equals("::Test::B"));
                TestHelper.Assert(ss2d2.GetType().GetIceTypeId()!.Equals("::Test::D1"));
                TestHelper.Assert(ss2d4.GetType().GetIceTypeId()!.Equals("::Test::B"));
            }
            catch (System.Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("sequence slicing (AMI)... ");
        output.Flush();
        {
            SS3 ss;
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

                ss = testPrx.sequenceTestAsync(ss1, ss2).Result;
            }
            TestHelper.Assert(ss.c1 != null);
            B? ss1b3 = ss.c1.s[0];
            B? ss1d5 = ss.c1.s[1];
            TestHelper.Assert(ss.c2 != null);
            B? ss1d6 = ss.c1.s[2];

            TestHelper.Assert(ss.c2 != null);
            B? ss2b3 = ss.c2.s[0];
            B? ss2d5 = ss.c2.s[1];
            B? ss2d6 = ss.c2.s[2];

            TestHelper.Assert(ss1b3!.pb == ss1b3);
            TestHelper.Assert(ss1d6!.pb == ss1b3);
            TestHelper.Assert(ss1d6!.pb == ss1b3);

            TestHelper.Assert(ss2b3!.pb == ss1b3);
            TestHelper.Assert(ss2d6!.pb == ss2b3);
            TestHelper.Assert(ss2d6!.pb == ss2b3);

            TestHelper.Assert(ss1b3!.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(ss1d5!.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(ss1d6!.GetType().GetIceTypeId()!.Equals("::Test::B"));

            TestHelper.Assert(ss2b3!.GetType().GetIceTypeId()!.Equals("::Test::B"));
            TestHelper.Assert(ss2d5!.GetType().GetIceTypeId()!.Equals("::Test::D1"));
            TestHelper.Assert(ss2d6!.GetType().GetIceTypeId()!.Equals("::Test::B"));
        }
        output.WriteLine("ok");

        output.Write("dictionary slicing... ");
        output.Flush();
        {
            try
            {
                var bin = new Dictionary<int, B?>();
                Dictionary<int, B?> bout;
                Dictionary<int, B?> ret;
                int i;
                for (i = 0; i < 10; ++i)
                {
                    string s = "D1." + i.ToString();
                    var d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin[i] = d1;
                }

                (ret, bout) = testPrx.dictionaryTest(bin);

                TestHelper.Assert(bout.Count == 10);
                for (i = 0; i < 10; ++i)
                {
                    B? b = bout[i * 10];
                    TestHelper.Assert(b != null);
                    string s = "D1." + i.ToString();
                    TestHelper.Assert(b.sb.Equals(s));
                    TestHelper.Assert(b.pb != null);
                    TestHelper.Assert(b.pb != b);
                    TestHelper.Assert(b.pb.sb.Equals(s));
                    TestHelper.Assert(b.pb.pb == b.pb);
                }

                TestHelper.Assert(ret.Count == 10);
                for (i = 0; i < 10; ++i)
                {
                    B? b = ret[i * 20];
                    TestHelper.Assert(b != null);
                    string s = "D1." + (i * 20).ToString();
                    TestHelper.Assert(b.sb.Equals(s));
                    TestHelper.Assert(b.pb == (i == 0 ? null : ret[(i - 1) * 20]));
                    var d1 = (D1)b;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.sd1.Equals(s));
                    TestHelper.Assert(d1.pd1 == d1);
                }
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("dictionary slicing (AMI)... ");
        output.Flush();
        {
            var bin = new Dictionary<int, B?>();
            int i;
            for (i = 0; i < 10; ++i)
            {
                string s = "D1." + i.ToString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin[i] = d1;
            }

            var result = testPrx.dictionaryTestAsync(bin).Result;
            Dictionary<int, B?> rv = result.ReturnValue;
            Dictionary<int, B?> bout = result.bout;

            TestHelper.Assert(bout.Count == 10);
            for (i = 0; i < 10; ++i)
            {
                B? b = bout[i * 10];
                TestHelper.Assert(b != null);
                string s = "D1." + i.ToString();
                TestHelper.Assert(b.sb.Equals(s));
                TestHelper.Assert(b.pb != null);
                TestHelper.Assert(b.pb != b);
                TestHelper.Assert(b.pb.sb.Equals(s));
                TestHelper.Assert(b.pb.pb == b.pb);
            }

            TestHelper.Assert(rv.Count == 10);
            for (i = 0; i < 10; ++i)
            {
                B? b = rv[i * 20];
                TestHelper.Assert(b != null);
                string s = "D1." + (i * 20).ToString();
                TestHelper.Assert(b.sb.Equals(s));
                TestHelper.Assert(b.pb == (i == 0 ? null : rv[(i - 1) * 20]));
                D1 d1 = (D1)b;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.sd1.Equals(s));
                TestHelper.Assert(d1.pd1 == d1);
            }
        }
        output.WriteLine("ok");

        output.Write("base exception thrown as base exception... ");
        output.Flush();
        {
            try
            {
                testPrx.throwBaseAsBase();
                TestHelper.Assert(false);
            }
            catch (BaseException e)
            {
                TestHelper.Assert(e.GetType().FullName!.Equals("Test.BaseException"));
                TestHelper.Assert(e.sbe.Equals("sbe"));
                TestHelper.Assert(e.pb != null);
                TestHelper.Assert(e.pb.sb.Equals("sb"));
                TestHelper.Assert(e.pb.pb == e.pb);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("base exception thrown as base exception (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.throwBaseAsBaseAsync().Wait();
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    var e = (BaseException)ae.InnerException;
                    TestHelper.Assert(e.sbe.Equals("sbe"));
                    TestHelper.Assert(e.pb != null);
                    TestHelper.Assert(e.pb.sb.Equals("sb"));
                    TestHelper.Assert(e.pb.pb == e.pb);
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("derived exception thrown as base exception... ");
        output.Flush();
        {
            try
            {
                testPrx.throwDerivedAsBase();
                TestHelper.Assert(false);
            }
            catch (DerivedException e)
            {
                TestHelper.Assert(e.GetType().FullName!.Equals("Test.DerivedException"));
                TestHelper.Assert(e.sbe.Equals("sbe"));
                TestHelper.Assert(e.pb != null);
                TestHelper.Assert(e.pb.sb.Equals("sb1"));
                TestHelper.Assert(e.pb.pb == e.pb);
                TestHelper.Assert(e.sde.Equals("sde1"));
                TestHelper.Assert(e.pd1 != null);
                TestHelper.Assert(e.pd1.sb.Equals("sb2"));
                TestHelper.Assert(e.pd1.pb == e.pd1);
                TestHelper.Assert(e.pd1.sd1.Equals("sd2"));
                TestHelper.Assert(e.pd1.pd1 == e.pd1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("derived exception thrown as base exception (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.throwDerivedAsBaseAsync().Wait();
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    DerivedException e = (DerivedException)ae.InnerException;
                    TestHelper.Assert(e.sbe.Equals("sbe"));
                    TestHelper.Assert(e.pb != null);
                    TestHelper.Assert(e.pb.sb.Equals("sb1"));
                    TestHelper.Assert(e.pb.pb == e.pb);
                    TestHelper.Assert(e.sde.Equals("sde1"));
                    TestHelper.Assert(e.pd1 != null);
                    TestHelper.Assert(e.pd1.sb.Equals("sb2"));
                    TestHelper.Assert(e.pd1.pb == e.pd1);
                    TestHelper.Assert(e.pd1.sd1.Equals("sd2"));
                    TestHelper.Assert(e.pd1.pd1 == e.pd1);
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("derived exception thrown as derived exception... ");
        output.Flush();
        {
            try
            {
                testPrx.throwDerivedAsDerived();
                TestHelper.Assert(false);
            }
            catch (DerivedException e)
            {
                TestHelper.Assert(e.GetType().FullName!.Equals("Test.DerivedException"));
                TestHelper.Assert(e.sbe.Equals("sbe"));
                TestHelper.Assert(e.pb != null);
                TestHelper.Assert(e.pb.sb.Equals("sb1"));
                TestHelper.Assert(e.pb.pb == e.pb);
                TestHelper.Assert(e.sde.Equals("sde1"));
                TestHelper.Assert(e.pd1 != null);
                TestHelper.Assert(e.pd1.sb.Equals("sb2"));
                TestHelper.Assert(e.pd1.pb == e.pd1);
                TestHelper.Assert(e.pd1.sd1.Equals("sd2"));
                TestHelper.Assert(e.pd1.pd1 == e.pd1);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("derived exception thrown as derived exception (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.throwDerivedAsDerivedAsync().Wait();
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    DerivedException e = (DerivedException)ae.InnerException;
                    TestHelper.Assert(e.sbe.Equals("sbe"));
                    TestHelper.Assert(e.pb != null);
                    TestHelper.Assert(e.pb.sb.Equals("sb1"));
                    TestHelper.Assert(e.pb.pb == e.pb);
                    TestHelper.Assert(e.sde.Equals("sde1"));
                    TestHelper.Assert(e.pd1 != null);
                    TestHelper.Assert(e.pd1.sb.Equals("sb2"));
                    TestHelper.Assert(e.pd1.pb == e.pd1);
                    TestHelper.Assert(e.pd1.sd1.Equals("sd2"));
                    TestHelper.Assert(e.pd1.pd1 == e.pd1);
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("unknown derived exception thrown as base exception... ");
        output.Flush();
        {
            try
            {
                testPrx.throwUnknownDerivedAsBase();
                TestHelper.Assert(false);
            }
            catch (BaseException e)
            {
                TestHelper.Assert(e.GetType().FullName!.Equals("Test.BaseException"));
                TestHelper.Assert(e.sbe.Equals("sbe"));
                TestHelper.Assert(e.pb != null);
                TestHelper.Assert(e.pb.sb.Equals("sb d2"));
                TestHelper.Assert(e.pb.pb == e.pb);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("unknown derived exception thrown as base exception (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.throwUnknownDerivedAsBaseAsync().Wait();
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    BaseException e = (BaseException)ae.InnerException;
                    TestHelper.Assert(e.sbe.Equals("sbe"));
                    TestHelper.Assert(e.pb != null);
                    TestHelper.Assert(e.pb.sb.Equals("sb d2"));
                    TestHelper.Assert(e.pb.pb == e.pb);
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("forward-declared class... ");
        output.Flush();
        {
            try
            {
                Forward? f = testPrx.useForward();
                TestHelper.Assert(f != null);
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("forward-declared class (AMI)... ");
        output.Flush();
        {
            TestHelper.Assert(testPrx.useForwardAsync().Result != null);
        }
        output.WriteLine("ok");

        output.Write("preserved classes... ");
        output.Flush();

        try
        {
            //
            // Server knows the most-derived class PDerived.
            //
            var pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            PBase? r = testPrx.exchangePBase(pd);
            TestHelper.Assert(r != null);
            PDerived p2 = (PDerived)r;
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.ps.Equals("preserved"));
            TestHelper.Assert(p2.pb == p2);
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            PBase? r = testPrx.exchangePBase(pu);
            TestHelper.Assert(r != null);
            TestHelper.Assert(!(r is PCUnknown));
            TestHelper.Assert(r.pi == 3);
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase? r = testPrx.exchangePBase(pcd);
            TestHelper.Assert(r is PCDerived);
            PCDerived p2 = (PCDerived)r;
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.pbs[0] == p2);
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase? r = testPrx.exchangePBase(pcd);
            TestHelper.Assert(r is CompactPCDerived);
            CompactPCDerived p2 = (CompactPCDerived)r;
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.pbs[0] == p2);
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            PCDerived3 pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            int i;
            for (i = 0; i < 300; ++i)
            {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase?[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            PBase? r = testPrx.exchangePBase(pcd);
            TestHelper.Assert(r is PCDerived3);
            PCDerived3 p3 = (PCDerived3)r;
            TestHelper.Assert(p3.pi == 3);
            for (i = 0; i < 300; ++i)
            {
                var p2 = (PCDerived2)p3.pbs[i]!;
                TestHelper.Assert(p2.pi == i);
                TestHelper.Assert(p2.pbs.Length == 1);
                TestHelper.Assert(p2.pbs[0] == null);
                TestHelper.Assert(p2.pcd2 == i);
            }
            TestHelper.Assert(p3.pcd2 == p3.pi);
            TestHelper.Assert(p3.pcd3 == p3.pbs[10]);
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved? p = testPrx.PBSUnknownAsPreserved();
            TestHelper.Assert(p != null);
            testPrx.checkPBSUnknown(p);
            IReadOnlyList<SliceInfo>? slices = p.GetSlicedData()!.Value.Slices;
            TestHelper.Assert(slices.Count == 1);
            TestHelper.Assert(slices[0].TypeId!.Equals("::Test::PSUnknown"));
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");

        output.Write("preserved classes (AMI)... ");
        output.Flush();
        {
            //
            // Server knows the most-derived class PDerived.
            //
            var pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            var p2 = (PDerived?)testPrx.exchangePBaseAsync(pd).Result;
            TestHelper.Assert(p2 != null);
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.ps.Equals("preserved"));
            TestHelper.Assert(p2.pb == p2);
        }

        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            PBase? r = testPrx.exchangePBaseAsync(pu).Result;
            TestHelper.Assert(r != null);
            TestHelper.Assert(!(r is PCUnknown));
            TestHelper.Assert(r.pi == 3);
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase? r = testPrx.exchangePBaseAsync(pcd).Result;
            TestHelper.Assert(r != null);
            PCDerived p2 = (PCDerived)r;
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.pbs[0] == p2);
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            var pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase? r = testPrx.exchangePBaseAsync(pcd).Result;
            TestHelper.Assert(r != null);
            var p2 = (CompactPCDerived)r;
            TestHelper.Assert(p2.pi == 3);
            TestHelper.Assert(p2.pbs[0] == p2);
        }

        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            var pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            for (int i = 0; i < 300; ++i)
            {
                var p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase?[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            PBase? r = testPrx.exchangePBaseAsync(pcd).Result;
            TestHelper.Assert(r != null);
            var p3 = (PCDerived3)r;
            TestHelper.Assert(p3.pi == 3);
            for (int i = 0; i < 300; ++i)
            {
                var p2 = (PCDerived2?)p3.pbs[i];
                TestHelper.Assert(p2 != null);
                TestHelper.Assert(p2.pi == i);
                TestHelper.Assert(p2.pbs.Length == 1);
                TestHelper.Assert(p2.pbs[0] == null);
                TestHelper.Assert(p2.pcd2 == i);
            }
            TestHelper.Assert(p3.pcd2 == p3.pi);
            TestHelper.Assert(p3.pcd3 == p3.pbs[10]);
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved? p = testPrx.PBSUnknownAsPreserved();
            testPrx.checkPBSUnknown(p);
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");

        output.Write("garbage collection for preserved classes... ");
        output.Flush();
        try
        {
            //
            // Relay a graph through the server.
            //
            {
                PNode c = new PNode();
                c.next = new PNode();
                c.next.next = new PNode();
                c.next.next.next = c;

                TestHelper.Assert(PNode.counter == 3);
                PNode? n = testPrx.exchangePNode(c);
                TestHelper.Assert(n != null);
                TestHelper.Assert(PNode.counter == 6);
                PNode.counter = 0;
                n.next = null;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. The preserved slice refers to a graph of PNode
            // objects.
            //
            {
                TestHelper.Assert(PNode.counter == 0);
                Preserved? p = testPrx.PBSUnknownAsPreservedWithGraph();
                testPrx.checkPBSUnknownWithGraph(p);
                TestHelper.Assert(PNode.counter == 3);
                PNode.counter = 0;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. A data member in the preserved slice refers to the
            // outer object, so the chain of references looks like this:
            //
            // outer.iceSlicedData_.outer
            //
            {
                Preserved.counter = 0;
                Preserved? p = testPrx.PBSUnknown2AsPreservedWithGraph();
                testPrx.checkPBSUnknown2WithGraph(p);
                TestHelper.Assert(Preserved.counter == 1);
                Preserved.counter = 0;
            }

            //
            // Throw a preserved exception where the most-derived type is unknown.
            // The preserved exception slice contains a class data member. This
            // object is also preserved, and its most-derived type is also unknown.
            // The preserved slice of the object contains a class data member that
            // refers to itself.
            //
            // The chain of references looks like this:
            //
            // ex.slicedData_.obj.iceSlicedData_.obj
            //
            try
            {
                TestHelper.Assert(Preserved.counter == 0);

                try
                {
                    testPrx.throwPreservedException();
                }
                catch (PreservedException)
                {
                    TestHelper.Assert(Preserved.counter == 1);
                }

                Preserved.counter = 0;
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");
        return testPrx;
    }
}

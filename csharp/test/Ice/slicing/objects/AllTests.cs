// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Slicing.Objects
{
    public partial class PNode
    {
        internal static int Counter;
#pragma warning disable CA1822 // Mark members as static
        partial void Initialize() => ++Counter;
#pragma warning restore CA1822 // Mark members as static
    }

    public partial class Preserved
    {
        internal static int Counter;

#pragma warning disable CA1822 // Mark members as static
        partial void Initialize() => ++Counter;
#pragma warning restore CA1822 // Mark members as static
    }

    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            output.Write("testing stringToProxy... ");
            output.Flush();
            var testPrx = ITestIntfPrx.Parse(helper.GetTestProxy("Test", 0), communicator);
            output.WriteLine("ok");

            output.Write("testing basic slicing... ");
            // server to client
            try
            {
                SBase? sb = testPrx.SBSUnknownDerivedAsSBase();
                TestHelper.Assert(sb != null && sb.Sb.Equals("SBSUnknownDerived.sb"));
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }

            // client to server
            try
            {
                testPrx.CUnknownAsSBase(new CUnknown("CUnknown.sb", "CUnknown.cu"));
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
                    TestHelper.Assert(TypeExtensions.GetIceTypeId(o.GetType())!.Equals("::ZeroC::Ice::Test::Slicing::Objects::SBase"));
                    sb = (SBase)o;
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
                TestHelper.Assert(sb != null && sb.Sb.Equals("SBase.sb"));
            }
            output.WriteLine("ok");

            output.Write("base as Object (AMI)... ");
            output.Flush();
            {
                AnyClass? o = testPrx.SBaseAsObjectAsync().Result;
                TestHelper.Assert(o != null);
                TestHelper.Assert(o.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::SBase"));
                var sb = (SBase)o;
                TestHelper.Assert(sb != null);
                TestHelper.Assert(sb.Sb.Equals("SBase.sb"));
            }
            output.WriteLine("ok");

            output.Write("base as base... ");
            output.Flush();
            {
                SBase? sb;
                try
                {
                    sb = testPrx.SBaseAsSBase();
                    TestHelper.Assert(sb != null && sb.Sb.Equals("SBase.sb"));
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
                TestHelper.Assert(sb != null && sb.Sb.Equals("SBase.sb"));
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
                    TestHelper.Assert(sb != null && sb.Sb.Equals("SBSKnownDerived.sb"));
                    sbskd = (SBSKnownDerived)sb;
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
                TestHelper.Assert(sbskd != null && sbskd.Sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            output.WriteLine("ok");

            output.Write("base with known derived as base (AMI)... ");
            output.Flush();
            {
                SBase? sb = testPrx.SBSKnownDerivedAsSBaseAsync().Result;
                TestHelper.Assert(sb != null && sb.Sb.Equals("SBSKnownDerived.sb"));
                var sbskd = (SBSKnownDerived)sb;
                TestHelper.Assert(sbskd != null);
                TestHelper.Assert(sbskd.Sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            output.WriteLine("ok");

            output.Write("base with known derived as known derived... ");
            output.Flush();
            {
                SBSKnownDerived? sbskd;
                try
                {
                    sbskd = testPrx.SBSKnownDerivedAsSBSKnownDerived();
                    TestHelper.Assert(sbskd != null && sbskd.Sbskd.Equals("SBSKnownDerived.sbskd"));
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
                TestHelper.Assert(sbskd != null && sbskd.Sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            output.WriteLine("ok");

            output.Write("base with unknown derived as base... ");
            output.Flush();
            {
                SBase? sb;
                try
                {
                    sb = testPrx.SBSUnknownDerivedAsSBase();
                    TestHelper.Assert(sb != null && sb.Sb.Equals("SBSUnknownDerived.sb"));
                }
                catch (Exception ex)
                {
                    output.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
            }
            try
            {
                // This test fails when using the compact format because the instance cannot
                // be sliced to a known type.
                testPrx.SBSUnknownDerivedAsSBaseCompact();
                TestHelper.Assert(false);
            }
            catch (InvalidDataException ex)
            {
                TestHelper.Assert(ex.Message.Contains("::ZeroC::Ice::Test::Slicing::Objects::SBSUnknownDerived"));
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
                TestHelper.Assert(sb != null && sb.Sb.Equals("SBSUnknownDerived.sb"));
            }

            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            try
            {
                SBase? sb = testPrx.SBSUnknownDerivedAsSBaseCompactAsync().Result;
            }
            catch (AggregateException ae)
            {
                TestHelper.Assert(ae.InnerException is InvalidDataException);
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
                    TestHelper.Assert(unknown.TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Objects::SUnknown"));
                    TestHelper.Assert(unknown.GetSlicedData() != null);
                    testPrx.CheckSUnknown(o);
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
                        var unknown = (UnknownSlicedClass?)testPrx.SUnknownAsObjectAsync().Result;
                        TestHelper.Assert(unknown != null);
                        TestHelper.Assert(unknown.TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Objects::SUnknown"));
                    }
                    catch (AggregateException ex)
                    {
                        output.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                }
                catch (Exception ex)
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
                    B? b = testPrx.OneElementCycle();
                    TestHelper.Assert(b != null);
                    TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b.Sb.Equals("B1.sb"));
                    TestHelper.Assert(b.Pb == b);
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
                B? b = testPrx.OneElementCycleAsync().Result;
                TestHelper.Assert(b != null);
                TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b.Sb.Equals("B1.sb"));
                TestHelper.Assert(b.Pb == b);
            }
            output.WriteLine("ok");

            output.Write("two-element cycle... ");
            output.Flush();
            {
                try
                {
                    B? b1 = testPrx.TwoElementCycle();
                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b1.Sb.Equals("B1.sb"));

                    B? b2 = b1.Pb;
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Sb.Equals("B2.sb"));
                    TestHelper.Assert(b2.Pb == b1);
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
                B? b1 = testPrx.TwoElementCycleAsync().Result;
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b1.Sb.Equals("B1.sb"));

                B? b2 = b1.Pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b2.Sb.Equals("B2.sb"));
                TestHelper.Assert(b2.Pb == b1);
            }
            output.WriteLine("ok");

            output.Write("known derived pointer slicing as base... ");
            output.Flush();
            {
                try
                {
                    B? b1 = testPrx.D1AsB();
                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b1.Pb != null);
                    TestHelper.Assert(b1.Pb != b1);
                    var d1 = (D1)b1;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(d1.Pd1 != null);
                    TestHelper.Assert(d1.Pd1 != b1);
                    TestHelper.Assert(b1.Pb == d1.Pd1);

                    B b2 = b1.Pb;
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.Pb == b1);
                    TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
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
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                TestHelper.Assert(b1.Pb != null);
                TestHelper.Assert(b1.Pb != b1);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.Pd1 != null);
                TestHelper.Assert(d1.Pd1 != b1);
                TestHelper.Assert(b1.Pb == d1.Pd1);

                B b2 = b1.Pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.Pb == b1);
                TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
            }
            output.WriteLine("ok");

            output.Write("known derived pointer slicing as derived... ");
            output.Flush();
            {
                try
                {
                    D1? d1 = testPrx.D1AsD1();
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(d1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(d1.Pb != null);
                    TestHelper.Assert(d1.Pb != d1);

                    B? b2 = d1.Pb;
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                    TestHelper.Assert(b2.Pb == d1);
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
                TestHelper.Assert(d1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(d1.Sb.Equals("D1.sb"));
                TestHelper.Assert(d1.Pb != null);
                TestHelper.Assert(d1.Pb != d1);

                B b2 = d1.Pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                TestHelper.Assert(b2.Pb == d1);
            }
            output.WriteLine("ok");

            output.Write("unknown derived pointer slicing as base... ");
            output.Flush();
            {
                try
                {
                    B? b2 = testPrx.D2AsB();
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                    TestHelper.Assert(b2.Pb != null);
                    TestHelper.Assert(b2.Pb != b2);

                    B? b1 = b2.Pb;
                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b1.Pb == b2);
                    var d1 = (D1)b1;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(d1.Pd1 == b2);
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
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                TestHelper.Assert(b2.Pb != null);
                TestHelper.Assert(b2.Pb != b2);

                B b1 = b2.Pb;
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                TestHelper.Assert(b1.Pb == b2);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.Pd1 == b2);
            }
            output.WriteLine("ok");

            output.Write("param ptr slicing with known first... ");
            output.Flush();
            {
                try
                {
                    (B? b1, B? b2) = testPrx.ParamTest1();

                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b1.Pb == b2);
                    var d1 = (D1)b1;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(d1.Pd1 == b2);

                    TestHelper.Assert(b2 != null);
                    // No factory, must be sliced
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                    TestHelper.Assert(b2.Pb == b1);
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
                (B? b1, B? b2) = testPrx.ParamTest1Async().Result;

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                TestHelper.Assert(b1.Pb == b2);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.Pd1 == b2);

                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B")); // No factory, must be sliced
                TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                TestHelper.Assert(b2.Pb == b1);
            }
            output.WriteLine("ok");

            output.Write("param ptr slicing with unknown first... ");
            output.Flush();
            {
                try
                {
                    B? b2;
                    B? b1;
                    (b2, b1) = testPrx.ParamTest2();

                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b1.Pb == b2);
                    var d1 = (D1)b1;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(d1.Pd1 == b2);

                    TestHelper.Assert(b2 != null);
                    // No factory, must be sliced
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                    TestHelper.Assert(b2.Pb == b1);
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
                (B? b2, B? b1) = testPrx.ParamTest2Async().Result;
                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                TestHelper.Assert(b1.Pb == b2);
                var d1 = (D1)b1;
                TestHelper.Assert(d1 != null);
                TestHelper.Assert(d1.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(d1.Pd1 == b2);

                TestHelper.Assert(b2 != null);
                // No factory, must be sliced
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(b2.Sb.Equals("D2.sb"));
                TestHelper.Assert(b2.Pb == b1);
            }
            output.WriteLine("ok");

            output.Write("return value identity with known first... ");
            output.Flush();
            {
                try
                {
                    (B? b1, B? b2, _) = testPrx.ReturnTest1();
                    TestHelper.Assert(b1 == b2);
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
                (B? b1, B? b2, _) = testPrx.ReturnTest1Async().Result;
                TestHelper.Assert(b1 == b2);
            }
            output.WriteLine("ok");

            output.Write("return value identity with unknown first... ");
            output.Flush();
            {
                try
                {
                    (B? b1, B? b2, B? b3) = testPrx.ReturnTest2();
                    TestHelper.Assert(b1 == b2);
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
                (B? b1, B? b2, _) = testPrx.ReturnTest2Async().Result;
                TestHelper.Assert(b1 == b2);
            }
            output.WriteLine("ok");

            output.Write("return value identity for input params known first... ");
            output.Flush();
            {
                try
                {
                    var d1 = new D1();
                    d1.Sb = "D1.sb";
                    d1.Sd1 = "D1.sd1";
                    var d3 = new D3();
                    d3.Pb = d1;
                    d3.Sb = "D3.sb";
                    d3.Sd3 = "D3.sd3";
                    d3.Pd3 = d1;
                    d1.Pb = d3;
                    d1.Pd1 = d3;

                    B? b1 = testPrx.ReturnTest3(d1, d3);

                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    var p1 = (D1)b1;
                    TestHelper.Assert(p1 != null);
                    TestHelper.Assert(p1.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(p1.Pd1 == b1.Pb);

                    B? b2 = b1.Pb;
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.Sb.Equals("D3.sb"));
                    // Sliced by server
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(b2.Pb == b1);
                    try
                    {
                        _ = (D3)b2;
                        TestHelper.Assert(false);
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
                var d1 = new D1();
                d1.Sb = "D1.sb";
                d1.Sd1 = "D1.sd1";
                var d3 = new D3();
                d3.Pb = d1;
                d3.Sb = "D3.sb";
                d3.Sd3 = "D3.sd3";
                d3.Pd3 = d1;
                d1.Pb = d3;
                d1.Pd1 = d3;

                B? b1 = testPrx.ReturnTest3Async(d1, d3).Result;

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.Sb.Equals("D1.sb"));
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                var p1 = (D1)b1;
                TestHelper.Assert(p1 != null);
                TestHelper.Assert(p1.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(p1.Pd1 == b1.Pb);

                B? b2 = b1.Pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.Sb.Equals("D3.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B")); // Sliced by server
                TestHelper.Assert(b2.Pb == b1);
                try
                {
                    var p3 = (D3)b2;
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
                    var d1 = new D1();
                    d1.Sb = "D1.sb";
                    d1.Sd1 = "D1.sd1";
                    var d3 = new D3();
                    d3.Pb = d1;
                    d3.Sb = "D3.sb";
                    d3.Sd3 = "D3.sd3";
                    d3.Pd3 = d1;
                    d1.Pb = d3;
                    d1.Pd1 = d3;

                    B? b1 = testPrx.ReturnTest3(d3, d1);

                    TestHelper.Assert(b1 != null);
                    TestHelper.Assert(b1.Sb.Equals("D3.sb"));
                    TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B")); // Sliced by server

                    try
                    {
                        var p1 = (D3)b1;
                        TestHelper.Assert(p1 != null);
                    }
                    catch (InvalidCastException)
                    {
                    }

                    B? b2 = b1.Pb;
                    TestHelper.Assert(b2 != null);
                    TestHelper.Assert(b2.Sb.Equals("D1.sb"));
                    TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(b2.Pb == b1);
                    var p3 = (D1)b2;
                    TestHelper.Assert(p3 != null);
                    TestHelper.Assert(p3.Sd1.Equals("D1.sd1"));
                    TestHelper.Assert(p3.Pd1 == b1);

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
                var d1 = new D1();
                d1.Sb = "D1.sb";
                d1.Sd1 = "D1.sd1";
                var d3 = new D3();
                d3.Pb = d1;
                d3.Sb = "D3.sb";
                d3.Sd3 = "D3.sd3";
                d3.Pd3 = d1;
                d1.Pb = d3;
                d1.Pd1 = d3;

                B? b1 = testPrx.ReturnTest3Async(d3, d1).Result;

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.Sb.Equals("D3.sb"));
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B")); // Sliced by server

                try
                {
                    var p1 = (D3)b1;
                    TestHelper.Assert(p1 != null);
                }
                catch (InvalidCastException)
                {
                }

                B? b2 = b1.Pb;
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.Sb.Equals("D1.sb"));
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(b2.Pb == b1);
                var p3 = (D1)b2;
                TestHelper.Assert(p3 != null);
                TestHelper.Assert(p3.Sd1.Equals("D1.sd1"));
                TestHelper.Assert(p3.Pd1 == b1);

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
                    (B? ret, B? p1, B? p2) = testPrx.ParamTest3();

                    TestHelper.Assert(p1 != null);
                    TestHelper.Assert(p1.Sb.Equals("D2.sb (p1 1)"));
                    TestHelper.Assert(p1.Pb == null);
                    TestHelper.Assert(p1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                    TestHelper.Assert(p2 != null);
                    TestHelper.Assert(p2.Sb.Equals("D2.sb (p2 1)"));
                    TestHelper.Assert(p2.Pb == null);
                    TestHelper.Assert(p2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                    TestHelper.Assert(ret != null);
                    TestHelper.Assert(ret.Sb.Equals("D1.sb (p2 2)"));
                    TestHelper.Assert(ret.Pb == null);
                    TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
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
                (B? ret, B? p1, B? p2) = testPrx.ParamTest3Async().Result;

                TestHelper.Assert(p1 != null);
                TestHelper.Assert(p1.Sb.Equals("D2.sb (p1 1)"));
                TestHelper.Assert(p1.Pb == null);
                TestHelper.Assert(p1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                TestHelper.Assert(p2 != null);
                TestHelper.Assert(p2.Sb.Equals("D2.sb (p2 1)"));
                TestHelper.Assert(p2.Pb == null);
                TestHelper.Assert(p2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                TestHelper.Assert(ret != null);
                TestHelper.Assert(ret.Sb.Equals("D1.sb (p2 2)"));
                TestHelper.Assert(ret.Pb == null);
                TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
            }
            output.WriteLine("ok");

            output.Write("remainder unmarshaling (4 instances)... ");
            output.Flush();
            {
                try
                {
                    (B? ret, B? b) = testPrx.ParamTest4();

                    TestHelper.Assert(b != null);
                    TestHelper.Assert(b.Sb.Equals("D4.sb (1)"));
                    TestHelper.Assert(b.Pb == null);
                    TestHelper.Assert(b.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                    TestHelper.Assert(ret != null);
                    TestHelper.Assert(ret.Sb.Equals("B.sb (2)"));
                    TestHelper.Assert(ret.Pb == null);
                    TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
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
                (B? b1, B? b2) = testPrx.ParamTest4Async().Result;

                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.Sb.Equals("D4.sb (1)"));
                TestHelper.Assert(b2.Pb == null);
                TestHelper.Assert(b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                TestHelper.Assert(b1 != null);
                TestHelper.Assert(b1.Sb.Equals("B.sb (2)"));
                TestHelper.Assert(b1.Pb == null);
                TestHelper.Assert(b1.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
            }
            output.WriteLine("ok");

            output.Write("param ptr slicing, instance marshaled in unknown derived as base... ");
            output.Flush();
            {
                try
                {
                    var b1 = new B();
                    b1.Sb = "B.sb(1)";
                    b1.Pb = b1;

                    var d3 = new D3();
                    d3.Sb = "D3.sb";
                    d3.Pb = d3;
                    d3.Sd3 = "D3.sd3";
                    d3.Pd3 = b1;

                    var b2 = new B();
                    b2.Sb = "B.sb(2)";
                    b2.Pb = b1;

                    B? ret = testPrx.ReturnTest3(d3, b2);

                    TestHelper.Assert(ret != null);
                    TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(ret.Sb.Equals("D3.sb"));
                    TestHelper.Assert(ret.Pb == ret);
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
                var b1 = new B();
                b1.Sb = "B.sb(1)";
                b1.Pb = b1;

                var d3 = new D3();
                d3.Sb = "D3.sb";
                d3.Pb = d3;
                d3.Sd3 = "D3.sd3";
                d3.Pd3 = b1;

                var b2 = new B();
                b2.Sb = "B.sb(2)";
                b2.Pb = b1;

                B? rv = testPrx.ReturnTest3Async(d3, b2).Result;

                TestHelper.Assert(rv != null);
                TestHelper.Assert(rv.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(rv.Sb.Equals("D3.sb"));
                TestHelper.Assert(rv.Pb == rv);
            }
            output.WriteLine("ok");

            output.Write("param ptr slicing, instance marshaled in unknown derived as derived... ");
            output.Flush();
            {
                try
                {
                    var d11 = new D1();
                    d11.Sb = "D1.sb(1)";
                    d11.Pb = d11;
                    d11.Sd1 = "D1.sd1(1)";

                    var d3 = new D3();
                    d3.Sb = "D3.sb";
                    d3.Pb = d3;
                    d3.Sd3 = "D3.sd3";
                    d3.Pd3 = d11;

                    var d12 = new D1();
                    d12.Sb = "D1.sb(2)";
                    d12.Pb = d12;
                    d12.Sd1 = "D1.sd1(2)";
                    d12.Pd1 = d11;

                    B? ret = testPrx.ReturnTest3(d3, d12);
                    TestHelper.Assert(ret != null);
                    TestHelper.Assert(ret.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(ret.Sb.Equals("D3.sb"));
                    TestHelper.Assert(ret.Pb == ret);
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
                var d11 = new D1();
                d11.Sb = "D1.sb(1)";
                d11.Pb = d11;
                d11.Sd1 = "D1.sd1(1)";

                var d3 = new D3();
                d3.Sb = "D3.sb";
                d3.Pb = d3;
                d3.Sd3 = "D3.sd3";
                d3.Pd3 = d11;

                var d12 = new D1();
                d12.Sb = "D1.sb(2)";
                d12.Pb = d12;
                d12.Sd1 = "D1.sd1(2)";
                d12.Pd1 = d11;

                B? rv = testPrx.ReturnTest3Async(d3, d12).Result;

                TestHelper.Assert(rv != null);
                TestHelper.Assert(rv.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(rv.Sb.Equals("D3.sb"));
                TestHelper.Assert(rv.Pb == rv);
            }
            output.WriteLine("ok");

            output.Write("sequence slicing... ");
            output.Flush();
            {
                try
                {
                    SS3 ss;
                    {
                        var ss1b = new B();
                        ss1b.Sb = "B.sb";
                        ss1b.Pb = ss1b;

                        var ss1d1 = new D1();
                        ss1d1.Sb = "D1.sb";
                        ss1d1.Sd1 = "D1.sd1";
                        ss1d1.Pb = ss1b;

                        var ss1d3 = new D3();
                        ss1d3.Sb = "D3.sb";
                        ss1d3.Sd3 = "D3.sd3";
                        ss1d3.Pb = ss1b;

                        var ss2b = new B();
                        ss2b.Sb = "B.sb";
                        ss2b.Pb = ss1b;

                        var ss2d1 = new D1();
                        ss2d1.Sb = "D1.sb";
                        ss2d1.Sd1 = "D1.sd1";
                        ss2d1.Pb = ss2b;

                        var ss2d3 = new D3();
                        ss2d3.Sb = "D3.sb";
                        ss2d3.Sd3 = "D3.sd3";
                        ss2d3.Pb = ss2b;

                        ss1d1.Pd1 = ss2b;
                        ss1d3.Pd3 = ss2d1;

                        ss2d1.Pd1 = ss1d3;
                        ss2d3.Pd3 = ss1d1;

                        var ss1 = new SS1(Array.Empty<B>());
                        ss1.S = new B[3];
                        ss1.S[0] = ss1b;
                        ss1.S[1] = ss1d1;
                        ss1.S[2] = ss1d3;

                        var ss2 = new SS2(Array.Empty<B>());
                        ss2.S = new B[3];
                        ss2.S[0] = ss2b;
                        ss2.S[1] = ss2d1;
                        ss2.S[2] = ss2d3;

                        ss = testPrx.SequenceTest(ss1, ss2);
                    }

                    TestHelper.Assert(ss.C1 != null);
                    B? ss1b2 = ss.C1.S[0];
                    B? ss1d2 = ss.C1.S[1];
                    TestHelper.Assert(ss.C2 != null);
                    B? ss1d4 = ss.C1.S[2];

                    TestHelper.Assert(ss.C2 != null);
                    B? ss2b2 = ss.C2.S[0];
                    B? ss2d2 = ss.C2.S[1];
                    B? ss2d4 = ss.C2.S[2];

                    TestHelper.Assert(ss1b2!.Pb == ss1b2);
                    TestHelper.Assert(ss1d2!.Pb == ss1b2);
                    TestHelper.Assert(ss1d4!.Pb == ss1b2);

                    TestHelper.Assert(ss2b2!.Pb == ss1b2);
                    TestHelper.Assert(ss2d2!.Pb == ss2b2);
                    TestHelper.Assert(ss2d4!.Pb == ss2b2);

                    TestHelper.Assert(ss1b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(ss1d2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(ss1d4.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                    TestHelper.Assert(ss2b2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                    TestHelper.Assert(ss2d2.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                    TestHelper.Assert(ss2d4.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                }
                catch (Exception ex)
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
                    var ss1b = new B();
                    ss1b.Sb = "B.sb";
                    ss1b.Pb = ss1b;

                    var ss1d1 = new D1();
                    ss1d1.Sb = "D1.sb";
                    ss1d1.Sd1 = "D1.sd1";
                    ss1d1.Pb = ss1b;

                    var ss1d3 = new D3();
                    ss1d3.Sb = "D3.sb";
                    ss1d3.Sd3 = "D3.sd3";
                    ss1d3.Pb = ss1b;

                    var ss2b = new B();
                    ss2b.Sb = "B.sb";
                    ss2b.Pb = ss1b;

                    var ss2d1 = new D1();
                    ss2d1.Sb = "D1.sb";
                    ss2d1.Sd1 = "D1.sd1";
                    ss2d1.Pb = ss2b;

                    var ss2d3 = new D3();
                    ss2d3.Sb = "D3.sb";
                    ss2d3.Sd3 = "D3.sd3";
                    ss2d3.Pb = ss2b;

                    ss1d1.Pd1 = ss2b;
                    ss1d3.Pd3 = ss2d1;

                    ss2d1.Pd1 = ss1d3;
                    ss2d3.Pd3 = ss1d1;

                    var ss1 = new SS1(Array.Empty<B>());
                    ss1.S = new B[3];
                    ss1.S[0] = ss1b;
                    ss1.S[1] = ss1d1;
                    ss1.S[2] = ss1d3;

                    var ss2 = new SS2(Array.Empty<B>());
                    ss2.S = new B[3];
                    ss2.S[0] = ss2b;
                    ss2.S[1] = ss2d1;
                    ss2.S[2] = ss2d3;

                    ss = testPrx.SequenceTestAsync(ss1, ss2).Result;
                }
                TestHelper.Assert(ss.C1 != null);
                B? ss1b3 = ss.C1.S[0];
                B? ss1d5 = ss.C1.S[1];
                TestHelper.Assert(ss.C2 != null);
                B? ss1d6 = ss.C1.S[2];

                TestHelper.Assert(ss.C2 != null);
                B? ss2b3 = ss.C2.S[0];
                B? ss2d5 = ss.C2.S[1];
                B? ss2d6 = ss.C2.S[2];

                TestHelper.Assert(ss1b3!.Pb == ss1b3);
                TestHelper.Assert(ss1d6!.Pb == ss1b3);
                TestHelper.Assert(ss1d6!.Pb == ss1b3);

                TestHelper.Assert(ss2b3!.Pb == ss1b3);
                TestHelper.Assert(ss2d6!.Pb == ss2b3);
                TestHelper.Assert(ss2d6!.Pb == ss2b3);

                TestHelper.Assert(ss1b3!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(ss1d5!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(ss1d6!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));

                TestHelper.Assert(ss2b3!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
                TestHelper.Assert(ss2d5!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::D1"));
                TestHelper.Assert(ss2d6!.GetType().GetIceTypeId()!.Equals("::ZeroC::Ice::Test::Slicing::Objects::B"));
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
                        d1.Sb = s;
                        d1.Pb = d1;
                        d1.Sd1 = s;
                        bin[i] = d1;
                    }

                    (ret, bout) = testPrx.DictionaryTest(bin);

                    TestHelper.Assert(bout.Count == 10);
                    for (i = 0; i < 10; ++i)
                    {
                        B? b = bout[i * 10];
                        TestHelper.Assert(b != null);
                        string s = "D1." + i.ToString();
                        TestHelper.Assert(b.Sb.Equals(s));
                        TestHelper.Assert(b.Pb != null);
                        TestHelper.Assert(b.Pb != b);
                        TestHelper.Assert(b.Pb.Sb.Equals(s));
                        TestHelper.Assert(b.Pb.Pb == b.Pb);
                    }

                    TestHelper.Assert(ret.Count == 10);
                    for (i = 0; i < 10; ++i)
                    {
                        B? b = ret[i * 20];
                        TestHelper.Assert(b != null);
                        string s = "D1." + (i * 20).ToString();
                        TestHelper.Assert(b.Sb.Equals(s));
                        TestHelper.Assert(b.Pb == (i == 0 ? null : ret[(i - 1) * 20]));
                        var d1 = (D1)b;
                        TestHelper.Assert(d1 != null);
                        TestHelper.Assert(d1.Sd1.Equals(s));
                        TestHelper.Assert(d1.Pd1 == d1);
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
                    var d1 = new D1();
                    d1.Sb = s;
                    d1.Pb = d1;
                    d1.Sd1 = s;
                    bin[i] = d1;
                }

                (Dictionary<int, B?> ReturnValue, Dictionary<int, B?> bout) result =
                    testPrx.DictionaryTestAsync(bin).Result;
                Dictionary<int, B?> rv = result.ReturnValue;
                Dictionary<int, B?> bout = result.bout;

                TestHelper.Assert(bout.Count == 10);
                for (i = 0; i < 10; ++i)
                {
                    B? b = bout[i * 10];
                    TestHelper.Assert(b != null);
                    string s = "D1." + i.ToString();
                    TestHelper.Assert(b.Sb.Equals(s));
                    TestHelper.Assert(b.Pb != null);
                    TestHelper.Assert(b.Pb != b);
                    TestHelper.Assert(b.Pb.Sb.Equals(s));
                    TestHelper.Assert(b.Pb.Pb == b.Pb);
                }

                TestHelper.Assert(rv.Count == 10);
                for (i = 0; i < 10; ++i)
                {
                    B? b = rv[i * 20];
                    TestHelper.Assert(b != null);
                    string s = "D1." + (i * 20).ToString();
                    TestHelper.Assert(b.Sb.Equals(s));
                    TestHelper.Assert(b.Pb == (i == 0 ? null : rv[(i - 1) * 20]));
                    var d1 = (D1)b;
                    TestHelper.Assert(d1 != null);
                    TestHelper.Assert(d1.Sd1.Equals(s));
                    TestHelper.Assert(d1.Pd1 == d1);
                }
            }
            output.WriteLine("ok");

            output.Write("base exception thrown as base exception... ");
            output.Flush();
            {
                try
                {
                    testPrx.ThrowBaseAsBase();
                    TestHelper.Assert(false);
                }
                catch (BaseException e)
                {
                    TestHelper.Assert(e.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Objects.BaseException"));
                    TestHelper.Assert(e.Sbe.Equals("sbe"));
                    TestHelper.Assert(e.Pb != null);
                    TestHelper.Assert(e.Pb.Sb.Equals("sb"));
                    TestHelper.Assert(e.Pb.Pb == e.Pb);
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
                    testPrx.ThrowBaseAsBaseAsync().Wait();
                }
                catch (AggregateException ae)
                {
                    try
                    {
                        TestHelper.Assert(ae.InnerException != null);
                        var e = (BaseException)ae.InnerException;
                        TestHelper.Assert(e.Sbe.Equals("sbe"));
                        TestHelper.Assert(e.Pb != null);
                        TestHelper.Assert(e.Pb.Sb.Equals("sb"));
                        TestHelper.Assert(e.Pb.Pb == e.Pb);
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
                    testPrx.ThrowDerivedAsBase();
                    TestHelper.Assert(false);
                }
                catch (DerivedException e)
                {
                    TestHelper.Assert(e.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Objects.DerivedException"));
                    TestHelper.Assert(e.Sbe.Equals("sbe"));
                    TestHelper.Assert(e.Pb != null);
                    TestHelper.Assert(e.Pb.Sb.Equals("sb1"));
                    TestHelper.Assert(e.Pb.Pb == e.Pb);
                    TestHelper.Assert(e.Sde.Equals("sde1"));
                    TestHelper.Assert(e.Pd1 != null);
                    TestHelper.Assert(e.Pd1.Sb.Equals("sb2"));
                    TestHelper.Assert(e.Pd1.Pb == e.Pd1);
                    TestHelper.Assert(e.Pd1.Sd1.Equals("sd2"));
                    TestHelper.Assert(e.Pd1.Pd1 == e.Pd1);
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
                    testPrx.ThrowDerivedAsBaseAsync().Wait();
                }
                catch (AggregateException ae)
                {
                    try
                    {
                        TestHelper.Assert(ae.InnerException != null);
                        var e = (DerivedException)ae.InnerException;
                        TestHelper.Assert(e.Sbe.Equals("sbe"));
                        TestHelper.Assert(e.Pb != null);
                        TestHelper.Assert(e.Pb.Sb.Equals("sb1"));
                        TestHelper.Assert(e.Pb.Pb == e.Pb);
                        TestHelper.Assert(e.Sde.Equals("sde1"));
                        TestHelper.Assert(e.Pd1 != null);
                        TestHelper.Assert(e.Pd1.Sb.Equals("sb2"));
                        TestHelper.Assert(e.Pd1.Pb == e.Pd1);
                        TestHelper.Assert(e.Pd1.Sd1.Equals("sd2"));
                        TestHelper.Assert(e.Pd1.Pd1 == e.Pd1);
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
                    testPrx.ThrowDerivedAsDerived();
                    TestHelper.Assert(false);
                }
                catch (DerivedException e)
                {
                    TestHelper.Assert(e.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Objects.DerivedException"));
                    TestHelper.Assert(e.Sbe.Equals("sbe"));
                    TestHelper.Assert(e.Pb != null);
                    TestHelper.Assert(e.Pb.Sb.Equals("sb1"));
                    TestHelper.Assert(e.Pb.Pb == e.Pb);
                    TestHelper.Assert(e.Sde.Equals("sde1"));
                    TestHelper.Assert(e.Pd1 != null);
                    TestHelper.Assert(e.Pd1.Sb.Equals("sb2"));
                    TestHelper.Assert(e.Pd1.Pb == e.Pd1);
                    TestHelper.Assert(e.Pd1.Sd1.Equals("sd2"));
                    TestHelper.Assert(e.Pd1.Pd1 == e.Pd1);
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
                    testPrx.ThrowDerivedAsDerivedAsync().Wait();
                }
                catch (AggregateException ae)
                {
                    try
                    {
                        TestHelper.Assert(ae.InnerException != null);
                        var e = (DerivedException)ae.InnerException;
                        TestHelper.Assert(e.Sbe.Equals("sbe"));
                        TestHelper.Assert(e.Pb != null);
                        TestHelper.Assert(e.Pb.Sb.Equals("sb1"));
                        TestHelper.Assert(e.Pb.Pb == e.Pb);
                        TestHelper.Assert(e.Sde.Equals("sde1"));
                        TestHelper.Assert(e.Pd1 != null);
                        TestHelper.Assert(e.Pd1.Sb.Equals("sb2"));
                        TestHelper.Assert(e.Pd1.Pb == e.Pd1);
                        TestHelper.Assert(e.Pd1.Sd1.Equals("sd2"));
                        TestHelper.Assert(e.Pd1.Pd1 == e.Pd1);
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
                    testPrx.ThrowUnknownDerivedAsBase();
                    TestHelper.Assert(false);
                }
                catch (BaseException e)
                {
                    TestHelper.Assert(e.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Objects.BaseException"));
                    TestHelper.Assert(e.Sbe.Equals("sbe"));
                    TestHelper.Assert(e.Pb != null);
                    TestHelper.Assert(e.Pb.Sb.Equals("sb d2"));
                    TestHelper.Assert(e.Pb.Pb == e.Pb);
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
                    testPrx.ThrowUnknownDerivedAsBaseAsync().Wait();
                }
                catch (AggregateException ae)
                {
                    try
                    {
                        TestHelper.Assert(ae.InnerException != null);
                        var e = (BaseException)ae.InnerException;
                        TestHelper.Assert(e.Sbe.Equals("sbe"));
                        TestHelper.Assert(e.Pb != null);
                        TestHelper.Assert(e.Pb.Sb.Equals("sb d2"));
                        TestHelper.Assert(e.Pb.Pb == e.Pb);
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
                    Forward? f = testPrx.UseForward();
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
                TestHelper.Assert(testPrx.UseForwardAsync().Result != null);
            }
            output.WriteLine("ok");

            output.Write("preserved classes... ");
            output.Flush();

            try
            {
                // Server knows the most-derived class PDerived.
                var pd = new PDerived();
                pd.Pi = 3;
                pd.Ps = "preserved";
                pd.Pb = pd;

                PBase? r = testPrx.ExchangePBase(pd);
                TestHelper.Assert(r != null);
                var p2 = (PDerived)r;
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Ps.Equals("preserved"));
                TestHelper.Assert(p2.Pb == p2);
            }
            catch (OperationNotExistException)
            {
            }

            try
            {
                // Server only knows the base (non-preserved) type, so the object is sliced.
                var pu = new PCUnknown();
                pu.Pi = 3;
                pu.Pu = "preserved";

                PBase? r = testPrx.ExchangePBase(pu);
                TestHelper.Assert(r != null);
                TestHelper.Assert(!(r is PCUnknown));
                TestHelper.Assert(r.Pi == 3);
            }
            catch (OperationNotExistException)
            {
            }

            try
            {
                // Server only knows the intermediate type Preserved. The object will be sliced to Preserved for the
                // 1.0 encoding; otherwise it should be returned intact.
                var pcd = new PCDerived(3, "", null, Array.Empty<PBase>());
                pcd.Pbs = new PBase[] { pcd };

                PBase? r = testPrx.ExchangePBase(pcd);
                TestHelper.Assert(r is PCDerived);
                var p2 = (PCDerived)r;
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Pbs[0] == p2);
            }
            catch (OperationNotExistException)
            {
            }

            try
            {
                // Server only knows the intermediate type Preserved. The object will be sliced to Preserved for the
                // 1.0 encoding; otherwise it should be returned intact.
                var pcd = new CompactPCDerived(3, "", null, Array.Empty<PBase>());
                pcd.Pbs = new PBase[] { pcd };

                PBase? r = testPrx.ExchangePBase(pcd);
                TestHelper.Assert(r is CompactPCDerived);
                var p2 = (CompactPCDerived)r;
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Pbs[0] == p2);
            }
            catch (OperationNotExistException)
            {
            }

            try
            {
                // Send an object that will have multiple preserved slices in the server. The object will be sliced
                // to Preserved for the 1.0 encoding.
                var pcd = new PCDerived3(3, "", null, Array.Empty<PBase>(), 0, null);

                // Sending more than 254 objects exercises the encoding for object ids.
                pcd.Pbs = new PBase[300];
                int i;
                for (i = 0; i < 300; ++i)
                {
                    var p2 = new PCDerived2(i, "", null, Array.Empty<PBase>(), i);
                    p2.Pbs = new PBase?[] { null }; // Nil reference. This slice should not have an indirection table.
                    pcd.Pbs[i] = p2;
                }
                pcd.Pcd2 = pcd.Pi;
                pcd.Pcd3 = pcd.Pbs[10];

                PBase? r = testPrx.ExchangePBase(pcd);
                TestHelper.Assert(r is PCDerived3);
                var p3 = (PCDerived3)r;
                TestHelper.Assert(p3.Pi == 3);
                for (i = 0; i < 300; ++i)
                {
                    var p2 = (PCDerived2)p3.Pbs[i]!;
                    TestHelper.Assert(p2.Pi == i);
                    TestHelper.Assert(p2.Pbs.Length == 1);
                    TestHelper.Assert(p2.Pbs[0] == null);
                    TestHelper.Assert(p2.Pcd2 == i);
                }
                TestHelper.Assert(p3.Pcd2 == p3.Pi);
                TestHelper.Assert(p3.Pcd3 == p3.Pbs[10]);
            }
            catch (OperationNotExistException)
            {
            }

            try
            {
                // Obtain an object with preserved slices and send it back to the server. The preserved slices should
                // be excluded for the 1.0 encoding, otherwise they should be included.
                Preserved? p = testPrx.PBSUnknownAsPreserved();
                TestHelper.Assert(p != null);
                testPrx.CheckPBSUnknown(p);
                IReadOnlyList<SliceInfo>? slices = p.GetSlicedData()!.Value.Slices;
                TestHelper.Assert(slices.Count == 1);
                TestHelper.Assert(slices[0].TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Objects::PSUnknown"));
            }
            catch (OperationNotExistException)
            {
            }

            output.WriteLine("ok");

            output.Write("preserved classes (AMI)... ");
            output.Flush();
            {
                // Server knows the most-derived class PDerived.
                var pd = new PDerived();
                pd.Pi = 3;
                pd.Ps = "preserved";
                pd.Pb = pd;

                var p2 = (PDerived?)testPrx.ExchangePBaseAsync(pd).Result;
                TestHelper.Assert(p2 != null);
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Ps.Equals("preserved"));
                TestHelper.Assert(p2.Pb == p2);
            }

            {
                // Server only knows the base (non-preserved) type, so the object is sliced.
                var pu = new PCUnknown();
                pu.Pi = 3;
                pu.Pu = "preserved";

                PBase? r = testPrx.ExchangePBaseAsync(pu).Result;
                TestHelper.Assert(r != null);
                TestHelper.Assert(!(r is PCUnknown));
                TestHelper.Assert(r.Pi == 3);
            }

            {
                // Server only knows the intermediate type Preserved. The object will be sliced to Preserved for the
                // 1.0 encoding; otherwise it should be returned intact.
                var pcd = new PCDerived(3, "", null, Array.Empty<PBase>());
                pcd.Pbs = new PBase[] { pcd };

                PBase? r = testPrx.ExchangePBaseAsync(pcd).Result;
                TestHelper.Assert(r != null);
                var p2 = (PCDerived)r;
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Pbs[0] == p2);
            }

            {
                // Server only knows the intermediate type Preserved. The object will be sliced to Preserved for the
                // 1.0 encoding; otherwise it should be returned intact.
                var pcd = new CompactPCDerived(3, "", null, Array.Empty<PBase>());
                pcd.Pbs = new PBase[] { pcd };

                PBase? r = testPrx.ExchangePBaseAsync(pcd).Result;
                TestHelper.Assert(r != null);
                var p2 = (CompactPCDerived)r;
                TestHelper.Assert(p2.Pi == 3);
                TestHelper.Assert(p2.Pbs[0] == p2);
            }

            {
                // Send an object that will have multiple preserved slices in the server. The object will be sliced to
                // Preserved for the 1.0 encoding.
                var pcd = new PCDerived3(3, "", null, Array.Empty<PBase>(), 0, null);

                // Sending more than 254 objects exercises the encoding for object ids.
                pcd.Pbs = new PBase[300];
                for (int i = 0; i < 300; ++i)
                {
                    var p2 = new PCDerived2(i, "", null, Array.Empty<PBase>(), i);
                    p2.Pbs = new PBase?[] { null }; // Nil reference. This slice should not have an indirection table.
                    pcd.Pbs[i] = p2;
                }
                pcd.Pcd2 = pcd.Pi;
                pcd.Pcd3 = pcd.Pbs[10];

                PBase? r = testPrx.ExchangePBaseAsync(pcd).Result;
                TestHelper.Assert(r != null);
                var p3 = (PCDerived3)r;
                TestHelper.Assert(p3.Pi == 3);
                for (int i = 0; i < 300; ++i)
                {
                    var p2 = (PCDerived2?)p3.Pbs[i];
                    TestHelper.Assert(p2 != null);
                    TestHelper.Assert(p2.Pi == i);
                    TestHelper.Assert(p2.Pbs.Length == 1);
                    TestHelper.Assert(p2.Pbs[0] == null);
                    TestHelper.Assert(p2.Pcd2 == i);
                }
                TestHelper.Assert(p3.Pcd2 == p3.Pi);
                TestHelper.Assert(p3.Pcd3 == p3.Pbs[10]);
            }

            try
            {
                // Obtain an object with preserved slices and send it back to the server. The preserved slices should
                // be excluded for the 1.0 encoding, otherwise they should be included.
                Preserved? p = testPrx.PBSUnknownAsPreserved();
                testPrx.CheckPBSUnknown(p);
            }
            catch (OperationNotExistException)
            {
            }

            output.WriteLine("ok");

            output.Write("garbage collection for preserved classes... ");
            output.Flush();
            try
            {
                // Relay a graph through the server.
                {
                    var c = new PNode();
                    c.Next = new PNode();
                    c.Next.Next = new PNode();
                    c.Next.Next.Next = c;

                    TestHelper.Assert(PNode.Counter == 3);
                    PNode? n = testPrx.ExchangePNode(c);
                    TestHelper.Assert(n != null);
                    TestHelper.Assert(PNode.Counter == 6);
                    PNode.Counter = 0;
                    n.Next = null;
                }

                // Obtain a preserved object from the server where the most-derived type is unknown. The preserved
                // slice refers to a graph of PNode objects.
                {
                    TestHelper.Assert(PNode.Counter == 0);
                    Preserved? p = testPrx.PBSUnknownAsPreservedWithGraph();
                    testPrx.CheckPBSUnknownWithGraph(p);
                    TestHelper.Assert(PNode.Counter == 3);
                    PNode.Counter = 0;
                }

                // Obtain a preserved object from the server where the most-derived type is unknown. A data member in
                // the preserved slice refers to the outer object, so the chain of references looks like this:
                //
                // outer.iceSlicedData_.outer
                {
                    Preserved.Counter = 0;
                    Preserved? p = testPrx.PBSUnknown2AsPreservedWithGraph();
                    testPrx.CheckPBSUnknown2WithGraph(p);
                    TestHelper.Assert(Preserved.Counter == 1);
                    Preserved.Counter = 0;
                }
                // Throw a preserved exception where the most-derived type is unknown. The preserved exception slice
                // contains a class data member. This object is also preserved, and its most-derived type is also
                // unknown. The preserved slice of the object contains a class data member that refers to itself.
                //
                // The chain of references looks like this:
                //
                // ex.slicedData_.obj.iceSlicedData_.obj
                try
                {
                    TestHelper.Assert(Preserved.Counter == 0);

                    try
                    {
                        testPrx.ThrowPreservedException();
                    }
                    catch (PreservedException)
                    {
                        TestHelper.Assert(Preserved.Counter == 1);
                    }

                    Preserved.Counter = 0;
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
            await testPrx.ShutdownAsync();
        }
    }
}

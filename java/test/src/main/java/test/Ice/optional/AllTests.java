// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import java.io.PrintWriter;
import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;

import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.OptionalFormat;
import com.zeroc.Ice.OutputStream;

import test.Ice.optional.Test.*;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Wrapper<T>
    {
        T value;
    }

    public static InitialPrx allTests(test.TestHelper helper, boolean collocated)
    {
        PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        FactoryI factory = new FactoryI();
        communicator.getValueFactoryManager().add(factory, "");

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrx.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        out.print("testing optional data members... ");
        out.flush();

        OneOptional oo1 = new OneOptional();
        test(!oo1.hasA());
        oo1.setA(15);
        test(oo1.hasA() && oo1.getA() == 15);

        OneOptional oo2 = new OneOptional(16);
        test(oo2.hasA() && oo2.getA() == 16);

        MultiOptional mo1 = new MultiOptional();
        mo1.setA((byte)15);
        mo1.setB(true);
        mo1.setC((short)19);
        mo1.setD(78);
        mo1.setE(99);
        mo1.setF((float)5.5);
        mo1.setG(1.0);
        mo1.setH("test");
        mo1.setI(MyEnum.MyEnumMember);
        mo1.setJ(communicator.stringToProxy("test"));
        mo1.setK(mo1);
        mo1.setBs(new byte[] { (byte)5 });
        mo1.setSs(new String[] { "test", "test2" });
        mo1.setIid(new java.util.HashMap<>());
        mo1.getIid().put(4, 3);
        mo1.setSid(new java.util.HashMap<>());
        mo1.getSid().put("test", 10);
        FixedStruct fs = new FixedStruct();
        fs.m = 78;
        mo1.setFs(fs);
        VarStruct vs = new VarStruct();
        vs.m = "hello";
        mo1.setVs(vs);

        mo1.setShs(new short[] { (short)1 });
        mo1.setEs(new MyEnum[] { MyEnum.MyEnumMember, MyEnum.MyEnumMember });
        mo1.setFss(new FixedStruct[] { fs });
        mo1.setVss(new VarStruct[] { vs });
        mo1.setOos(new OneOptional[] { oo1 });
        mo1.setOops(new com.zeroc.Ice.ObjectPrx[] { communicator.stringToProxy("test") });

        mo1.setIed(new java.util.HashMap<>());
        mo1.getIed().put(4, MyEnum.MyEnumMember);
        mo1.setIfsd(new java.util.HashMap<>());
        mo1.getIfsd().put(4, fs);
        mo1.setIvsd(new java.util.HashMap<>());
        mo1.getIvsd().put(5, vs);
        mo1.setIood(new java.util.HashMap<>());
        mo1.getIood().put(5, new OneOptional(15));
        mo1.setIoopd(new java.util.HashMap<>());
        mo1.getIoopd().put(5, communicator.stringToProxy("test"));

        mo1.setBos(new boolean[] { false, true, false });

        mo1.setSer(new SerializableClass(58));

        test(mo1.getA() == (byte)15);
        test(mo1.getB());
        test(mo1.isB());
        test(mo1.getC() == (short)19);
        test(mo1.getD() == 78);
        test(mo1.getE() == 99);
        test(mo1.getF() == (float)5.5);
        test(mo1.getG() == 1.0);
        test(mo1.getH().equals("test"));
        test(mo1.getI() == MyEnum.MyEnumMember);
        test(mo1.getJ().equals(communicator.stringToProxy("test")));
        test(mo1.getK() == mo1);
        test(java.util.Arrays.equals(mo1.getBs(), new byte[] { (byte)5 }));
        test(java.util.Arrays.equals(mo1.getSs(), new String[] { "test", "test2" }));
        test(mo1.getIid().get(4) == 3);
        test(mo1.getSid().get("test") == 10);
        test(mo1.getFs().equals(new FixedStruct(78)));
        test(mo1.getVs().equals(new VarStruct("hello")));

        test(mo1.getShs()[0] == (short)1);
        test(mo1.getEs()[0] == MyEnum.MyEnumMember && mo1.getEs()[1] == MyEnum.MyEnumMember);
        test(mo1.getFss()[0].equals(new FixedStruct(78)));
        test(mo1.getVss()[0].equals(new VarStruct("hello")));
        test(mo1.getOos()[0] == oo1);
        test(mo1.getOops()[0].equals(communicator.stringToProxy("test")));

        test(mo1.getIed().get(4) == MyEnum.MyEnumMember);
        test(mo1.getIfsd().get(4).equals(new FixedStruct(78)));
        test(mo1.getIvsd().get(5).equals(new VarStruct("hello")));
        test(mo1.getIood().get(5).getA() == 15);
        test(mo1.getIoopd().get(5).equals(communicator.stringToProxy("test")));

        test(java.util.Arrays.equals(mo1.getBos(), new boolean[] { false, true, false }));

        test(mo1.getSer().equals(new SerializableClass(58)));

        out.println("ok");

        out.print("testing marshaling... ");
        out.flush();

        OneOptional oo4 = (OneOptional)initial.pingPong(new OneOptional());
        test(!oo4.hasA());

        OneOptional oo5 = (OneOptional)initial.pingPong(oo1);
        test(oo1.getA() == oo5.getA());

        MultiOptional mo4 = (MultiOptional)initial.pingPong(new MultiOptional());
        test(!mo4.hasA());
        test(!mo4.hasB());
        test(!mo4.hasC());
        test(!mo4.hasD());
        test(!mo4.hasE());
        test(!mo4.hasF());
        test(!mo4.hasG());
        test(!mo4.hasH());
        test(!mo4.hasI());
        test(!mo4.hasJ());
        test(!mo4.hasK());
        test(!mo4.hasBs());
        test(!mo4.hasSs());
        test(!mo4.hasIid());
        test(!mo4.hasSid());
        test(!mo4.hasFs());
        test(!mo4.hasVs());

        test(!mo4.hasShs());
        test(!mo4.hasEs());
        test(!mo4.hasFss());
        test(!mo4.hasVss());
        test(!mo4.hasOos());
        test(!mo4.hasOops());

        test(!mo4.hasIed());
        test(!mo4.hasIfsd());
        test(!mo4.hasIvsd());
        test(!mo4.hasIood());
        test(!mo4.hasIoopd());

        test(!mo4.hasBos());

        test(!mo4.hasSer());

        final boolean supportsJavaSerializable = initial.supportsJavaSerializable();
        if(!supportsJavaSerializable)
        {
            mo1.clearSer();
        }

        MultiOptional mo5 = (MultiOptional)initial.pingPong(mo1);
        test(mo5.getA() == mo1.getA());
        test(mo5.getB() == mo1.getB());
        test(mo5.getC() == mo1.getC());
        test(mo5.getD() == mo1.getD());
        test(mo5.getE() == mo1.getE());
        test(mo5.getF() == mo1.getF());
        test(mo5.getG() == mo1.getG());
        test(mo5.getH().equals(mo1.getH()));
        test(mo5.getI() == mo1.getI());
        test(mo5.getJ().equals(mo1.getJ()));
        test(mo5.getK() == mo5);
        test(java.util.Arrays.equals(mo5.getBs(), mo1.getBs()));
        test(java.util.Arrays.equals(mo5.getSs(), mo1.getSs()));
        test(mo5.getIid().get(4) == 3);
        test(mo5.getSid().get("test") == 10);
        test(mo5.getFs().equals(mo1.getFs()));
        test(mo5.getVs().equals(mo1.getVs()));
        test(java.util.Arrays.equals(mo5.getShs(), mo1.getShs()));
        test(mo5.getEs()[0] == MyEnum.MyEnumMember && mo1.getEs()[1] == MyEnum.MyEnumMember);
        test(mo5.getFss()[0].equals(new FixedStruct(78)));
        test(mo5.getVss()[0].equals(new VarStruct("hello")));
        test(mo5.getOos()[0].getA() == 15);
        test(mo5.getOops()[0].equals(communicator.stringToProxy("test")));

        test(mo5.getIed().get(4) == MyEnum.MyEnumMember);
        test(mo5.getIfsd().get(4).equals(new FixedStruct(78)));
        test(mo5.getIvsd().get(5).equals(new VarStruct("hello")));
        test(mo5.getIood().get(5).getA() == 15);
        test(mo5.getIoopd().get(5).equals(communicator.stringToProxy("test")));

        test(java.util.Arrays.equals(mo5.getBos(), new boolean[] { false, true, false }));

        if(supportsJavaSerializable)
        {
            test(mo5.getSer().equals(mo1.getSer()));
        }

        // Clear the first half of the optional parameters
        MultiOptional mo6 = new MultiOptional();
        mo6.setB(mo5.getB());
        mo6.setD(mo5.getD());
        mo6.setF(mo5.getF());
        mo6.setH(mo5.getH());
        mo6.setJ(mo5.getJ());
        mo6.setBs(mo5.getBs());
        mo6.setIid(mo5.getIid());
        mo6.setFs(mo5.getFs());
        mo6.setShs(mo5.getShs());
        mo6.setFss(mo5.getFss());
        mo6.setOos(mo5.getOos());
        mo6.setIfsd(mo5.getIfsd());
        mo6.setIood(mo5.getIood());
        mo6.setBos(mo5.getBos());

        MultiOptional mo7 = (MultiOptional)initial.pingPong(mo6);
        test(!mo7.hasA());
        test(mo7.getB() == mo1.getB());
        test(!mo7.hasC());
        test(mo7.getD() == mo1.getD());
        test(!mo7.hasE());
        test(mo7.getF() == mo1.getF());
        test(!mo7.hasG());
        test(mo7.getH().equals(mo1.getH()));
        test(!mo7.hasI());
        test(mo7.getJ().equals(mo1.getJ()));
        test(!mo7.hasK());
        test(java.util.Arrays.equals(mo7.getBs(), mo1.getBs()));
        test(!mo7.hasSs());
        test(mo7.getIid().get(4) == 3);
        test(!mo7.hasSid());
        test(mo7.getFs().equals(mo1.getFs()));
        test(!mo7.hasVs());

        test(java.util.Arrays.equals(mo7.getShs(), mo1.getShs()));
        test(!mo7.hasEs());
        test(mo7.getFss()[0].equals(new FixedStruct(78)));
        test(!mo7.hasVss());
        test(mo7.getOos()[0].getA() == 15);
        test(!mo7.hasOops());

        test(!mo7.hasIed());
        test(mo7.getIfsd().get(4).equals(new FixedStruct(78)));
        test(!mo7.hasIvsd());
        test(mo7.getIood().get(5).getA() == 15);
        test(!mo7.hasIoopd());

        test(java.util.Arrays.equals(mo7.getBos(), new boolean[] { false, true, false }));

        // Clear the second half of the optional parameters
        MultiOptional mo8 = new MultiOptional();
        mo8.setA(mo5.getA());
        mo8.setC(mo5.getC());
        mo8.setE(mo5.getE());
        mo8.setG(mo5.getG());
        mo8.setI(mo5.getI());
        mo8.setK(mo8);
        mo8.setSs(mo5.getSs());
        mo8.setSid(mo5.getSid());
        mo8.setVs(mo5.getVs());

        mo8.setEs(mo5.getEs());
        mo8.setVss(mo5.getVss());
        mo8.setOops(mo5.getOops());

        mo8.setIed(mo5.getIed());
        mo8.setIvsd(mo5.getIvsd());
        mo8.setIoopd(mo5.getIoopd());

        MultiOptional mo9 = (MultiOptional)initial.pingPong(mo8);
        test(mo9.getA() == mo1.getA());
        test(!mo9.hasB());
        test(mo9.getC() == mo1.getC());
        test(!mo9.hasD());
        test(mo9.getE() == mo1.getE());
        test(!mo9.hasF());
        test(mo9.getG() == mo1.getG());
        test(!mo9.hasH());
        test(mo9.getI() == mo1.getI());
        test(!mo9.hasJ());
        test(mo9.getK() == mo9);
        test(!mo9.hasBs());
        test(java.util.Arrays.equals(mo9.getSs(), mo1.getSs()));
        test(!mo9.hasIid());
        test(mo9.getSid().get("test") == 10);
        test(!mo9.hasFs());
        test(mo9.getVs().equals(mo1.getVs()));

        test(!mo9.hasShs());
        test(mo9.getEs()[0] == MyEnum.MyEnumMember && mo1.getEs()[1] == MyEnum.MyEnumMember);
        test(!mo9.hasFss());
        test(mo9.getVss()[0].equals(new VarStruct("hello")));
        test(!mo9.hasOos());
        test(mo9.getOops()[0].equals(communicator.stringToProxy("test")));

        test(mo9.getIed().get(4) == MyEnum.MyEnumMember);
        test(!mo9.hasIfsd());
        test(mo9.getIvsd().get(5).equals(new VarStruct("hello")));
        test(!mo9.hasIood());
        test(mo9.getIoopd().get(5).equals(communicator.stringToProxy("test")));

        test(!mo9.hasBos());

        {
            OptionalWithCustom owc1 = new OptionalWithCustom();
            java.util.ArrayList<SmallStruct> l = new java.util.ArrayList<>();
            l.add(new SmallStruct((byte)5));
            l.add(new SmallStruct((byte)6));
            l.add(new SmallStruct((byte)7));
            owc1.setL(l);
            owc1.setS(new ClassVarStruct(5));
            OptionalWithCustom owc2 = (OptionalWithCustom)initial.pingPong(owc1);
            test(owc2.hasL());
            test(owc2.getL().equals(l));
            test(owc2.hasS());
            test(owc2.getS().a == 5);
        }

        //
        // Send a request using blobjects. Upon receival, we don't read
        // any of the optional members. This ensures the optional members
        // are skipped even if the receiver knows nothing about them.
        //
        factory.setEnabled(true);
        OutputStream os = new OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(oo1);
        os.endEncapsulation();
        byte[] inEncaps = os.finished();
        com.zeroc.Ice.Object.Ice_invokeResult inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
        test(inv.returnValue);
        InputStream in = new InputStream(communicator, inv.outParams);
        in.startEncapsulation();
        final Wrapper<TestObjectReader> cb = new Wrapper<>();
        in.readValue(v -> cb.value = v, TestObjectReader.class);
        in.endEncapsulation();
        test(cb.value != null);

        os = new OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(mo1);
        os.endEncapsulation();
        inEncaps = os.finished();
        inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
        test(inv.returnValue);
        in = new InputStream(communicator, inv.outParams);
        in.startEncapsulation();
        in.readValue(v -> cb.value = v, TestObjectReader.class);
        in.endEncapsulation();
        test(cb.value != null);
        factory.setEnabled(false);

        //
        // Use the 1.0 encoding with operations whose only class parameters are optional.
        //
        Optional<OneOptional> oo = Optional.of(new OneOptional(53));
        initial.sendOptionalClass(true, oo);
        InitialPrx initial2 = initial.ice_encodingVersion(com.zeroc.Ice.Util.Encoding_1_0);
        initial2.sendOptionalClass(true, oo);

        oo = initial.returnOptionalClass(true);
        test(oo.isPresent());
        oo = initial2.returnOptionalClass(true);
        test(!oo.isPresent());

        Recursive[] recursive1 = new Recursive[1];
        recursive1[0] = new Recursive();
        Recursive[] recursive2 = new Recursive[1];
        recursive2[0] = new Recursive();
        recursive1[0].setValue(recursive2);
        Recursive outer = new Recursive();
        outer.setValue(recursive1);
        initial.pingPong(outer);

        G g = new G();
        g.setGg1Opt(new G1("gg1Opt"));
        g.gg2 = new G2(10);
        g.setGg2Opt(new G2(20));
        g.gg1 = new G1("gg1");
        g = initial.opG(g);
        test("gg1Opt".equals(g.getGg1Opt().a));
        test(10 == g.gg2.a);
        test(20 == g.getGg2Opt().a);
        test("gg1".equals(g.gg1.a));

        initial.opVoid();

        os = new OutputStream(communicator);
        os.startEncapsulation();
        os.writeOptional(1, OptionalFormat.F4);
        os.writeInt(15);
        os.writeOptional(1, OptionalFormat.VSize);
        os.writeString("test");
        os.endEncapsulation();
        inEncaps = os.finished();
        inv = initial.ice_invoke("opVoid", OperationMode.Normal, inEncaps);
        test(inv.returnValue);

        out.println("ok");

        out.print("testing marshaling of large containers with fixed size elements... ");
        out.flush();
        MultiOptional mc = new MultiOptional();

        mc.setBs(new byte[1000]);
        mc.setShs(new short[300]);

        mc.setFss(new FixedStruct[300]);
        for(int i = 0; i < 300; ++i)
        {
            mc.getFss()[i] = new FixedStruct();
        }

        mc.setIfsd(new java.util.HashMap<>());
        for(int i = 0; i < 300; ++i)
        {
            mc.getIfsd().put(i, new FixedStruct());
        }

        mc = (MultiOptional)initial.pingPong(mc);
        test(mc.getBs().length == 1000);
        test(mc.getShs().length == 300);
        test(mc.getFss().length == 300);
        test(mc.getIfsd().size() == 300);

        factory.setEnabled(true);
        os = new OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(mc);
        os.endEncapsulation();
        inEncaps = os.finished();
        inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
        test(inv.returnValue);
        in = new InputStream(communicator, inv.outParams);
        in.startEncapsulation();
        in.readValue(v -> cb.value = v, TestObjectReader.class);
        in.endEncapsulation();
        test(cb.value != null);
        factory.setEnabled(false);

        out.println("ok");

        out.print("testing tag marshaling... ");
        out.flush();
        {
            B b = new B();
            B b2 = (B)initial.pingPong(b);
            test(!b2.hasMa());
            test(!b2.hasMb());
            test(!b2.hasMc());

            b.setMa(10);
            b.setMb(11);
            b.setMc(12);
            b.setMd(13);

            b2 = (B)initial.pingPong(b);
            test(b2.getMa() == 10);
            test(b2.getMb() == 11);
            test(b2.getMc() == 12);
            test(b2.getMd() == 13);

            factory.setEnabled(true);
            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(b);
            os.endEncapsulation();
            inEncaps = os.finished();
            inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
            test(inv.returnValue);
            in = new InputStream(communicator, inv.outParams);
            in.startEncapsulation();
            Wrapper<com.zeroc.Ice.Value> cbv = new Wrapper<>();
            in.readValue(v -> cbv.value = v);
            in.endEncapsulation();
            test(cbv.value != null);
            factory.setEnabled(false);
        }
        out.println("ok");

        out.print("testing marshalling of objects with optional objects...");
        out.flush();
        {
            F f = new F();

            f.setAf(new A());
            f.ae = f.getAf();

            F rf = (F)initial.pingPong(f);
            test(rf.ae == rf.getAf());

            factory.setEnabled(true);
            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(f);
            os.endEncapsulation();
            inEncaps = os.finished();
            in = new InputStream(communicator, inEncaps);
            in.startEncapsulation();
            final Wrapper<F> w = new Wrapper<>();
            in.readValue(v -> w.value = v.getF(), FObjectReader.class);
            in.endEncapsulation();
            factory.setEnabled(false);
            test(w.value.ae != null && !w.value.hasAf());
        }
        out.println("ok");

        out.print("testing optional with default values... ");
        out.flush();
        {
            WD wd = (WD)initial.pingPong(new WD());
            test(wd.getA() == 5);
            test(wd.getS().equals("test"));
            wd.clearA();
            wd.clearS();
            wd = (WD)initial.pingPong(wd);
            test(!wd.hasA());
            test(!wd.hasS());
        }
        out.println("ok");

        if(communicator.getProperties().getPropertyAsInt("Ice.Default.SlicedFormat") > 0)
        {
            out.print("testing marshaling with unknown class slices... ");
            out.flush();
            {
                C c = new C();
                c.ss = "test";
                c.setMs("testms");
                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(c);
                os.endEncapsulation();
                inEncaps = os.finished();
                factory.setEnabled(true);
                inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
                test(inv.returnValue);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                Wrapper<CObjectReader> ccb = new Wrapper<>();
                in.readValue(v -> ccb.value = v, CObjectReader.class);
                in.endEncapsulation();
                test(ccb.value != null);
                factory.setEnabled(false);

                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.startEncapsulation();
                com.zeroc.Ice.Value d = new DObjectWriter();
                os.writeValue(d);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("pingPong", OperationMode.Normal, inEncaps);
                test(inv.returnValue);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                Wrapper<DObjectReader> dcb = new Wrapper<>();
                in.readValue(v -> dcb.value = v, DObjectReader.class);
                in.endEncapsulation();
                test(dcb.value != null);
                dcb.value.check();
                factory.setEnabled(false);
            }
            out.println("ok");

            out.print("testing optionals with unknown classes...");
            out.flush();
            {
                A a = new A();

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(a);
                os.writeOptional(1, OptionalFormat.Class);
                os.writeValue(new DObjectWriter());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opClassAndUnknownOptional", OperationMode.Normal, inEncaps);
                test(inv.returnValue);

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
            out.println("ok");
        }

        out.print("testing optional parameters... ");
        out.flush();
        final boolean reqParams = initial.supportsRequiredParams();

        {
            Optional<Byte> p1 = Optional.empty();
            Initial.OpByteResult r = initial.opByte(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of((byte)56);
            r = initial.opByte(p1);
            test(r.returnValue.get() == (byte)56 && r.p3.get() == (byte)56);
            r = initial.opByteAsync(p1).join();
            test(r.returnValue.get() == (byte)56 && r.p3.get() == (byte)56);

            if(reqParams)
            {
                Initial.OpByteReqResult rr = initial.opByteReq(p1.get());
                test(rr.returnValue.get() == (byte)56 && rr.p3.get() == (byte)56);
                rr = initial.opByteReqAsync(p1.get()).join();
                test(rr.returnValue.get() == (byte)56 && rr.p3.get() == (byte)56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F1);
                os.writeByte(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opByteReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readByte(1).get() == (byte)56);
                test(in.readByte(3).get() == (byte)56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<Boolean> p1 = Optional.empty();
            Initial.OpBoolResult r = initial.opBool(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(true);
            r = initial.opBool(p1);
            test(r.returnValue.get() == true && r.p3.get() == true);
            r = initial.opBoolAsync(p1).join();
            test(r.returnValue.get() == true && r.p3.get() == true);

            if(reqParams)
            {
                Initial.OpBoolReqResult rr = initial.opBoolReq(true);
                test(rr.returnValue.get() == true && rr.p3.get() == true);
                rr = initial.opBoolReqAsync(true).join();
                test(rr.returnValue.get() == true && rr.p3.get() == true);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F1);
                os.writeBool(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opBoolReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readBool(1).get() == true);
                test(in.readBool(3).get() == true);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<Short> p1 = Optional.empty();
            Initial.OpShortResult r = initial.opShort(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of((short)56);
            r = initial.opShort(p1);
            test(r.returnValue.get() == 56 && r.p3.get() == 56);
            r = initial.opShortAsync(p1).join();
            test(r.returnValue.get() == 56 && r.p3.get() == 56);

            if(reqParams)
            {
                Initial.OpShortReqResult rr = initial.opShortReq(p1.get());
                test(rr.returnValue.get() == 56 && rr.p3.get() == 56);
                rr = initial.opShortReqAsync(p1.get()).join();
                test(rr.returnValue.get() == 56 && rr.p3.get() == 56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F2);
                os.writeShort(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opShortReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readShort(1).get() == 56);
                test(in.readShort(3).get() == 56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            OptionalInt p1 = OptionalInt.empty();
            Initial.OpIntResult r = initial.opInt(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = OptionalInt.of(56);
            r = initial.opInt(p1);
            test(r.returnValue.getAsInt() == 56 && r.p3.getAsInt() == 56);
            r = initial.opIntAsync(p1).join();
            test(r.returnValue.getAsInt() == 56 && r.p3.getAsInt() == 56);

            if(reqParams)
            {
                Initial.OpIntReqResult rr = initial.opIntReq(p1.getAsInt());
                test(rr.returnValue.getAsInt() == 56 && rr.p3.getAsInt() == 56);
                rr = initial.opIntReqAsync(p1.getAsInt()).join();
                test(rr.returnValue.getAsInt() == 56 && rr.p3.getAsInt() == 56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F4);
                os.writeInt(p1.getAsInt());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opIntReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readInt(1).getAsInt() == 56);
                test(in.readInt(3).getAsInt() == 56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            OptionalLong p1 = OptionalLong.empty();
            Initial.OpLongResult r = initial.opLong(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = OptionalLong.of(56);
            r = initial.opLong(p1);
            test(r.returnValue.getAsLong() == 56 && r.p3.getAsLong() == 56);
            r = initial.opLongAsync(p1).join();
            test(r.returnValue.getAsLong() == 56 && r.p3.getAsLong() == 56);

            if(reqParams)
            {
                Initial.OpLongReqResult rr = initial.opLongReq(p1.getAsLong());
                test(rr.returnValue.getAsLong() == 56 && rr.p3.getAsLong() == 56);
                rr = initial.opLongReqAsync(p1.getAsLong()).join();
                test(rr.returnValue.getAsLong() == 56 && rr.p3.getAsLong() == 56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(1, OptionalFormat.F8);
                os.writeLong(p1.getAsLong());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opLongReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readLong(2).getAsLong() == 56);
                test(in.readLong(3).getAsLong() == 56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<Float> p1 = Optional.empty();
            Initial.OpFloatResult r = initial.opFloat(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of((float)1.0);
            r = initial.opFloat(p1);
            test(r.returnValue.get() == 1.0 && r.p3.get() == 1.0);
            r = initial.opFloatAsync(p1).join();
            test(r.returnValue.get() == 1.0 && r.p3.get() == 1.0);

            if(reqParams)
            {
                Initial.OpFloatReqResult rr = initial.opFloatReq(p1.get());
                test(rr.returnValue.get() == 1.0 && rr.p3.get() == 1.0);
                rr = initial.opFloatReqAsync(p1.get()).join();
                test(rr.returnValue.get() == 1.0 && rr.p3.get() == 1.0);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F4);
                os.writeFloat(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opFloatReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readFloat(1).get() == 1.0);
                test(in.readFloat(3).get() == 1.0);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            OptionalDouble p1 = OptionalDouble.empty();
            Initial.OpDoubleResult r = initial.opDouble(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = OptionalDouble.of(1.0);
            r = initial.opDouble(p1);
            test(r.returnValue.getAsDouble() == 1.0 && r.p3.getAsDouble() == 1.0);
            r = initial.opDoubleAsync(p1).join();
            test(r.returnValue.getAsDouble() == 1.0 && r.p3.getAsDouble() == 1.0);

            if(reqParams)
            {
                Initial.OpDoubleReqResult rr = initial.opDoubleReq(p1.getAsDouble());
                test(rr.returnValue.getAsDouble() == 1.0 && rr.p3.getAsDouble() == 1.0);
                rr = initial.opDoubleReqAsync(p1.getAsDouble()).join();
                test(rr.returnValue.getAsDouble() == 1.0 && rr.p3.getAsDouble() == 1.0);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.F8);
                os.writeDouble(p1.getAsDouble());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opDoubleReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readDouble(1).getAsDouble() == 1.0);
                test(in.readDouble(3).getAsDouble() == 1.0);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<String> p1 = Optional.empty();
            Initial.OpStringResult r = initial.opString(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of("test");
            r = initial.opString(p1);
            test(r.returnValue.get().equals("test") && r.p3.get().equals("test"));
            r = initial.opStringAsync(p1).join();
            test(r.returnValue.get().equals("test") && r.p3.get().equals("test"));

            if(reqParams)
            {
                Initial.OpStringReqResult rr = initial.opStringReq(p1.get());
                test(rr.returnValue.get().equals("test") && rr.p3.get().equals("test"));
                rr = initial.opStringReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals("test") && rr.p3.get().equals("test"));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeString(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opStringReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readString(1).get().equals("test"));
                test(in.readString(3).get().equals("test"));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<MyEnum> p1 = Optional.empty();
            Initial.OpMyEnumResult r = initial.opMyEnum(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(MyEnum.MyEnumMember);
            r = initial.opMyEnum(p1);
            test(r.returnValue.get() == MyEnum.MyEnumMember && r.p3.get() == MyEnum.MyEnumMember);
            r = initial.opMyEnumAsync(p1).join();
            test(r.returnValue.get() == MyEnum.MyEnumMember && r.p3.get() == MyEnum.MyEnumMember);

            if(reqParams)
            {
                Initial.OpMyEnumReqResult rr = initial.opMyEnumReq(p1.get());
                test(rr.returnValue.get() == MyEnum.MyEnumMember && rr.p3.get() == MyEnum.MyEnumMember);
                rr = initial.opMyEnumReqAsync(p1.get()).join();
                test(rr.returnValue.get() == MyEnum.MyEnumMember && rr.p3.get() == MyEnum.MyEnumMember);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.Size);
                MyEnum.ice_write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opMyEnumReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.Size));
                test(MyEnum.ice_read(in) == MyEnum.MyEnumMember);
                test(in.readOptional(3, OptionalFormat.Size));
                test(MyEnum.ice_read(in) == MyEnum.MyEnumMember);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<SmallStruct> p1 = Optional.empty();
            Initial.OpSmallStructResult r = initial.opSmallStruct(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new SmallStruct((byte)56));
            r = initial.opSmallStruct(p1);
            test(r.returnValue.get().m == (byte)56 && r.p3.get().m == (byte)56);
            r = initial.opSmallStruct(Optional.of(new SmallStruct()));
            test(r.returnValue.get().m == (byte)0 && r.p3.get().m == (byte)0);
            r = initial.opSmallStructAsync(p1).join();
            test(r.returnValue.get().m == (byte)56 && r.p3.get().m == (byte)56);

            if(reqParams)
            {
                Initial.OpSmallStructReqResult rr = initial.opSmallStructReq(p1.get());
                test(rr.returnValue.get().m == (byte)56 && rr.p3.get().m == (byte)56);
                rr = initial.opSmallStructReqAsync(p1.get()).join();
                test(rr.returnValue.get().m == (byte)56 && rr.p3.get().m == (byte)56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(1);
                SmallStruct.ice_write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opSmallStructReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                SmallStruct f = SmallStruct.ice_read(in);
                test(f.m == (byte)56);
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                f = SmallStruct.ice_read(in);
                test(f.m == (byte)56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<FixedStruct> p1 = Optional.empty();
            Initial.OpFixedStructResult r = initial.opFixedStruct(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new FixedStruct(56));
            r = initial.opFixedStruct(p1);
            test(r.returnValue.get().m == 56 && r.p3.get().m == 56);
            r = initial.opFixedStructAsync(p1).join();
            test(r.returnValue.get().m == 56 && r.p3.get().m == 56);

            if(reqParams)
            {
                Initial.OpFixedStructReqResult rr = initial.opFixedStructReq(p1.get());
                test(rr.returnValue.get().m == 56 && rr.p3.get().m == 56);
                rr = initial.opFixedStructReqAsync(p1.get()).join();
                test(rr.returnValue.get().m == 56 && rr.p3.get().m == 56);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(4);
                FixedStruct.ice_write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opFixedStructReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                FixedStruct f = FixedStruct.ice_read(in);
                test(f.m == 56);
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                f = FixedStruct.ice_read(in);
                test(f.m == 56);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<VarStruct> p1 = Optional.empty();
            Initial.OpVarStructResult r = initial.opVarStruct(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new VarStruct("test"));
            r = initial.opVarStruct(p1);
            test(r.returnValue.get().m.equals("test") && r.p3.get().m.equals("test"));
            r = initial.opVarStructAsync(p1).join();
            test(r.returnValue.get().m.equals("test") && r.p3.get().m.equals("test"));

            if(reqParams)
            {
                Initial.OpVarStructReqResult rr = initial.opVarStructReq(p1.get());
                test(rr.returnValue.get().m.equals("test") && rr.p3.get().m.equals("test"));
                rr = initial.opVarStructReqAsync(p1.get()).join();
                test(rr.returnValue.get().m.equals("test") && rr.p3.get().m.equals("test"));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                VarStruct.ice_write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opVarStructReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.FSize));
                in.skip(4);
                VarStruct v = VarStruct.ice_read(in);
                test(v.m.equals("test"));
                test(in.readOptional(3, OptionalFormat.FSize));
                in.skip(4);
                v = VarStruct.ice_read(in);
                test(v.m.equals("test"));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<OneOptional> p1 = Optional.empty();
            Initial.OpOneOptionalResult r = initial.opOneOptional(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new OneOptional(58));
            r = initial.opOneOptional(p1);
            test(r.returnValue.get().getA() == 58 && r.p3.get().getA() == 58);
            r = initial.opOneOptionalAsync(p1).join();
            test(r.returnValue.get().getA() == 58 && r.p3.get().getA() == 58);

            if(reqParams)
            {
                Initial.OpOneOptionalReqResult rr = initial.opOneOptionalReq(p1.get());
                test(rr.returnValue.get().getA() == 58 && rr.p3.get().getA() == 58);
                rr = initial.opOneOptionalReqAsync(p1.get()).join();
                test(rr.returnValue.get().getA() == 58 && rr.p3.get().getA() == 58);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.Class);
                os.writeValue(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opOneOptionalReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                Wrapper<java.util.Optional<OneOptional>> p2cb = new Wrapper<>();
                in.readValue(1, v -> p2cb.value = v, OneOptional.class);
                Wrapper<java.util.Optional<OneOptional>> p3cb = new Wrapper<>();
                in.readValue(3, v -> p3cb.value = v, OneOptional.class);
                in.endEncapsulation();
                test(p2cb.value.get().getA() == 58 && p3cb.value.get().getA() == 58);

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<com.zeroc.Ice.ObjectPrx> p1 = Optional.empty();
            Initial.OpOneOptionalProxyResult r = initial.opOneOptionalProxy(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(communicator.stringToProxy("test"));
            r = initial.opOneOptionalProxy(p1);
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));
            r = initial.opOneOptionalProxyAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpOneOptionalProxyReqResult rr = initial.opOneOptionalProxyReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));
                rr = initial.opOneOptionalProxyReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeProxy(p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opOneOptionalProxyReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readProxy(1).get().equals(p1.get()));
                test(in.readProxy(3).get().equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }
        out.println("ok");

        out.print("testing optional parameters and sequences... ");
        out.flush();
        {
            Optional<byte[]> p1 = Optional.empty();
            Initial.OpByteSeqResult r = initial.opByteSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new byte[100]);
            java.util.Arrays.fill(p1.get(), (byte)56);
            r = initial.opByteSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opByteSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpByteSeqReqResult rr = initial.opByteSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opByteSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeByteSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opByteSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readByteSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readByteSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<boolean[]> p1 = Optional.empty();
            Initial.OpBoolSeqResult r = initial.opBoolSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new boolean[100]);
            r = initial.opBoolSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opBoolSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpBoolSeqReqResult rr = initial.opBoolSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opBoolSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeBoolSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opBoolSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readBoolSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readBoolSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<short[]> p1 = Optional.empty();
            Initial.OpShortSeqResult r = initial.opShortSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new short[100]);
            java.util.Arrays.fill(p1.get(), (short)56);
            r = initial.opShortSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opShortSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpShortSeqReqResult rr = initial.opShortSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opShortSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 2 + (p1.get().length > 254 ? 5 : 1));
                os.writeShortSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opShortSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readShortSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readShortSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<int[]> p1 = Optional.empty();
            Initial.OpIntSeqResult r = initial.opIntSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new int[100]);
            java.util.Arrays.fill(p1.get(), 56);
            r = initial.opIntSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opIntSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpIntSeqReqResult rr = initial.opIntSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opIntSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                os.writeIntSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opIntSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readIntSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readIntSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<long[]> p1 = Optional.empty();
            Initial.OpLongSeqResult r = initial.opLongSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new long[100]);
            java.util.Arrays.fill(p1.get(), 56);
            r = initial.opLongSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opLongSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpLongSeqReqResult rr = initial.opLongSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opLongSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 8 + (p1.get().length > 254 ? 5 : 1));
                os.writeLongSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opLongSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readLongSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readLongSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<float[]> p1 = Optional.empty();
            Initial.OpFloatSeqResult r = initial.opFloatSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new float[100]);
            java.util.Arrays.fill(p1.get(), (float)1.0);
            r = initial.opFloatSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opFloatSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpFloatSeqReqResult rr = initial.opFloatSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opFloatSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                os.writeFloatSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opFloatSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readFloatSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readFloatSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<double[]> p1 = Optional.empty();
            Initial.OpDoubleSeqResult r = initial.opDoubleSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new double[100]);
            java.util.Arrays.fill(p1.get(), 1.0);
            r = initial.opDoubleSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opDoubleSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpDoubleSeqReqResult rr = initial.opDoubleSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opDoubleSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 8 + (p1.get().length > 254 ? 5 : 1));
                os.writeDoubleSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opDoubleSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readDoubleSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readDoubleSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<String[]> p1 = Optional.empty();
            Initial.OpStringSeqResult r = initial.opStringSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new String[10]);
            java.util.Arrays.fill(p1.get(), "test1");
            r = initial.opStringSeq(p1);
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));
            r = initial.opStringSeqAsync(p1).join();
            test(java.util.Arrays.equals(r.returnValue.get(), p1.get()) &&
                 java.util.Arrays.equals(r.p3.get(), p1.get()));

            if(reqParams)
            {
                Initial.OpStringSeqReqResult rr = initial.opStringSeqReq(p1.get());
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));
                rr = initial.opStringSeqReqAsync(p1.get()).join();
                test(java.util.Arrays.equals(rr.returnValue.get(), p1.get()) &&
                     java.util.Arrays.equals(rr.p3.get(), p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeStringSeq(p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opStringSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(java.util.Arrays.equals(in.readStringSeq(1).get(), p1.get()));
                test(java.util.Arrays.equals(in.readStringSeq(3).get(), p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<SmallStruct[]> p1 = Optional.empty();
            Initial.OpSmallStructSeqResult r = initial.opSmallStructSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new SmallStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new SmallStruct();
            }
            r = initial.opSmallStructSeq(p1);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.p3.get()[i].equals(p1.get()[i]));
            }
            r = initial.opSmallStructSeqAsync(p1).join();
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.returnValue.get()[i].equals(p1.get()[i]));
            }

            if(reqParams)
            {
                Initial.OpSmallStructSeqReqResult rr = initial.opSmallStructSeqReq(p1.get());
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }
                rr = initial.opSmallStructSeqReqAsync(p1.get()).join();
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length + (p1.get().length > 254 ? 5 : 1));
                SmallStructSeqHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opSmallStructSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                SmallStruct[] arr = SmallStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                arr = SmallStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<java.util.List<SmallStruct>> p1 = Optional.empty();
            Initial.OpSmallStructListResult r = initial.opSmallStructList(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new java.util.ArrayList<>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new SmallStruct());
            }
            r = initial.opSmallStructList(p1);
            test(r.returnValue.get().equals(p1.get()));
            r = initial.opSmallStructListAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpSmallStructListReqResult rr = initial.opSmallStructListReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()));
                rr = initial.opSmallStructListReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().size() + (p1.get().size() > 254 ? 5 : 1));
                SmallStructListHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opSmallStructListReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                java.util.List<SmallStruct> arr = SmallStructListHelper.read(in);
                test(arr.equals(p1.get()));
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                arr = SmallStructListHelper.read(in);
                test(arr.equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<FixedStruct[]> p1 = Optional.empty();
            Initial.OpFixedStructSeqResult r = initial.opFixedStructSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new FixedStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new FixedStruct();
            }
            r = initial.opFixedStructSeq(p1);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.returnValue.get()[i].equals(p1.get()[i]));
            }
            r = initial.opFixedStructSeqAsync(p1).join();
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.returnValue.get()[i].equals(p1.get()[i]));
            }

            if(reqParams)
            {
                Initial.OpFixedStructSeqReqResult rr = initial.opFixedStructSeqReq(p1.get());
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }
                rr = initial.opFixedStructSeqReqAsync(p1.get()).join();
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                FixedStructSeqHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opFixedStructSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                FixedStruct[] arr = FixedStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                arr = FixedStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<java.util.List<FixedStruct>> p1 = Optional.empty();
            Initial.OpFixedStructListResult r = initial.opFixedStructList(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new java.util.ArrayList<>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new FixedStruct());
            }
            r = initial.opFixedStructList(p1);
            test(r.returnValue.get().equals(p1.get()));
            r = initial.opFixedStructListAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpFixedStructListReqResult rr = initial.opFixedStructListReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()));
                rr = initial.opFixedStructListReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().size() * 4 + (p1.get().size() > 254 ? 5 : 1));
                FixedStructListHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opFixedStructListReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                java.util.List<FixedStruct> arr = FixedStructListHelper.read(in);
                test(arr.equals(p1.get()));
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                arr = FixedStructListHelper.read(in);
                test(arr.equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<VarStruct[]> p1 = Optional.empty();
            Initial.OpVarStructSeqResult r = initial.opVarStructSeq(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new VarStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new VarStruct("");
            }
            r = initial.opVarStructSeq(p1);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.returnValue.get()[i].equals(p1.get()[i]));
            }
            r = initial.opVarStructSeqAsync(p1).join();
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(r.returnValue.get()[i].equals(p1.get()[i]));
            }

            if(reqParams)
            {
                Initial.OpVarStructSeqReqResult rr = initial.opVarStructSeqReq(p1.get());
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }
                rr = initial.opVarStructSeqReqAsync(p1.get()).join();
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(rr.returnValue.get()[i].equals(p1.get()[i]));
                }

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                VarStructSeqHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opVarStructSeqReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.FSize));
                in.skip(4);
                VarStruct[] arr = VarStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, OptionalFormat.FSize));
                in.skip(4);
                arr = VarStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        if(supportsJavaSerializable)
        {
            Optional<SerializableClass> p1 = Optional.empty();
            Initial.OpSerializableResult r = initial.opSerializable(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new SerializableClass(58));
            r = initial.opSerializable(p1);
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));
            r = initial.opSerializableAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpSerializableReqResult rr = initial.opSerializableReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));
                rr = initial.opSerializableReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSerializable(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opSerializableReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                SerializableClass sc = in.readSerializable(SerializableClass.class);
                test(sc.equals(p1.get()));
                test(in.readOptional(3, OptionalFormat.VSize));
                sc = in.readSerializable(SerializableClass.class);
                test(sc.equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }
        out.println("ok");

        out.print("testing optional parameters and dictionaries... ");
        out.flush();
        {
            Optional<java.util.Map<Integer, Integer>> p1 = Optional.empty();
            Initial.OpIntIntDictResult r = initial.opIntIntDict(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new java.util.HashMap<>());
            p1.get().put(1, 2);
            p1.get().put(2, 3);
            r = initial.opIntIntDict(p1);
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));
            r = initial.opIntIntDictAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpIntIntDictReqResult rr = initial.opIntIntDictReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));
                rr = initial.opIntIntDictReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.VSize);
                os.writeSize(p1.get().size() * 8 + (p1.get().size() > 254 ? 5 : 1));
                IntIntDictHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opIntIntDictReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.VSize));
                in.skipSize();
                java.util.Map<Integer, Integer> m = IntIntDictHelper.read(in);
                test(m.equals(p1.get()));
                test(in.readOptional(3, OptionalFormat.VSize));
                in.skipSize();
                m = IntIntDictHelper.read(in);
                test(m.equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<java.util.Map<String, Integer>> p1 = Optional.empty();
            Initial.OpStringIntDictResult r = initial.opStringIntDict(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new java.util.HashMap<>());
            p1.get().put("1", 1);
            p1.get().put("2", 2);
            r = initial.opStringIntDict(p1);
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));
            r = initial.opStringIntDictAsync(p1).join();
            test(r.returnValue.get().equals(p1.get()) && r.p3.get().equals(p1.get()));

            if(reqParams)
            {
                Initial.OpStringIntDictReqResult rr = initial.opStringIntDictReq(p1.get());
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));
                rr = initial.opStringIntDictReqAsync(p1.get()).join();
                test(rr.returnValue.get().equals(p1.get()) && rr.p3.get().equals(p1.get()));

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                StringIntDictHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opStringIntDictReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.FSize));
                in.skip(4);
                java.util.Map<String, Integer> m = StringIntDictHelper.read(in);
                test(m.equals(p1.get()));
                test(in.readOptional(3, OptionalFormat.FSize));
                in.skip(4);
                m = StringIntDictHelper.read(in);
                test(m.equals(p1.get()));
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Optional<java.util.Map<Integer, OneOptional>> p1 = Optional.empty();
            Optional<java.util.Map<Integer, OneOptional>> p3 = Optional.empty();
            Initial.OpIntOneOptionalDictResult r = initial.opIntOneOptionalDict(p1);
            test(!r.returnValue.isPresent() && !r.p3.isPresent());

            p1 = Optional.of(new java.util.HashMap<>());
            p1.get().put(1, new OneOptional(15));
            p1.get().put(2, new OneOptional(12));
            r = initial.opIntOneOptionalDict(p1);
            test(r.returnValue.get().get(1).getA() == 15 && r.p3.get().get(1).getA() == 15);
            test(r.returnValue.get().get(2).getA() == 12 && r.p3.get().get(2).getA() == 12);
            r = initial.opIntOneOptionalDictAsync(p1).join();
            test(r.returnValue.get().get(1).getA() == 15 && r.p3.get().get(1).getA() == 15);
            test(r.returnValue.get().get(2).getA() == 12 && r.p3.get().get(2).getA() == 12);

            if(reqParams)
            {
                Initial.OpIntOneOptionalDictReqResult rr = initial.opIntOneOptionalDictReq(p1.get());
                test(rr.returnValue.get().get(1).getA() == 15 && rr.p3.get().get(1).getA() == 15);
                test(rr.returnValue.get().get(2).getA() == 12 && rr.p3.get().get(2).getA() == 12);
                rr = initial.opIntOneOptionalDictReqAsync(p1.get()).join();
                test(rr.returnValue.get().get(1).getA() == 15 && rr.p3.get().get(1).getA() == 15);
                test(rr.returnValue.get().get(2).getA() == 12 && rr.p3.get().get(2).getA() == 12);

                os = new OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, OptionalFormat.FSize);
                int pos = os.startSize();
                IntOneOptionalDictHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                inv = initial.ice_invoke("opIntOneOptionalDictReq", OperationMode.Normal, inEncaps);
                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                test(in.readOptional(1, OptionalFormat.FSize));
                in.skip(4);
                java.util.Map<Integer, OneOptional> m = IntOneOptionalDictHelper.read(in);
                test(m.get(1).getA() == 15 && m.get(2).getA() == 12);
                test(in.readOptional(3, OptionalFormat.FSize));
                in.skip(4);
                m = IntOneOptionalDictHelper.read(in);
                test(m.get(1).getA() == 15 && m.get(2).getA() == 12);
                in.endEncapsulation();

                in = new InputStream(communicator, inv.outParams);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            F f = new F();
            f.setAf(new A());
            f.getAf().requiredA = 56;
            f.ae = f.getAf();

            os = new OutputStream(communicator);
            os.startEncapsulation();
            os.writeOptional(1, OptionalFormat.Class);
            os.writeValue(f);
            os.writeOptional(2, OptionalFormat.Class);
            os.writeValue(f.ae);
            os.endEncapsulation();
            inEncaps = os.finished();

            in = new InputStream(communicator, inEncaps);
            in.startEncapsulation();
            final Wrapper<java.util.Optional<A>> w = new Wrapper<>();
            in.readValue(2, v -> w.value = v, A.class);
            in.endEncapsulation();
            test(w.value.get() != null && w.value.get().requiredA == 56);
        }
        out.println("ok");

        out.print("testing exception optionals... ");
        out.flush();
        {
            try
            {
                OptionalInt a = OptionalInt.empty();
                Optional<String> b = Optional.empty();
                Optional<OneOptional> o = Optional.empty();
                initial.opOptionalException(a, b, o);
            }
            catch(OptionalException ex)
            {
                test(!ex.hasA());
                test(!ex.hasB());
                test(!ex.hasO());
            }

            try
            {
                OptionalInt a = OptionalInt.of(30);
                Optional<String> b = Optional.of("test");
                Optional<OneOptional> o = Optional.of(new OneOptional(53));
                initial.opOptionalException(a, b, o);
            }
            catch(OptionalException ex)
            {
                test(ex.getA() == 30);
                test(ex.getB().equals("test"));
                test(ex.getO().getA() == 53);
            }

            try
            {
                //
                // Use the 1.0 encoding with an exception whose only class members are optional.
                //
                OptionalInt a = OptionalInt.of(30);
                Optional<String> b = Optional.of("test");
                Optional<OneOptional> o = Optional.of(new OneOptional(53));
                initial2.opOptionalException(a, b, o);
            }
            catch(OptionalException ex)
            {
                test(!ex.hasA());
                test(!ex.hasB());
                test(!ex.hasO());
            }

            try
            {
                OptionalInt a = OptionalInt.empty();
                Optional<String> b = Optional.empty();
                Optional<OneOptional> o = Optional.empty();
                initial.opDerivedException(a, b, o);
            }
            catch(DerivedException ex)
            {
                test(!ex.hasA());
                test(!ex.hasB());
                test(!ex.hasO());
                test(!ex.hasSs());
                test(!ex.hasO2());
            }
            catch(OptionalException ex)
            {
                test(false);
            }

            try
            {
                OptionalInt a = OptionalInt.of(30);
                Optional<String> b = Optional.of("test2");
                Optional<OneOptional> o = Optional.of(new OneOptional(53));
                initial.opDerivedException(a, b, o);
            }
            catch(DerivedException ex)
            {
                test(ex.getA() == 30);
                test(ex.getB().equals("test2"));
                test(ex.getO().getA() == 53);
                test(ex.getSs().equals("test2"));
                test(ex.getO2().getA() == 53);
            }
            catch(OptionalException ex)
            {
                test(false);
            }

            try
            {
                OptionalInt a = OptionalInt.empty();
                Optional<String> b = Optional.empty();
                Optional<OneOptional> o = Optional.empty();
                initial.opRequiredException(a, b, o);
            }
            catch(RequiredException ex)
            {
                test(!ex.hasA());
                test(!ex.hasB());
                test(!ex.hasO());
                test(ex.ss.equals("test"));
                test(ex.o2 == null);
            }
            catch(OptionalException ex)
            {
                test(false);
            }

            try
            {
                OptionalInt a = OptionalInt.of(30);
                Optional<String> b = Optional.of("test2");
                Optional<OneOptional> o = Optional.of(new OneOptional(53));
                initial.opRequiredException(a, b, o);
            }
            catch(RequiredException ex)
            {
                test(ex.getA() == 30);
                test(ex.getB().equals("test2"));
                test(ex.getO().getA() == 53);
                test(ex.ss.equals("test2"));
                test(ex.o2.getA() == 53);
            }
            catch(OptionalException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("testing optionals with marshaled results... ");
        out.flush();
        {
            test(initial.opMStruct1().isPresent());
            test(initial.opMDict1().isPresent());
            test(initial.opMSeq1().isPresent());
            test(initial.opMG1().isPresent());

            {
                Initial.OpMStruct2Result result = initial.opMStruct2(Optional.empty());
                test(!result.returnValue.isPresent() && !result.returnValue.isPresent());

                SmallStruct p1 = new SmallStruct();
                result = initial.opMStruct2(Optional.of(p1));
                test(result.returnValue.get().equals(p1) && result.p2.get().equals(p1));
            }
            {
                Initial.OpMSeq2Result result = initial.opMSeq2(Optional.empty());
                test(!result.p2.isPresent() && !result.returnValue.isPresent());

                String[] p1 = { "hello" };
                result = initial.opMSeq2(Optional.of(p1));
                test(java.util.Arrays.equals(result.p2.get(), p1) &&
                     java.util.Arrays.equals(result.returnValue.get(), p1));
            }
            {
                Initial.OpMDict2Result result = initial.opMDict2(Optional.empty());
                test(!result.p2.isPresent() && !result.returnValue.isPresent());

                java.util.Map<String, Integer> p1 = new java.util.HashMap<>();
                p1.put("test", 54);
                result = initial.opMDict2(Optional.of(p1));
                test(result.p2.get().equals(p1) && result.returnValue.get().equals(p1));
            }
            {
                Initial.OpMG2Result result = initial.opMG2(Optional.empty());
                test(!result.p2.isPresent() && !result.returnValue.isPresent());

                G p1 = new G();
                result = initial.opMG2(Optional.of(p1));
                test(result.p2.get() == result.returnValue.get());
            }
        }
        out.println("ok");

        return initial;
    }

    private static class TestObjectReader extends com.zeroc.Ice.ValueReader
    {
        @Override
        public void read(InputStream in)
        {
            in.startValue();
            in.startSlice();
            in.endSlice();
            in.endValue(false);
        }
    }

    private static class BObjectReader extends com.zeroc.Ice.ValueReader
    {
        @Override
        public void read(InputStream in)
        {
            in.startValue();
            // ::Test::B
            in.startSlice();
            in.readInt();
            in.endSlice();
            // ::Test::A
            in.startSlice();
            in.readInt();
            in.endSlice();
            in.endValue(false);
        }
    }

    private static class CObjectReader extends com.zeroc.Ice.ValueReader
    {
        @Override
        public void read(InputStream in)
        {
            in.startValue();
            // ::Test::C
            in.startSlice();
            in.skipSlice();
            // ::Test::B
            in.startSlice();
            in.readInt();
            in.endSlice();
            // ::Test::A
            in.startSlice();
            in.readInt();
            in.endSlice();
            in.endValue(false);
        }
    }

    private static class DObjectWriter extends com.zeroc.Ice.ValueWriter
    {
        @Override
        public void write(OutputStream out)
        {
            out.startValue(null);
            // ::Test::D
            out.startSlice("::Test::D", -1, false);
            out.writeString("test");
            out.writeOptional(1, OptionalFormat.FSize);
            String[] o = { "test1", "test2", "test3", "test4" };
            int pos = out.startSize();
            out.writeStringSeq(o);
            out.endSize(pos);
            A a = new A();
            a.setMc(18);
            out.writeOptional(1000, OptionalFormat.Class);
            out.writeValue(a);
            out.endSlice();
            // ::Test::B
            out.startSlice(B.ice_staticId(), -1, false);
            int v = 14;
            out.writeInt(v);
            out.endSlice();
            // ::Test::A
            out.startSlice(A.ice_staticId(), -1, true);
            out.writeInt(v);
            out.endSlice();
            out.endValue();
        }
    }

    private static class DObjectReader extends com.zeroc.Ice.ValueReader
    {
        @Override
        public void read(InputStream in)
        {
            in.startValue();
            // ::Test::D
            in.startSlice();
            String s = in.readString();
            test(s.equals("test"));
            String[] o = in.readStringSeq(1).get();
            test(o.length == 4 &&
                 o[0].equals("test1") && o[1].equals("test2") && o[2].equals("test3") && o[3].equals("test4"));
            in.readValue(1000, v -> a.value = v, A.class);
            in.endSlice();
            // ::Test::B
            in.startSlice();
            in.readInt();
            in.endSlice();
            // ::Test::A
            in.startSlice();
            in.readInt();
            in.endSlice();
            in.endValue(false);
        }

        void check()
        {
            test(a.value.get().getMc() == 18);
        }

        private Wrapper<java.util.Optional<A>> a = new Wrapper<>();
    }

    private static class FObjectReader extends com.zeroc.Ice.ValueReader
    {
        @Override
        public void read(InputStream in)
        {
            _f = new F();
            in.startValue();
            in.startSlice();
            // Don't read af on purpose
            //in.read(1, _f.af);
            in.endSlice();
            in.startSlice();
            in.readValue(v -> _f.ae = v, A.class);
            in.endSlice();
            in.endValue(false);
        }

        F getF()
        {
            return _f;
        }

        private F _f;
    }

    private static class FactoryI implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String typeId)
        {
            if(!_enabled)
            {
                return null;
            }

            if(typeId.equals(OneOptional.ice_staticId()))
            {
                return new TestObjectReader();
            }
            else if(typeId.equals(MultiOptional.ice_staticId()))
            {
                return new TestObjectReader();
            }
            else if(typeId.equals(B.ice_staticId()))
            {
                return new BObjectReader();
            }
            else if(typeId.equals(C.ice_staticId()))
            {
                return new CObjectReader();
            }
            else if(typeId.equals("::Test::D"))
            {
                return new DObjectReader();
            }
            else if(typeId.equals("::Test::F"))
            {
                return new FObjectReader();
            }

            return null;
        }

        void setEnabled(boolean enabled)
        {
            _enabled = enabled;
        }

        private boolean _enabled;
    }
}

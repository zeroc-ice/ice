// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;
import java.io.PrintWriter;

import test.Ice.optional.Test.*;

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

    public static InitialPrx
    allTests(test.Util.Application app, boolean collocated, PrintWriter out)
    {
        Ice.Communicator communicator = app.communicator();

        FactoryI factory = new FactoryI();
        communicator.getValueFactoryManager().add(factory, "");

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
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
        mo1.setJ(MultiOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test")));
        mo1.setK(mo1);
        mo1.setBs(new byte[] { (byte)5 });
        mo1.setSs(new String[] { "test", "test2" });
        mo1.setIid(new java.util.HashMap<Integer, Integer>());
        mo1.getIid().put(4, 3);
        mo1.setSid(new java.util.HashMap<String, Integer>());
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
        mo1.setOops(new OneOptionalPrx[] { OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test")) });

        mo1.setIed(new java.util.HashMap<Integer, MyEnum>());
        mo1.getIed().put(4, MyEnum.MyEnumMember);
        mo1.setIfsd(new java.util.HashMap<Integer, FixedStruct>());
        mo1.getIfsd().put(4, fs);
        mo1.setIvsd(new java.util.HashMap<Integer, VarStruct>());
        mo1.getIvsd().put(5, vs);
        mo1.setIood(new java.util.HashMap<Integer, OneOptional>());
        mo1.getIood().put(5, new OneOptional(15));
        mo1.setIoopd(new java.util.HashMap<Integer, OneOptionalPrx>());
        mo1.getIoopd().put(5, OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test")));

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
        test(mo1.getJ().equals(MultiOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));
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
        test(mo1.getOops()[0].equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

        test(mo1.getIed().get(4) == MyEnum.MyEnumMember);
        test(mo1.getIfsd().get(4).equals(new FixedStruct(78)));
        test(mo1.getIvsd().get(5).equals(new VarStruct("hello")));
        test(mo1.getIood().get(5).getA() == 15);
        test(mo1.getIoopd().get(5).equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

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
        test(mo5.getOops()[0].equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

        test(mo5.getIed().get(4) == MyEnum.MyEnumMember);
        test(mo5.getIfsd().get(4).equals(new FixedStruct(78)));
        test(mo5.getIvsd().get(5).equals(new VarStruct("hello")));
        test(mo5.getIood().get(5).getA() == 15);
        test(mo5.getIoopd().get(5).equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

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
        test(mo9.getOops()[0].equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

        test(mo9.getIed().get(4) == MyEnum.MyEnumMember);
        test(!mo9.hasIfsd());
        test(mo9.getIvsd().get(5).equals(new VarStruct("hello")));
        test(!mo9.hasIood());
        test(mo9.getIoopd().get(5).equals(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test"))));

        test(!mo9.hasBos());

        {
            OptionalWithCustom owc1 = new OptionalWithCustom();
            java.util.ArrayList<SmallStruct> l = new java.util.ArrayList<SmallStruct>();
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
        Ice.OutputStream os = new Ice.OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(oo1);
        os.endEncapsulation();
        byte[] inEncaps = os.finished();
        Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
        test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
        Ice.InputStream in = new Ice.InputStream(communicator, outEncaps.value);
        in.startEncapsulation();
        ReadValueCallbackI cb = new ReadValueCallbackI();
        in.readValue(cb);
        in.endEncapsulation();
        test(cb.obj != null && cb.obj instanceof TestObjectReader);

        os = new Ice.OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(mo1);
        os.endEncapsulation();
        inEncaps = os.finished();
        test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
        in = new Ice.InputStream(communicator, outEncaps.value);
        in.startEncapsulation();
        in.readValue(cb);
        in.endEncapsulation();
        test(cb.obj != null && cb.obj instanceof TestObjectReader);
        factory.setEnabled(false);

        //
        // Use the 1.0 encoding with operations whose only class parameters are optional.
        //
        Ice.Optional<OneOptional> oo = new Ice.Optional<OneOptional>(new OneOptional(53));
        initial.sendOptionalClass(true, oo);
        InitialPrx initial2 = (InitialPrx)initial.ice_encodingVersion(Ice.Util.Encoding_1_0);
        initial2.sendOptionalClass(true, oo);

        initial.returnOptionalClass(true, oo);
        test(oo.isSet());
        initial2.returnOptionalClass(true, oo);
        test(!oo.isSet());

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

        os = new Ice.OutputStream(communicator);
        os.startEncapsulation();
        os.writeOptional(1, Ice.OptionalFormat.F4);
        os.writeInt(15);
        os.writeOptional(1, Ice.OptionalFormat.VSize);
        os.writeString("test");
        os.endEncapsulation();
        inEncaps = os.finished();
        test(initial.ice_invoke("opVoid", Ice.OperationMode.Normal, inEncaps, outEncaps));

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

        mc.setIfsd(new java.util.HashMap<Integer, FixedStruct>());
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
        os = new Ice.OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(mc);
        os.endEncapsulation();
        inEncaps = os.finished();
        outEncaps = new Ice.ByteSeqHolder();
        test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
        in = new Ice.InputStream(communicator, outEncaps.value);
        in.startEncapsulation();
        in.readValue(cb);
        in.endEncapsulation();
        test(cb.obj != null && cb.obj instanceof TestObjectReader);
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
            os = new Ice.OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(b);
            os.endEncapsulation();
            inEncaps = os.finished();
            outEncaps = new Ice.ByteSeqHolder();
            test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
            in = new Ice.InputStream(communicator, outEncaps.value);
            in.startEncapsulation();
            in.readValue(cb);
            in.endEncapsulation();
            test(cb.obj != null);
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
            os = new Ice.OutputStream(communicator);
            os.startEncapsulation();
            os.writeValue(f);
            os.endEncapsulation();
            inEncaps = os.finished();
            in = new Ice.InputStream(communicator, inEncaps);
            in.startEncapsulation();
            final FHolder fholder = new FHolder();
            in.readValue(new Ice.ReadValueCallback()
                {
                    @Override
                    public void valueReady(Ice.Object obj)
                    {
                        fholder.value = ((FObjectReader)obj).getF();
                    }
                });
            in.endEncapsulation();
            factory.setEnabled(false);
            rf = fholder.value;
            test(rf.ae != null && !rf.hasAf());
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
                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(c);
                os.endEncapsulation();
                inEncaps = os.finished();
                factory.setEnabled(true);
                test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.readValue(cb);
                in.endEncapsulation();
                test(cb.obj instanceof CObjectReader);
                factory.setEnabled(false);

                factory.setEnabled(true);
                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                Ice.Object d = new DObjectWriter();
                os.writeValue(d);
                os.endEncapsulation();
                inEncaps = os.finished();
                test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, outEncaps));
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.readValue(cb);
                in.endEncapsulation();
                test(cb.obj != null && cb.obj instanceof DObjectReader);
                ((DObjectReader)cb.obj).check();
                factory.setEnabled(false);
            }
            out.println("ok");

            out.print("testing optionals with unknown classes...");
            out.flush();
            {
                A a = new A();

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeValue(a);
                os.writeOptional(1, Ice.OptionalFormat.Class);
                os.writeValue(new DObjectWriter());
                os.endEncapsulation();
                inEncaps = os.finished();
                test(initial.ice_invoke("opClassAndUnknownOptional", Ice.OperationMode.Normal, inEncaps, outEncaps));

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
            out.println("ok");
        }

        out.print("testing optional parameters... ");
        out.flush();
        final boolean reqParams = initial.supportsRequiredParams();

        {

            Ice.ByteOptional p1 = new Ice.ByteOptional();
            Ice.ByteOptional p3 = new Ice.ByteOptional();
            Ice.ByteOptional p2 = initial.opByte(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set((byte)56);
            p2 = initial.opByte(p1, p3);
            test(p2.get() == (byte)56 && p3.get() == (byte)56);
            Ice.AsyncResult r = initial.begin_opByte(p1);
            p2 = initial.end_opByte(p3, r);
            test(p2.get() == (byte)56 && p3.get() == (byte)56);
            p2 = initial.opByte(new Ice.ByteOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opByteReq(p1.get(), p3);
                test(p2.get() == (byte)56 && p3.get() == (byte)56);
                r = initial.begin_opByteReq(p1.get());
                p2 = initial.end_opByteReq(p3, r);
                test(p2.get() == (byte)56 && p3.get() == (byte)56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F1);
                os.writeByte(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opByteReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F1));
                test(in.readByte() == (byte)56);
                test(in.readOptional(3, Ice.OptionalFormat.F1));
                test(in.readByte() == (byte)56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.BooleanOptional p1 = new Ice.BooleanOptional();
            Ice.BooleanOptional p3 = new Ice.BooleanOptional();
            Ice.BooleanOptional p2 = initial.opBool(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(true);
            p2 = initial.opBool(p1, p3);
            test(p2.get() == true && p3.get() == true);
            Ice.AsyncResult r = initial.begin_opBool(p1);
            p2 = initial.end_opBool(p3, r);
            test(p2.get() == true && p3.get() == true);
            p2 = initial.opBool(new Ice.BooleanOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opBoolReq(true, p3);
                test(p2.get() == true && p3.get() == true);
                r = initial.begin_opBoolReq(true);
                p2 = initial.end_opBoolReq(p3, r);
                test(p2.get() == true && p3.get() == true);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F1);
                os.writeBool(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opBoolReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F1));
                test(in.readBool() == true);
                test(in.readOptional(3, Ice.OptionalFormat.F1));
                test(in.readBool() == true);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.ShortOptional p1 = new Ice.ShortOptional();
            Ice.ShortOptional p3 = new Ice.ShortOptional();
            Ice.ShortOptional p2 = initial.opShort(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set((short)56);
            p2 = initial.opShort(p1, p3);
            test(p2.get() == 56 && p3.get() == 56);
            Ice.AsyncResult r = initial.begin_opShort(p1);
            p2 = initial.end_opShort(p3, r);
            test(p2.get() == 56 && p3.get() == 56);
            p2 = initial.opShort(new Ice.ShortOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opShortReq(p1.get(), p3);
                test(p2.get() == 56 && p3.get() == 56);
                r = initial.begin_opShortReq(p1.get());
                p2 = initial.end_opShortReq(p3, r);
                test(p2.get() == 56 && p3.get() == 56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F2);
                os.writeShort(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opShortReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F2));
                test(in.readShort() == 56);
                test(in.readOptional(3, Ice.OptionalFormat.F2));
                test(in.readShort() == 56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.IntOptional p1 = new Ice.IntOptional();
            Ice.IntOptional p3 = new Ice.IntOptional();
            Ice.IntOptional p2 = initial.opInt(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(56);
            p2 = initial.opInt(p1, p3);
            test(p2.get() == 56 && p3.get() == 56);
            Ice.AsyncResult r = initial.begin_opInt(p1);
            p2 = initial.end_opInt(p3, r);
            test(p2.get() == 56 && p3.get() == 56);
            p2 = initial.opInt(new Ice.IntOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opIntReq(p1.get(), p3);
                test(p2.get() == 56 && p3.get() == 56);
                r = initial.begin_opIntReq(p1.get());
                p2 = initial.end_opIntReq(p3, r);
                test(p2.get() == 56 && p3.get() == 56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F4);
                os.writeInt(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F4));
                test(in.readInt() == 56);
                test(in.readOptional(3, Ice.OptionalFormat.F4));
                test(in.readInt() == 56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.LongOptional p1 = new Ice.LongOptional();
            Ice.LongOptional p3 = new Ice.LongOptional();
            Ice.LongOptional p2 = initial.opLong(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(56);
            p2 = initial.opLong(p1, p3);
            test(p2.get() == 56 && p3.get() == 56);
            Ice.AsyncResult r = initial.begin_opLong(p1);
            p2 = initial.end_opLong(p3, r);
            test(p2.get() == 56 && p3.get() == 56);
            p2 = initial.opLong(new Ice.LongOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opLongReq(p1.get(), p3);
                test(p2.get() == 56 && p3.get() == 56);
                r = initial.begin_opLongReq(p1.get());
                p2 = initial.end_opLongReq(p3, r);
                test(p2.get() == 56 && p3.get() == 56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(1, Ice.OptionalFormat.F8);
                os.writeLong(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opLongReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(2, Ice.OptionalFormat.F8));
                test(in.readLong() == 56);
                test(in.readOptional(3, Ice.OptionalFormat.F8));
                test(in.readLong() == 56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.FloatOptional p1 = new Ice.FloatOptional();
            Ice.FloatOptional p3 = new Ice.FloatOptional();
            Ice.FloatOptional p2 = initial.opFloat(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set((float)1.0);
            p2 = initial.opFloat(p1, p3);
            test(p2.get() == 1.0 && p3.get() == 1.0);
            Ice.AsyncResult r = initial.begin_opFloat(p1);
            p2 = initial.end_opFloat(p3, r);
            test(p2.get() == 1.0 && p3.get() == 1.0);
            p2 = initial.opFloat(new Ice.FloatOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opFloatReq(p1.get(), p3);
                test(p2.get() == 1.0 && p3.get() == 1.0);
                r = initial.begin_opFloatReq(p1.get());
                p2 = initial.end_opFloatReq(p3, r);
                test(p2.get() == 1.0 && p3.get() == 1.0);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F4);
                os.writeFloat(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFloatReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F4));
                test(in.readFloat() == 1.0);
                test(in.readOptional(3, Ice.OptionalFormat.F4));
                test(in.readFloat() == 1.0);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.DoubleOptional p1 = new Ice.DoubleOptional();
            Ice.DoubleOptional p3 = new Ice.DoubleOptional();
            Ice.DoubleOptional p2 = initial.opDouble(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(1.0);
            p2 = initial.opDouble(p1, p3);
            test(p2.get() == 1.0 && p3.get() == 1.0);
            Ice.AsyncResult r = initial.begin_opDouble(p1);
            p2 = initial.end_opDouble(p3, r);
            test(p2.get() == 1.0 && p3.get() == 1.0);
            p2 = initial.opDouble(new Ice.DoubleOptional(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opDoubleReq(p1.get(), p3);
                test(p2.get() == 1.0 && p3.get() == 1.0);
                r = initial.begin_opDoubleReq(p1.get());
                p2 = initial.end_opDoubleReq(p3, r);
                test(p2.get() == 1.0 && p3.get() == 1.0);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.F8);
                os.writeDouble(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opDoubleReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.F8));
                test(in.readDouble() == 1.0);
                test(in.readOptional(3, Ice.OptionalFormat.F8));
                test(in.readDouble() == 1.0);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<String> p1 = new Ice.Optional<String>();
            Ice.Optional<String> p3 = new Ice.Optional<String>();
            Ice.Optional<String> p2 = initial.opString(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set("test");
            p2 = initial.opString(p1, p3);
            test(p2.get().equals("test") && p3.get().equals("test"));
            Ice.AsyncResult r = initial.begin_opString(p1);
            p2 = initial.end_opString(p3, r);
            test(p2.get().equals("test") && p3.get().equals("test"));
            p2 = initial.opString(new Ice.Optional<String>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opStringReq(p1.get(), p3);
                test(p2.get().equals("test") && p3.get().equals("test"));
                r = initial.begin_opStringReq(p1.get());
                p2 = initial.end_opStringReq(p3, r);
                test(p2.get().equals("test") && p3.get().equals("test"));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeString(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opStringReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                test(in.readString().equals("test"));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                test(in.readString().equals("test"));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<MyEnum> p1 = new Ice.Optional<MyEnum>();
            Ice.Optional<MyEnum> p3 = new Ice.Optional<MyEnum>();
            Ice.Optional<MyEnum> p2 = initial.opMyEnum(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(MyEnum.MyEnumMember);
            p2 = initial.opMyEnum(p1, p3);
            test(p2.get() == MyEnum.MyEnumMember && p3.get() == MyEnum.MyEnumMember);
            p1.set(MyEnum.MyEnumMember);
            p2 = initial.opMyEnum(new Ice.Optional<MyEnum>((MyEnum)null), p3); // Test null enum
            test(p2.get() == MyEnum.MyEnumMember && p3.get() == MyEnum.MyEnumMember);
            Ice.AsyncResult r = initial.begin_opMyEnum(p1);
            p2 = initial.end_opMyEnum(p3, r);
            test(p2.get() == MyEnum.MyEnumMember && p3.get() == MyEnum.MyEnumMember);
            p2 = initial.opMyEnum(new Ice.Optional<MyEnum>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opMyEnumReq(p1.get(), p3);
                test(p2.get() == MyEnum.MyEnumMember && p3.get() == MyEnum.MyEnumMember);
                r = initial.begin_opMyEnumReq(p1.get());
                p2 = initial.end_opMyEnumReq(p3, r);
                test(p2.get() == MyEnum.MyEnumMember && p3.get() == MyEnum.MyEnumMember);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.Size);
                MyEnum.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opMyEnumReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.Size));
                test(MyEnum.read(in) == MyEnum.MyEnumMember);
                test(in.readOptional(3, Ice.OptionalFormat.Size));
                test(MyEnum.read(in) == MyEnum.MyEnumMember);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<SmallStruct> p1 = new Ice.Optional<SmallStruct>();
            Ice.Optional<SmallStruct> p3 = new Ice.Optional<SmallStruct>();
            Ice.Optional<SmallStruct> p2 = initial.opSmallStruct(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new SmallStruct((byte)56));
            p2 = initial.opSmallStruct(p1, p3);
            test(p2.get().m == (byte)56 && p3.get().m == (byte)56);
            p2 = initial.opSmallStruct(new Ice.Optional<SmallStruct>((SmallStruct)null), p3); // Test null struct
            test(p2.get().m == (byte)0 && p3.get().m == (byte)0);
            Ice.AsyncResult r = initial.begin_opSmallStruct(p1);
            p2 = initial.end_opSmallStruct(p3, r);
            test(p2.get().m == (byte)56 && p3.get().m == (byte)56);
            p2 = initial.opSmallStruct(new Ice.Optional<SmallStruct>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opSmallStructReq(p1.get(), p3);
                test(p2.get().m == (byte)56 && p3.get().m == (byte)56);
                r = initial.begin_opSmallStructReq(p1.get());
                p2 = initial.end_opSmallStructReq(p3, r);
                test(p2.get().m == (byte)56 && p3.get().m == (byte)56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(1);
                SmallStruct.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStructReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                SmallStruct f = SmallStruct.read(in, null);
                test(f.m == (byte)56);
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                SmallStruct.read(in, f);
                test(f.m == (byte)56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<FixedStruct> p1 = new Ice.Optional<FixedStruct>();
            Ice.Optional<FixedStruct> p3 = new Ice.Optional<FixedStruct>();
            Ice.Optional<FixedStruct> p2 = initial.opFixedStruct(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new FixedStruct(56));
            p2 = initial.opFixedStruct(p1, p3);
            test(p2.get().m == 56 && p3.get().m == 56);
            Ice.AsyncResult r = initial.begin_opFixedStruct(p1);
            p2 = initial.end_opFixedStruct(p3, r);
            test(p2.get().m == 56 && p3.get().m == 56);
            p2 = initial.opFixedStruct(new Ice.Optional<FixedStruct>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opFixedStructReq(p1.get(), p3);
                test(p2.get().m == 56 && p3.get().m == 56);
                r = initial.begin_opFixedStructReq(p1.get());
                p2 = initial.end_opFixedStructReq(p3, r);
                test(p2.get().m == 56 && p3.get().m == 56);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(4);
                FixedStruct.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStructReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                FixedStruct f = FixedStruct.read(in, null);
                test(f.m == 56);
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                FixedStruct.read(in, f);
                test(f.m == 56);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<VarStruct> p1 = new Ice.Optional<VarStruct>();
            Ice.Optional<VarStruct> p3 = new Ice.Optional<VarStruct>();
            Ice.Optional<VarStruct> p2 = initial.opVarStruct(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new VarStruct("test"));
            p2 = initial.opVarStruct(p1, p3);
            test(p2.get().m.equals("test") && p3.get().m.equals("test"));
            Ice.AsyncResult r = initial.begin_opVarStruct(p1);
            p2 = initial.end_opVarStruct(p3, r);
            test(p2.get().m.equals("test") && p3.get().m.equals("test"));
            p2 = initial.opVarStruct(new Ice.Optional<VarStruct>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opVarStructReq(p1.get(), p3);
                test(p2.get().m.equals("test") && p3.get().m.equals("test"));
                r = initial.begin_opVarStructReq(p1.get());
                p2 = initial.end_opVarStructReq(p3, r);
                test(p2.get().m.equals("test") && p3.get().m.equals("test"));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                VarStruct.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opVarStructReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                VarStruct v = VarStruct.read(in, null);
                test(v.m.equals("test"));
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                VarStruct.read(in, v);
                test(v.m.equals("test"));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<OneOptional> p1 = new Ice.Optional<OneOptional>();
            Ice.Optional<OneOptional> p3 = new Ice.Optional<OneOptional>();
            Ice.Optional<OneOptional> p2 = initial.opOneOptional(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new OneOptional(58));
            p2 = initial.opOneOptional(p1, p3);
            test(p2.get().getA() == 58 && p3.get().getA() == 58);
            Ice.AsyncResult r = initial.begin_opOneOptional(p1);
            p2 = initial.end_opOneOptional(p3, r);
            test(p2.get().getA() == 58 && p3.get().getA() == 58);
            p2 = initial.opOneOptional(new Ice.Optional<OneOptional>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opOneOptionalReq(p1.get(), p3);
                test(p2.get().getA() == 58 && p3.get().getA() == 58);
                r = initial.begin_opOneOptionalReq(p1.get());
                p2 = initial.end_opOneOptionalReq(p3, r);
                test(p2.get().getA() == 58 && p3.get().getA() == 58);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.Class);
                os.writeValue(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opOneOptionalReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.Class));
                ReadValueCallbackI p2cb = new ReadValueCallbackI();
                in.readValue(p2cb);
                test(in.readOptional(3, Ice.OptionalFormat.Class));
                ReadValueCallbackI p3cb = new ReadValueCallbackI();
                in.readValue(p3cb);
                in.endEncapsulation();
                test(((OneOptional)p2cb.obj).getA() == 58 && ((OneOptional)p3cb.obj).getA() == 58);

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<OneOptionalPrx> p1 = new Ice.Optional<OneOptionalPrx>();
            Ice.Optional<OneOptionalPrx> p3 = new Ice.Optional<OneOptionalPrx>();
            Ice.Optional<OneOptionalPrx> p2 = initial.opOneOptionalProxy(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test")));
            p2 = initial.opOneOptionalProxy(p1, p3);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opOneOptionalProxy(p1);
            p2 = initial.end_opOneOptionalProxy(p3, r);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            p2 = initial.opOneOptionalProxy(new Ice.Optional<OneOptionalPrx>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opOneOptionalProxyReq(p1.get(), p3);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
                r = initial.begin_opOneOptionalProxyReq(p1.get());
                p2 = initial.end_opOneOptionalProxyReq(p3, r);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeProxy(p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opOneOptionalProxyReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                test(in.readProxy().equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                test(in.readProxy().equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }
        out.println("ok");

        out.print("testing optional parameters and sequences... ");
        out.flush();
        {
            Ice.Optional<byte[]> p1 = new Ice.Optional<byte[]>();
            Ice.Optional<byte[]> p3 = new Ice.Optional<byte[]>();
            Ice.Optional<byte[]> p2 = initial.opByteSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new byte[100]);
            java.util.Arrays.fill(p1.get(), (byte)56);
            p2 = initial.opByteSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opByteSeq(p1);
            p2 = initial.end_opByteSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opByteSeq(new Ice.Optional<byte[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opByteSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opByteSeqReq(p1.get());
                p2 = initial.end_opByteSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeByteSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opByteSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                test(java.util.Arrays.equals(in.readByteSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                test(java.util.Arrays.equals(in.readByteSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<boolean[]> p1 = new Ice.Optional<boolean[]>();
            Ice.Optional<boolean[]> p3 = new Ice.Optional<boolean[]>();
            Ice.Optional<boolean[]> p2 = initial.opBoolSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new boolean[100]);
            p2 = initial.opBoolSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opBoolSeq(p1);
            p2 = initial.end_opBoolSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opBoolSeq(new Ice.Optional<boolean[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opBoolSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opBoolSeqReq(p1.get());
                p2 = initial.end_opBoolSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeBoolSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opBoolSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                test(java.util.Arrays.equals(in.readBoolSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                test(java.util.Arrays.equals(in.readBoolSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<short[]> p1 = new Ice.Optional<short[]>();
            Ice.Optional<short[]> p3 = new Ice.Optional<short[]>();
            Ice.Optional<short[]> p2 = initial.opShortSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new short[100]);
            java.util.Arrays.fill(p1.get(), (short)56);
            p2 = initial.opShortSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opShortSeq(p1);
            p2 = initial.end_opShortSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opShortSeq(new Ice.Optional<short[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opShortSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opShortSeqReq(p1.get());
                p2 = initial.end_opShortSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 2 + (p1.get().length > 254 ? 5 : 1));
                os.writeShortSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opShortSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readShortSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readShortSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<int[]> p1 = new Ice.Optional<int[]>();
            Ice.Optional<int[]> p3 = new Ice.Optional<int[]>();
            Ice.Optional<int[]> p2 = initial.opIntSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new int[100]);
            java.util.Arrays.fill(p1.get(), 56);
            p2 = initial.opIntSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opIntSeq(p1);
            p2 = initial.end_opIntSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opIntSeq(new Ice.Optional<int[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opIntSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opIntSeqReq(p1.get());
                p2 = initial.end_opIntSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                os.writeIntSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readIntSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readIntSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<long[]> p1 = new Ice.Optional<long[]>();
            Ice.Optional<long[]> p3 = new Ice.Optional<long[]>();
            Ice.Optional<long[]> p2 = initial.opLongSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new long[100]);
            java.util.Arrays.fill(p1.get(), 56);
            p2 = initial.opLongSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opLongSeq(p1);
            p2 = initial.end_opLongSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opLongSeq(new Ice.Optional<long[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opLongSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opLongSeqReq(p1.get());
                p2 = initial.end_opLongSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 8 + (p1.get().length > 254 ? 5 : 1));
                os.writeLongSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opLongSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readLongSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readLongSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<float[]> p1 = new Ice.Optional<float[]>();
            Ice.Optional<float[]> p3 = new Ice.Optional<float[]>();
            Ice.Optional<float[]> p2 = initial.opFloatSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new float[100]);
            java.util.Arrays.fill(p1.get(), (float)1.0);
            p2 = initial.opFloatSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opFloatSeq(p1);
            p2 = initial.end_opFloatSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opFloatSeq(new Ice.Optional<float[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opFloatSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opFloatSeqReq(p1.get());
                p2 = initial.end_opFloatSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                os.writeFloatSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFloatSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readFloatSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readFloatSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<double[]> p1 = new Ice.Optional<double[]>();
            Ice.Optional<double[]> p3 = new Ice.Optional<double[]>();
            Ice.Optional<double[]> p2 = initial.opDoubleSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new double[100]);
            java.util.Arrays.fill(p1.get(), 1.0);
            p2 = initial.opDoubleSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opDoubleSeq(p1);
            p2 = initial.end_opDoubleSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opDoubleSeq(new Ice.Optional<double[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opDoubleSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opDoubleSeqReq(p1.get());
                p2 = initial.end_opDoubleSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 8 + (p1.get().length > 254 ? 5 : 1));
                os.writeDoubleSeq(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opDoubleSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readDoubleSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                test(java.util.Arrays.equals(in.readDoubleSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<String[]> p1 = new Ice.Optional<String[]>();
            Ice.Optional<String[]> p3 = new Ice.Optional<String[]>();
            Ice.Optional<String[]> p2 = initial.opStringSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new String[10]);
            java.util.Arrays.fill(p1.get(), "test1");
            p2 = initial.opStringSeq(p1, p3);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            Ice.AsyncResult r = initial.begin_opStringSeq(p1);
            p2 = initial.end_opStringSeq(p3, r);
            test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
            p2 = initial.opStringSeq(new Ice.Optional<String[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opStringSeqReq(p1.get(), p3);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));
                r = initial.begin_opStringSeqReq(p1.get());
                p2 = initial.end_opStringSeqReq(p3, r);
                test(java.util.Arrays.equals(p2.get(), p1.get()) && java.util.Arrays.equals(p3.get(), p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                os.writeStringSeq(p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opStringSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                test(java.util.Arrays.equals(in.readStringSeq(), p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                test(java.util.Arrays.equals(in.readStringSeq(), p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<SmallStruct[]> p1 = new Ice.Optional<SmallStruct[]>();
            Ice.Optional<SmallStruct[]> p3 = new Ice.Optional<SmallStruct[]>();
            Ice.Optional<SmallStruct[]> p2 = initial.opSmallStructSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new SmallStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new SmallStruct();
            }
            p2 = initial.opSmallStructSeq(p1, p3);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            Ice.AsyncResult r = initial.begin_opSmallStructSeq(p1);
            p2 = initial.end_opSmallStructSeq(p3, r);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            p2 = initial.opSmallStructSeq(new Ice.Optional<SmallStruct[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opSmallStructSeqReq(p1.get(), p3);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }
                r = initial.begin_opSmallStructSeqReq(p1.get());
                p2 = initial.end_opSmallStructSeqReq(p3, r);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length + (p1.get().length > 254 ? 5 : 1));
                SmallStructSeqHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStructSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                SmallStruct[] arr = SmallStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                arr = SmallStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<java.util.List<SmallStruct>> p1 = new Ice.Optional<java.util.List<SmallStruct>>();
            Ice.Optional<java.util.List<SmallStruct>> p3 = new Ice.Optional<java.util.List<SmallStruct>>();
            Ice.Optional<java.util.List<SmallStruct>> p2 = initial.opSmallStructList(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new java.util.ArrayList<SmallStruct>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new SmallStruct());
            }
            p2 = initial.opSmallStructList(p1, p3);
            test(p2.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opSmallStructList(p1);
            p2 = initial.end_opSmallStructList(p3, r);
            test(p2.get().equals(p1.get()));
            p2 = initial.opSmallStructList(new Ice.Optional<java.util.List<SmallStruct>>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opSmallStructListReq(p1.get(), p3);
                test(p2.get().equals(p1.get()));
                r = initial.begin_opSmallStructListReq(p1.get());
                p2 = initial.end_opSmallStructListReq(p3, r);
                test(p2.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().size() + (p1.get().size() > 254 ? 5 : 1));
                SmallStructListHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSmallStructListReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                java.util.List<SmallStruct> arr = SmallStructListHelper.read(in);
                test(arr.equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                arr = SmallStructListHelper.read(in);
                test(arr.equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<FixedStruct[]> p1 = new Ice.Optional<FixedStruct[]>();
            Ice.Optional<FixedStruct[]> p3 = new Ice.Optional<FixedStruct[]>();
            Ice.Optional<FixedStruct[]> p2 = initial.opFixedStructSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new FixedStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new FixedStruct();
            }
            p2 = initial.opFixedStructSeq(p1, p3);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            Ice.AsyncResult r = initial.begin_opFixedStructSeq(p1);
            p2 = initial.end_opFixedStructSeq(p3, r);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            p2 = initial.opFixedStructSeq(new Ice.Optional<FixedStruct[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opFixedStructSeqReq(p1.get(), p3);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }
                r = initial.begin_opFixedStructSeqReq(p1.get());
                p2 = initial.end_opFixedStructSeqReq(p3, r);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().length * 4 + (p1.get().length > 254 ? 5 : 1));
                FixedStructSeqHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStructSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                FixedStruct[] arr = FixedStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                arr = FixedStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<java.util.List<FixedStruct>> p1 = new Ice.Optional<java.util.List<FixedStruct>>();
            Ice.Optional<java.util.List<FixedStruct>> p3 = new Ice.Optional<java.util.List<FixedStruct>>();
            Ice.Optional<java.util.List<FixedStruct>> p2 = initial.opFixedStructList(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new java.util.ArrayList<FixedStruct>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new FixedStruct());
            }
            p2 = initial.opFixedStructList(p1, p3);
            test(p2.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opFixedStructList(p1);
            p2 = initial.end_opFixedStructList(p3, r);
            test(p2.get().equals(p1.get()));
            p2 = initial.opFixedStructList(new Ice.Optional<java.util.List<FixedStruct>>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opFixedStructListReq(p1.get(), p3);
                test(p2.get().equals(p1.get()));
                r = initial.begin_opFixedStructListReq(p1.get());
                p2 = initial.end_opFixedStructListReq(p3, r);
                test(p2.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().size() * 4 + (p1.get().size() > 254 ? 5 : 1));
                FixedStructListHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opFixedStructListReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                java.util.List<FixedStruct> arr = FixedStructListHelper.read(in);
                test(arr.equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                arr = FixedStructListHelper.read(in);
                test(arr.equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<VarStruct[]> p1 = new Ice.Optional<VarStruct[]>();
            Ice.Optional<VarStruct[]> p3 = new Ice.Optional<VarStruct[]>();
            Ice.Optional<VarStruct[]> p2 = initial.opVarStructSeq(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new VarStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new VarStruct("");
            }
            p2 = initial.opVarStructSeq(p1, p3);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            Ice.AsyncResult r = initial.begin_opVarStructSeq(p1);
            p2 = initial.end_opVarStructSeq(p3, r);
            for(int i = 0; i < p1.get().length; ++i)
            {
                test(p2.get()[i].equals(p1.get()[i]));
            }
            p2 = initial.opVarStructSeq(new Ice.Optional<VarStruct[]>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opVarStructSeqReq(p1.get(), p3);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }
                r = initial.begin_opVarStructSeqReq(p1.get());
                p2 = initial.end_opVarStructSeqReq(p3, r);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(p2.get()[i].equals(p1.get()[i]));
                }

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                VarStructSeqHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opVarStructSeqReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                VarStruct[] arr = VarStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                arr = VarStructSeqHelper.read(in);
                for(int i = 0; i < p1.get().length; ++i)
                {
                    test(arr[i].equals(p1.get()[i]));
                }
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        if(supportsJavaSerializable)
        {
            Ice.Optional<SerializableClass> p1 = new Ice.Optional<SerializableClass>();
            Ice.Optional<SerializableClass> p3 = new Ice.Optional<SerializableClass>();
            Ice.Optional<SerializableClass> p2 = initial.opSerializable(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new SerializableClass(58));
            p2 = initial.opSerializable(p1, p3);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opSerializable(p1);
            p2 = initial.end_opSerializable(p3, r);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            p2 = initial.opSerializable(new Ice.Optional<SerializableClass>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opSerializableReq(p1.get(), p3);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
                r = initial.begin_opSerializableReq(p1.get());
                p2 = initial.end_opSerializableReq(p3, r);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSerializable(p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opSerializableReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                SerializableClass sc = SerializableHelper.read(in);
                test(sc.equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                sc = SerializableHelper.read(in);
                test(sc.equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }
        out.println("ok");

        out.print("testing optional parameters and dictionaries... ");
        out.flush();
        {
            Ice.Optional<java.util.Map<Integer, Integer>> p1 = new Ice.Optional<java.util.Map<Integer, Integer>>();
            Ice.Optional<java.util.Map<Integer, Integer>> p3 = new Ice.Optional<java.util.Map<Integer, Integer>>();
            Ice.Optional<java.util.Map<Integer, Integer>> p2 = initial.opIntIntDict(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new java.util.HashMap<Integer, Integer>());
            p1.get().put(1, 2);
            p1.get().put(2, 3);
            p2 = initial.opIntIntDict(p1, p3);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opIntIntDict(p1);
            p2 = initial.end_opIntIntDict(p3, r);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            p2 = initial.opIntIntDict(new Ice.Optional<java.util.Map<Integer, Integer>>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opIntIntDictReq(p1.get(), p3);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
                r = initial.begin_opIntIntDictReq(p1.get());
                p2 = initial.end_opIntIntDictReq(p3, r);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.VSize);
                os.writeSize(p1.get().size() * 8 + (p1.get().size() > 254 ? 5 : 1));
                IntIntDictHelper.write(os, p1.get());
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntIntDictReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.VSize));
                in.skipSize();
                java.util.Map<Integer, Integer> m = IntIntDictHelper.read(in);
                test(m.equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.VSize));
                in.skipSize();
                m = IntIntDictHelper.read(in);
                test(m.equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<java.util.Map<String, Integer>> p1 = new Ice.Optional<java.util.Map<String, Integer>>();
            Ice.Optional<java.util.Map<String, Integer>> p3 = new Ice.Optional<java.util.Map<String, Integer>>();
            Ice.Optional<java.util.Map<String, Integer>> p2 = initial.opStringIntDict(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new java.util.HashMap<String, Integer>());
            p1.get().put("1", 1);
            p1.get().put("2", 2);
            p2 = initial.opStringIntDict(p1, p3);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            Ice.AsyncResult r = initial.begin_opStringIntDict(p1);
            p2 = initial.end_opStringIntDict(p3, r);
            test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
            p2 = initial.opStringIntDict(new Ice.Optional<java.util.Map<String, Integer>>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opStringIntDictReq(p1.get(), p3);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));
                r = initial.begin_opStringIntDictReq(p1.get());
                p2 = initial.end_opStringIntDictReq(p3, r);
                test(p2.get().equals(p1.get()) && p3.get().equals(p1.get()));

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                StringIntDictHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opStringIntDictReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                java.util.Map<String, Integer> m = StringIntDictHelper.read(in);
                test(m.equals(p1.get()));
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                m = StringIntDictHelper.read(in);
                test(m.equals(p1.get()));
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            Ice.Optional<java.util.Map<Integer, OneOptional>> p1 = new Ice.Optional<java.util.Map<Integer, OneOptional>>();
            Ice.Optional<java.util.Map<Integer, OneOptional>> p3 = new Ice.Optional<java.util.Map<Integer, OneOptional>>();
            Ice.Optional<java.util.Map<Integer, OneOptional>> p2 = initial.opIntOneOptionalDict(p1, p3);
            test(!p2.isSet() && !p3.isSet());

            p1.set(new java.util.HashMap<Integer, OneOptional>());
            p1.get().put(1, new OneOptional(15));
            p1.get().put(2, new OneOptional(12));
            p2 = initial.opIntOneOptionalDict(p1, p3);
            test(p2.get().get(1).getA() == 15 && p3.get().get(1).getA() == 15);
            test(p2.get().get(2).getA() == 12 && p3.get().get(2).getA() == 12);
            Ice.AsyncResult r = initial.begin_opIntOneOptionalDict(p1);
            p2 = initial.end_opIntOneOptionalDict(p3, r);
            test(p2.get().get(1).getA() == 15 && p3.get().get(1).getA() == 15);
            test(p2.get().get(2).getA() == 12 && p3.get().get(2).getA() == 12);
            p2 = initial.opIntOneOptionalDict(new Ice.Optional<java.util.Map<Integer, OneOptional>>(), p3);
            test(!p2.isSet() && !p3.isSet()); // Ensure out parameter is cleared.

            if(reqParams)
            {
                p2 = initial.opIntOneOptionalDictReq(p1.get(), p3);
                test(p2.get().get(1).getA() == 15 && p3.get().get(1).getA() == 15);
                test(p2.get().get(2).getA() == 12 && p3.get().get(2).getA() == 12);
                r = initial.begin_opIntOneOptionalDictReq(p1.get());
                p2 = initial.end_opIntOneOptionalDictReq(p3, r);
                test(p2.get().get(1).getA() == 15 && p3.get().get(1).getA() == 15);
                test(p2.get().get(2).getA() == 12 && p3.get().get(2).getA() == 12);

                os = new Ice.OutputStream(communicator);
                os.startEncapsulation();
                os.writeOptional(2, Ice.OptionalFormat.FSize);
                int pos = os.startSize();
                IntOneOptionalDictHelper.write(os, p1.get());
                os.endSize(pos);
                os.endEncapsulation();
                inEncaps = os.finished();
                initial.ice_invoke("opIntOneOptionalDictReq", Ice.OperationMode.Normal, inEncaps, outEncaps);
                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                test(in.readOptional(1, Ice.OptionalFormat.FSize));
                in.skip(4);
                java.util.Map<Integer, OneOptional> m = IntOneOptionalDictHelper.read(in);
                test(m.get(1).getA() == 15 && m.get(2).getA() == 12);
                test(in.readOptional(3, Ice.OptionalFormat.FSize));
                in.skip(4);
                m = IntOneOptionalDictHelper.read(in);
                test(m.get(1).getA() == 15 && m.get(2).getA() == 12);
                in.endEncapsulation();

                in = new Ice.InputStream(communicator, outEncaps.value);
                in.startEncapsulation();
                in.endEncapsulation();
            }
        }

        {
            F f = new F();
            f.setAf(new A());
            f.getAf().requiredA = 56;
            f.ae = f.getAf();

            os = new Ice.OutputStream(communicator);
            os.startEncapsulation();
            os.writeOptional(1, Ice.OptionalFormat.Class);
            os.writeValue(f);
            os.writeOptional(2, Ice.OptionalFormat.Class);
            os.writeValue(f.ae);
            os.endEncapsulation();
            inEncaps = os.finished();

            in = new Ice.InputStream(communicator, inEncaps);
            in.startEncapsulation();
            test(in.readOptional(2, Ice.OptionalFormat.Class));
            final AHolder a = new AHolder();
            in.readValue(new Ice.ReadValueCallback()
                {
                    @Override
                    public void valueReady(Ice.Object obj)
                    {
                        a.value = (A)obj;
                    }
                });
            in.endEncapsulation();
            test(a.value != null && a.value.requiredA == 56);
        }
        out.println("ok");

        out.print("testing exception optionals... ");
        out.flush();
        {
            try
            {
                Ice.IntOptional a = new Ice.IntOptional();
                Ice.Optional<String> b = new Ice.Optional<String>();
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>();
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
                Ice.IntOptional a = new Ice.IntOptional(30);
                Ice.Optional<String> b = new Ice.Optional<String>("test");
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>(new OneOptional(53));
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
                Ice.IntOptional a = new Ice.IntOptional(30);
                Ice.Optional<String> b = new Ice.Optional<String>("test");
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>(new OneOptional(53));
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
                Ice.IntOptional a = new Ice.IntOptional();
                Ice.Optional<String> b = new Ice.Optional<String>();
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>();
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
                Ice.IntOptional a = new Ice.IntOptional(30);
                Ice.Optional<String> b = new Ice.Optional<String>("test2");
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>(new OneOptional(53));
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
                Ice.IntOptional a = new Ice.IntOptional();
                Ice.Optional<String> b = new Ice.Optional<String>();
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>();
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
                Ice.IntOptional a = new Ice.IntOptional(30);
                Ice.Optional<String> b = new Ice.Optional<String>("test2");
                Ice.Optional<OneOptional> o = new Ice.Optional<OneOptional>(new OneOptional(53));
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

        //
        // Use reflection to load TwowaysLambdaAMI as that is only supported with Java >= 1.8
        //
        try
        {
            Class<?> cls = IceInternal.Util.findClass("test.Ice.optional.lambda.AllTests", null);
            if(cls != null)
            {
                java.lang.reflect.Method allTests = cls.getDeclaredMethod("allTests",
                    new Class<?>[]{test.Util.Application.class, java.io.PrintWriter.class});
                allTests.invoke(null, app, out);
            }
        }
        catch(java.lang.NoSuchMethodException ex)
        {
            throw new RuntimeException(ex);
        }
        catch(java.lang.IllegalAccessException ex)
        {
            throw new RuntimeException(ex);
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
            throw new RuntimeException(ex);
        }

        return initial;
    }

    private static class TestObjectReader extends Ice.ObjectReader
    {
        @Override
        public void read(Ice.InputStream in)
        {
            in.startValue();
            in.startSlice();
            in.endSlice();
            in.endValue(false);
        }
    }

    private static class BObjectReader extends Ice.ObjectReader
    {
        @Override
        public void read(Ice.InputStream in)
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

    private static class CObjectReader extends Ice.ObjectReader
    {
        @Override
        public void read(Ice.InputStream in)
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

    private static class DObjectWriter extends Ice.ObjectWriter
    {
        @Override
        public void write(Ice.OutputStream out)
        {
            out.startValue(null);
            // ::Test::D
            out.startSlice("::Test::D", -1, false);
            out.writeString("test");
            out.writeOptional(1, Ice.OptionalFormat.FSize);
            String[] o = { "test1", "test2", "test3", "test4" };
            int pos = out.startSize();
            out.writeStringSeq(o);
            out.endSize(pos);
            A a = new A();
            a.setMc(18);
            out.writeOptional(1000, Ice.OptionalFormat.Class);
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

    private static class DObjectReader extends Ice.ObjectReader
    {
        @Override
        public void read(Ice.InputStream in)
        {
            in.startValue();
            // ::Test::D
            in.startSlice();
            String s = in.readString();
            test(s.equals("test"));
            test(in.readOptional(1, Ice.OptionalFormat.FSize));
            in.skip(4);
            String[] o = in.readStringSeq();
            test(o.length == 4 &&
                 o[0].equals("test1") && o[1].equals("test2") && o[2].equals("test3") && o[3].equals("test4"));
            test(in.readOptional(1000, Ice.OptionalFormat.Class));
            in.readValue(a);
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
            test(((A)a.obj).getMc() == 18);
        }

        private ReadValueCallbackI a = new ReadValueCallbackI();
    }

    private static class FObjectReader extends Ice.ObjectReader
    {
        @Override
        public void read(Ice.InputStream in)
        {
            _f = new F();
            in.startValue();
            in.startSlice();
            // Don't read af on purpose
            //in.read(1, _f.af);
            in.endSlice();
            in.startSlice();
            in.readValue(new Ice.ReadValueCallback()
                {
                    @Override
                    public void valueReady(Ice.Object obj)
                    {
                        _f.ae = (A)obj;
                    }
                });
            in.endSlice();
            in.endValue(false);
        }

        F getF()
        {
            return _f;
        }

        private F _f;
    }

    private static class FactoryI implements Ice.ValueFactory
    {
        @Override
        public Ice.Object create(String typeId)
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

    private static class ReadValueCallbackI implements Ice.ReadValueCallback
    {
        @Override
        public void valueReady(Ice.Object obj)
        {
            this.obj = obj;
        }

        Ice.Object obj;
    }
}

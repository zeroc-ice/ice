// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test._InitialDisp;

public class Collocated extends test.Util.Application
{
    private static class MyValueFactory implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String type)
        {
            if(type.equals("::Test::B"))
            {
                return new BI();
            }
            else if(type.equals("::Test::C"))
            {
                return new CI();
            }
            else if(type.equals("::Test::D"))
            {
                return new DI();
            }
            else if(type.equals("::Test::E"))
            {
                return new EI();
            }
            else if(type.equals("::Test::F"))
            {
                return new FI();
            }
            else if(type.equals("::Test::I"))
            {
                return new HI();
            }
            else if(type.equals("::Test::J"))
            {
                return new HI();
            }
            else if(type.equals("::Test::H"))
            {
                return new HI();
            }

            assert (false); // Should never be reached
            return null;
        }
    }

    @SuppressWarnings("deprecation")
    private static class MyObjectFactory implements com.zeroc.Ice.ObjectFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String type)
        {
            return null;
        }

        @Override
        public void destroy()
        {
            //
        }
    }

    @SuppressWarnings("deprecation")
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
        communicator.getValueFactoryManager().add(factory, "::Test::B");
        communicator.getValueFactoryManager().add(factory, "::Test::C");
        communicator.getValueFactoryManager().add(factory, "::Test::D");
        communicator.getValueFactoryManager().add(factory, "::Test::E");
        communicator.getValueFactoryManager().add(factory, "::Test::F");
        communicator.getValueFactoryManager().add(factory, "::Test::I");
        communicator.getValueFactoryManager().add(factory, "::Test::J");
        communicator.getValueFactoryManager().add(factory, "::Test::H");

        communicator.addObjectFactory(new MyObjectFactory(), "TestOF");

        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        _InitialDisp initial = new InitialI(adapter);
        adapter.add(initial, com.zeroc.Ice.Util.stringToIdentity("initial"));
        UnexpectedObjectExceptionTestI object = new UnexpectedObjectExceptionTestI();
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("uoet"));
        AllTests.allTests(communicator, getWriter());
        //
        // We must call shutdown even in the collocated case for cyclic dependency cleanup.
        //
        initial.shutdown(null);
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        return r;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }
}

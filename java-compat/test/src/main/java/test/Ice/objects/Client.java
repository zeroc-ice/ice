// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.InitialPrx;

public class Client extends test.Util.Application
{
    private static class MyValueFactory implements Ice.ValueFactory
    {
        @Override
        public Ice.Object create(String type)
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
                return new II();
            }
            else if(type.equals("::Test::J"))
            {
                return new JI();
            }
            else if(type.equals("::Test::H"))
            {
                return new HI();
            }

            assert (false); // Should never be reached
            return null;
        }
    }

    private static class MyObjectFactory implements Ice.ObjectFactory
    {
        @Override
        public Ice.Object create(String type)
        {
            return null;
        }

        @Override
        public void destroy()
        {
            //
        }
    }

    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        Ice.ValueFactory factory = new MyValueFactory();
        communicator.getValueFactoryManager().add(factory, "::Test::B");
        communicator.getValueFactoryManager().add(factory, "::Test::C");
        communicator.getValueFactoryManager().add(factory, "::Test::D");
        communicator.getValueFactoryManager().add(factory, "::Test::E");
        communicator.getValueFactoryManager().add(factory, "::Test::F");
        communicator.getValueFactoryManager().add(factory, "::Test::I");
        communicator.getValueFactoryManager().add(factory, "::Test::J");
        communicator.getValueFactoryManager().add(factory, "::Test::H");

        communicator.addObjectFactory(new MyObjectFactory(), "TestOF");

        InitialPrx initial = AllTests.allTests(communicator, getWriter());
        initial.shutdown();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}

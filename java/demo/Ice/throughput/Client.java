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

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        final String refProperty = "Throughput.Throughput";
        String ref = properties.getProperty(refProperty);
        if(ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        ThroughputPrx throughput = ThroughputPrxHelper.checkedCast(base);
        if(throughput == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        // Initial ping to setup the connection.
        throughput.ice_ping();

        long tmsec = System.currentTimeMillis();

        final int repetitions = 100;

        System.out.println("sending and receiving " + repetitions + " sequences of size " + seqSize.value +
                           " (this may take a while)");
        byte[] seq = new byte[seqSize.value];
        for(int i = 0; i < repetitions; i++)
        {
            throughput.echoByteSeq(seq);
        }

        double dmsec = System.currentTimeMillis() - tmsec;

        System.out.println("time for " + repetitions + " sequences: " + dmsec + "ms");
        System.out.println("time per sequence: " + (dmsec / repetitions) + "ms");

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties(args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
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

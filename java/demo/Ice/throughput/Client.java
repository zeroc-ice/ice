// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    private static void
    menu()
    {
        System.out.println(
            "usage:\n" +
	    "s: send byte sequence\n" +
	    "o: send byte sequence as oneway\n" +
	    "r: receive byte sequence\n" +
	    "e: echo (send and receive) byte sequence\n" +
	    "x: exit\n" +
            "?: help\n");
    }

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
	ThroughputPrx throughputOneway = ThroughputPrxHelper.uncheckedCast(throughput.ice_oneway());

        byte[] seq = new byte[seqSize.value];

	menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String line = null;
        do
        {
            try
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }

		// Initial ping to setup the connection.
		throughput.ice_ping();
		
		long tmsec = System.currentTimeMillis();
		final int repetitions = 100;

                if(line.equals("s") || line.equals("o") || line.equals("r") || line.equals("e"))
                {
		    char c = line.charAt(0);

		    switch(c)
		    {
			case 's':
			case 'o':
			{
			    System.out.print("sending");
			    break;
			}
			
			case 'r':
			{
			    System.out.print("receiving");
			    break;
			}
			
			case 'e':
			{
			    System.out.print("sending and receiving");
			    break;
			}
		    }
		    
		    System.out.print(" " + repetitions + " sequences of size " + seqSize.value);
		    
		    if(c == 'o')
		    {
			System.out.print(" as oneway");
		    }
		    
		    System.out.println("...");
		    
		    for(int i = 0; i < repetitions; ++i)
		    {
			switch(c)
			{
			    case 's':
			    {
				throughput.sendByteSeq(seq);
				break;
			    }
			    
			    case 'o':
			    {
				throughputOneway.sendByteSeq(seq);
				break;
			    }
			    
			    case 'r':
			    {
				throughput.recvByteSeq();
				break;
			    }
			    
			    case 'e':
			    {
				throughput.echoByteSeq(seq);
				break;
			    }
			}
		    }

		    double dmsec = System.currentTimeMillis() - tmsec;
		    System.out.println("time for " + repetitions + " sequences: " + dmsec  + "ms");
		    System.out.println("time per sequence: " + dmsec / repetitions + "ms");
		    double mbit = repetitions * seqSize.value * 8.0 / dmsec / 1000.0;
		    if(c == 'e')
		    {
			mbit *= 2;
		    }
		    System.out.println("throughput: " + mbit + " MBit/s");
		}
		else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
                }
                else
                {
                    System.out.println("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!line.equals("x"));

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
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

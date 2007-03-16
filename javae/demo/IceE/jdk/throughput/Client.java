// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
        "\n" +
        "toggle type of data to send:\n" +
        "1: sequence of bytes (default)\n" +
        "2: sequence of strings (\"hello\")\n" +
        "3: sequence of structs with a string (\"hello\") and a double\n" +
        "4: sequence of structs with two ints and a double\n" +
        "\n" +
        "select test to run:\n" +
        "t: Send sequence as twoway\n" +
        "o: Send sequence as oneway\n" +
        "r: Receive sequence\n" +
        "e: Echo (send and receive) sequence\n" +
        "\n" +
        "other commands:\n" +
        "s: shutdown server\n" +
        "x: exit\n" +
        "?: help\n");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        //
	// Check if we need to run with small sequences.
	//
	int reduce = 1;
	for(int i = 0; i < args.length; ++i)
	{
	    if(args[i].equals("--small"))
	    {
	        reduce = 100;
	    }
	}

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

        byte[] byteSeq = new byte[ByteSeqSize.value / reduce];

	String[] stringSeq = new String[StringSeqSize.value / reduce];
	for(int i = 0; i < StringSeqSize.value / reduce; ++i)
	{
	    stringSeq[i] = "hello";
	}

	StringDouble[] structSeq = new StringDouble[StringDoubleSeqSize.value / reduce];
	for(int i = 0; i < StringDoubleSeqSize.value / reduce; ++i)
	{
	    structSeq[i] = new StringDouble();
	    structSeq[i].s = "hello";
	    structSeq[i].d = 3.14;
	}

	Fixed[] fixedSeq = new Fixed[FixedSeqSize.value / reduce];
	for(int i = 0; i < FixedSeqSize.value / reduce; ++i)
	{
	    fixedSeq[i] = new Fixed();
	    fixedSeq[i].i = 0;
	    fixedSeq[i].j = 0;
	    fixedSeq[i].d = 0;
	}

	menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

	char currentType = '1';
	int seqSize = ByteSeqSize.value / reduce;

	// Initial ping to setup the connection.
	throughput.ice_ping();
	
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

		long tmsec = System.currentTimeMillis();
		final int repetitions = 100;

		if(line.equals("1") || line.equals("2") || line.equals("3") || line.equals("4"))
		{
		    currentType = line.charAt(0);

		    switch(currentType)
		    {
		        case '1':
			{
			    System.out.println("using byte sequences");
			    seqSize = ByteSeqSize.value;
			    break;
			}

			case '2':
			{
			    System.out.println("using string sequences");
			    seqSize = StringSeqSize.value;
			    break;
			}

			case '3':
			{
			    System.out.println("using variable-length struct sequences");
			    seqSize = StringDoubleSeqSize.value;
			    break;
			}

			case '4':
			{
			    System.out.println("using fixed-length struct sequences");
			    seqSize = FixedSeqSize.value;
			    break;
			}
		    }
		}
                else if(line.equals("t") || line.equals("o") || line.equals("r") || line.equals("e"))
                {
		    char c = line.charAt(0);

		    switch(c)
		    {
			case 't':
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
		    
		    System.out.print(" " + repetitions);
		    switch(currentType)
		    {
		        case '1':
			{
			    System.out.print(" byte");
			    break;
			}

		        case '2':
			{
			    System.out.print(" string");
			    break;
			}

		        case '3':
			{
			    System.out.print(" variable-length struct");
			    break;
			}

		        case '4':
			{
			    System.out.print(" fixed-length struct");
			    break;
			}
		    }
		    
		    System.out.print(" sequences of size " + seqSize);
		    
		    if(c == 'o')
		    {
			System.out.print(" as oneway");
		    }
		    
		    System.out.println("...");
		    
		    for(int i = 0; i < repetitions; ++i)
		    {
		        switch(currentType)
			{
			    case '1':
			    {
			        switch(c)
			        {
			            case 't':
			            {
				        throughput.sendByteSeq(byteSeq);
				        break;
			            }
			    
			            case 'o':
			            {
				        throughputOneway.sendByteSeq(byteSeq);
				        break;
			            }
			    
			            case 'r':
			            {
				        throughput.recvByteSeq();
				        break;
			            }
			    
			            case 'e':
			            {
				        throughput.echoByteSeq(byteSeq);
				        break;
			            }
			        }
				break;
			    }

			    case '2':
			    {
			        switch(c)
			        {
			            case 't':
			            {
				        throughput.sendStringSeq(stringSeq);
				        break;
			            }
			    
			            case 'o':
			            {
				        throughputOneway.sendStringSeq(stringSeq);
				        break;
			            }
			    
			            case 'r':
			            {
				        throughput.recvStringSeq();
				        break;
			            }
			    
			            case 'e':
			            {
				        throughput.echoStringSeq(stringSeq);
				        break;
			            }
			        }
				break;
			    }

			    case '3':
			    {
			        switch(c)
			        {
			            case 't':
			            {
				        throughput.sendStructSeq(structSeq);
				        break;
			            }
			    
			            case 'o':
			            {
				        throughputOneway.sendStructSeq(structSeq);
				        break;
			            }
			    
			            case 'r':
			            {
				        throughput.recvStructSeq();
				        break;
			            }
			    
			            case 'e':
			            {
				        throughput.echoStructSeq(structSeq);
				        break;
			            }
			        }
				break;
			    }

			    case '4':
			    {
			        switch(c)
			        {
			            case 't':
			            {
				        throughput.sendFixedSeq(fixedSeq);
				        break;
			            }
			    
			            case 'o':
			            {
				        throughputOneway.sendFixedSeq(fixedSeq);
				        break;
			            }
			    
			            case 'r':
			            {
				        throughput.recvFixedSeq();
				        break;
			            }
			    
			            case 'e':
			            {
				        throughput.echoFixedSeq(fixedSeq);
				        break;
			            }
			        }
				break;
			    }
			}
		    }

		    double dmsec = System.currentTimeMillis() - tmsec;
		    System.out.println("time for " + repetitions + " sequences: " + dmsec  + "ms");
		    System.out.println("time per sequence: " + dmsec / repetitions + "ms");
		    int wireSize = 0;
		    switch(currentType)
		    {
		        case '1':
			{
			    wireSize = 1;
			    break;
			}

			case '2':
			{
			    wireSize = stringSeq[0].length();
			    break;
			}

			case '3':
			{
			    wireSize = structSeq[0].s.length();
			    wireSize += 8; // Size of double on the wire.
			    break;
			}

			case '4':
			{
			    wireSize = 16; // Size of two ints and a double on the wire.
			    break;
			}
		    }
		    double mbit = repetitions * seqSize * wireSize * 8.0 / dmsec / 1000.0;
		    if(c == 'e')
		    {
			mbit *= 2;
		    }
		    System.out.println("throughput: " + new java.text.DecimalFormat("#.##").format(mbit) + "Mbps");
		}
		else if(line.equals("s"))
		{
		    throughput.shutdown();
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
	    Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.load("config");
            communicator = Ice.Util.initialize(args, initData);
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

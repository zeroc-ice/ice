// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

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

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        ThroughputPrx throughput = ThroughputPrxHelper.checkedCast(communicator().propertyToProxy("Throughput.Proxy"));
        if(throughput == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        ThroughputPrx throughputOneway = ThroughputPrxHelper.uncheckedCast(throughput.ice_oneway());

        byte[] byteSeq = new byte[ByteSeqSize.value];

        String[] stringSeq = new String[StringSeqSize.value];
        for(int i = 0; i < StringSeqSize.value; ++i)
        {
            stringSeq[i] = "hello";
        }

        StringDouble[] structSeq = new StringDouble[StringDoubleSeqSize.value];
        for(int i = 0; i < StringDoubleSeqSize.value; ++i)
        {
            structSeq[i] = new StringDouble();
            structSeq[i].s = "hello";
            structSeq[i].d = 3.14;
        }

        Fixed[] fixedSeq = new Fixed[FixedSeqSize.value];
        for(int i = 0; i < FixedSeqSize.value; ++i)
        {
            fixedSeq[i] = new Fixed();
            fixedSeq[i].i = 0;
            fixedSeq[i].j = 0;
            fixedSeq[i].d = 0;
        }

        //
        // A method needs to be invoked thousands of times before the
        // JIT compiler will convert it to native code. To ensure an
        // accurate throughput measurement, we need to "warm up" the
        // JIT compiler.
        //
        {
            byte[] emptyBytes= new byte[1];
            String[] emptyStrings = new String[1];
            StringDouble[] emptyStructs = new StringDouble[1];
            emptyStructs[0] = new StringDouble();
            Fixed[] emptyFixed = new Fixed[1];
            emptyFixed[0] = new Fixed();

            throughput.startWarmup();

            System.out.print("warming up the client/server...");
            System.out.flush();
            for(int i = 0; i < 10000; i++)
            {
                throughput.sendByteSeq(emptyBytes);
                throughput.sendStringSeq(emptyStrings);
                throughput.sendStructSeq(emptyStructs);
                throughput.sendFixedSeq(emptyFixed);

                throughput.recvByteSeq();
                throughput.recvStringSeq();
                throughput.recvStructSeq();
                throughput.recvFixedSeq();
                
                throughput.echoByteSeq(emptyBytes);
                throughput.echoStringSeq(emptyStrings);
                throughput.echoStructSeq(emptyStructs);
                throughput.echoFixedSeq(emptyFixed);
            }
            throughput.endWarmup();
            
            System.out.println(" ok");
        }

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        char currentType = '1';
        int seqSize = ByteSeqSize.value;

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
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}

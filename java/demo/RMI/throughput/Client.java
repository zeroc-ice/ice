// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

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
        "t: Send sequence\n" +
        "r: Receive sequence\n" +
        "e: Echo (send and receive) sequence\n" +
        "\n" +
        "other commands:\n" +
        "s: shutdown server\n" +
        "x: exit\n" +
        "?: help\n");
    }

    public int
    run(String args[]) throws java.lang.Exception
    {
        String registryHost;
        if(args.length == 0)
        {
            registryHost = "127.0.0.1";
        }
        else if(args.length == 1)
        {
            registryHost = args[0];
        }
        else
        {
            System.err.println("usage: java Client [registryHost]");
            return 1;
        }


        if(System.getSecurityManager() == null)
        {
            System.setSecurityManager(new SecurityManager());
        }
        Registry registry = LocateRegistry.getRegistry(registryHost);
        Throughput throughput = (Throughput)registry.lookup("throughput");

        byte[] byteSeq = new byte[Throughput.ByteSeqSize];

        String[] stringSeq = new String[Throughput.StringSeqSize];
        for(int i = 0; i < Throughput.StringSeqSize; ++i)
        {
            stringSeq[i] = new String("hello");
        }

        Throughput.StringDouble[] structSeq = new Throughput.StringDouble[Throughput.StringDoubleSeqSize];
        for(int i = 0; i < Throughput.StringDoubleSeqSize; ++i)
        {
            structSeq[i] = new Throughput.StringDouble();
            structSeq[i].s = new String("Hello");
            structSeq[i].d = 3.14;
        }

        Throughput.Fixed[] fixedSeq = new Throughput.Fixed[Throughput.FixedSeqSize];
        for(int i = 0; i < Throughput.FixedSeqSize; ++i)
        {
            fixedSeq[i] = new Throughput.Fixed();
            fixedSeq[i].i = 0;
            fixedSeq[i].j = 0;
            fixedSeq[i].d = 0;
        }

        {
            byte[] emptyBytes= new byte[1];
            String[] emptyStrings = new String[1];
            Throughput.StringDouble[] emptyStructs = new Throughput.StringDouble[1];
            emptyStructs[0] = new Throughput.StringDouble();
            Throughput.Fixed[] emptyFixed = new Throughput.Fixed[1];
            emptyFixed[0] = new Throughput.Fixed();

            System.out.print("warming up...");
            System.out.flush();
            for(int i = 0; i < 100; i++)
            {
                throughput.sendByteSeq(emptyBytes);
                throughput.recvByteSeq();
                throughput.echoByteSeq(emptyBytes);

                throughput.sendStringSeq(emptyStrings);
                throughput.recvStringSeq();
                throughput.echoFixedSeq(emptyFixed);

                throughput.sendStructSeq(emptyStructs);
                throughput.recvStructSeq();
                throughput.echoStructSeq(emptyStructs);

                throughput.sendFixedSeq(emptyFixed);
                throughput.recvFixedSeq();
                throughput.echoStringSeq(emptyStrings);
            }
            System.out.println();
        }

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        char currentType = '1';
        int seqSize = Throughput.ByteSeqSize;

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
                            seqSize = Throughput.ByteSeqSize;
                            break;
                        }

                        case '2':
                        {
                            System.out.println("using string sequences");
                            seqSize = Throughput.StringSeqSize;
                            break;
                        }

                        case '3':
                        {
                            System.out.println("using variable-length struct sequences");
                            seqSize = Throughput.StringDoubleSeqSize;
                            break;
                        }

                        case '4':
                        {
                            System.out.println("using fixed-length struct sequences");
                            seqSize = Throughput.FixedSeqSize;
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
                    
                    System.out.print(" sequences of size " + seqSize + "...");
                    
                    
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
                    int size = 0;
                    switch(currentType)
                    {
                        case '1':
                        {
                            size = 1;
                            break;
                        }

                        case '2':
                        {
                            size = stringSeq[0].length();
                            break;
                        }

                        case '3':
                        {
                            size = structSeq[0].s.length() + 8;
                            break;
                        }

                        case '4':
                        {
                            size = 16; // Two ints and a double.
                            break;
                        }
                    }
                    double mbit = repetitions * seqSize * size * 8.0 / dmsec / 1000.0;
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
            catch(Exception ex)
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
        int status;
        try
        {
            status = app.run(args);
        }
        catch(Exception e)
        {
            System.err.println(e);
            status = 1;
        }
        System.exit(status);
    }
}

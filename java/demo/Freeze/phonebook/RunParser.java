// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class RunParser
{
    static void
    usage(String appName)
    {
	System.err.println("Usage: " + appName + " [options] [file...]\n");
	System.err.print(
	    "Options:\n" +
	    "-h, --help           Show this message.\n");
	//"-v, --version        Display the Ice version.\n"
    }

    static int
    runParser(String appName, String[] args, Ice.Communicator communicator)
    {
	String commands;
	boolean debug = false;
	
	int idx = 1;
	while (idx < args.length)
	{
	    if (args[idx].equals("-h") | args[idx].equals("--help"))
	    {
		usage(appName);
		return 0;
	    }
/*
  else if (args[idx].equals("-v") || args[idx].equals("--version"))
  {
  cout + ICE_STRING_VERSION + endl;
  return 0;
  }
*/
	    else if (args[idx].charAt(0) == '-')
	    {
		System.err.println(appName + ": unknown option `" + args[idx] + "'");
		usage(appName);
		return 1;
	    }
	    else
	    {
		++idx;
	    }
	}

	Ice.Properties properties = communicator.getProperties();
	String refProperty = "PhoneBook.PhoneBook";
	String ref = properties.getProperty(refProperty);
	if (ref == null)
	{
	    System.err.println(appName +  ": property `" + refProperty + "' not set");
	    return 1;
	}

	Ice.ObjectPrx base = communicator.stringToProxy(ref);
	PhoneBookPrx phoneBook = PhoneBookPrxHelper.checkedCast(base);
	if (phoneBook == null)
	{
	    System.err.println(appName + ": invalid object reference");
	    return 1;
	}

	Parser parser = new Parser(communicator, phoneBook);
	int status = 0;

	int parseStatus = parser.parse();
	if (parseStatus != 0)
	{
	    status = 1;
	}

	return status;
    }
}

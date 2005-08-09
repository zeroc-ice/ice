// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// CLDC does not provide reflection APIs, so we need to assume the presence of a packageless Client or Server
// class.
//
// The test driver wrappers need to initiate the tests through the test driver's 'run' methods to circumvent calls
// to System.exit() in the driver's 'main()' methods. While this means the wrapper has to initialize a communicator
// instance, this is actually a good thing as it affords some control over the driver while it is running.
// 
class WrapperBase
{
    public
    WrapperBase(String[] args, TestApplication app, java.io.PrintStream out)
    {
	Ice.Properties properties = Ice.Util.createProperties();
	java.io.InputStream is = getClass().getResourceAsStream("config");
	if(is != null)
	{
	    properties.load(is);
	}
	_communicator = Ice.Util.initializeWithProperties(args, properties);
	_args = args;
	_app = app;
	_out = out;
    }

    public void
    shutdown()
    {
	if(_communicator != null)
	{
	    try
	    {
		_communicator.shutdown();
		_communicator.destroy();
		_communicator = null;
	    }
	    catch(Exception ex)
	    {
		_app.message("Exception occurred on shutdown: " + ex.toString());
	    }
	}
    }

    protected Ice.Communicator _communicator;
    protected String[] _args;
    protected TestApplication _app;
    protected java.io.PrintStream _out;
}

class ClientWrapper 
    extends WrapperBase 
    implements Runnable
{
    public
    ClientWrapper(String[] args, TestApplication app, java.io.PrintStream out)
    {
	super(args, app, out);
    }
    
    public void
    run()
    {
	try
	{
	    if(_communicator != null)
	    {
		Client.run(_args, _communicator, _out);
	    }
	}
	catch(Exception ex)
	{
	    _out.println(ex.toString());
	    ex.printStackTrace();
	}
    }
}

class ServerWrapper 
    extends WrapperBase
    implements Runnable
{
    public
    ServerWrapper(String[] args, TestApplication app, java.io.PrintStream out)
    {
	super(args, app, out);
    }

    public void
    run()
    {
	//
	// TODO: The server wrapper should provide some visual feedback to indicate that the server is up and
	// running. The other wrappers will pretty much have immediate feedback because they will actually be
	// running tests.
	//
	try
	{
	    if(_communicator != null)
	    {
		Server.run(_args, _communicator, _out);
	    }
	    _app.done();
	}
	catch(Exception ex)
	{
	    _out.println(ex.toString());
	    ex.printStackTrace();
	}
    }
}

class CollocatedWrapper 
    extends WrapperBase
    implements Runnable
{
    public
    CollocatedWrapper(String[] args, TestApplication app, java.io.PrintStream out)
    {
	super(args, app, out);
    }

    public void
    run()
    {
	try
	{
	    if(_communicator != null)
	    {
		Collocated.run(_args, _communicator, _out); 
	    }
	    _app.done();
	}
	catch(Exception ex)
	{
	    _out.println(ex.toString());
	    ex.printStackTrace();
	}
    }
}

//
// Wraps a test driver class to run it within a MIDP environment.
//
public class TestApplication
    extends javax.microedition.midlet.MIDlet
    implements javax.microedition.lcdui.CommandListener
{

    class ShutdownTask 
	implements Runnable
    {
	public void
	run()
	{
	    if(_client != null)
	    {
		_client.shutdown();
	    }
	    if(_server != null)
	    {
		_server.shutdown();
	    }
	    if(_colloc != null)
	    {
		_colloc.shutdown();
	    }
	}
    }

    //
    // Break a space delimited string into individual tokens.
    // 
    private String[]
    parseArgs(String argString)
    {
	if(argString == null || argString.length() == 0)
	{
	    return new String[0];
	}

	java.util.Vector v = new java.util.Vector();
	int current = 0;
	int end = 0;
	while(true)
	{
	    //
	    // Locate space.
	    //
	    end = argString.indexOf(' ', current);
	    if(end == -1)
	    {
		v.addElement(argString.substring(current));
		break;
	    }

	    //
	    // Extract token.
	    //
	    String s = argString.substring(current, end);
	    if(s.length() > 0)
	    {
		v.addElement(s);
	    }

	    //
	    // Move down the string, finishing if we've reached the end.
	    //
	    current = end + 1;
	    if(current >= argString.length())
	    {
		break;
	    }
	}

	String[] results = new String[v.size()];
	v.copyInto(results);
	return results;
    }
    
    protected void
    startApp()
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	try
	{
	    if(_display == null)
	    {
		_display = javax.microedition.lcdui.Display.getDisplay(this);
		_outList.addCommand(CMD_EXIT);
		_outList.setCommandListener(this);
	    }
	    _out = new java.io.PrintStream(new Test.StreamListAdapter(_outList));

	    String runServer = getAppProperty("runserver");
	    String serverArgString = getAppProperty("serverargs");

	    if(runServer != null && runServer.length() != 0 &&
		    (runServer.equalsIgnoreCase("yes") || runServer.equalsIgnoreCase("true")))
	    {
		_server = new ServerWrapper(parseArgs(serverArgString), this, _out);
	    }

	    String runClient =  getAppProperty("runclient");
	    String clientArgString = getAppProperty("clientargs");
	    if(runClient != null && runClient.length() != 0 &&
		    (runClient.equalsIgnoreCase("yes") || runClient.equalsIgnoreCase("true")))
	    {
		_client = new ClientWrapper(parseArgs(clientArgString), this, _out);
		_outList.addCommand(CMD_STARTCLIENT);
	    }

	    String exitOnFinish = getAppProperty("autoexit");
	    if(exitOnFinish != null && (exitOnFinish.equalsIgnoreCase("yes") || exitOnFinish.equalsIgnoreCase("true")))
	    {
		_autoexit = true;
	    }

	    //
	    // If neither the client or server configurations are specified, then we assume a collocated test
	    // configuration.
	    //
	    if(_client == null && _server == null)
	    {
		//
		// If there is no server or client configuration assume a collocated case.
		//
		String collocArgs = getAppProperty("collocatedargs");
		_colloc = new CollocatedWrapper(parseArgs(collocArgs), this, _out);
	    }
	    _display.setCurrent(_outList);
	}
	catch(Exception ex)
	{
	    javax.microedition.lcdui.Alert a = 
		new javax.microedition.lcdui.Alert("startApp alert", ex.getMessage(),
			null, javax.microedition.lcdui.AlertType.ERROR);
	    a.setTimeout(javax.microedition.lcdui.Alert.FOREVER);
	    _display.setCurrent(a);
	    throw new javax.microedition.midlet.MIDletStateChangeException(ex.getMessage());
	}

	if(_colloc != null)
	{
	    message("Starting up colloc");
	    new Thread(_colloc).start();
	    message("Colloc started");
	}
	else if(_server != null)
	{
	    new Thread(_server).start();
	}
    }

    protected void
    pauseApp()
    {
	//
	// The test wrapper will not pause properly because it does not have direct access to the wrapped test
	// driver's communicators.
	//
    }

    protected void
    destroyApp(boolean unconditional)
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	new Thread(new ShutdownTask()).start();
    }

    public void
    commandAction(javax.microedition.lcdui.Command cmd, javax.microedition.lcdui.Displayable source)
    {
	if(source == _outList)
	{
	    if(cmd == CMD_EXIT)
	    {
		try
		{
		    destroyApp(true);
		    notifyDestroyed();
		}
		catch(javax.microedition.midlet.MIDletStateChangeException ex)
		{
		}
	    }
	    else if(cmd == CMD_STARTCLIENT)
	    {
		_outList.removeCommand(CMD_STARTCLIENT);
		new Thread(_client).start();
		_display.setCurrent(_outList);
	    }
	}
    }

    public void
    message(String msg)
    {
	_out.println(msg);
    }

    void
    done()
    {
	if(_autoexit)
	{
	    try
	    {
		destroyApp(true);
		notifyDestroyed();
	    }
	    catch(javax.microedition.midlet.MIDletStateChangeException ex)
	    {
		message(ex.toString());
	    }
	}
	message("Done!");
    }
    
    private javax.microedition.lcdui.List _outList =
        new javax.microedition.lcdui.List("Standard Output",
					  javax.microedition.lcdui.Choice.EXCLUSIVE);
    private java.io.PrintStream _out;

    private static final int CMD_PRIORITY = 1;

    private javax.microedition.lcdui.Command CMD_EXIT =
        new javax.microedition.lcdui.Command("Exit", javax.microedition.lcdui.Command.EXIT, CMD_PRIORITY);
    private javax.microedition.lcdui.Command CMD_STARTCLIENT =
        new javax.microedition.lcdui.Command("Run", javax.microedition.lcdui.Command.ITEM, CMD_PRIORITY);
   
    private javax.microedition.lcdui.Display _display;

    private ServerWrapper _server;
    private ClientWrapper _client;
    private CollocatedWrapper _colloc;
    private boolean _autoexit = false;
}

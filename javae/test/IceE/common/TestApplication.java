// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
	    shutdown();
	}
    }
    
    protected void
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
		message("Exception occurred on shutdown: " + ex.toString());
	    }
	}
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
		_screen.addCommand(CMD_EXIT);
		_screen.setCommandListener(this);
	    }
	    _out = new java.io.PrintStream(new Test.StreamListAdapter((javax.microedition.lcdui.List)_screen));
	    _display.setCurrent(_screen);

	    Ice.Properties properties = Ice.Util.createProperties();
	    java.io.InputStream is = getClass().getResourceAsStream("config");
	    if(is != null)
	    {
		properties.load(is);
	    }
	    _communicator = Ice.Util.initializeWithProperties(new String[0], properties);
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
	if(source == _screen)
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
	}
    }

    protected void
    message(String msg)
    {
	_out.println(msg);
    }

    protected void
    done()
    {
	message("Done!");
    }
    
    protected javax.microedition.lcdui.Display _display;
    protected javax.microedition.lcdui.Screen _screen =
        new javax.microedition.lcdui.List("Standard Output",
					  javax.microedition.lcdui.Choice.EXCLUSIVE);

    protected java.io.PrintStream _out;

    protected static final int CMD_PRIORITY = 1;

    protected javax.microedition.lcdui.Command CMD_EXIT =
        new javax.microedition.lcdui.Command("Exit", javax.microedition.lcdui.Command.EXIT, CMD_PRIORITY);

    protected Ice.Communicator _communicator;
}

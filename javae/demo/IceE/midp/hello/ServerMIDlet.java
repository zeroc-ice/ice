// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ServerMIDlet
    extends javax.microedition.midlet.MIDlet
    implements javax.microedition.lcdui.CommandListener
{
    static class HelloI extends Demo._HelloDisp
    {
	public
	HelloI(javax.microedition.lcdui.StringItem msg)
	{
	    _msg = msg;
	}

	public void
	sayHello(int delay, Ice.Current current)
	{
	    _msg.setText("Hello World!");
	}

	public void
	shutdown(Ice.Current current)
	{
	    _msg.setText("received shutdown request");
	}

	javax.microedition.lcdui.StringItem _msg = null;
    }

    class StartServer implements Runnable
    {
	public void
	run()
	{
	    try
	    {
		Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("Hello");
		Ice.Object object = new HelloI(_msg);
		adapter.add(object, _communicator.stringToIdentity("hello"));
		adapter.activate();
		_msg.setText("Using address " + System.getProperty("microedition.hostname"));
	    }
	    catch(Exception ex)
	    {
		_msg.setText("Unable to initialize Ice server, please check your configuration and start again.");
	    }
	}
    }

    class StopServer implements Runnable
    {
	public void
	run()
	{
	    handleExitCmd();
	}
    }
    
    protected void
    startApp()
    {
	java.io.InputStream is = getClass().getResourceAsStream("config");
	Ice.InitializationData initData = new Ice.InitializationData();
	initData.properties = Ice.Util.createProperties();
	initData.properties.load(is);
	_communicator = Ice.Util.initialize(new String[0], initData);

	if(_display == null)
	{
	    _display = javax.microedition.lcdui.Display.getDisplay(this);
	    _form = new javax.microedition.lcdui.Form("Ice - Hello World Server");
	    _form.append(_msg);
	    _form.addCommand(CMD_EXIT);
	    _form.setCommandListener(this);
	}
	_display.setCurrent(_form);
	new Thread(new StartServer()).start();
    }

    protected void
    pauseApp()
    {
	if(_communicator != null)
	{
	    try
	    {
		_communicator.destroy();
		_communicator = null;
	    }
	    catch(Exception ex)
	    {
	    }
	}
    }

    protected void
    destroyApp(boolean unconditional)
    {
	if(_communicator != null)
	{
	    try
	    {
		_communicator.destroy();
		_communicator = null;
	    }
	    catch(Exception ex)
	    {
	    }
	}
    }

    public void
    commandAction(javax.microedition.lcdui.Command cmd, javax.microedition.lcdui.Displayable source)
    {
	if(source == _form && cmd == CMD_EXIT)
	{
	    new Thread(new StopServer()).start();
	}
    }

    public void
    handleExitCmd()
    {
	destroyApp(true);
	notifyDestroyed();
    }

    public javax.microedition.lcdui.Form
    getForm()
    {
	return _form;
    }

    private javax.microedition.lcdui.Form _form;
    private javax.microedition.lcdui.Display _display;

    private Ice.Communicator _communicator;
    private Demo.HelloPrx _helloPrx;

    private static final int CMD_PRIORITY = 1;

    private javax.microedition.lcdui.Command CMD_EXIT =
        new javax.microedition.lcdui.Command("Exit", javax.microedition.lcdui.Command.EXIT, CMD_PRIORITY);

    private javax.microedition.lcdui.StringItem _msg =
        new javax.microedition.lcdui.StringItem("\nStatus: ", "(no status)");
}


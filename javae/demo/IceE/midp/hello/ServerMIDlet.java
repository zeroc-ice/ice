// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ServerMIDlet
    extends javax.microedition.midlet.MIDlet
    implements javax.microedition.lcdui.CommandListener
{
    private javax.microedition.lcdui.Form _form;
    private javax.microedition.lcdui.Display _display;

    private Ice.Communicator _communicator;
    private Demo.HelloPrx _helloPrx;

    private static final int CMD_PRIORITY = 1;

    private static final javax.microedition.lcdui.Command CMD_EXIT =
        new javax.microedition.lcdui.Command("Exit", javax.microedition.lcdui.Command.EXIT, CMD_PRIORITY);

    private static final javax.microedition.lcdui.Command CMD_START =
        new javax.microedition.lcdui.Command("Start", javax.microedition.lcdui.Command.ITEM, CMD_PRIORITY);

    private static final javax.microedition.lcdui.StringItem _msg =
        new javax.microedition.lcdui.StringItem("\nStatus: ", "(no status)");

    static class HelloI extends Demo._HelloDisp
    {
	public
	HelloI(javax.microedition.lcdui.StringItem msg)
	{
	    _msg = msg;
	}

	public void
	sayHello(Ice.Current current)
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
    
    protected void
    startApp()
    {
	java.io.InputStream is = getClass().getResourceAsStream("config");
	Ice.Properties properties = Ice.Util.createProperties();
	properties.load(is);
	_communicator = Ice.Util.initializeWithProperties(new String[0], properties);

	if(_display == null)
	{
	    _display = javax.microedition.lcdui.Display.getDisplay(this);
	    _form = new javax.microedition.lcdui.Form("Ice - Hello World Server");
	    _form.append("Select the `Hello' command to activate the hello server.\n");
	    _form.append(_msg);
	    _form.addCommand(CMD_EXIT);
	    _form.addCommand(CMD_START);
	    _form.setCommandListener(this);
	}
	_display.setCurrent(_form);
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
	if(source == _form)
	{
	    if(cmd == CMD_EXIT)
	    {
		handleExitCmd();
	    }
	    else if(cmd == CMD_START)
	    {
		handleStartCmd();
	    }
	}
    }

    public void
    handleStartCmd()
    {
	try
	{
	    Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("Hello");
	    _msg.setText("init'd adapter");
	    Ice.Object object = new HelloI(_msg);
	    _msg.setText("servant init'd");
	    adapter.add(object, Ice.Util.stringToIdentity("hello"));
	    _msg.setText("servant added");
	    adapter.activate();
	}
	catch(Exception ex)
	{
	    ex.printStackTrace();
//	    _msg.setText("Unable to initialize Ice server, please check your configuration and start again.");
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
}


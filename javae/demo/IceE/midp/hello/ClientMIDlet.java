// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ClientMIDlet
    extends javax.microedition.midlet.MIDlet
    implements javax.microedition.lcdui.CommandListener
{
    class HelloRequest implements Runnable
    {
	public void
	run()
	{
	    handleHelloCmd();
	}
    }

    class Shutdown implements Runnable
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
	    _form = new javax.microedition.lcdui.Form("Ice - Hello World Client");
	    _form.append("Select the `Hello' command to send a request to the hello server.\n");
	    _form.append(_msg);
	    _form.addCommand(CMD_EXIT);
	    _form.addCommand(CMD_HELLO);
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
		new Thread(new Shutdown()).start();
	    }
	    else if(cmd == CMD_HELLO)
	    {
		new Thread(_helloRequest).start();
	    }
	}
    }

    public void
    handleHelloCmd()
    {
	if(_helloPrx == null)
	{
	    Ice.Properties properties = _communicator.getProperties();
	    String proxy = properties.getProperty("Hello.Proxy");
	    if(proxy == null || proxy.length() == 0)
	    {
		_msg.setText("(unable to retrieve reference, please check the config file in the demo directory)");
	    }
	    try
	    {
		Ice.ObjectPrx base = _communicator.stringToProxy(proxy);
		_helloPrx = HelloPrxHelper.checkedCast(base);
	    }
	    catch(Exception ex)
	    {
		_msg.setText("'sayHello()' failed");
		return;
	    }
	}
	try
	{
	    _helloPrx.sayHello(0);
	    _msg.setText("'sayHello()' succeeded");
	}
	catch(Exception ex)
	{
	    _msg.setText("'sayHello()' failed");
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

    private HelloRequest _helloRequest = new HelloRequest();

    private javax.microedition.lcdui.Form _form;
    private javax.microedition.lcdui.Display _display;

    private Ice.Communicator _communicator;
    private Demo.HelloPrx _helloPrx;

    private static final int CMD_PRIORITY = 1;

    private javax.microedition.lcdui.Command CMD_EXIT =
        new javax.microedition.lcdui.Command("Exit", javax.microedition.lcdui.Command.EXIT, CMD_PRIORITY);

    private javax.microedition.lcdui.Command CMD_HELLO =
        new javax.microedition.lcdui.Command("Hello", javax.microedition.lcdui.Command.ITEM, CMD_PRIORITY);

    private javax.microedition.lcdui.StringItem _msg =
        new javax.microedition.lcdui.StringItem("\nStatus: ", "(no requests sent)");
}

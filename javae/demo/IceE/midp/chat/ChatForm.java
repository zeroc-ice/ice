// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

public class ChatForm extends Form implements CommandListener
{
    public
    ChatForm(ChatMIDlet parent, String user, Demo.ChatSessionPrx session, Glacier2.RouterPrx router)
    {
	super("Chat Demo");

	_parent = parent;
	_user = user;
	_session = session;
	_router = router;

	StringItem str = new StringItem(null, "Messages");
	_message = new TextField(null, "", 255, TextField.ANY);

	_console = new Console(null, null, null, getWidth(), 4);

	str.setLayout(Item.LAYOUT_TOP | Item.LAYOUT_LEFT | Item.LAYOUT_VSHRINK);
	_message.setLayout(Item.LAYOUT_BOTTOM | Item.LAYOUT_LEFT | Item.LAYOUT_EXPAND | Item.LAYOUT_VSHRINK);
	_console.setLayout(Item.LAYOUT_TOP | Item.LAYOUT_LEFT | Item.LAYOUT_EXPAND | Item.LAYOUT_VEXPAND);

	append(str);
	append(_console);
	append(_message);

	addCommand(new Command("Send", Command.SCREEN, 0));
	addCommand(new Command("Exit", Command.EXIT, 1));
	setCommandListener(this);

	_pingThread = new PingThread();
	_pingThread.start();
    }

    public Console
    getConsole()
    {
	return _console;
    }

    public void
    commandAction(Command c, Displayable s)
    {
	if(c.getCommandType() == Command.EXIT)
	{
	    new ExitThread().start();
	}
	else
	{
	    String message = _message.getString();
	    _message.setString("");
	    new SendThread(message).start();
	}
    }

    private class PingThread extends Thread
    {
	public synchronized void
	run()
	{
	    while(!_destroy)
	    {
		try
		{
		    wait(20000); // 20 seconds.
		}
		catch(Exception ex)
		{
		}

		if(_destroy)
		{
		    break;
		}

		try
		{
		    _session.ice_ping();
		}
		catch(Exception ex)
		{
		    break;
		}
	    }
	}

	public void
	destroy()
	{
	    synchronized(this)
	    {
		_destroy = true;
		notify();
	    }
	    try
	    {
		join();
	    }
	    catch(Exception ex)
	    {
		// Ignore.
	    }
	}

	boolean _destroy = false;
    }

    private class ExitThread extends Thread
    {
	public void
	run()
	{
	    ChatForm.this._pingThread.destroy();
	    try
	    {
		ChatForm.this._router.destroySession();
	    }
	    catch(Exception ex)
	    {
		// Ignore.
	    }
	    ChatForm.this._parent.destroy();
	}
    }

    private class SendThread extends Thread
    {
	SendThread(String message)
	{
	    _message = message;
	}

	public void
	run()
	{
	    try
	    {
		_session.say(_message);
	    }
	    catch(Exception ex)
	    {
	    }
	}

	String _message;
    }

    private ChatMIDlet _parent;
    private String _user;
    private Demo.ChatSessionPrx _session;
    private Glacier2.RouterPrx _router;
    private Console _console;
    private TextField _message;
    private PingThread _pingThread;
}

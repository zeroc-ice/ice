// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

public class LoginForm extends Form implements CommandListener, Runnable
{
    public
    LoginForm(ChatMIDlet parent, Ice.Communicator communicator)
    {
	super("Login");

	_parent = parent;
	_communicator = communicator;

	_user = new TextField("Name", "", 255, TextField.ANY);
	_password = new TextField("Password", "", 255, TextField.PASSWORD);
	_server = new TextField("Server", "", 255, TextField.ANY);
	_port = new TextField("Port", "", 255, TextField.NUMERIC);

	append(_user);
	append(_password);
	append(_server);
	append(_port);

	addCommand(new Command("OK", Command.OK, 0));
	addCommand(new Command("Exit", Command.EXIT, 1));
	setCommandListener(this);
    }

    public void
    commandAction(Command c, Displayable s)
    {
	if(c.getCommandType() == Command.EXIT)
	{
	    _parent.destroy();
	}
	else
	{
	    Thread t = new Thread(this);
	    t.start();
	}
    }

    public void
    run()
    {
	//
	// Ensure that the username and password match.
	//
	String user = _user.getString();
	String password = _password.getString();
	if(!user.equals(password))
	{
	    Alert alert = new Alert("Login Error", "Name and password do not match", null, AlertType.ERROR);
	    alert.setTimeout(Alert.FOREVER);
	    Display.getDisplay(_parent).setCurrent(alert, this);
	    return;
	}

	//
	// Validate server address.
	//
	String server = _server.getString().trim();
	if(server.length() == 0)
	{
	    Alert alert = new Alert("Login Error", "Server address is required", null, AlertType.ERROR);
	    alert.setTimeout(Alert.FOREVER);
	    Display.getDisplay(_parent).setCurrent(alert, this);
	    return;
	}

	//
	// Validate port number.
	//
	String port = _port.getString().trim();
	if(port.length() == 0)
	{
	    Alert alert = new Alert("Login Error", "Port number is required", null, AlertType.ERROR);
	    alert.setTimeout(Alert.FOREVER);
	    Display.getDisplay(_parent).setCurrent(alert, this);
	    return;
	}

	Demo.ChatSessionPrx session = null;
	try
	{
	    String routerStr = "Glacier2/router:tcp -h " + server + " -p " + port;
	    Glacier2.RouterPrx router =
		Glacier2.RouterPrxHelper.checkedCast(_communicator.stringToProxy(routerStr));
	    if(router != null)
	    {
		_communicator.setDefaultRouter(router);

		Ice.Properties properties = _communicator.getProperties();
		properties.setProperty("Chat.Client.Router", routerStr);
		properties.setProperty("Chat.Client.Endpoints", "");

		session = Demo.ChatSessionPrxHelper.uncheckedCast(router.createSession(user, password));

		String category = router.getServerProxy().ice_getIdentity().category;
		Ice.Identity callbackReceiverIdent = new Ice.Identity();
		callbackReceiverIdent.name = "callbackReceiver";
		callbackReceiverIdent.category = category;

		ChatForm cf = new ChatForm(_parent, user, session);

		Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("Chat.Client");
		Demo.ChatCallbackPrx callback = Demo.ChatCallbackPrxHelper.uncheckedCast(
		    adapter.add(new ChatCallbackI(cf.getConsole()), callbackReceiverIdent));
		adapter.activate();
		{
		    Ice.Object o = adapter.find(callbackReceiverIdent);
		    if(o == null)
		    {
			System.err.println("\n\nMissing servant for " + Ice.Util.identityToString(callbackReceiverIdent) + "!!\n\n");
		    }
		    else
		    {
			System.err.println("\n\nFound servant for " + Ice.Util.identityToString(callbackReceiverIdent) + "!!\n\n");
		    }
		}

		session.setCallback(callback);

		Display.getDisplay(_parent).setCurrent(cf);
	    }
	    else
	    {
		Alert alert = new Alert("Router Error", "Router is not a Glacier2 router", null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		Display.getDisplay(_parent).setCurrent(alert, this);
		return;
	    }
	}
	catch(Exception ex)
	{
	    Alert alert = new Alert("Ice Error", ex.getMessage(), null, AlertType.ERROR);
	    alert.setTimeout(Alert.FOREVER);
	    Display.getDisplay(_parent).setCurrent(alert, this);
	    return;
	}
    }

    private ChatMIDlet _parent;
    private Ice.Communicator _communicator;
    private TextField _user;
    private TextField _password;
    private TextField _server;
    private TextField _port;
}

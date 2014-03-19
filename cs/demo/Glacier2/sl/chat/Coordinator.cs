// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Glacier2;

namespace chat
{
    public enum ClientState { Disconnected, Connecting, Connected, ConnectionLost, Disconnecting }

    public class Coordinator : Glacier2.SessionCallback
    {
        public class LoginData
        {
            public LoginData()
            {
                host = "localhost";
                username = "test";
                password = "";
            }

            public String host;
            public String username;
            public String password;
        }

        private class ChatCallbackI : Demo.ChatCallbackDisp_
        {
            public override void
            message(string data, Ice.Current current)
            {
                Coordinator.getInstance().getChatPage().appendMessage(data + Environment.NewLine);
            }
        }

        public Coordinator()
        {
            _instance = this;
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.dispatcher = delegate(System.Action action, Ice.Connection connection)
            {
                try
                {
                    _loginPage.Dispatcher.BeginInvoke(action);
                }
                catch(Ice.CommunicatorDestroyedException ex)
                {
                    _mainPage.setState(ClientState.ConnectionLost);
                    _mainPage.setError(ex.ToString());
                }
            };
            _factory = new SessionFactoryHelper(initData, this);
        }

        public void doLogin(LoginData loginData)
        {
            _mainPage.setState(ClientState.Connecting);
            _loginData = loginData;
            //
            // IceSSL isn't supported with Silverlight.
            //
            _factory.setSecure(false);
            _factory.setRouterHost(_loginData.host);
            _factory.setPort(4502);
            _factory.setRouterIdentity(new Ice.Identity("router", "DemoGlacier2"));
            _session = _factory.connect(_loginData.username, _loginData.password);
        }

        public void
        destroySession()
        {
            if(_session != null)
            {
                _mainPage.setState(ClientState.Disconnecting);
                _session.destroy();
            }
        }

        public void
        sendMessage(string message)
        {
            _chat.begin_say(message).whenCompleted(delegate(Ice.Exception ex)
            {
                _chatPage.appendMessage("<system-message> - " + ex.ToString() + Environment.NewLine);
            });
        }

        public void connectFailed(SessionHelper session, Exception ex)
        {
            // If the session has been reassigned avoid the
            // spurious callback.
            if(session != _session)
            {
                return;
            }
            _mainPage.setState(ClientState.Disconnected);
            _mainPage.setError("Connect Failed:" + Environment.NewLine + ex.ToString());
        }

        public void connected(SessionHelper session)
        {
            // If the session has been reassigned avoid the
            // spurious callback.
            if(session != _session)
            {
                return;
            }

            Ice.Object servant = new ChatCallbackI();

            Demo.ChatCallbackPrx callback = Demo.ChatCallbackPrxHelper.uncheckedCast(_session.addWithUUID(servant));
            _chat = Demo.ChatSessionPrxHelper.uncheckedCast(_session.session());
            _chat.begin_setCallback(callback).whenCompleted(
                            delegate()
                                {
                                    _mainPage.setState(ClientState.Connected);
                                },
                            delegate(Ice.Exception ex)
                                {
                                    if(_session != null)
                                    {
                                        _session.destroy();
                                    }
                                });
        }

        public void createdCommunicator(SessionHelper session)
        {
        }

        public void disconnected(SessionHelper session)
        {
            // If the session has been reassigned avoid the
            // spurious callback.
            if(session != _session)
            {
                return;
            }
            _session = null;
            _mainPage.setState(ClientState.Disconnected);
        }

        public void setChatPage(ChatPage page)
        {
            _chatPage = page;
        }

        public ChatPage getChatPage()
        {
            return _chatPage;
        }

        public void setLoginPage(LoginPage page)
        {
            _loginPage = page;
        }

        public LoginPage getLoginPage()
        {
            return _loginPage;
        }

        public void setMainPage(MainPage page)
        {
            _mainPage = page;
        }

        public MainPage getMainPage()
        {
            return _mainPage;
        }

        public static Coordinator getInstance()
        {
            return _instance;
        }

        private SessionFactoryHelper _factory;
        private LoginData _loginData;
        private SessionHelper _session;
        private Demo.ChatSessionPrx _chat;
        private MainPage _mainPage;
        private LoginPage _loginPage;
        private ChatPage _chatPage;
        private static Coordinator _instance;
    }
}

// **********************************************************************
//
// Copyright(c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

using Demo;

namespace Glacier2.chat.client
{
    public class LoginData
    {
        public LoginData()
        {
            routerHost = "127.0.0.1";
            userName = "test";
            password = "";
        }

        public String routerHost;
        public String userName;
        public String password;
    }

    class Util
    {
        static public void
        locateOnScreen(System.Windows.Window window)
        {
            window.Left = (System.Windows.SystemParameters.PrimaryScreenWidth - window.Width) / 2;
            window.Top = (System.Windows.SystemParameters.PrimaryScreenHeight - window.Height) / 2;
        }

        static public void
        centerWindow(System.Windows.Window w1, System.Windows.Window w)
        {
            w1.Top = w.Top + ((w.Height - w1.Height) / 2);
            w1.Left = w.Left + ((w.Width - w1.Width) / 2);
        }
    }

    /// <summary>
    /// Interaction logic for ChatWindow.xaml
    /// </summary>
    public partial class ChatWindow : Window, Glacier2.SessionFactoryHelper.Callback
    {
        private class ChatCallbackI : Demo.ChatCallbackDisp_
        {
            public ChatCallbackI(ChatWindow window)
            {
                _window = window;
            }
            public override void
            message(string data, Ice.Current current)
            {
                _window.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                            {
                                _window.appendMessage(data);
                            }
                 );
            }
            ChatWindow _window;
        }

        class AMI_ChatSession_sayI : Demo.AMI_ChatSession_say
        {
            public AMI_ChatSession_sayI(ChatWindow window)
            {
                _window = window;
            }

            public override void
            ice_exception(Ice.Exception ex)
            {
                _window.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                {
                    _window.appendMessage("<system-message> - " + ex.ToString());
                });
            }

            public override void
            ice_response()
            {
            }

            private ChatWindow _window;
        }

        class AMI_ChatSession_setCallbackI : Demo.AMI_ChatSession_setCallback
        {
            public AMI_ChatSession_setCallbackI(ChatWindow window)
            {
                _window = window;
            }

            public override void 
            ice_response()
            {
                _window.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                {
                    _window.closeCancelDialog();
                    _window.input.IsEnabled = true;
                    _window.status.Content = "Connected with " + _window._loginData.routerHost;
                });
            }

            public override void
            ice_exception(Ice.Exception ex)
            {
                _window.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                {
                    if(_window._session != null)
                    {
                        _window._session.destroy();
                    }
                });
            }
            private ChatWindow _window;
        }

        public ChatWindow()
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config.client");

            _factory = new SessionFactoryHelper(properties, this);
            InitializeComponent();
            Util.locateOnScreen(this);
        }

        private void
        login(object sender, ExecutedRoutedEventArgs args)
        {
            LoginDialog loginDialog = new LoginDialog(_loginData);
            Util.centerWindow(loginDialog, this);
            if(loginDialog.ShowModal())
            {
                status.Content = "Connecting";
                _factory.setRouterHost(_loginData.routerHost);
                _factory.setRouterIdentity(new Ice.Identity("router", "DemoGlacier2"));
                _session = _factory.connect(_loginData.userName, _loginData.password);

                _cancelDialog = new CancelDialog();
                Util.centerWindow(_cancelDialog, this);
                if(_cancelDialog.ShowModal())
                {
                    if(_session != null)
                    {
                        _session.destroy();
                        _session = null;
                        status.Content = "Connection canceled";
                    }
                }
            }
        }

        private void
        logout(object sender, ExecutedRoutedEventArgs args)
        {
            status.Content = "Disconnecting";
            _session.destroy();
            _session = null;
        }

        private void
        exit(object sender, ExecutedRoutedEventArgs args)
        {
            Close();
        }

        private void
        windowClosed(object sender, EventArgs e)
        {
            lock(this)
            {
                if(_session != null)
                {
                    _session.destroy();
                    _session = null;
                }
            }
            App.Current.Shutdown(0);
        }

        private void
        isLogoutEnabled(object sender, CanExecuteRoutedEventArgs args)
        {
            args.CanExecute = _session != null && _session.isConnected();
        }

        private void
        isLoginEnabled(object sender,CanExecuteRoutedEventArgs args)
        {
            args.CanExecute = _session == null || !_session.isConnected();
        }

        //
        // Event handler attached to txtChatImputLine onKeyDown.
        // If the key is the Enter key, it sends the message asynchronously
        // and cleans the input line; otherwise, it does nothing.
        //
        private void
        sendMessage(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                string message = input.Text.Trim();
                if(message.Length > 0)
                {
                    _chat.say_async(new AMI_ChatSession_sayI(this), message);
                }
                input.Text = "";
            }
        }

        private void
        scrollDown(object sender, SizeChangedEventArgs e)
        {
            txtMessages.ScrollToEnd();
        }

        private void
        closeCancelDialog()
        {
            if(_cancelDialog != null)
            {
                _cancelDialog.Close();
                _cancelDialog = null;
            }
        }

        public void
        appendMessage(string message)
        {
            txtMessages.AppendText(message);
            txtMessages.ScrollToEnd();
        }

        private LoginData _loginData = new LoginData();
        private CancelDialog _cancelDialog = new CancelDialog();
        private Glacier2.SessionFactoryHelper _factory;
        private Glacier2.SessionHelper _session;
        private Demo.ChatSessionPrx _chat;

        #region Callback Members

        public void connectFailed(SessionHelper session, Exception ex)
        {
            // If the session has been reassigned avoid the
            // spurious callback.
            if(session != _session)
            {
                return;
            }

            closeCancelDialog();
            status.Content = ex.GetType();
        }

        public void connected(SessionHelper session)
        {
            // If the session has been reassigned avoid the
            // spurious callback.
            if(session != _session)
            {
                return;
            }

            Ice.Object servant = new ChatCallbackI(this);

            Demo.ChatCallbackPrx callback = Demo.ChatCallbackPrxHelper.uncheckedCast(
                _session.addWithUUID(servant));
            _chat = Demo.ChatSessionPrxHelper.uncheckedCast(_session.session());
            _chat.setCallback_async(new AMI_ChatSession_setCallbackI(this), callback);
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
            closeCancelDialog();
            _session = null;
            _chat = null;
            input.IsEnabled = false;
            status.Content = "Disconnected";
        }

        public Dispatcher getDispatcher()
        {
            return this.Dispatcher;// System.Windows.Application.Current.Dispatcher;
        }

        #endregion
    }
}

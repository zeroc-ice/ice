// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Navigation;

namespace chat
{
    public partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            Coordinator.getInstance().setMainPage(this);
            mainFrame.JournalOwnership = JournalOwnership.OwnsJournal;
        }

        public void setState(ClientState state)
        {
            _state = state;
            string page = mainFrame.Source.OriginalString;
            switch (state)
            {
                case ClientState.Connected:
                {
                    page = "/ChatPage.xaml";
                    break;
                }
                case ClientState.Disconnected:
                {
                    page = "/LoginPage.xaml";
                    break;
                }
                case ClientState.Disconnecting:
                {
                    page = "/DisconnectingPage.xaml";
                    break;
                }
                case ClientState.Connecting:
                {
                    LoginPage loginPage = Coordinator.getInstance().getLoginPage();
                    loginPage.txtError.Text = "";
                    page = "/ConnectingPage.xaml";
                    break;
                }
                case ClientState.ConnectionLost:
                {
                    ChatPage chatPage = Coordinator.getInstance().getChatPage();
                    chatPage.status.Text = "Not Connected";
                    break;
                }
                default:
                {
                    break;
                }
            }

            if(!mainFrame.Source.OriginalString.Equals(page))
            {
                mainFrame.Source = new Uri(page, UriKind.Relative);
            }
        }

        public void setError(string msg)
        {
            if (_state == ClientState.Disconnected)
            {
                LoginPage page = Coordinator.getInstance().getLoginPage();
                page.txtError.Text = msg;
            }
            else
            {
                Coordinator.getInstance().getChatPage().appendMessage(msg);
            }
        }

        public ClientState getState()
        {
            return _state;
        }

        private ClientState _state;
    }
}

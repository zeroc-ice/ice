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
    public partial class ChatPage : Page
    {
        public ChatPage()
        {
            InitializeComponent();
            Coordinator.getInstance().setChatPage(this);
            if (Coordinator.getInstance().getMainPage().getState() == ClientState.Connected)
            {
                status.Text = "Connected";
            }
            else
            {
                status.Text = "Not Connected";
            }
        }


        public void appendMessage(string message)
        {
            txtMessages.Text += message;
            txtMessages.SelectionLength = 0;
            txtMessages.SelectionStart = txtMessages.Text.Length;
        }

        private void
        sendMessage(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                string message = input.Text.Trim();
                if(message.Length > 0)
                {
                    Coordinator.getInstance().sendMessage(message);
                }
                input.Text = "";
            }
        }

        private void btnLogoutClick(object sender, RoutedEventArgs e)
        {
            Coordinator.getInstance().destroySession();
        }
    }
}

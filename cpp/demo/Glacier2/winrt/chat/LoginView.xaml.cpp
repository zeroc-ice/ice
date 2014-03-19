// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "pch.h"
#include "LoginView.xaml.h"
#include "MainPage.xaml.h"

using namespace chat;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

LoginView::LoginView()
{
    InitializeComponent();
    MainPage::instance()->_loginView = this;
}

void
LoginView::setError(String^ err)
{
    signin->IsEnabled = true;
    errorBorder->Visibility = Windows::UI::Xaml::Visibility::Visible;
    error->Text = err;
}

void LoginView::OnNavigatedTo(NavigationEventArgs^)
{
    LoginData loginData = MainPage::instance()->coordinator()->loginData();
    if(!loginData.hostname.empty())
    {
        hostname->Text = ref new String(IceUtil::stringToWstring(loginData.hostname).c_str());
    }
    if(!loginData.username.empty())
    {
        username->Text = ref new String(IceUtil::stringToWstring(loginData.username).c_str());
    }
    if(!loginData.password.empty())
    {
        password->Password = ref new String(IceUtil::stringToWstring(loginData.password).c_str());
    }
}

void chat::LoginView::signinClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    LoginData loginData;
    loginData.hostname = IceUtil::wstringToString(hostname->Text->Data());
    loginData.username = IceUtil::wstringToString(username->Text->Data());
    loginData.password = IceUtil::wstringToString(password->Password->Data());
    signin->IsEnabled = false;
    error->Text = "";
    errorBorder->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    MainPage::instance()->coordinator()->signIn(loginData);
}

// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "pch.h"
#include "ChatView.xaml.h"
#include "MainPage.xaml.h"
#include <string>

using namespace chat;
using namespace std;
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

ChatView::ChatView()
{
    InitializeComponent();
    MainPage::instance()->_chatView = this;
}

void
ChatView::setError(String^ err)
{
    appendMessage(L"<system-message> " + err);
}

void 
ChatView::appendMessage(String^ message)
{
    messages->Text += message + L"\n";
    messages->UpdateLayout();
    Scroller->ScrollToVerticalOffset(Scroller->ScrollableHeight);
}

void chat::ChatView::inputKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
    if(e->Key == Windows::System::VirtualKey::Enter && !input->Text->IsEmpty())
    {
        string msg = IceUtil::wstringToString(input->Text->Data());
        input->Text = "";
        MainPage::instance()->coordinator()->say(msg);
    }
}

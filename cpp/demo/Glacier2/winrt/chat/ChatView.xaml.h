// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include "ChatView.g.h"

namespace chat
{

public ref class ChatView sealed
{
public:
        
    ChatView();
    void setError(Platform::String^ err);
    void appendMessage(Platform::String^ message);
        
private:
        
    void inputKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
};

}

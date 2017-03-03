// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <ViewController.g.h>
#include <collection.h>
#include <map>
#include <string>
#include <memory>

namespace
{
class MainHelperI;
class ProcessControllerI;
class ControllerHelper;
}

namespace Controller
{

[Windows::Foundation::Metadata::WebHostHidden]
public ref class ViewController sealed
{
public:

    ViewController();
    virtual ~ViewController();


protected:

    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
private:

    friend class MainHelperI;
    friend class ProcessControllerI;
    friend class ControllerHelper;
    HINSTANCE loadDll(const std::string&);
    void println(const std::string&);
    void Hostname_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

    std::map<std::string, HINSTANCE> _dlls;
};

}

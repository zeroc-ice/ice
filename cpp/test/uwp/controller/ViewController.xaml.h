// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <mutex>

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

private:

    friend class MainHelperI;
    friend class ProcessControllerI;
    friend class ControllerHelper;
    HINSTANCE loadDll(const std::string&);
    void unloadDll(const std::string&);
    void println(const std::string&);
    std::string getHost() const;
    void Hostname_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

    std::map<std::string, std::pair<HINSTANCE, unsigned int>> _dlls;
    std::mutex _mutex;
};

}

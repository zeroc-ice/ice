//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <ControllerView.g.h>
#include <collection.h>
#include <map>
#include <string>
#include <memory>
#include <mutex>

namespace
{
class ControllerI;
class ProcessControllerI;
}

namespace Test
{

[Windows::Foundation::Metadata::WebHostHidden]
public ref class ControllerView sealed
{
public:

    ControllerView();
    virtual ~ControllerView();

private:

    friend class ControllerI;
    friend class ProcessControllerI;
    void println(const std::string&);
    std::string getHost() const;
    void Hostname_SelectionChanged(Platform::Object^, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^);
};

}

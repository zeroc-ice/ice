// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <MainPage.g.h>
#include <collection.h>
#include <vector>
#include <memory>

#include <Ice/Ice.h>

namespace TestSuite
{

struct TestConfiguration
{
    TestConfiguration(const std::string& desc = "",
                      const std::string& configName = "",
                      bool localOnly = false,
                      const std::vector<std::string>& options = std::vector<std::string>(),
                      const std::vector<std::string>& languages = std::vector<std::string>());

    const std::string desc;
    const std::string configName;
    const bool localOnly;
    const std::vector<std::string> options;
    const std::vector<std::string> languages;
};
typedef std::shared_ptr<TestConfiguration> TestConfigurationPtr;

struct TestCase
{
    TestCase(const std::string&, const std::string&, const std::string&, const std::string& = "",
             const std::string& = "", const std::string& = "");

    const std::string name;
    const std::string prefix;
    const std::string client;
    const std::string server;
    const std::string serverAMD;
    const std::string collocated;
    bool sslSupport;
    bool ipv6Support;
    bool wsSupport;
    std::vector<TestConfigurationPtr> configurations;
};
typedef std::shared_ptr<TestCase> TestCasePtr;

template<typename T> T^
findChild(Windows::UI::Xaml::DependencyObject^ parent, Platform::String^ name)
{
    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; ++i)
    {
        DependencyObject^ object = VisualTreeHelper::GetChild(parent, i);
        T^ child = dynamic_cast<T^>(object);
        FrameworkElement^ element = dynamic_cast<FrameworkElement^>(object);

        if(child && element && element->Name == name)
        {
            return child;
        }
        else if(object)
        {
            child = findChild<T>(object, name);
            if(child)
            {
                return child;
            }
        }
    }
    return nullptr;
}

class DllCache
{
public:

    ~DllCache();

    HINSTANCE loadDll(const std::string&);

private:

    std::map<std::string, HINSTANCE> _dlls;
};

[Windows::Foundation::Metadata::WebHostHidden]
public ref class MainPage sealed
{
public:

    MainPage();

    void completed();
    void failed(Platform::String^ reason);
    void printToConsoleOutput(Platform::String^ message, bool newline);

protected:

    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

private:

    void btnRun_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void btnStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void runSelectedTest();

    void Output_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Configuration_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Tests_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Language_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);


    std::string selectedLanguage();
    std::string selectedProtocol();
    void initializeSupportedProtocols();
    void initializeSupportedTests();

    Ice::CommunicatorPtr communicator();

    Platform::Collections::Vector<Platform::String^>^ _names;
    Platform::Collections::Vector<Platform::String^>^ _protocols;
    Platform::Collections::Vector<Platform::String^>^ _messages;

    std::vector<TestCasePtr> _allTests;
    DllCache _dlls;

    Windows::UI::Xaml::Controls::ListBox^ _tests;
    Windows::UI::Xaml::Controls::ComboBox^ _language;
    Windows::UI::Xaml::Controls::TextBox^ _host;
    Windows::UI::Xaml::Controls::ComboBox^ _protocol;
    Windows::UI::Xaml::Controls::CheckBox^ _loop;
    Windows::UI::Xaml::Controls::CheckBox^ _serialize;
    Windows::UI::Xaml::Controls::CheckBox^ _ipv6;
    Windows::UI::Xaml::Controls::Button^ _run;
    Windows::UI::Xaml::Controls::Button^ _stop;
    Windows::UI::Xaml::Controls::ListBox^ _output;
    Ice::CommunicatorPtr _communicator;
};

}

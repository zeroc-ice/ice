//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include "App.g.h"

namespace Test
{
/// <summary>
/// Provides application-specific behavior to supplement the default Application class.
/// </summary>
ref class App sealed
{
public:
    App();
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ pArgs) override;

private:
    void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
    void SessionRevoked(Platform::Object^,
            Windows::ApplicationModel::ExtendedExecution::Foreground::ExtendedExecutionForegroundRevokedEventArgs^);
    Windows::ApplicationModel::ExtendedExecution::Foreground::ExtendedExecutionForegroundSession^ _session;
};
}

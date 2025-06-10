/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "AzCore/Module/ModuleManagerBus.h"
#include "ConversationCanvasApplication.h"

int main(int argc, char **argv) {
  const AZ::Debug::Trace tracer;
  AzQtComponents::AzQtApplication::InitializeDpiScaling();

  ConversationCanvas::ConversationCanvasApplication app(&argc, &argv);
  if (app.LaunchLocalServer()) {
    AtomToolsFramework::AtomToolsApplication::StartupParameters params{};
    AtomToolsFramework::AtomToolsApplication::Descriptor descriptor{};

    descriptor.m_modules.emplace_back(
        AZ::DynamicModuleDescriptor{"GraphModel.Editor.Static"});
    descriptor.m_modules.emplace_back(
        AZ::DynamicModuleDescriptor{"GraphModel.Editor"});

    app.Start({}, {});
    app.RunMainLoop();
    app.Stop();
  }

  return 0;
}

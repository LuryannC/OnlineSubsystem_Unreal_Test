// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionRewardSystem.h"
#include "MissionRewardSubsystem.h"

#define LOCTEXT_NAMESPACE "FMissionRewardSystemModule"

void FMissionRewardSystemModule::StartupModule()
{
// #if WITH_EDITOR
// 	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
// 	{
// 		SettingsModule->RegisterSettings(
// 			"Project",        // container: Project Settings
// 			"Plugins",        // category
// 			"Mission Reward System",        // section
// 			LOCTEXT("PluginName",  "Mission Reward System"),
// 			LOCTEXT("PluginDesc",  "Configure Mission Reward System data."),
// 			GetMutableDefault<UMissionRewardSubsystem>());
// 	}
// #endif
}

void FMissionRewardSystemModule::ShutdownModule()
{
// #if WITH_EDITOR
// 	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
// 	{
// 		SettingsModule->UnregisterSettings("Project", "Plugins", "Mission Reward System");
// 	}
// #endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMissionRewardSystemModule, MissionRewardSystem)
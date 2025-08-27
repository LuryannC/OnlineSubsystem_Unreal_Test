// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionBase.h"
#include "MissionRewardSettings.h"
#include "MissionRewardSystemLog.h"

UMissionBase::UMissionBase()
{
	//
	//MissionData = FMissionStruct();
}

void UMissionBase::InitialiseMission()
{
	bIsCompleted = false;
	RuntimeConditions.Empty();

	// Register conditions to complete
	for (const FMissionCondition& Condition : MissionData.Conditions)
	{
		FRuntimeCondition Runtime;
		Runtime.EventTag = Condition.EventTag;
		Runtime.Target   = FMath::Max(1, Condition.TargetCount);
		Runtime.Current  = 0;
		RuntimeConditions.Add(Runtime);
	}

	ShowMissionDebugData();
}

FMissionStruct UMissionBase::BP_GetMissionData() const
{
	FMissionStruct MissionDataCopy = MissionData;
	
	UE_LOG(MissionRewardSystemLog, Log, TEXT("Break Test"));
	return MissionData;
}

void UMissionBase::SetMissionCompleted()
{
	bIsCompleted = true;
	MissionData.bIsCompleted = true;
}

void UMissionBase::OnGameplayEvent(const FGameplayTag& EventTag, const int32 Amount)
{
	if (bIsCompleted || Amount <= 0)
	{
		return;
	}

	bool bAnyProgressed = false;

	if (RuntimeConditions.Num() == 0)
	{
		UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionBase::OnGameplayEvent - No runtime conditions found. Skipping progress."));
		return;
	}

	for (FRuntimeCondition& RC : RuntimeConditions)
	{
		// Exact match is cheapest; Although MatchesTag allow for hierarchy support EventTag.MatchesTag(RC.EventTag)
		if (EventTag.MatchesTagExact(RC.EventTag))
		{
			const int32 Old = RC.Current;
			RC.Current = FMath::Clamp(RC.Current + Amount, 0, RC.Target);
			if (RC.Current != Old)
			{
				bAnyProgressed = true;
			}
		}
	}
	

	if (!bAnyProgressed)
	{
		return;
	}

	bool bAllComplete = true;

	for (const FRuntimeCondition& RC : RuntimeConditions)
	{
		if (RC.Target <= 0)
		{
			UE_LOG(MissionRewardSystemLog, Warning, TEXT("UMissionBase::OnGameplayEvent - Invalid RC.Target (%d) for tag %s"), RC.Target, *RC.EventTag.ToString());
			bAllComplete = false;
			continue;
		}
		
		if (RC.Current < RC.Target)
		{
			bAllComplete = false;
			UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionBase::OnGameplayEvent - Condition %s no met. Current: %i - Target %i"), *RC.EventTag.ToString(), RC.Current, RC.Target);
		}
	}

	OnProgressUpdated.Broadcast(this);

	UE_LOG(MissionRewardSystemLog, Log, TEXT("UMissionBase::OnGameplayEvent - Gameplay event fired for %s with %i amount"), *EventTag.ToString(), Amount);

	if (bAllComplete)
	{
		bIsCompleted = true;
		MissionData.bIsCompleted = true;
		OnMissionCompleted.Broadcast(this);
	}

	ShowMissionDebugData();
}

void UMissionBase::SetRuntimeConditionsFromSavedFile(const TArray<FRuntimeCondition>& InRuntimeConditions)
{
	RuntimeConditions = InRuntimeConditions;
	ShowMissionDebugData();
}

void UMissionBase::ShowMissionDebugData()
{
#if WITH_EDITOR
	if (const UMissionRewardSettings* Settings = GetDefault<UMissionRewardSettings>())
	{
		if (Settings->bShowDebugMessages)
		{
			UE_LOG(MissionRewardSystemLog, Log, TEXT("---------------- MISSION ----------------"));
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Mission: %s"), *MissionData.MissionID.ToString());
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Num of Conditions: %i"), RuntimeConditions.Num());
			for (const auto& Condition : RuntimeConditions)
			{
				UE_LOG(MissionRewardSystemLog, Log, TEXT("Condition: %s | Current: %i | Target: %i"), *Condition.EventTag.ToString(), Condition.Current, Condition.Target);
			}
			UE_LOG(MissionRewardSystemLog, Log, TEXT("Mission completed: %hs"), bIsCompleted ? ("True") : ("False"));
			UE_LOG(MissionRewardSystemLog, Log, TEXT("-----------------------------------------"));
		}
	}
#endif
}



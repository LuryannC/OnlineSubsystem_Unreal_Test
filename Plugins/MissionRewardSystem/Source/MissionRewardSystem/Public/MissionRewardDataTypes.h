// Made by Luryann A. Cervi. Please visit: https://luryanncervi.com.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "MissionRewardDataTypes.generated.h"

class URewardBase;

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FMissionCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Event"), Category="MissionRewardSystem")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MissionRewardSystem")
	int32 TargetCount = 1;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	int32 Current = 0;
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FMissionStruct
{
	GENERATED_BODY()

	FMissionStruct() = default;

	FMissionStruct(const FMissionStruct& Other)
	: InstanceID(Other.InstanceID)
	, MissionID(Other.MissionID)
	, DisplayName(Other.DisplayName)
	, Description(Other.Description)
	, Icon(Other.Icon)
	, Conditions(Other.Conditions)
	, Rewards(Other.Rewards)
	, bIsCompleted(Other.bIsCompleted)
	{}

	UPROPERTY()
	FGuid InstanceID;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	FName MissionID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	TArray<FMissionCondition> Conditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	TArray<TSoftClassPtr<URewardBase>> Rewards;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	bool bIsCompleted = false;

	bool operator==(const FMissionStruct& Other) const
	{
		return InstanceID == Other.InstanceID;
	}
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FProgressedMissions
{
	GENERATED_BODY()

	FProgressedMissions() = default;

	FProgressedMissions(const FMissionStruct& InMission)
	: InstanceID(InMission.InstanceID)
	, ConditionsProgress(InMission.Conditions)
	{}
	
	UPROPERTY()
	FGuid InstanceID;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	TArray<FMissionCondition> ConditionsProgress;
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FGrantedMission
{
	GENERATED_BODY()

	FGrantedMission() {}
	FGrantedMission(const TSoftClassPtr<class UMissionBase>& InClass, const FGuid InInstanceID)
	: MissionClass(InClass)
	, InstanceID(InInstanceID)
	{}
	
	UPROPERTY()
	TSoftClassPtr<class UMissionBase> MissionClass;
	
	UPROPERTY()
	FGuid InstanceID;
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FCompletedMission
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid InstanceID;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	FName MissionID;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	FMissionStruct MissionData;

	FCompletedMission() {}
	FCompletedMission(const FMissionStruct& InData)
		: InstanceID(InData.InstanceID), MissionID(InData.MissionID), MissionData(InData) {}
};


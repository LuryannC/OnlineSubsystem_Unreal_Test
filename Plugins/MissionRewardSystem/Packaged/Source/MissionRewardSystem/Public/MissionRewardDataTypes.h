#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "MissionRewardDataTypes.generated.h"

class URewardBase;

UENUM(BlueprintType)
enum class EUnlockReasonFailReason : uint8
{
	NotFound         UMETA(DisplayName="Not Found"),
	AlreadyCompleted UMETA(DisplayName="Already Completed"),
	Unknown          UMETA(DisplayName="Unknown"),
	Success          UMETA(DisplayName="Success"),
};

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
	: MissionID(Other.MissionID)
	, DisplayName(Other.DisplayName)
	, Description(Other.Description)
	, Icon(Other.Icon)
	, Conditions(Other.Conditions)
	, Rewards(Other.Rewards)
	, bIsCompleted(Other.bIsCompleted)
	{}
	
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
		return MissionID == Other.MissionID;
	}
};

USTRUCT(BlueprintType)
struct MISSIONREWARDSYSTEM_API FProgressedMissions
{
	GENERATED_BODY()

	FProgressedMissions() = default;

	FProgressedMissions(const FMissionStruct& InMission)
	: MissionID(InMission.MissionID)
	, ConditionsProgress(InMission.Conditions)
	{}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MissionRewardSystem")
	FName MissionID;

	UPROPERTY(BlueprintReadOnly, Category="MissionRewardSystem")
	TArray<FMissionCondition> ConditionsProgress;
};
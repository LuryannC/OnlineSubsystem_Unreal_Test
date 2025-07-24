// Fill out your copyright notice in the Description page of Project Settings.


#include "Rewards/MRS_Gold.h"

bool UMRS_Gold::GrantReward_Implementation(APlayerController* Recipient)
{
	if (!Recipient) return false;

	UE_LOG(MissionRewardSystemLog, Display, TEXT("UMRS_Gold::GrantReward_Implementation - %s rewarded with %i gold(s)"), *Recipient->GetName(), Amount);

	return true;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ClusterHUD.generated.h"

/**
 *
 */
UCLASS()
class DYNAMICCLUSTER_API AClusterHUD final : public AHUD
{
	GENERATED_BODY()
	AClusterHUD();

	void DrawHUD() override;
};

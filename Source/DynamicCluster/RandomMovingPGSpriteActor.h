// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperGroupedSpriteActor.h"
#include "Cluster.h"


#include "RandomMovingPGSpriteActor.generated.h"


/**
 * 
 */
UCLASS()
class DYNAMICCLUSTER_API ARandomMovingPGSpriteActor : public APaperGroupedSpriteActor
{
	GENERATED_BODY()

public:
	ARandomMovingPGSpriteActor();

	void Tick(float DeltaTime) override;

	TArray<TArray<FVector2D>> getClusterBoundry()
	{
		return ClusterBoundry;
	};

private:
	size_t numOfObjs;
	TArray<FVector> Velocities;
	TArray<FVector2D> position;

	TArray<TArray<FVector2D>> ClusterBoundry;

	TMap<unsigned, FLinearColor> colors;

	Cluster cluster;
	int numOfClusters;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperGroupedSpriteActor.h"
#include "Cluster.h"
#include "PaperSprite.h"
#include "Unit.h"

#include "RandomMovingPGSpriteActor.generated.h"

USTRUCT()
struct FConvexHull
{
	GENERATED_BODY()

	FConvexHull() :
		Size(0)
	{
		
	}
	TArray<FVector2D> Vertices;

	FVector2D operator[](const int32 I)
	{
		return Vertices[I];
	}

	void Add(const FVector2D Vertex)
	{
		Vertices.Add(Vertex);
		Size++;
	}
	int Num()
	{
		return Size;
	}
private:
	int Size;
};


// TODO
struct FTypeInfo
{

	UPaperSprite* Sprite;
	size_t NumOfObjects;
	TArray<Unit> Units;
	TArray<FConvexHull> ClusterBound;
	FCluster Cluster;

};

/**
 *
 */
UCLASS()
class DYNAMICCLUSTER_API ARandomMovingPgSpriteActor final : public APaperGroupedSpriteActor
{
	GENERATED_BODY()

public:
	ARandomMovingPgSpriteActor();

	void Tick(float DeltaTime) override;
	void BeginPlay() override;

	//For Hud to draw cluster
	TArray<FConvexHull> GetClusterBound() const
	{
		return ClusterBound;
	}

	UFUNCTION(BlueprintCallable)
		void Filter(TArray<bool> I_FilterRelation, TArray<bool> I_FilterUnitType);

private:

	//sprite resource
	UPROPERTY()
	UPaperSprite* Sprite;


	//objects properties
	size_t NumOfObjects;
	TArray<Unit> Units;

	void InitObjects();
	void AddObjects();
	void MoveObjects();
	void RenderObjects();


	//cluster
	TMap<unsigned, FLinearColor> Colors;
	TArray<FConvexHull> ClusterBound;
	FCluster Cluster;

	void InitColors();
	void InitClusters();
	void AddClusters();
	void UpdateClusters();
	void RenderClusters();
	void CalcClusterBound(TArray<FVector2D>& CurInstances);
	void GetCurCluster(std::vector<unsigned>& Labels, TSortedMap<unsigned, TArray<FVector2D>>& Clusters);
	bool PointInScreen(float X, float Y) const;


	// zoom utility
	int PreLevel;
	int GetZoomLevel() const;


	//filter
	//TArray<bool> FilterRelation;
	//TArray<bool> FilterUnitType;

	//array of index
	TArray<int> FilteredIndex;

	//set of index
	TArray<TSet<int>> UnitRelationSetsIndex;
	TArray<TSet<int>> UnitTypeSetsIndex;

	bool FilterStateChanged;
	void InitFilter();

	// TODO
	TMap<EUnitType, TMap<EUnitRelation, TArray<Unit>>> GetSetsFromFilteredUnits();

};

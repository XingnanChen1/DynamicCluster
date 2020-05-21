// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomMovingPGSpriteActor.h"

#include "ConstructorHelpers.h"
#include "PaperGroupedSpriteComponent.h"
#include "PaperSprite.h"
#include "EngineUtils.h"

#include "TopDownPlayerController.h"
#include "TopDownSpectatorPawn.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

ARandomMovingPgSpriteActor::ARandomMovingPgSpriteActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorEnableCollision(false);

	Sprite = ConstructorHelpers::FObjectFinder<UPaperSprite>(TEXT("PaperSprite'/Game/Sprites/testSprite.testSprite'")).
		Object;
	InitColors();
	//the following var should be construct after world exist
	//preLevel
	//
}


void ARandomMovingPgSpriteActor::InitColors()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				const float R = (255.f - 30 * i) / 255.f;
				const float G = (255.f - 30 * j) / 255.f;
				const float B = (255.f - 30 * k) / 255.f;

				Colors.Add(64 * i + 8 * j + k, {R, G, B});
			}
		}
	}
}

void ARandomMovingPgSpriteActor::InitObjects()
{
	NumOfObjects = 10000;
	const float Min_Pos = -240000;
	const float Max_Pos = 240000;
	const float Min_Speed = -30;
	const float Max_Speed = 30;

	const float Eps = 1e-3;

	//TSet<int> EmptySet;

	UnitTypeSetsIndex.Init({}, static_cast<int>(EUnitType::Count));
	UnitRelationSetsIndex.Init({}, static_cast<int>(EUnitRelation::Count));
	FilterStateChanged = false;
	
	Units.Init(Unit(), NumOfObjects);
	for (int i = 0; i < NumOfObjects; i++)
	{
		auto CurUnitRelation = static_cast<EUnitRelation>(FMath::FloorToInt(FMath::FRandRange(0, static_cast<float>(EUnitRelation::Count) - Eps)));
		auto CurUnitType = static_cast<EUnitType>(FMath::FloorToInt(FMath::FRandRange(0, static_cast<float>(EUnitType::Count) - Eps)));
		
		Units[i] = Unit(
			FVector2D(FMath::FRandRange(Min_Speed, Max_Speed), FMath::FRandRange(Min_Speed, Max_Speed)), 
			FVector2D(FMath::FRandRange(Min_Pos, Max_Pos), FMath::FRandRange(Min_Pos, Max_Pos)),
			CurUnitRelation,
			CurUnitType);

		
		
		UnitTypeSetsIndex[static_cast<int>(CurUnitType)].Add(i);
		UnitRelationSetsIndex[static_cast<int>(CurUnitRelation)].Add(i);

	}
}

void ARandomMovingPgSpriteActor::AddObjects()
{
	for (int i = 0; i < FilteredIndex.Num(); i++)
	{
		GetRenderComponent()->AddInstance(
			FTransform(FRotator(0, 0, -90),
			           FVector(Units[FilteredIndex[i]].Position, 0),
			           FVector(10)), Sprite);
	}
}

void ARandomMovingPgSpriteActor::MoveObjects()
{
	for (int i = 0; i < NumOfObjects; i++)
		Units[i].Position += Units[i].Velocity;
}

void ARandomMovingPgSpriteActor::RenderObjects()
{
	const auto CurLevel = GetZoomLevel();
	auto Render_Component = GetRenderComponent();

	if (CurLevel == PreLevel)
	{
		for (int i = 0; i < FilteredIndex.Num(); i++)
		{
			GetRenderComponent()->UpdateInstanceTransform(i,
			                                              FTransform(FRotator(0, 0, -90),
			                                                         FVector(Units[FilteredIndex[i]].Position, 0),
			                                                         FVector(10)),
			                                              false,
			                                              true);
		}
	}
	else
	{
		ClusterBound.Empty();
		Render_Component->ClearInstances();
		AddObjects();
	}
}


void ARandomMovingPgSpriteActor::InitFilter()
{
	for(int i =0; i<NumOfObjects;i++)
	{
		FilteredIndex.Add(i);
	}
}

void ARandomMovingPgSpriteActor::InitClusters()
{
	TArray<FVector2D> Positions;

	//for (const auto& Unit : Units)
	//{
	//	Positions.Add(Unit.Position);
	//}

	for (int i = 0; i < FilteredIndex.Num(); i++)
	{
		Positions.Add(Units[FilteredIndex[i]].Position);
	}


	
	for (int i = 0; i < Cluster.KofLevel.size(); i++)
	{
		if (FilteredIndex.Num() / 3 > Cluster.KofLevel[i])
			Cluster.Kmeans_Lloyd(Positions, i);
		else
			Cluster.Reset(i);
	}
}


void ARandomMovingPgSpriteActor::AddClusters()
{
	auto Labels = Cluster.GetLabels(GetZoomLevel());

	TSortedMap<unsigned, TArray<FVector2D>> Clusters;


	for (int i = 0; i < Labels.size(); i++)
	{
		if (Clusters.Find(Labels[i]))
		{
			Clusters[Labels[i]].Add(Units[FilteredIndex[i]].Position);
		}
		else
			Clusters.Add(Labels[i], {});
	}

	for (auto Label_Instances : Clusters)
	{
		const auto Label = Label_Instances.Get<0>();
		auto CurInstances = Label_Instances.Get<1>();


		auto Cur_Mean = Cluster.GetMeans(GetZoomLevel())[Label];


		if (PointInScreen(Cur_Mean[0], Cur_Mean[1]))
		{
			CalcClusterBound(CurInstances);
		}

		GetRenderComponent()->AddInstance(
			FTransform(
				FRotator(0, 0, -90),
				FVector(Cur_Mean[0], Cur_Mean[1], 0),
				FVector(10)),
			Sprite, false, Colors[Label * 512 / Cluster.KofLevel[GetZoomLevel()]]);
	}
}

void ARandomMovingPgSpriteActor::UpdateClusters()
{
	TArray<FVector2D> Positions;

	for (int i = 0; i < FilteredIndex.Num(); i++)
	{
		Positions.Add(Units[FilteredIndex[i]].Position);

	}
	
	//for(const auto& Unit: Units)
	//{
	//	Positions.Add(Unit.Position);
	//}

	Cluster.Kmeans_Lloyd_Online(Positions, GetZoomLevel());
}

void ARandomMovingPgSpriteActor::GetCurCluster(std::vector<unsigned>& Labels,
                                               TSortedMap<unsigned, TArray<FVector2D>>& Clusters)
{
	for (int i = 0; i < Labels.size(); i++)
	{
		if (Clusters.Find(Labels[i]))
		{
			Clusters[Labels[i]].Add(Units[FilteredIndex[i]].Position);
		}
		else
			Clusters.Add(Labels[i], {});
	}
}

bool ARandomMovingPgSpriteActor::PointInScreen(const float X, const float Y) const
{
	const APlayerController* PlayerController = *TActorIterator<ATopDownPlayerController>(GetWorld());
	if (PlayerController)
	{
		FVector2D ScreenLocation;
		PlayerController->ProjectWorldLocationToScreen(
			FVector(X, Y, 0)
			, ScreenLocation);

		int32 ScreenWidth = 0;
		int32 ScreenHeight = 0;
		PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

		const int32 ScreenX = static_cast<int32>(ScreenLocation.X);
		const int32 ScreenY = static_cast<int32>(ScreenLocation.Y);


		if (ScreenX >= 0 && ScreenY >= 0 && ScreenX < ScreenWidth && ScreenY < ScreenHeight)
		{
			return true;
		}
	}

	return false;
}

void ARandomMovingPgSpriteActor::RenderClusters()
{
	const auto CurLevel = GetZoomLevel();
	auto Render_Component = GetRenderComponent();
	auto Labels = Cluster.GetLabels(CurLevel);
	ClusterBound.Empty();

	TSortedMap<unsigned, TArray<FVector2D>> Clusters;
	GetCurCluster(Labels, Clusters);

	if (CurLevel != PreLevel)
		Render_Component->ClearInstances();


	for (const auto& Label_Instances : Clusters)
	{
		const auto Label = Label_Instances.Get<0>();
		auto CurInstances = Label_Instances.Get<1>();
		auto Cur_Mean = Cluster.GetMeans(CurLevel)[Label];


		if (PointInScreen(Cur_Mean[0], Cur_Mean[1]))
		{
			CalcClusterBound(CurInstances);
		}
		if (CurLevel != PreLevel)
		{
			Render_Component->AddInstance(
				FTransform(
					FRotator(0, 0, -90),
					FVector(Cur_Mean[0], Cur_Mean[1], 0),
					FVector(10)),
				Sprite, false, Colors[Label * 512 / Cluster.KofLevel[CurLevel]]);
		}
		else
		{
			Render_Component->UpdateInstanceTransform(Label,
			                                          FTransform(
				                                          FRotator(0, 0, -90),
				                                          FVector(Cur_Mean[0], Cur_Mean[1], 0),
				                                          FVector(10)),
			                                          false, true);
		}
	}
}

void ARandomMovingPgSpriteActor::CalcClusterBound(TArray<FVector2D>& CurInstances)
{
	const APlayerController* PlayerController = *TActorIterator<ATopDownPlayerController>(GetWorld());

	TArray<int32> Indices;
	ConvexHull2D::ComputeConvexHull2(CurInstances, Indices);
	FConvexHull Vertexes;
	for (auto Idx : Indices)
	{
		FVector2D Cur_Loc;
		PlayerController->ProjectWorldLocationToScreen(
			FVector(CurInstances[Idx], 0)
			, Cur_Loc);
		Vertexes.Add(Cur_Loc);
	}
	if(Vertexes.Num() > 0)
		ClusterBound.Add(Vertexes);
}

void ARandomMovingPgSpriteActor::BeginPlay()
{
	Super::BeginPlay();

	InitObjects();
	InitFilter();
	InitClusters();

	PreLevel = GetZoomLevel();

	if (GetZoomLevel() == 0)
		AddObjects();
	else
		AddClusters();
}

void ARandomMovingPgSpriteActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto CurLevel = GetZoomLevel();

	MoveObjects();

	if(FilterStateChanged)
	{
		UE_LOG(LogTemp, Warning, TEXT("filter changed"));

		InitClusters();

		GetRenderComponent()->ClearInstances();
		ClusterBound.Empty();
		if (CurLevel == 0 || FilteredIndex.Num() / 3 <= Cluster.KofLevel[CurLevel])
			AddObjects();
		else
			AddClusters();
		
		FilterStateChanged = false;
	}else
	{
		if (CurLevel == 0 || FilteredIndex.Num() / 3 <= Cluster.KofLevel[CurLevel])
			RenderObjects();
		else
		{
			UpdateClusters();
			RenderClusters();
		}
	}



	PreLevel = CurLevel;
}

int ARandomMovingPgSpriteActor::GetZoomLevel() const
{
	const int HeightPerZoomLevel = 70000;
	return static_cast<int>((*TActorIterator<ATopDownSpectatorPawn>{GetWorld()})->GetCameraHeight() / HeightPerZoomLevel);
}


void ARandomMovingPgSpriteActor::Filter(TArray<bool> I_FilterRelation, TArray<bool> I_FilterUnitType)
{

	TArray<EUnitRelation> FilterRelations;
	for (int i = 0; i < I_FilterRelation.Num(); i++)
	{
		if (I_FilterRelation[i])
			FilterRelations.Add(static_cast<EUnitRelation>(i));
	}

	TSet<int> FilteredByRelationIndex;
	for (int i = 0; i < FilterRelations.Num(); i++)
	{
		if (i == 0)
			FilteredByRelationIndex = UnitRelationSetsIndex[i];
		else
			FilteredByRelationIndex = FilteredByRelationIndex.Union(UnitRelationSetsIndex[i]);
	}


	TArray<EUnitType> FilterUnitType;

	for (int i = 0; i < I_FilterUnitType.Num(); i++)
	{
		if (I_FilterUnitType[i])
			FilterUnitType.Add(static_cast<EUnitType>(i));
	}

	TSet<int> FilteredByUnitTypeIndex;
	for (int i = 0; i < FilterUnitType.Num(); i++)
	{
		if (i == 0)
			FilteredByUnitTypeIndex = UnitTypeSetsIndex[i];
		else
			FilteredByUnitTypeIndex = FilteredByUnitTypeIndex.Union(UnitTypeSetsIndex[i]);
	}


	auto tmpSet = FilteredByUnitTypeIndex.Intersect(FilteredByRelationIndex);
	TArray<int> tmp;
	for (auto It = tmpSet.CreateConstIterator(); It; ++It)
	{
		tmp.Add(*It);
	}

	// !!!
	//thread safety issue, need lock
	//if actor is thread safe, then this shouldn't be a problem
	FilteredIndex = tmp;


	FilterStateChanged = true;
}

TMap<EUnitType,TMap<EUnitRelation,TArray<Unit>>> ARandomMovingPgSpriteActor::GetSetsFromFilteredUnits()
{
	TMap<EUnitType, TMap<EUnitRelation, TArray<Unit>>> Ret;

	for (int i = 0; i< FilteredIndex.Num(); i++)
	{
		auto CurUnit = Units[FilteredIndex[i]];
		Ret[CurUnit.UnitType][CurUnit.UnitRelation].Add(CurUnit);
	}

	return Ret;
}
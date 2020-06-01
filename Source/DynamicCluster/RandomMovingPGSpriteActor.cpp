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
	FilterStateChanged = false;

	for (int i = 0; i < static_cast<int>(EUnitType::Count); i++)
	{
		auto CurUnitType = static_cast<EUnitType>(i);
		TMap<EUnitRelation, FTypeInfo> cur;
		for (int j = 0; j < static_cast<int>(EUnitRelation::Count); j++)
		{
			auto CurUnitRelation = static_cast<EUnitRelation>(j);

			TArray<Unit> Units;
			
			Units.Init(Unit(), 2500);
			for (int k = 0; k < 2500; k++)
			{
				Units[k] = Unit(
					FVector2D(FMath::FRandRange(Min_Speed, Max_Speed), FMath::FRandRange(Min_Speed, Max_Speed)),
					FVector2D(FMath::FRandRange(Min_Pos, Max_Pos), FMath::FRandRange(Min_Pos, Max_Pos)),
					CurUnitRelation,
					CurUnitType);
			}
			
			cur.Add(TPair<EUnitRelation,FTypeInfo>{CurUnitRelation,FTypeInfo(Sprite,2500,Units,{},FCluster())});
		}
		Data.Add(CurUnitType, cur);


	}
}

void ARandomMovingPgSpriteActor::AddObjects()
{
	for (auto curType : FilterUnitType)
	{
		for (auto curRelation : FilterRelations)
		{
			auto curTypeInfo = Data[curType][curRelation];

			for (auto& unit : curTypeInfo.Units)
			{
				GetRenderComponent()->AddInstance(
					FTransform(FRotator(0, 0, -90),
						FVector(unit.Position, 0),
						FVector(10)), curTypeInfo.Sprite);
			}

		}
	}
}

void ARandomMovingPgSpriteActor::MoveObjects()
{

	for (auto& curType : Data)
	{
		for (auto& curRelation : curType.Get<1>())
		{
			auto& curTypeInfo = curRelation.Get<1>();

			for (auto& unit : curTypeInfo.Units)
			{
				unit.Position += unit.Velocity;
			}

		}
	}
}

void ARandomMovingPgSpriteActor::RenderObjects()
{
	const auto CurLevel = GetZoomLevel();
	auto Render_Component = GetRenderComponent();

	if (CurLevel == PreLevel)
	{
		int count = 0;
		for (auto curType : FilterUnitType)
		{
			for (auto curRelation : FilterRelations)
			{
				auto curTypeInfo = Data[curType][curRelation];
				for (auto& unit : curTypeInfo.Units)
				{
					GetRenderComponent()->UpdateInstanceTransform(count,
						FTransform(FRotator(0, 0, -90),
							FVector(unit.Position, 0),
							FVector(10)),
						false,
						true);
					count++;
				}
			}
		}
		
	}
	else
	{
		Render_Component->ClearInstances();

		ClearBound();
		
		AddObjects();
	}
}


void ARandomMovingPgSpriteActor::InitFilter()
{
	for(int i = 0; i< static_cast<int>(EUnitType::Count);i++)
	{
		FilterUnitType.Add(static_cast<EUnitType>(i));
	}
	for (int i = 0; i < static_cast<int>(EUnitRelation::Count); i++)
	{
		FilterRelations.Add(static_cast<EUnitRelation>(i));
	}

}

void ARandomMovingPgSpriteActor::InitClusters()
{
	for(auto& curType : FilterUnitType)
	{
		for (auto& curRelation : FilterRelations)
		{
			auto& curTypeInfo = Data[curType][curRelation];
			auto& curCluster = curTypeInfo.Cluster;

			TArray<FVector2D> Positions;

			for (int i = 0; i < curTypeInfo.NumOfObjects; i++)
			{
				Positions.Add(curTypeInfo.Units[i].Position);
			}
			
			for (int i = 0; i < curCluster.KofLevel.size(); i++)
			{
				if (curTypeInfo.NumOfObjects / 2 > curCluster.KofLevel[i])
					curCluster.Kmeans_Lloyd(Positions, i);
			}
		}
	}
	
}


void ARandomMovingPgSpriteActor::AddClusters()
{

	for (auto& curType : FilterUnitType)
	{
		for (auto& curRelation : FilterRelations)
		{
			auto& curTypeInfo = Data[curType][curRelation];
			auto Labels = curTypeInfo.Cluster.GetLabels(GetZoomLevel());

			TSortedMap<unsigned, TArray<FVector2D>> Clusters;

			for (int i = 0; i < Labels.size(); i++)
			{
				if (Clusters.Find(Labels[i]))
				{
					Clusters[Labels[i]].Add(curTypeInfo.Units[i].Position);
				}
				else
					Clusters.Add(Labels[i], {});
			}

			for (auto Label_Instances : Clusters)
			{
				const auto Label = Label_Instances.Get<0>();
				auto CurInstances = Label_Instances.Get<1>();


				auto Cur_Mean = curTypeInfo.Cluster.GetMeans(GetZoomLevel())[Label];


				if (PointInScreen(Cur_Mean[0], Cur_Mean[1]))
				{
					CalcClusterBound(CurInstances, curTypeInfo.ClusterBound);
				}

				GetRenderComponent()->AddInstance(
					FTransform(
						FRotator(0, 0, -90),
						FVector(Cur_Mean[0], Cur_Mean[1], 0),
						FVector(10)),
					curTypeInfo.Sprite, false, Colors[Label * 512 / curTypeInfo.Cluster.KofLevel[GetZoomLevel()]]); 
			}
		}
	}

}

void ARandomMovingPgSpriteActor::UpdateClusters()
{
	for (auto& curType : FilterUnitType)
	{
		for (auto& curRelation : FilterRelations)
		{
			auto& curTypeInfo = Data[curType][curRelation];
			auto& curCluster = curTypeInfo.Cluster;

			TArray<FVector2D> Positions;

			for (int i = 0; i < curTypeInfo.NumOfObjects; i++)
			{
				Positions.Add(curTypeInfo.Units[i].Position);
			}

			for (int i = 0; i < curCluster.KofLevel.size(); i++)
			{
				if (curTypeInfo.NumOfObjects / 2 > curCluster.KofLevel[i])
					curCluster.Kmeans_Lloyd_Online(Positions, i);
			}
		}
	}
}

void ARandomMovingPgSpriteActor::GetCurCluster(
	std::vector<unsigned>& Labels,
	TSortedMap<unsigned, TArray<FVector2D>>& Clusters,
	TArray<Unit>& Units)
{
	for (int i = 0; i < Labels.size(); i++)
	{
		if (Clusters.Find(Labels[i]))
		{
			Clusters[Labels[i]].Add(Units[i].Position);
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

	ClearBound();

	if (CurLevel != PreLevel)
		Render_Component->ClearInstances();

	int count = 0;
	for (auto& curType : FilterUnitType)
	{
		for (auto& curRelation : FilterRelations)
		{
			auto& curTypeInfo = Data[curType][curRelation];
			auto& curCluster = curTypeInfo.Cluster;
			auto Labels = curCluster.GetLabels(CurLevel);

			TSortedMap<unsigned, TArray<FVector2D>> Clusters;
			GetCurCluster(Labels, Clusters, curTypeInfo.Units);

			for (const auto& Label_Instances : Clusters)
			{
				const auto Label = Label_Instances.Get<0>();
				auto CurInstances = Label_Instances.Get<1>();
				auto Cur_Mean = curCluster.GetMeans(CurLevel)[Label];


				if (PointInScreen(Cur_Mean[0], Cur_Mean[1]))
				{
					CalcClusterBound(CurInstances,curTypeInfo.ClusterBound);
				}
				if (CurLevel != PreLevel)
				{
					Render_Component->AddInstance(
						FTransform(
							FRotator(0, 0, -90),
							FVector(Cur_Mean[0], Cur_Mean[1], 0),
							FVector(10)),
						Sprite, false, Colors[Label * 512 / curCluster.KofLevel[CurLevel]]);
				}
				else
				{
					Render_Component->UpdateInstanceTransform(count,
						FTransform(
							FRotator(0, 0, -90),
							FVector(Cur_Mean[0], Cur_Mean[1], 0),
							FVector(10)),
						false, true);
					count++;
				}
			}
		}
	}
	
	

}

void ARandomMovingPgSpriteActor::CalcClusterBound(TArray<FVector2D>& CurInstances ,TArray<FConvexHull>& bound)
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
		bound.Add(Vertexes);
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

void ARandomMovingPgSpriteActor::ClearBound()
{
	for (auto& curType : Data)
	{
		for (auto& curRelation : curType.Get<1>())
		{
			auto& curTypeInfo = curRelation.Get<1>();

			curTypeInfo.ClusterBound.Empty();

		}
	}
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
		ClearBound();
		
		if (CurLevel == 0 || FilterUnitType.Num() * FilterRelations.Num() *2500 / 3 <= FCluster::KofLevel[CurLevel])
			AddObjects();
		else
			AddClusters();
		
		FilterStateChanged = false;
	}else
	{
		if (CurLevel == 0 || FilterUnitType.Num() * FilterRelations.Num() * 2500 / 3 <= FCluster::KofLevel[CurLevel])
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

	TSet<EUnitRelation> TS_FilterRelations;
	for (int i = 0; i < I_FilterRelation.Num(); i++)
	{
		if (I_FilterRelation[i])
			TS_FilterRelations.Add(static_cast<EUnitRelation>(i));
	}

	TSet<EUnitType> TS_FilterUnitType;

	for (int i = 0; i < I_FilterUnitType.Num(); i++)
	{
		if (I_FilterUnitType[i])
			TS_FilterUnitType.Add(static_cast<EUnitType>(i));
	}

	// !!!
	//thread safety issue, need lock
	//if actor is thread safe, then this shouldn't be a problem
	FilterUnitType = TS_FilterUnitType;
	FilterRelations = TS_FilterRelations;


	FilterStateChanged = true;
}

//TMap<EUnitType,TMap<EUnitRelation,TArray<Unit>>> ARandomMovingPgSpriteActor::GetSetsFromFilteredUnits()
//{
//	TMap<EUnitType, TMap<EUnitRelation, TArray<Unit>>> Ret;
//
//	for (int i = 0; i< FilteredIndex.Num(); i++)
//	{
//		auto CurUnit = Units[FilteredIndex[i]];
//		Ret[CurUnit.UnitType][CurUnit.UnitRelation].Add(CurUnit);
//	}
//
//	return Ret;
//}
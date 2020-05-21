// Fill out your copyright notice in the Description page of Project Settings.


#include "ClusterHUD.h"

#include "RandomMovingPGSpriteActor.h"
#include "EngineUtils.h"


AClusterHUD::AClusterHUD()
{
	UE_LOG(LogTemp, Warning, TEXT("load HUD"));
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
}

void AClusterHUD::DrawHUD()
{
	ARandomMovingPgSpriteActor* GroupSprite = *TActorIterator<ARandomMovingPgSpriteActor>{GetWorld()};

	if (GroupSprite)
	{
		auto ClusterBoundary = GroupSprite->GetClusterBound();

		if (ClusterBoundary.Num() != 0)


			for (auto Boundary : ClusterBoundary)
			{
				auto Pre = Boundary[0];

				for (int i = 1; i < Boundary.Vertices.Num(); i++)
				{
					const auto Cur = Boundary[i];
					DrawLine(Pre.X, Pre.Y, Cur.X, Cur.Y, FLinearColor(1, 1, 1), 1);

					Pre = Cur;
				}
				DrawLine(Pre.X, Pre.Y, Boundary[0].X, Boundary[0].Y, FLinearColor(1, 1, 1), 1);
			}
	}
}

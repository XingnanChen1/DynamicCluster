// Fill out your copyright notice in the Description page of Project Settings.


#include "ClusterHUD.h"
#include "RandomMovingPGSpriteActor.h"


AClusterHUD::AClusterHUD()
{
	UE_LOG(LogTemp, Warning, TEXT("load HUD"));
	ARandomMovingPGSpriteActor* GroupSprite = Cast<ARandomMovingPGSpriteActor>(GetOwner());

	if(GroupSprite)
	{
		auto clusterBoundry = GroupSprite->getClusterBoundry();
		for (auto boundry : clusterBoundry)
		{
			auto pre = boundry[0];

			for (int i = 1; i < boundry.Num(); i++)
			{
				auto cur = boundry[i];
				DrawLine(pre.X, pre.Y, cur.X, cur.Y, FLinearColor(1, 1, 1), 5);
				pre = cur;
			}
			DrawLine(pre.X, pre.Y, boundry[0].X, boundry[0].Y, FLinearColor(1, 1, 1), 5);
		}

	}

	
}

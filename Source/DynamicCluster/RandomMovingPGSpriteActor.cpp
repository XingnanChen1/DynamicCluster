// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomMovingPGSpriteActor.h"

#include "ConstructorHelpers.h"
#include "PaperGroupedSpriteComponent.h"
#include "PaperSprite.h"

#include "TopDownPlayerController.h"



ARandomMovingPGSpriteActor::ARandomMovingPGSpriteActor() {

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8 ;j++){
			for(int  k = 0;k <8; k++){
				float R = (255.f - 30 * i)/ 255.f;
				float G = (255.f - 30 * j) / 255.f;
				float B = (255.f - 30 * k) / 255.f;

				colors.Add(64 * i + 8 * j + k, { R,G,B });
			}
		}
	}

	
	numOfClusters = 512;
	numOfObjs = 10000;
	float min_pos = -120000;
	float max_pos = 120000;
	float min_speed = -5;
	float max_speed = 5;


	Velocities.Init(FVector::ZeroVector, numOfObjs);
	position.Init(FVector2D::ZeroVector,numOfObjs);
	for (int i = 0; i < numOfObjs; i++) {
		Velocities[i] = FVector(FMath::FRandRange(min_speed, max_speed), FMath::FRandRange(min_speed, max_speed), 0.f);
		position[i] = FVector2D(FMath::FRandRange(min_pos, max_pos), FMath::FRandRange(min_pos, max_pos));	
	}
	
	auto sprite = ConstructorHelpers::FObjectFinder<UPaperSprite>(TEXT("PaperSprite'/Game/Sprites/testSprite.testSprite'")).Object;
	auto labels = cluster.kmeans_lloyd(position, numOfClusters);

	TSortedMap<unsigned, TArray<FVector2D>> clusters;

	for (int i = 0; i < labels.size(); i++)
	{
		if (clusters.Find(labels[i]))
		{
			clusters[labels[i]].Add(position[i]);
		}
		else
			clusters.Add(labels[i], {});

	}

	
	for(auto label_instances: clusters)
	{
		auto label = label_instances.Get<0>();
		auto curInstances = label_instances.Get<1>();
		
		float x = 0;
		float y = 0;

		
		for(auto instance:curInstances)
		{
			x += instance.X;
			y += instance.Y;
		}



		const APlayerController* PlayerController = Cast<const ATopDownPlayerController>(GetInstigatorController());

		FVector2D ScreenLocation;
		PlayerController->ProjectWorldLocationToScreen(
			FVector(x / curInstances.Num(), y / curInstances.Num(), 0)
			, ScreenLocation);

		int32 ScreenWidth = 0;
		int32 ScreenHeight = 0;
		PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

		int32 ScreenX = (int32)ScreenLocation.X;
		int32 ScreenY = (int32)ScreenLocation.Y;

		if(ScreenX >= 0 && ScreenY >= 0 && ScreenX < ScreenWidth&& ScreenY < ScreenHeight)
		{
			TArray<int32> indicies;
			ConvexHull2D::ComputeConvexHull2(curInstances, indicies);
			TArray<FVector2D> Vertexes;
			for (auto idx : indicies)
			{
				FVector2D ScreenLocation;
				PlayerController->ProjectWorldLocationToScreen(
					FVector(curInstances[idx], 0)
					, ScreenLocation);
				Vertexes.Add(ScreenLocation);
			}
			ClusterBoundry.Add(Vertexes);
		}
		auto sprite = ConstructorHelpers::FObjectFinder<UPaperSprite>(TEXT("PaperSprite'/Game/Sprites/testSprite.testSprite'")).Object;
		GetRenderComponent()->AddInstance(
			FTransform(
				FRotator(0, 0, -90),
				FVector(x/curInstances.Num(), y/curInstances.Num(), 0),
				FVector(10)),
			sprite, false, colors[label * 512 / numOfClusters]);
	}
	
	//for (int i = 0; i < labels.size(); i++)
	//{
	//	GetRenderComponent()->AddInstance(
	//		FTransform(
	//			FRotator(0, 0, -90),
	//			FVector(position[i], 0),
	//			FVector(10)),
	//		sprite,false, colors[labels[i] * 512/ numOfClusters]);
	//}
	
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
}

void ARandomMovingPGSpriteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto render_component = GetRenderComponent();

	//for (int i = 0; i < numOfObjs; i++) {
	//	FTransform instanceTransform;
	//	if(render_component->GetInstanceTransform(i, instanceTransform))
	//	{
	//		instanceTransform.AddToTranslation(Velocities[i]);
	//		render_component->UpdateInstanceTransform(i, instanceTransform,false,false);
	//		position[i][0] = instanceTransform.GetTranslation()[0];
	//		position[i][1] = instanceTransform.GetTranslation()[1];
	//	}
	//}
	//

	for (int i = 0; i < numOfObjs; i++) {
		position[i][0] += Velocities[i][0];
		position[i][1] += Velocities[i][1];
	}
	
	auto labels = cluster.kmeans_lloyd_online(position, numOfClusters);

	
	TSortedMap<unsigned, TArray<FVector2D>> clusters;

	for (int i = 0; i < labels.size(); i++)
	{
		if (clusters.Find(labels[i]))
		{
			clusters[labels[i]].Add(position[i]);
		}
		else
			clusters.Add(labels[i], {});
	}

	for (auto label_instances : clusters)
	{
		auto label = label_instances.Get<0>();
		auto curInstances = label_instances.Get<1>();

		float x = 0;
		float y = 0;


		for (auto instance : curInstances)
		{
			x += instance.X;
			y += instance.Y;
		}



		
		FTransform instanceTransform;

		if(render_component->GetInstanceTransform(label, instanceTransform))
		{
			auto nextTranslation = FMath::Lerp(instanceTransform.GetTranslation(), FVector(x / curInstances.Num(), y / curInstances.Num(),0), 0.01);
			instanceTransform.SetTranslation(nextTranslation);
			render_component->UpdateInstanceTransform(label, instanceTransform,false,false);
			render_component->UpdateInstanceColor(label, colors[label * 512 / numOfClusters]);

		}

	}
	
	//for (int i = 0; i < numOfObjs; i++) {
	//	FTransform instanceTransform;
	//	if(render_component->GetInstanceTransform(i, instanceTransform))
	//	{
	//		render_component->UpdateInstanceColor(i, colors[labels[i] * 512 / numOfClusters]);
	//	}

	//}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownSpectatorPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"


ATopDownSpectatorPawn::ATopDownSpectatorPawn(const FObjectInitializer& ObjectInitializer)
{
	SetActorLocation(FVector::ZeroVector);

	//Set Default Camera Values
	CameraZoomSpeed = 1000.0f;
	CameraScrollBoundary = 25.0f;
	//TODO: While selecting units, the camera CANNOT move!
	bCanMoveCamera = true;

	// intialize the camera
	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("RTS Camera"));
	CameraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CameraComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50000.0f));


}

void ATopDownSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* inputComponent)
{
	check(inputComponent);

	//Bind Mouse Wheel Zooming Actions
	inputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATopDownSpectatorPawn::Zoom<EZoomType::ZoomIn>);
	inputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATopDownSpectatorPawn::Zoom<EZoomType::ZoomOut>);

}



template<EZoomType T>
void ATopDownSpectatorPawn::Zoom()
{
	//Don't execute any further if the camera can't move
	if (!bCanMoveCamera)
		return;
	auto newLocation = FVector(0.0f, 0.0f, 0.0f);

	//Reposition the camera in the local space
	switch (T)
	{
		case EZoomType::ZoomIn: {
			newLocation.Z = CameraComponent->GetComponentLocation().Z - CameraZoomSpeed;
			break;
		}
		case EZoomType::ZoomOut: {
			newLocation.Z = CameraComponent->GetComponentLocation().Z + CameraZoomSpeed;
			break;
		}
	}

	CameraComponent->SetRelativeLocation(newLocation);
}
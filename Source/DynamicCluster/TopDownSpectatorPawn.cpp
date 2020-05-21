// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownSpectatorPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"


ATopDownSpectatorPawn::ATopDownSpectatorPawn(const FObjectInitializer& ObjectInitializer)
{
	SetActorLocation(FVector::ZeroVector);

	//Set Default Camera Values
	CameraZoomSpeed = 2000.0f;
	CameraScrollBoundary = 25.0f;
	//TODO: While selecting units, the camera CANNOT move!
	bCanMoveCamera = true;

	// initialize the camera
	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("RTS Camera"));
	CameraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CameraComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100000.0f));
}

void ATopDownSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* I_InputComponent)
{
	check(I_InputComponent);

	//Bind Mouse Wheel Zooming Actions
	I_InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATopDownSpectatorPawn::Zoom<EZoomType::ZoomIn>);
	I_InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATopDownSpectatorPawn::Zoom<EZoomType::ZoomOut>);
}


float ATopDownSpectatorPawn::GetCameraHeight() const
{
	return CameraComponent->GetComponentLocation().Z;
}


template <EZoomType T>
// ReSharper disable once CppMemberFunctionMayBeConst
void ATopDownSpectatorPawn::Zoom()
{
	//Don't execute any further if the camera can't move
	if (!bCanMoveCamera)
		return;
	const auto Cur_Height = GetCameraHeight();

	auto NewLocation = FVector(0.0f, 0.0f, 0.0f);

	//Reposition the camera in the local space
	switch (T)
	{
	case EZoomType::ZoomIn:
		{
			NewLocation.Z = Cur_Height - CameraZoomSpeed;
			break;
		}
	case EZoomType::ZoomOut:
		{
			NewLocation.Z = Cur_Height + CameraZoomSpeed;
			break;
		}
	}

	if (NewLocation.Z <= 2000 || NewLocation.Z >= 558000)
		return;


	CameraComponent->SetRelativeLocation(NewLocation);
}

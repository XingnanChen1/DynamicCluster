// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "TopDownSpectatorPawn.generated.h"


enum class EZoomType : uint8
{
	ZoomIn ,
	ZoomOut
};

/**
 * 
 */
UCLASS()
class DYNAMICCLUSTER_API ATopDownSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	ATopDownSpectatorPawn(const FObjectInitializer& ObjectInitializer);

	/** Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		class UCameraComponent* CameraComponent;

	/** Camera Zoom Speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraZoomSpeed;

	/** Camera Movement Speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraMovementSpeed;

	/** Camera Scroll Boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraScrollBoundary;

	/** Should the camera move? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bCanMoveCamera;

private:
	/** Sets up player inputs
	 *    @param InputComponent - Input Component
	 */
	void SetupPlayerInputComponent(class UInputComponent* inputComponent);


public:
	/** Zooms The Camera */
	template<EZoomType T>
	UFUNCTION()
		void Zoom();


};

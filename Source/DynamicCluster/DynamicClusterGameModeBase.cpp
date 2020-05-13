// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicClusterGameModeBase.h"

#include "ClusterHUD.h"
#include "TopDownPlayerController.h"
#include "TopDownSpectatorPawn.h"

ADynamicClusterGameModeBase :: ADynamicClusterGameModeBase() {
	PlayerControllerClass = ATopDownPlayerController::StaticClass();
	DefaultPawnClass = ATopDownSpectatorPawn::StaticClass();
	HUDClass = AClusterHUD::StaticClass();
}
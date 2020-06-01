// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitType.h"
/**
 * 
 */
class DYNAMICCLUSTER_API Unit
{
	

public:
	Unit() :
		Velocity(FVector2D::ZeroVector),
		Position(FVector2D::ZeroVector),
		UnitRelation(EUnitRelation::Enemy),
		UnitType(EUnitType::Ground)
	{
		//meaningless init
		
	}
	
	Unit(FVector2D I_Velocity, FVector2D I_Position, EUnitRelation I_UnitRelation, EUnitType I_UnitType) :
		Velocity(I_Velocity),
		Position(I_Position),
		UnitRelation(I_UnitRelation),
		UnitType(I_UnitType)
	{
		
	}
	
	~Unit();


	FVector2D Velocity;
	FVector2D Position;
	EUnitRelation UnitRelation;
	EUnitType UnitType;

};

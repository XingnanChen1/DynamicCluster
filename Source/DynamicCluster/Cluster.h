// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <array>
/**
 * 
 */
class DYNAMICCLUSTER_API Cluster
{

public:
	Cluster();
	std::vector<unsigned> kmeans_lloyd(TArray<FVector2D>& data, int k);

	std::vector<unsigned> kmeans_lloyd_online(TArray<FVector2D>& data, int k);

	~Cluster();

private:
	std::vector<std::array<float, 2>> means;
};

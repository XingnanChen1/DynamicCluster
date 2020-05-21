// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <array>

/**
 *
 */


class DYNAMICCLUSTER_API FCluster
{
public:
	FCluster();
	void Kmeans_Lloyd(TArray<FVector2D>& Data, int Level);

	void Kmeans_Lloyd_Online(TArray<FVector2D>& Data, int Level);


	std::vector<unsigned> GetLabels(const int Level)
	{
		return Labels[Level];
	}

	std::vector<std::array<float, 2>> GetMeans(const int Level)
	{
		return Means[Level];
	}


	~FCluster();
	const std::vector<int> KofLevel = {512, 512, 256, 128, 64, 32, 16, 8};

	void Reset(const int Level)
	{
		Means[Level].clear();
		Labels[Level].clear();
	}

private:
	std::vector<std::vector<unsigned>> Labels;
	std::vector<std::vector<std::array<float, 2>>> Means;
};

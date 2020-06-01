// Fill out your copyright notice in the Description page of Project Settings.


#include "Cluster.h"

#include "dkm/dkm.hpp"

const std::vector<int> FCluster::KofLevel = { 128, 128, 64, 64, 32, 32, 16, 8 };


FCluster::FCluster()
{
	Means = std::vector<std::vector<std::array<float, 2>>>(KofLevel.size());
	Labels = std::vector<std::vector<unsigned>>(KofLevel.size());
}

FCluster::~FCluster()
{
}

void FCluster::Kmeans_Lloyd(TArray<FVector2D>& Data, const int Level)
{
	std::vector<std::array<float, 2>> I_Data(Data.Num());

	for (int i = 0; i < Data.Num(); i++)
	{
		const std::array<float, 2> Tmp = {Data[i][0], Data[i][1]};
		I_Data[i] = Tmp;
	}
	auto Ret = Dkm::Kmeans_Lloyd(I_Data, Dkm::TClustering_Parameters<float>(KofLevel[Level]));
	Means[Level] = std::get<0>(Ret);
	Labels[Level] = std::get<1>(Ret);
}

void FCluster::Kmeans_Lloyd_Online(TArray<FVector2D>& Data, const int Level)
{
	std::vector<std::array<float, 2>> I_Data(Data.Num());

	for (int i = 0; i < Data.Num(); i++)
	{
		I_Data[i] = {Data[i][0], Data[i][1]};
	}
	Labels[Level] = Dkm::Kmeans_Lloyd_Online(I_Data, Dkm::TClustering_Parameters<float>(KofLevel[Level]), Means[Level]);
}

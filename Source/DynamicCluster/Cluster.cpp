// Fill out your copyright notice in the Description page of Project Settings.


#include "Cluster.h"

#include "dkm/dkm.hpp"

Cluster::Cluster()
{

}

Cluster::~Cluster()
{
}

std::vector<unsigned> Cluster::kmeans_lloyd(TArray<FVector2D>& data, int k)
{
	std::vector < std::array<float, 2>> i_data(data.Num());

	for(int i = 0; i<data.Num();i++)
	{
		std::array<float, 2> tmp = { data[i][0],data[i][1] };
		i_data[i] = tmp;
	}
	auto ret = dkm::kmeans_lloyd(i_data, dkm::clustering_parameters<float>(k));
	means = std::get<0>(ret);
	return  std::get<1>(ret);
}

std::vector<unsigned> Cluster::kmeans_lloyd_online(TArray<FVector2D>& data, int k)
{
	std::vector < std::array<float, 2>> i_data(data.Num());

	for (int i = 0; i < data.Num(); i++)
	{
		std::array<float, 2> tmp = { data[i][0],data[i][1] };
		i_data[i] = tmp;
	}
	return  dkm::kmeans_lloyd_online(i_data, dkm::clustering_parameters<float>(k), means);
}


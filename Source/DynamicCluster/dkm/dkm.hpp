#pragma once

// only included in case there's a C++11 compiler out there that doesn't support `#pragma once`
#ifndef DKM_KMEANS_H
#define DKM_KMEANS_H

#include "Runtime/Core/Public/Async/ParallelFor.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>

#include <cstdint>
#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

/*
DKM - A k-means implementation that is generic across variable data dimensions.
*/
namespace Dkm
{
	/*
	These functions are all private implementation details and shouldn't be referenced outside of this
	file.
	*/
	namespace Details
	{
		/*
		Calculate the square of the distance between two points.
		*/
		template <typename T, size_t N>
		T Distance_Squared(const std::array<T, N>& Point_A, const std::array<T, N>& Point_B)
		{
			T D_Squared = T();
			for (typename std::array<T, N>::size_type i = 0; i < N; ++i)
			{
				auto Delta = Point_A[i] - Point_B[i];
				D_Squared += Delta * Delta;
			}
			return D_Squared;
		}

		template <typename T, size_t N>
		auto Distance(const std::array<T, N>& Point_A, const std::array<T, N>& Point_B) -> T
		{
			return std::sqrt(Distance_Squared(Point_A, Point_B));
		}

		/*
		Calculate the smallest distance between each of the data points and any of the input means.
		*/
		template <typename T, size_t N>
		std::vector<T> Closest_Distance(
			const std::vector<std::array<T, N>>& Means, const std::vector<std::array<T, N>>& Data)
		{
			std::vector<T> Distances;
			Distances.reserve(Data.size());
			for (auto& d : Data)
			{
				T Closest = Distance_Squared(d, Means[0]);
				for (auto& m : Means)
				{
					T Distance = Distance_Squared(d, m);
					if (Distance < Closest)
						Closest = Distance;
				}
				Distances.push_back(Closest);
			}
			return Distances;
		}

		/*
		This is an alternate initialization method based on the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)
		initialization algorithm.
		*/
		template <typename T, size_t N>
		std::vector<std::array<T, N>> Random_Plusplus(const std::vector<std::array<T, N>>& Data, const uint32_t K,
		                                              const uint64_t Seed)
		{
			assert(k > 0);
			assert(data.size() > 0);
			using Input_Size_T = typename std::array<T, N>::size_type;
			std::vector<std::array<T, N>> Means;
			// Using a very simple PRBS generator, parameters selected according to
			// https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
			std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, UINT64_MAX>
				Rand_Engine(Seed);

			// Select first mean at random from the set
			{
				std::uniform_int_distribution<Input_Size_T> Uniform_Generator(0, Data.size() - 1);
				Means.push_back(Data[Uniform_Generator(Rand_Engine)]);
			}

			for (uint32_t Count = 1; Count < K; ++Count)
			{
				// Calculate the distance to the closest mean for each data point
				auto Distances = Details::Closest_Distance(Means, Data);
				// Pick a random point weighted by the distance from existing means
				// TODO: This might convert floating point weights to int, distorting the distribution for small weights
#if !defined(_MSC_VER) || _MSC_VER >= 1900
				std::discrete_distribution<Input_Size_T> Generator(Distances.begin(), Distances.end());
#else  // MSVC++ older than 14.0
				input_size_t i = 0;
				std::discrete_distribution<input_size_t> generator(distances.size(), 0.0, 0.0, [&distances, &i](double) { return distances[i++]; });
#endif
				Means.push_back(Data[Generator(Rand_Engine)]);
			}
			return Means;
		}

		/*
		Calculate the index of the mean a particular data point is closest to (euclidean distance)
		*/
		template <typename T, size_t N>
		uint32_t Closest_Mean(const std::array<T, N>& Point, const std::vector<std::array<T, N>>& Means)
		{
			assert(!means.empty());
			T Smallest_Distance = Distance_Squared(Point, Means[0]);
			typename std::array<T, N>::size_type Index = 0;
			for (size_t i = 1; i < Means.size(); ++i)
			{
				T Distance = Distance_Squared(Point, Means[i]);
				if (Distance < Smallest_Distance)
				{
					Smallest_Distance = Distance;
					Index = i;
				}
			}
			return Index;
		}

		/*
		Calculate the index of the mean each data point is closest to (euclidean distance).
		*/
		template <typename T, size_t N>
		std::vector<uint32_t> Calculate_Clusters(
			const std::vector<std::array<T, N>>& Data, const std::vector<std::array<T, N>>& Means)
		{
			std::vector<uint32_t> Clusters(Data.size());

			ParallelFor(Data.size(), [&](int32 Idx)
			{
				Clusters[Idx] = Closest_Mean(Data[Idx], Means);
			});

			//for (auto& point : data) {
			//	clusters.push_back(closest_mean(point, means));
			//}
			return Clusters;
		}

		/*
		Calculate means based on data points and their cluster assignments.
		*/
		template <typename T, size_t N>
		std::vector<std::array<T, N>> Calculate_Means(const std::vector<std::array<T, N>>& Data,
		                                              const std::vector<uint32_t>& Clusters,
		                                              const std::vector<std::array<T, N>>& Old_Means,
		                                              uint32_t K)
		{
			std::vector<std::array<T, N>> Means(K);
			std::vector<T> Count(K, T());
			for (size_t i = 0; i < std::min(Clusters.size(), Data.size()); ++i)
			{
				auto& Mean = Means[Clusters[i]];
				Count[Clusters[i]] += 1;
				for (size_t j = 0; j < std::min(Data[i].size(), Mean.size()); ++j)
				{
					Mean[j] += Data[i][j];
				}
			}
			for (size_t i = 0; i < K; ++i)
			{
				if (Count[i] == 0)
				{
					Means[i] = Old_Means[i];
				}
				else
				{
					for (size_t j = 0; j < Means[i].size(); ++j)
					{
						Means[i][j] /= Count[i];
					}
				}
			}
			return Means;
		}

		template <typename T, size_t N>
		std::vector<T> Deltas(
			const std::vector<std::array<T, N>>& Old_Means, const std::vector<std::array<T, N>>& Means)
		{
			std::vector<T> Distances;
			Distances.reserve(Means.size());
			assert(old_means.size() == means.size());
			for (size_t i = 0; i < Means.size(); ++i)
			{
				Distances.push_back(Distance(Means[i], Old_Means[i]));
			}
			return Distances;
		}

		template <typename T>
		bool Deltas_Below_Limit(const std::vector<T>& Deltas, T Min_Delta)
		{
			for (T d : Deltas)
			{
				if (d > Min_Delta)
				{
					return false;
				}
			}
			return true;
		}
	} // namespace details

	/*
	clustering_parameters is the configuration used for running the kmeans_lloyd algorithm.

	It requires a k value for initialization, and can subsequently be configured with your choice
	of optional parameters, including:
	* Maximum iteration count; the algorithm will terminate if it reaches this iteration count
	  before converging on a solution. The results returned are the means and cluster assignments
	  calculated in the last iteration before termination.
	* Minimum delta; the algorithm will terminate if the change in position of all means is
	  smaller than the specified distance.
	* Random seed; if present, this will be used in place of `std::random_device` for kmeans++
	  initialization. This can be used to ensure reproducible/deterministic behavior.
	*/
	template <typename T>
	class TClustering_Parameters
	{
	public:
		explicit TClustering_Parameters(const uint32_t K) :
			K(K),
			bHas_Max_Iter(false), Max_Iter(),
			bHas_Min_Delta(false), Min_Delta(),
			bHas_Rand_Seed(false), Rand_Seed()
		{
		}

		void Set_Max_Iteration(const uint64_t I_Max_Iter)
		{
			Max_Iter = I_Max_Iter;
			bHas_Max_Iter = true;
		}

		void Set_Min_Delta(T I_Min_Delta)
		{
			Min_Delta = I_Min_Delta;
			bHas_Min_Delta = true;
		}

		void Set_Random_Seed(const uint64_t I_Rand_Seed)
		{
			Rand_Seed = I_Rand_Seed;
			bHas_Rand_Seed = true;
		}

		bool Has_Max_Iteration() const { return bHas_Max_Iter; }
		bool Has_Min_Delta() const { return bHas_Min_Delta; }
		bool Has_Random_Seed() const { return bHas_Rand_Seed; }

		uint32_t Get_K() const { return K; }
		uint64_t Get_Max_Iteration() const { return Max_Iter; }
		T Get_Min_Delta() const { return Min_Delta; }
		uint64_t Get_Random_Seed() const { return Rand_Seed; }

	private:
		uint32_t K;
		bool bHas_Max_Iter;
		uint64_t Max_Iter;
		bool bHas_Min_Delta;
		T Min_Delta;
		bool bHas_Rand_Seed;
		uint64_t Rand_Seed;
	};

	/*
	Implementation of k-means generic across the data type and the dimension of each data item. Expects
	the data to be a vector of fixed-size arrays. Generic parameters are the type of the base data (T)
	and the dimensionality of each data point (N). All points must have the same dimensionality.

	e.g. points of the form (X, Y, Z) would be N = 3.

	Takes a `clustering_parameters` struct for algorithm configuration. See the comments for the
	`clustering_parameters` struct for more information about the configuration values and how they
	affect the algorithm.

	Returns a std::tuple containing:
	  0: A vector holding the means for each cluster from 0 to k-1.
	  1: A vector containing the cluster number (0 to k-1) for each corresponding element of the input
		 data vector.

	Implementation details:
	This implementation of k-means uses [Lloyd's Algorithm](https://en.wikipedia.org/wiki/Lloyd%27s_algorithm)
	with the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)
	used for initializing the means.

	*/
	template <typename T, size_t N>
	std::tuple<std::vector<std::array<T, N>>, std::vector<uint32_t>> Kmeans_Lloyd(
		const std::vector<std::array<T, N>>& Data, const TClustering_Parameters<T>& Parameters)
	{
		static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value,
			"kmeans_lloyd requires the template parameter T to be a signed arithmetic type (e.g. float, double, int)");
		assert(parameters.Get_K() > 0); // k must be greater than zero
		assert(data.size() >= parameters.Get_K()); // there must be at least k data points
		std::random_device Rand_Device;
		uint64_t Seed = Parameters.Has_Random_Seed() ? Parameters.Get_Random_Seed() : Rand_Device();
		std::vector<std::array<T, N>> Means = Details::Random_Plusplus(Data, Parameters.Get_K(), Seed);

		std::vector<std::array<T, N>> Old_Means;
		std::vector<std::array<T, N>> Old_Old_Means;
		std::vector<uint32_t> Clusters;
		// Calculate new means until convergence is reached or we hit the maximum iteration count
		uint64_t Count = 0;
		do
		{
			Clusters = Details::Calculate_Clusters(Data, Means);
			Old_Old_Means = Old_Means;
			Old_Means = Means;
			Means = Details::Calculate_Means(Data, Clusters, Old_Means, Parameters.Get_K());
			++Count;
		}
		while (Means != Old_Means && Means != Old_Old_Means
			&& !(Parameters.Has_Max_Iteration() && Count == Parameters.Get_Max_Iteration())
			&& !(Parameters.Has_Min_Delta() && Details::Deltas_Below_Limit(
				Details::Deltas(Old_Means, Means), Parameters.Get_Min_Delta())));

		return std::tuple<std::vector<std::array<T, N>>, std::vector<uint32_t>>(Means, Clusters);
	}


	/*
	Implementation of k-means generic across the data type and the dimension of each data item. Expects
	the data to be a vector of fixed-size arrays. Generic parameters are the type of the base data (T)
	and the dimensionality of each data point (N). All points must have the same dimensionality.

	e.g. points of the form (X, Y, Z) would be N = 3.

	Takes a `clustering_parameters` struct for algorithm configuration. See the comments for the
	`clustering_parameters` struct for more information about the configuration values and how they
	affect the algorithm.

	Returns a std::tuple containing:
	  0: A vector holding the means for each cluster from 0 to k-1.
	  1: A vector containing the cluster number (0 to k-1) for each corresponding element of the input
		 data vector.

	Implementation details:
	This implementation of k-means uses [Lloyd's Algorithm](https://en.wikipedia.org/wiki/Lloyd%27s_algorithm)
	with the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)
	used for initializing the means.

	*/
	template <typename T, size_t N>
	std::vector<uint32_t> Kmeans_Lloyd_Online(
		const std::vector<std::array<T, N>>& Data,
		const TClustering_Parameters<T>& Parameters,
		std::vector<std::array<T, N>>& Means)
	{
		static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value,
			"kmeans_lloyd_online requires the template parameter T to be a signed arithmetic type (e.g. float, double, int)"
		);
		assert(parameters.Get_K() > 0); // k must be greater than zero
		assert(data.size() >= parameters.Get_K()); // there must be at least k data points

		std::vector<std::array<T, N>> Old_Means;
		std::vector<std::array<T, N>> Old_Old_Means;
		std::vector<uint32_t> Clusters;
		// Calculate new means until convergence is reached or we hit the maximum iteration count
		uint64_t Count = 0;
		do
		{
			Clusters = Details::Calculate_Clusters(Data, Means);
			Old_Old_Means = Old_Means;
			Old_Means = Means;
			Means = Details::Calculate_Means(Data, Clusters, Old_Means, Parameters.Get_K());
			++Count;
		}
		while (Means != Old_Means && Means != Old_Old_Means
			&& !(Parameters.Has_Max_Iteration() && Count == Parameters.Get_Max_Iteration())
			&& !(Parameters.Has_Min_Delta() && Details::Deltas_Below_Limit(
				Details::Deltas(Old_Means, Means), Parameters.Get_Min_Delta())));

		return std::vector<uint32_t>(Clusters);
	}
} // namespace dkm

#endif /* DKM_KMEANS_H */

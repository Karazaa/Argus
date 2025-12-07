// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"

// Encapsulates the allocators used by a sparse array in a single type.
// I don't know why the InBitArrayAllocator is size 4 to start with. This is just replacing FDefaultBitArrayAllocator which for some
// unexplained reason is just an inline allocator of size 3.
template<uint32		NumPreAllocatedElements,
		typename	InElementAllocator = ArgusContainerAllocator<NumPreAllocatedElements>,
		typename	InBitArrayAllocator = ArgusContainerAllocator<4u>
		>
class ArgusSparseArrayAllocator
{
public:

	typedef InElementAllocator ElementAllocator;
	typedef InBitArrayAllocator BitArrayAllocator;
};

template<
	uint32		NumPreAllocatedElements,
	typename	InSparseArrayAllocator = ArgusSparseArrayAllocator<NumPreAllocatedElements>,
	typename	InHashAllocator = ArgusContainerAllocator<1u>,
	uint32		AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
	uint32		BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
	uint32		MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
class ArgusSetAllocator
{
public:
	/** Computes the number of hash buckets to use for a given number of elements. */
	static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
	{
		if (NumHashedElements >= MinNumberOfHashedElements)
		{
			return FPlatformMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
		}

		return 1;
	}

	typedef InSparseArrayAllocator SparseArrayAllocator;
	typedef InHashAllocator        HashAllocator;
};

template<
	uint32		NumPreAllocatedElements,
	typename	InSparseArrayAllocator,
	typename	InHashAllocator,
	uint32		AverageNumberOfElementsPerHashBucket,
	uint32		BaseNumberOfHashBuckets,
	uint32		MinNumberOfHashedElements
	>
struct TAllocatorTraits<ArgusSetAllocator<NumPreAllocatedElements, InSparseArrayAllocator, InHashAllocator, AverageNumberOfElementsPerHashBucket, BaseNumberOfHashBuckets, MinNumberOfHashedElements>> :
	TAllocatorTraitsBase<ArgusSetAllocator<NumPreAllocatedElements, InSparseArrayAllocator, InHashAllocator, AverageNumberOfElementsPerHashBucket, BaseNumberOfHashBuckets, MinNumberOfHashedElements>>
{
	enum
	{
		SupportsFreezeMemoryImage = TAllocatorTraits<InSparseArrayAllocator>::SupportsFreezeMemoryImage && TAllocatorTraits<InHashAllocator>::SupportsFreezeMemoryImage,
	};
};
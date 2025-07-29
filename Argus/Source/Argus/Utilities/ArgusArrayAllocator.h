// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/ContainerAllocationPolicies.h"

/*
 * The purpose of this class is to be the allocator used with TArrays throughout Argus. This allocator will behave similarly to TInlineAllocator in that it will have a predicted initial capacity reserved in
 * the ArgusArenaAllocator defined at compile time. If we exceed this capacity, we will allocate more contiguous memory in the ArgusArenaAllocator and importantly, LOG OUT THE DETAILS OF THE ALLOCATION.
 * We want to do this so we can begin to hone in on what kind of allocation needs we are going to have for dynamic arrays and minimize the quantity of resizes while also not pre-allocating unused chunks of memory.
 * 
 * Please refer to FContainerAllocatorInterface on line 272 of ContainerAllocationPolicies.h which serves as an example interface (not to be used) of functions to implement in order to have calls from inside templates function.
 */

template <uint32 NumPreAllocatedElements, int IndexSize>
class SizedArgusArrayAllocator
{
public:
	/* The integral type to be used for element counts and indices used by the allocator and container - must be signed */
	using SizeType = typename TBitsToSizeType<IndexSize>::Type;
private:
	using USizeType = std::make_unsigned_t<SizeType>;
public:

	/* Determines whether the user of the allocator may use the ForAnyElementType inner class. */
	enum { NeedsElementType = true };

	/* Determines whether the user of the allocator should do range checks */
	enum { RequireRangeCheck = true };

	class ForAnyElementType
	{
		template <uint32 NumPreAllocatedElements, int IndexSize>
		friend class SizedArgusArrayAllocator;

	public:
		/* Default constructor. */
		ForAnyElementType() : Data(nullptr) {}

		/* Destructor. */
		FORCEINLINE ~ForAnyElementType()
		{
			if (Data)
			{
				// TODO JAMES: Add slack tracking debug info.
				FMemory::Free(Data);
			}
		}

		/*
		 * Moves the state of another allocator into this one.  The allocator can be different.
		 *
		 * Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
		 * @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
		 */
		template <typename OtherAllocator>
		FORCEINLINE void MoveToEmptyFromOtherAllocator(typename OtherAllocator::ForAnyElementType& other)
		{
			// TODO JAMES: Error here. This should never get called since SupportsMoveFromOtherAllocator is false in the allocator traits for ArgusArrayAllocator. Because we are allocating from an arena,
			// we can't support swapping allocator types and the data they manage.
		}

		/*
		 * Moves the state of another allocator into this one.
		 *
		 * Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
		 * @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
		 */
		FORCEINLINE void MoveToEmpty(ForAnyElementType& other)
		{
			checkSlow((void*)this != (void*)&other);

			if (Data)
			{
				// TODO JAMES: Add slack tracking debug info. Free needs arena equivalent.
				FMemory::Free(Data);
			}

			Data = other.Data;
			other.Data = nullptr;
		}

		/* Accesses the container's current data. */
		FORCEINLINE FScriptContainerElement* GetAllocation() const
		{
			return Data;
		}

		/*
		 * Resizes the container's allocation.
		 * @param currentNum - The number of elements that are currently constructed at the front of the allocation.
		 * @param newMax - The number of elements to allocate space for.
		 * @param numBytesPerElement - The number of bytes/element.
		 */
		void ResizeAllocation(SizeType currentNum, SizeType newMax, SIZE_T numBytesPerElement)
		{
			// Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
			if (Data || newMax)
			{
				static_assert(sizeof(SizeType) <= sizeof(SIZE_T), "SIZE_T is expected to handle all possible sizes");

				// Check for under/overflow
				bool bInvalidResize = newMax < 0 || numBytesPerElement < 1 || numBytesPerElement >(SIZE_T)MAX_int32;
				if constexpr (sizeof(SizeType) == sizeof(SIZE_T))
				{
					bInvalidResize = bInvalidResize || (SIZE_T)(USizeType)newMax > (SIZE_T)TNumericLimits<SizeType>::Max() / numBytesPerElement;
				}
				if (UNLIKELY(bInvalidResize))
				{
					UE::Core::Private::OnInvalidSizedHeapAllocatorNum(IndexSize, newMax, numBytesPerElement);
				}

				// TODO JAMES: Slack tracking and Realloc needs arena equivalent.
				Data = (FScriptContainerElement*)FMemory::Realloc(Data, newMax * numBytesPerElement);
			}
		}

		/*
		 * Resizes the container's allocation.
		 * @param currentNum - The number of elements that are currently constructed at the front of the allocation.
		 * @param newMax - The number of elements to allocate space for.
		 * @param numBytesPerElement - The number of bytes/element.
		 * @param alignmentOfElement - The alignment of the element type.
		 *
		 * @note  This overload only exists if TAllocatorTraits<Allocator>::SupportsElementAlignment == true.
		 */
		void ResizeAllocation(SizeType currentNum, SizeType newMax, SIZE_T numBytesPerElement, uint32 alignmentOfElement)
		{
			// Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
			if (Data || newMax)
			{
				static_assert(sizeof(SizeType) <= sizeof(SIZE_T), "SIZE_T is expected to handle all possible sizes");

				// Check for under/overflow
				bool bInvalidResize = newMax < 0 || numBytesPerElement < 1 || numBytesPerElement >(SIZE_T)MAX_int32;
				if constexpr (sizeof(SizeType) == sizeof(SIZE_T))
				{
					bInvalidResize = bInvalidResize || ((SIZE_T)(USizeType)newMax > (SIZE_T)TNumericLimits<SizeType>::Max() / numBytesPerElement);
				}
				if (UNLIKELY(bInvalidResize))
				{
					UE::Core::Private::OnInvalidSizedHeapAllocatorNum(IndexSize, newMax, numBytesPerElement);
				}

				// TODO JAMES: Slack tracking and Realloc needs arena equivalent.
				Data = (FScriptContainerElement*)FMemory::Realloc(Data, newMax * numBytesPerElement, alignmentOfElement);
			}
		}

		FORCEINLINE SizeType CalculateSlackReserve(SizeType newMax, SIZE_T numBytesPerElement) const
		{
			return DefaultCalculateSlackReserve(newMax, numBytesPerElement, true);
		}

		FORCEINLINE SizeType CalculateSlackReserve(SizeType newMax, SIZE_T numBytesPerElement, uint32 alignmentOfElement) const
		{
			return DefaultCalculateSlackReserve(newMax, numBytesPerElement, true, (uint32)alignmentOfElement);
		}

		FORCEINLINE SizeType CalculateSlackShrink(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement) const
		{
			return DefaultCalculateSlackShrink(newMax, currentMax, numBytesPerElement, true);
		}

		FORCEINLINE SizeType CalculateSlackShrink(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement, uint32 alignmentOfElement) const
		{
			return DefaultCalculateSlackShrink(newMax, currentMax, numBytesPerElement, true, (uint32)alignmentOfElement);
		}

		FORCEINLINE SizeType CalculateSlackGrow(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement) const
		{
			return DefaultCalculateSlackGrow(newMax, currentMax, numBytesPerElement, true);
		}

		FORCEINLINE SizeType CalculateSlackGrow(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement, uint32 alignmentOfElement) const
		{
			return DefaultCalculateSlackGrow(newMax, currentMax, numBytesPerElement, true, (uint32)alignmentOfElement);
		}

		SIZE_T GetAllocatedSize(SizeType currentMax, SIZE_T numBytesPerElement) const
		{
			return currentMax * numBytesPerElement;
		}

		bool HasAllocation() const
		{
			return !!Data;
		}

		SizeType GetInitialCapacity() const
		{
			return NumPreAllocatedElements;
		}

	private:
		/** A pointer to the container's elements. */
		FScriptContainerElement* Data;
	};

	/*
	 * A class that receives both the explicit allocation policy template parameters specified by the user of the container,
	 * but also the implicit ElementType template parameter from the container type.
	 */
	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	public:
		/* Default constructor. */
		ForElementType()
		{
		}

		FORCEINLINE ElementType* GetAllocation() const
		{
			return (ElementType*)ForAnyElementType::GetAllocation();
		}
	};
};

template <uint32 NumInlineElements>
using ArgusArrayAllocator = SizedArgusArrayAllocator<NumInlineElements, 32>;

template <uint32 NumPreAllocatedElements, int IndexSize>
struct TAllocatorTraits< SizedArgusArrayAllocator<NumPreAllocatedElements, IndexSize> > : TAllocatorTraitsBase< SizedArgusArrayAllocator<NumPreAllocatedElements, IndexSize> >
{
	enum { IsZeroConstruct = true };
	enum { SupportsElementAlignment = true };
	enum { SupportsSlackTracking = false };
	enum { SupportsMoveFromOtherAllocator = false };
};
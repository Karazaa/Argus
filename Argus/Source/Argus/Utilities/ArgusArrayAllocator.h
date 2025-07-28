// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/ContainerAllocationPolicies.h"

/*
 * The purpose of this class is to be the allocator used with TArrays throughout Argus. This allocator will behave similarly to TInlineAllocator in that it will have a predicted initial capacity reserved in
 * the ArgusArenaAllocator defined at compile time. If we exceed this capacity, we will allocate more continguous memory in the ArgusArenaAllocator and importantly, LOG OUT THE DETAILS OF THE ALLOCATION.
 * We want to do this so we can begin to hone in on what kind of allocation needs we are going to have for dynamic arrays and minimize the quantity of resizes while also not pre-allocating unused chunks of memory.
 * 
 * Please refer to FContainerAllocatorInterface on line 272 of ContainerAllocationPolicies.h which serves as an example interface (not to be used) of functions to implement in order to have calls from inside templates function.
 */

template <uint32 NumPreAllocatedElements, int IndexSize>
class ArgusArrayAllocator
{
public:
	/* The integral type to be used for element counts and indices used by the allocator and container - must be signed */
	using SizeType = typename TBitsToSizeType<IndexSize>::Type;

	/* Determines whether the user of the allocator may use the ForAnyElementType inner class. */
	enum { NeedsElementType = true };

	/* Determines whether the user of the allocator should do range checks */
	enum { RequireRangeCheck = true };

	class ForAnyElementType
	{
		template <int, typename>
		friend class ArgusArrayAllocator;

	public:
		/* Default constructor. */
		ForAnyElementType() : Data(nullptr) {}

		/* Destructor. */
		FORCEINLINE ~ForAnyElementType()
		{
			if (Data)
			{
#if UE_ENABLE_ARRAY_SLACK_TRACKING
				FArraySlackTrackingHeader::Free(Data);
#else
				BaseMallocType::Free(Data);
#endif
			}
		}

		/*
		 * Moves the state of another allocator into this one.
		 *
		 * Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
		 * @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
		 */
		void MoveToEmpty(ForElementType& Other);

		/*
		 * Moves the state of another allocator into this one.  The allocator can be different, and the type must be specified.
		 * This function should only be called if TAllocatorTraits<AllocatorType>::SupportsMoveFromOtherAllocator is true.
		 *
		 * Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
		 * @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
		 */
		template <typename OtherAllocatorType>
		void MoveToEmptyFromOtherAllocator(typename OtherAllocatorType::template ForElementType<ElementType>& Other);

		/* Accesses the container's current data. */
		FORCEINLINE FScriptContainerElement* GetAllocation() const
		{
			return Data;
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
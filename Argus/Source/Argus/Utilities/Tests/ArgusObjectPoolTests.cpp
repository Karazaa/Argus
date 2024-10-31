// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMacros.h"
#include "ArgusObjectPool.h"
#include "Misc/AutomationTest.h"

struct TestPoolable : public IObjectPoolable
{
	float m_dummyValue = 0.0f;

	virtual void Reset() override
	{
		m_dummyValue = 0.0f;
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusObjectPoolTakeTest, "Argus.Utilities.ArgusObjectPool.TakeTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusObjectPoolTakeTest::RunTest(const FString& Parameters)
{
	ArgusObjectPool<TestPoolable> objectPool;
	int numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool initially.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that there are no available objects in it to begin with."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	TestPoolable* testPoolable = objectPool.Take();
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a take.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

#pragma region Test that the taken object from the pool is not null.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that the returned pointer is not null."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		testPoolable != nullptr
	);
#pragma endregion

	delete testPoolable;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusObjectPoolReleaseTest, "Argus.Utilities.ArgusObjectPool.ReleaseTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusObjectPoolReleaseTest::RunTest(const FString& Parameters)
{
	ArgusObjectPool<TestPoolable> objectPool;

	TestPoolable* testPoolable = nullptr;
	objectPool.Release(testPoolable);
	int numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a fake release.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s on null, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Release)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	testPoolable = objectPool.Take();
	testPoolable->m_dummyValue = 100.0f;
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a take.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	objectPool.Release(testPoolable);
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that the pointer pointing to the object is nulled out when calling release.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s and then %s, and testing that the remainding pointer is properly nulled out."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			ARGUS_NAMEOF(ArgusObjectPool::Release)
		),
		testPoolable == nullptr
	);
#pragma endregion

#pragma region Test that there is one available object in the object pool after a take and release.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s and then %s, and testing that there is one available object in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			ARGUS_NAMEOF(ArgusObjectPool::Release)
		),
		numAvailableObjects,
		1
	);
#pragma endregion

	testPoolable = objectPool.Take();
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that an object is properly Reset after being taken from the object pool.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s and then %s, then %s again, and validating that the returned object has been reset."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			ARGUS_NAMEOF(ArgusObjectPool::Release),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		testPoolable->m_dummyValue,
		0.0f
	);
#pragma endregion

#pragma region Test that there are no available objects in the object pool after a take and release and then take again.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s and then %s, then %s again, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			ARGUS_NAMEOF(ArgusObjectPool::Release),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	delete testPoolable;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusObjectPoolClearPoolTest, "Argus.Utilities.ArgusObjectPool.ClearPool", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusObjectPoolClearPoolTest::RunTest(const FString& Parameters)
{
	ArgusObjectPool<TestPoolable> objectPool;
	constexpr int numObjectToTest = 100;

	TestPoolable* testPoolables[numObjectToTest];
	for (int i = 0; i < numObjectToTest; ++i)
	{
		testPoolables[i] = objectPool.Take();
	}
	for (int i = 0; i < numObjectToTest; ++i)
	{
		objectPool.Release(testPoolables[i]);
	}

	int numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are 100 available objects after 100 takes followed by 100 releases.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s %d times in a row, and then calling %s %d times in a row and testing that there are %d available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			numObjectToTest,
			ARGUS_NAMEOF(ArgusObjectPool::Release),
			numObjectToTest,
			numObjectToTest
		),
		numAvailableObjects,
		numObjectToTest
	);
#pragma endregion

	objectPool.ClearPool();
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects after clearing the object pool.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s %d times in a row, and then calling %s %d times in a row, then calling %s and testing that there are 0 available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			numObjectToTest,
			ARGUS_NAMEOF(ArgusObjectPool::Release),
			numObjectToTest,
			ARGUS_NAMEOF(ArgusObjectPool::ClearPool)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	return true;
}

#if WITH_AUTOMATION_TESTS
#endif //WITH_AUTOMATION_TESTS
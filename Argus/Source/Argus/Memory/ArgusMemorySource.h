// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

/*
 *
 * Arena to be used for all of the dynamic allocations in the gameplay logic of Argus. I can't control what the engine does with its allocations¯\_(ツ)_/¯
 *
 */

class ArgusMemorySource
{
public:
	static void Initialize(SIZE_T memorySourceSize = k_10MB, uint32 alignment = DEFAULT_ALIGNMENT);
	static void TearDown();

private:
	static constexpr SIZE_T k_10MB = 1048576;
	static void* s_RawDataRoot;
};
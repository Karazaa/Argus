// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "IconWidget.generated.h"

UCLASS()
class UIconWidget : public UArgusUIElement
{
	GENERATED_BODY()

public:
	// Populate function

protected:
	TFunction<void(int32)> m_onClickedListener = nullptr;

private:
	int32 m_index = 0;
};
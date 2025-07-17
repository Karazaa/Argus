// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "IconWidget.generated.h"

class UImage;

UCLASS()
class UIconWidget : public UArgusUIElement
{
	GENERATED_BODY()

public:
	void Populate(const TFunction<void(uint16)>& onClickedCallback, uint16 identifier);
	void SetBrush(const FSlateBrush& brush);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& nMouseEvent) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UImage> m_icon = nullptr;

	TFunction<void(uint16)> m_onClickedCallback = nullptr;

private:
	uint16 m_identifier = 0u;
};
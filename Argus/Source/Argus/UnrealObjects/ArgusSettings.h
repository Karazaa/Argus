// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEditorDataLookups.h"
#include "Engine/DeveloperSettings.h"
#include "ArgusSettings.generated.h"

UCLASS(config = Game, DefaultConfig)
class UArgusSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(Config, EditAnywhere)
	TSoftObjectPtr<UArgusEditorDataLookups> m_argusEditorDataLookups = nullptr;
#endif // WITH_EDITORONLY_DATA
};
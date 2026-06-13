/// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/TeamColorRecord.h"
#include "Developer/AssetTools/Public/AssetTypeActions_Base.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "TeamColorRecordFactory.generated.h"

#define LOCTEXT_NAMESPACE "TeamColorRecordFactory"

UCLASS()
class UTeamColorRecordFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTeamColorRecordFactory(const FObjectInitializer& objectInitializer);
	virtual UObject* FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning) override;

protected:
	virtual bool IsMacroFactory() const { return false; }
};

class FAssetTypeActions_TeamColorRecord : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("TeamColorRecord", "TeamColorRecord"); }
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override { return FColor(79, 239, 174); }

	virtual FText GetAssetDescription(const FAssetData& assetData) const override { return LOCTEXT("TeamColorRecordDesc", "An TeamColorRecord for the TeamColorRecord Static Database."); }
	virtual UClass* GetSupportedClass() const override { return UTeamColorRecord::StaticClass(); }
	virtual const TArray<FText>& GetSubMenus() const override { return m_subMenus; };

private:
	TArray<FText> m_subMenus{ LOCTEXT("StaticDataSubmenu", "Static Records") };
};


#undef LOCTEXT_NAMESPACE

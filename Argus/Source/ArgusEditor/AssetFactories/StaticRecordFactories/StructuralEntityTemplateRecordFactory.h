/// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/StructuralEntityTemplateRecord.h"
#include "Developer/AssetTools/Public/AssetTypeActions_Base.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "StructuralEntityTemplateRecordFactory.generated.h"

#define LOCTEXT_NAMESPACE "StructuralEntityTemplateRecordFactory"

UCLASS()
class UStructuralEntityTemplateRecordFactory : public UFactory
{
	GENERATED_BODY()

public:
	UStructuralEntityTemplateRecordFactory(const FObjectInitializer& objectInitializer);
	virtual UObject* FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning) override;

protected:
	virtual bool IsMacroFactory() const { return false; }
};

class FAssetTypeActions_StructuralEntityTemplateRecord : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("StructuralEntityTemplateRecord", "StructuralEntityTemplateRecord"); }
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override { return FColor(79, 239, 174); }

	virtual FText GetAssetDescription(const FAssetData& assetData) const override { return LOCTEXT("StructuralEntityTemplateRecordDesc", "An StructuralEntityTemplateRecord for the StructuralEntityTemplateRecord Static Database."); }
	virtual UClass* GetSupportedClass() const override { return UStructuralEntityTemplateRecord::StaticClass(); }
	virtual const TArray<FText>& GetSubMenus() const override { return m_subMenus; };

private:
	TArray<FText> m_subMenus{ LOCTEXT("StaticDataSubmenu", "Static Records") };
};


#undef LOCTEXT_NAMESPACE

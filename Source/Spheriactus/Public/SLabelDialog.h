// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/SListView.h"


class SStartWidgetSwitcher; // forward declaration

 class SPHERIACTUS_API SLabelDialog : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLabelDialog)
        {}
        SLATE_ARGUMENT(TWeakPtr<SStartWidgetSwitcher>, StartWidgetSwitcher)
    SLATE_END_ARGS()

       void Construct(const FArguments& InArgs);

       TMap<FString, FLinearColor>* GetLabelEntries();

private:
    TMap<FString, FLinearColor> LabelEntries;
    TWeakPtr<SStartWidgetSwitcher> StartWidgetSwitcher;

    void AddEntry();
    void RemoveEntry(const FString& Key);
    void OnNextButtonClicked();
    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> InKey, const TSharedRef<STableViewBase>& OwnerTable);

    TArray<TSharedPtr<FString>> EntryKeys;
    TSharedPtr<SListView<TSharedPtr<FString>>> ListView;

    TSharedPtr<FSlateColorBrush>                GridLineBrush;
    TSharedPtr<FSlateRoundedBoxBrush>           SolidPickerBrush;
};
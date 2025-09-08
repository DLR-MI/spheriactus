// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SFileDialog.h"
#include "SLabelDialog.h"

class SPHERIACTUS_API SStartWidgetSwitcher : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStartWidgetSwitcher)
        {}
        SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

    UFUNCTION(BlueprintCallable, Category = "Custom")
    FString GetSelectedFilePath() const;

    TMap<FString, FLinearColor>* RetrieveLabelEntries() const;

    void CloseWindow();
    void CloseWindowAndQuitGame(const TSharedRef<SWindow>& Window);
    void SwitchToWidget(int32 WidgetIndex);

private:

    void OnFileSelected(const FString& FilePath);
    void OnFileDialogCancelled();
    void SwitchToLabelDialog();

    TSharedPtr<SWidgetSwitcher> WidgetSwitcher;
    TSharedPtr<SLabelDialog> LabelDialogWidget;
    TSharedPtr<SWindow> ParentWindow;

    TMap<FString, FLinearColor> LabelEntries;
    FString SelectedFilePath;

    enum WidgetIndex
    {
        LabelDialogIndex
    };

    bool QuitGame = true;
};
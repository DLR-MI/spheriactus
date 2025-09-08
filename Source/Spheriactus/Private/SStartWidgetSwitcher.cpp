// Copyright (c) Bruno Pereira Costa


#include "SStartWidgetSwitcher.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SWidgetSwitcher.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SStartWidgetSwitcher::Construct(const FArguments& InArgs)
{
    ParentWindow = InArgs._ParentWindow;

    ChildSlot
        [
            SAssignNew(WidgetSwitcher, SWidgetSwitcher)
                + SWidgetSwitcher::Slot()
                [
                    SAssignNew(LabelDialogWidget, SLabelDialog)
                        .StartWidgetSwitcher(SharedThis(this))
                ]
        ];

    // Set the window closed callback to be executed right before the window is closed
    ParentWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &SStartWidgetSwitcher::CloseWindowAndQuitGame));
}

void SStartWidgetSwitcher::CloseWindow()
{
    QuitGame = false; // when the window is closed the callback is triggered, therefore this should be false to avoid quitting game
    //UE_LOG(LogTemp, Warning, TEXT("ATTEMPTING TO CLOSE"));
    if (ParentWindow.IsValid())
    {
        ParentWindow->RequestDestroyWindow();
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("ATTEMPTING TO CLOSE NOT SUCCESSFUL"));
    QuitGame = true; // back to true for cases where "x" is pressed
}

void SStartWidgetSwitcher::CloseWindowAndQuitGame(const TSharedRef<SWindow>& Window)
{
    if (GEngine && GEngine->GameViewport && QuitGame)
    {
        UKismetSystemLibrary::QuitGame(GEngine->GameViewport->GetWorld(), GEngine->GameViewport->GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
    }
}

void SStartWidgetSwitcher::SwitchToWidget(int32 WidgetIndex)
{
    if (WidgetSwitcher.IsValid())
    {
        WidgetSwitcher->SetActiveWidgetIndex(WidgetIndex);
    }
}

void SStartWidgetSwitcher::OnFileSelected(const FString& FilePath)
{
    SelectedFilePath = FilePath;
    UE_LOG(LogTemp, Warning, TEXT("Selected file: %s"), *SelectedFilePath);
    SwitchToLabelDialog();
}

void SStartWidgetSwitcher::OnFileDialogCancelled()
{
     FGenericPlatformMisc::RequestExit(false);
}

void SStartWidgetSwitcher::SwitchToLabelDialog()
{
    WidgetSwitcher->SetActiveWidget(LabelDialogWidget.ToSharedRef());
}

FString SStartWidgetSwitcher::GetSelectedFilePath() const
{
    return SelectedFilePath;
}

TMap<FString, FLinearColor>* SStartWidgetSwitcher::RetrieveLabelEntries() const
{
    if (LabelDialogWidget.IsValid())
    {
        return LabelDialogWidget->GetLabelEntries();
    }
    return nullptr;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
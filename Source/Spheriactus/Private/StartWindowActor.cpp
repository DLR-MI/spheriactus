// Copyright (c) Bruno Pereira Costa


#include "StartWindowActor.h"
#include "Engine/GameViewportClient.h"

AStartWindowActor::AStartWindowActor()
{
 	// Set this actor to call Tick() every frame.  We can turn this off to improve performance if you don't need it.
    // PrimaryActorTick.bCanEverTick = false;
}

void AStartWindowActor::OpenStartWindow(const FString& DefaultPath, FString& OutFileName)
{
    SFileDialog::OpenFileDialogOutput FileDialogResult = SFileDialog::OpenFileDialog(DefaultPath);
    OutFileName = FileDialogResult.OutFileName;
    this->SelectedFilePath = OutFileName;
    bool bFileSelected = FileDialogResult.bOpened;
    UE_LOG(LogTemp, Warning, TEXT("Selected file: %s"), *OutFileName);
    
    if (bFileSelected)
    {
        // Step 1: Create the StartWindow
        StartWindow = SNew(SWindow)
            .ClientSize(FVector2D(600, 400))
            .AutoCenter(EAutoCenter::PreferredWorkArea)
            .SupportsMaximize(false)
            .SupportsMinimize(false)
            .IsInitiallyMaximized(false)
            .Title(FText::FromString(TEXT("VR Point Cloud Labelling Tool")));
        
        // Step 2: Create the StartWidgetSwitcher and pass the StartWindow reference
        StartWidgetSwitcher = SNew(SStartWidgetSwitcher)
            .ParentWindow(StartWindow);

        // Step 3: Assign the StartWidgetSwitcher to StartWindow
        StartWindow->SetContent(StartWidgetSwitcher.ToSharedRef());
         
        // Step 4: Add StartWindow to the Slate application
        FSlateApplication::Get().AddModalWindow(StartWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow());
    }
    else
    {
        UKismetSystemLibrary::QuitGame(GEngine->GameViewport->GetWorld(), GEngine->GameViewport->GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
    }

}

TMap<FString, FLinearColor> AStartWindowActor::GetLabelEntries() const
{
    if (StartWidgetSwitcher.IsValid())
    {
        return *StartWidgetSwitcher->RetrieveLabelEntries();
    }
    return TMap<FString, FLinearColor>();
}

FString AStartWindowActor::GetExportFilePath() const
{
    if (StartWidgetSwitcher.IsValid() && !SelectedFilePath.IsEmpty())
    {
        // Split the file path into directory, filename, and extension
        FString Directory;
        FString Filename;
        FString Extension;

        // Break the full path into its components
        FPaths::Split(SelectedFilePath, Directory, Filename, Extension);

        // Check if filename already contains "_labelled"
        if (Filename.Contains("_labelled"))
        {
            // If already labelled, return the original path
            return SelectedFilePath;
        }

        // Construct the new filename with "_labelled" before the extension
        FString NewFilename = Filename + "_labelled";

        // Reconstruct the full path with the new filename
        return FPaths::Combine(Directory, NewFilename + "." + Extension);
    }
    return FString();
}

FString AStartWindowActor::GetImportFilePath() const
{
    if (StartWidgetSwitcher.IsValid() && !SelectedFilePath.IsEmpty())
    {
        return SelectedFilePath;
    }
    return FString();
}

TMap<int32, FLinearColor> AStartWindowActor::GetClassificationColors() const
{
    // Retrieve the existing map of strings to colors
    TMap<FString, FLinearColor> StringColorMap = GetLabelEntries();

    // Create a new map of integers to colors
    TMap<int32, FLinearColor> IntColorMap;

    // Iterate through the string-to-color map and add the colors to the new map (default: 0, white)
    IntColorMap.Add(0, FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));  // White with 0.0 alpha
    int32 Index = 1;
    for (const auto& Pair : StringColorMap)
    {
        IntColorMap.Add(Index, Pair.Value);
        Index++;
    }

    return IntColorMap;
}
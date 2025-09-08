// Copyright (c) Bruno Pereira Costa

#include "SFileDialog.h"
#include "SlateOptMacros.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"

SFileDialog::OpenFileDialogOutput SFileDialog::OpenFileDialog(const FString& DefaultPath)
{
    OpenFileDialogOutput Output;
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
        uint32 SelectionFlag = 0; //A value of 0 represents single file selection while a value of 1 represents multiple file selection
        TArray<FString> OutFileNames;
        //FString FileTypes = TEXT("Text Files (*.txt)|*.txt|All Files (*.*)|*.*");
        FString FileTypes = TEXT("Text Files (*.txt)|*.txt");
        Output.bOpened = DesktopPlatform->OpenFileDialog(ParentWindowHandle, FString("Please select the file to be imported"), DefaultPath, FString(""), FileTypes, SelectionFlag, OutFileNames);

        if (Output.bOpened && OutFileNames.Num() > 0)
        {
            Output.OutFileName = OutFileNames[0];
        }
    }
    return Output;
}
// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Slate.h"
#include "IDesktopPlatform.h"
#include "FileExplorer.generated.h"

UCLASS()
class SPHERIACTUS_API AFileExplorer : public AActor
{
	GENERATED_BODY()
	
	public:
		AFileExplorer();
		/*
		* Opens a file dialog for the specified data. Leave FileTypes empty to be able to select any files.
		* Filetypes must be in the format of: <File type Description>|*.<actual extension>
		* We can combine multiple extensions by placing ";" between them
		* For example: Text Files|*.txt|Excel files|*.csv|Image Files|*.png;*.jpg;*.bmp will display 3 lines for 3 different type of files.
		*/
		UFUNCTION(BlueprintCallable, Category = "FilePicker")
		void OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames);
};
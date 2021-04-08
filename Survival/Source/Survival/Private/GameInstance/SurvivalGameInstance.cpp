// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/GameInstance/SurvivalGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

USurvivalGameInstance::USurvivalGameInstance()
{
	
}

void USurvivalGameInstance::Init()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Bind Delegates Here
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USurvivalGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USurvivalGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USurvivalGameInstance::OnJoinSessionComplete);
		}
	}
}

void USurvivalGameInstance::OnCreateSessionComplete(FName SeverName, bool bSucceeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionCompleted, Succedded: %d"), bSucceeded);
	if (bSucceeded)
	{
		GetWorld()->ServerTravel("/Game/Maps/ThirdPersonExampleMap?listen");
	}
}

void USurvivalGameInstance::OnFindSessionComplete(bool bSucceeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted, Succedded: %d"), bSucceeded);
	if (bSucceeded)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("ServerCount: %d"), SearchResults.Num());
		if (SearchResults.Num())
		{
			SessionInterface->JoinSession(0, "My Session", SearchResults[0]);
			UE_LOG(LogTemp, Warning, TEXT("Joining Server"));
		}
	}
}

void USurvivalGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted"));
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if (JoinAddress != "")
			PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);

	}
}

void USurvivalGameInstance::CreateServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Created Server"));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false; // put as true eventually
	SessionSettings.bIsLANMatch = true; // put as false
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 5;

	SessionInterface->CreateSession(0, FName("My Session"), SessionSettings);
}

void USurvivalGameInstance::JoinServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Joined Server"));

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}




// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SurvivalGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVAL_API USurvivalGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USurvivalGameInstance();

protected:
	
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
protected:
	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SeverName, bool bSucceeded);

	virtual void OnFindSessionComplete(bool bSucceeded);

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	UFUNCTION(BlueprintCallable)
	void CreateServer();

	UFUNCTION(BlueprintCallable)
	void JoinServer();
};


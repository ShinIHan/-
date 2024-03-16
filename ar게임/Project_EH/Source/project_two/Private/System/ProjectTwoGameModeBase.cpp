// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProjectTwoGameModeBase.h"
#include "Kismet/GameplayStatics.h"

AProjectTwoGameModeBase::AProjectTwoGameModeBase()
{

}

void AProjectTwoGameModeBase::BeginPlay()
{
	auto GI = GetGameInstance();
	PT_GameInstatnce = Cast<UProjectTwoGameInstance>(GI);

	switch (PT_GameInstatnce->Type) 
	{
		case ENetworkType::TE_None:
		{
			UE_LOG(LogTemp, Log, TEXT("AProjectTwoGameModeBase::Game Instance NetworkType Type is None"));
			break;
		}
		case ENetworkType::TE_Local:
		{
			UE_LOG(LogTemp, Log, TEXT("AProjectTwoGameModeBase::Game Instance NetworkType Type is Local"));
			break;
		}
		case ENetworkType::TE_Cloud:
		{
			UE_LOG(LogTemp, Log, TEXT("AProjectTwoGameModeBase::Game Instance NetworkType Type is Cloud"));
			break;
		}
	}

	if (PT_GameInstatnce->Type == ENetworkType::TE_Cloud)
	{
		Super::BeginPlay();
		Start();
	}
	else
		Super::BeginPlay();
}

void AProjectTwoGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PT_GameInstatnce->Type == ENetworkType::TE_Cloud)
	{
		Stop();
		Super::EndPlay(EndPlayReason);
	}
	else
		Super::EndPlay(EndPlayReason);
}

void AProjectTwoGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if (PT_GameInstatnce->Type == ENetworkType::TE_Cloud)
	{
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}

void AProjectTwoGameModeBase::Logout(AController* Exiting)
{
	if (PT_GameInstatnce->Type == ENetworkType::TE_Cloud)
	{
		if (Exiting != nullptr)
		{
			APlayerState* PlayerState = Exiting->PlayerState;
			AProjectTwoPlayerState* PT_PlayerState = Cast<AProjectTwoPlayerState>(PlayerState);
			AProjectTwoGameState* PT_GameState = GetGameState<AProjectTwoGameState>();

			auto networkSubsystem = GetGameInstance()->GetSubsystem<UNetworkTCPSubsystem>();
			if (networkSubsystem != nullptr)
			{
				networkSubsystem->SendDisconnectPlayerPacket();
			}

			if (PT_PlayerState != nullptr && PT_GameState != nullptr) {
				const FString& PlayerSub = PT_PlayerState->PlayerSub;
				PT_GameState->RemovePlayer(*PlayerSub);
				UE_LOG(LogTemp, Log, TEXT("AProjectTwoGameModeBase::Logout(), RemovePlayer"));
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("AProjectTwoGameModeBase::Logout(), PlayerState or GameState is nullptr."));
			}
		}
		Super::Logout(Exiting);
	}
}

FString AProjectTwoGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	auto GI = GetGameInstance();
	PT_GameInstatnce = Cast<UProjectTwoGameInstance>(GI);

	if (PT_GameInstatnce == NULL) return FString();

	if (PT_GameInstatnce->Type == ENetworkType::TE_Cloud)
	{
		FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

		const FString& PlayerId = UGameplayStatics::ParseOption(Options, "PlayerId");

		auto networkSubsystem = GetGameInstance()->GetSubsystem<UNetworkTCPSubsystem>();
		if (networkSubsystem != nullptr)
		{
			networkSubsystem->SendConnectPlayerPacket();
		}

		AProjectTwoPlayerState* PT_PlayerState = Cast<AProjectTwoPlayerState>(NewPlayerController->PlayerState);
		AProjectTwoGameState* PT_GameState = GetGameState<AProjectTwoGameState>();
		if (PT_GameState != nullptr && PT_PlayerState != nullptr)
		{
			PT_GameState->AddPlayer(*PlayerId, NewPlayerController);
			UE_LOG(LogTemp, Log, TEXT("AProjectTwoGameModeBase::InitNewPlayer(), AddPlayer"));
			PT_PlayerState->PlayerSub = *PlayerId;
		}

		return InitializedString;
	}
	else
		return FString();
}


void AProjectTwoGameModeBase::Start()
{
	if (true == GetGameInstance()->IsDedicatedServerInstance()) {
		if (pResourceUsageSender == nullptr) {
			pResourceUsageSender = NewObject<UResourceUsageSender>(GetTransientPackage(), UResourceUsageSender::StaticClass());
			pResourceUsageSender->AddToRoot();
			pResourceUsageSender->Start(GetGameInstance());
		}
	}
}

void AProjectTwoGameModeBase::Stop()
{
	if (pResourceUsageSender != nullptr) {
		pResourceUsageSender->Stop();
		pResourceUsageSender->RemoveFromRoot();
	}
}

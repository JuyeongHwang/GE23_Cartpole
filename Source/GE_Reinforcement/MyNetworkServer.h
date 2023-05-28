// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Async/Async.h"
#include "Sockets.h"
#include "Kismet/GameplayStatics.h"
#include "PoleActor.h"
#include "MyNetworkServer.generated.h"

UCLASS()
class GE_REINFORCEMENT_API AMyNetworkServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyNetworkServer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TArray<bool> HasInput;
	TArray<bool> AppliedInput;
	TArray<FString> Input_;
	TFuture<void> ClientConnectionFinishedFuture;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsConnectionOpen = false;
	bool WaitingForConnection = false;


	UFUNCTION(BlueprintImplementableEvent)
		void OnPause();

	UFUNCTION(BlueprintImplementableEvent)
		void OnResume();
	
	void SendObservation();
	void Close_Connection();
	void OpenConnection();
	void ManageConnection();
	FSocket* ListenSocket = NULL;
	FSocket* ConnectionSocket = NULL;

	UPROPERTY(Transient, VisibleAnywhere)

	TArray<AActor*> apoles;
	TArray<APoleActor*> rPoles;

	TArray<FString> memory;
};

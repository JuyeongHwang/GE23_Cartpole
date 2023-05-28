// Fill out your copyright notice in the Description page of Project Settings.
#include "MyNetworkServer.h"

// Sets default values
AMyNetworkServer::AMyNetworkServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyNetworkServer::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("I'm NetworkServer.cpp"));
	OpenConnection();

	//*** Init ***
	TSubclassOf<APoleActor> classToFind;
	classToFind = APoleActor::StaticClass();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), classToFind, apoles);
	UE_LOG(LogTemp, Warning, TEXT("Get Actor: %d"), apoles.Num());


	APoleActor* caster = NULL;

	for (int i = 0; i < apoles.Num(); i++) {
		caster = Cast<APoleActor>(apoles[i]);
		// caster->Reset_myEnv(i);
		rPoles.Add(caster);
	}

	HasInput.Init(false,apoles.Num());
	AppliedInput.Init(false, apoles.Num());
	Input_.Init("0", apoles.Num());
}

void AMyNetworkServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Close_Connection();
}

// Called every frame
void AMyNetworkServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int _id = 0; _id < apoles.Num(); _id++) {
		
		if (Input_[_id] == "10") {
			rPoles[_id]->Reset_myEnv(_id);
		}
		else if (Input_[_id] == "11") {

		}
		else {
			// Apply Speed
			rPoles[_id]->ApplySpeed(DeltaTime);
		}

		ManageConnection();

		if (Input_[_id] != "10" && AppliedInput[_id]) {
			AppliedInput[_id] = false;
			OnPause();

			FVector vel2 = rPoles[_id]->Pole->GetPhysicsAngularVelocityInDegrees();
			FVector vel = rPoles[_id]->Base->GetPhysicsLinearVelocity();
			FRotator rotator = rPoles[_id]->Pole->GetRelativeRotation();
			FVector currLocation = rPoles[_id]->Base->GetComponentLocation();
			//FVector currLocation2 = rPoles[_id]->Goal->GetComponentLocation();

			// 1, 4
			FString convert = FString::SanitizeFloat(rotator.Pitch) + "," + FString::SanitizeFloat(currLocation.X)
				+ "," + FString::SanitizeFloat(vel.X) + "," + FString::SanitizeFloat(vel2.Y); //+"," + FString::SanitizeFloat(currLocation2.X);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *convert);
			memory.Add(convert);
			UE_LOG(LogTemp, Warning, TEXT("memory size: %d"), memory.Num());
			TCHAR* serializedChar = convert.GetCharArray().GetData();
			int32 size = FCString::Strlen(serializedChar);
			int32 sent = 0;
			ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);

		}

		if (Input_[_id] == "10") {
			Input_[_id] = "11";
		}

		if (HasInput[_id]) {
			HasInput[_id] = false;
		}
	}

}


void AMyNetworkServer::ManageConnection()
{

	// Accept Connection
	if (WaitingForConnection) {
		//UE_LOG(LogTemp, Warning, TEXT("Waiting"));
		TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool hasConnection = false;
		if (ListenSocket->HasPendingConnection(hasConnection) && hasConnection) {
			ConnectionSocket = ListenSocket->Accept(*RemoteAddress, TEXT("Connection"));
			WaitingForConnection = false;
			UE_LOG(LogTemp, Warning, TEXT("incoming connection2"));
			// Start Recv Thread
			ClientConnectionFinishedFuture = Async(EAsyncExecution::LargeThreadPool, [&]() {
				UE_LOG(LogTemp, Warning, TEXT("recv thread started2"));
					
					//******repeat*******
					while (IsConnectionOpen) {
						uint32 size;
						TArray<uint8> ReceivedData;

						if (ConnectionSocket->HasPendingData(size)) {
							ReceivedData.Init(0, 1024);
							int32 Read = 0;
							ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
							FString ManageReceivedData = "";
							if (ReceivedData.Num() > 0)
							{
								// start input data processing *********
								ReceivedData.Add(0);
								UE_LOG(LogTemp, Log, TEXT("StringFromBinaryArray %d"), ReceivedData.Num());
								FString ReceivedUE4String = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));
								UE_LOG(LogTemp, Warning, TEXT("hi? : %s"), *ReceivedUE4String);

								TArray<FString> Out;
								ReceivedUE4String.ParseIntoArray(Out, TEXT(","), true);
								int index = 0;
								int value = 0;
								FString index_, value_;
								for (int i = 0; i < Out.Num(); i++) {
									Out[i].Split(TEXT(":"), &index_, &value_);
									UE_LOG(LogTemp, Warning, TEXT("index: %s, value: %s"), *index_, *value_);
									index = FCString::Atof(*index_.TrimQuotes());
									value = FCString::Atof(*value_);
								}
								// end input data processing ********
								
								//->
								Input_[index] = value_;
								HasInput[index] = true;
								if (HasInput[index]) {
									OnResume();
									AppliedInput[index] = true;

									if (Input_[index] == "10") {
										//nothing!
									}
									else {
										if (Input_[index] == "0")
											value = -1;
										//Move!
										rPoles[index]->Move_XAxis(value);
									}
								}

							}
						}
					}

				}
			);
		}
	}
}




void AMyNetworkServer::Close_Connection()
{
	if (IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Closing Connection2"));
		IsConnectionOpen = false;

		ListenSocket->Close();
	}
}

void AMyNetworkServer::OpenConnection()
{
	if (!IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Openning Connection2"));
		IsConnectionOpen = true;
		WaitingForConnection = true;

		FIPv4Address IPAddress;
		FIPv4Address::Parse(FString("127.0.0.1"), IPAddress);
		FIPv4Endpoint Endpoint(IPAddress, (uint16)5050);

		ListenSocket = FTcpSocketBuilder(TEXT("TcpSocket")).AsReusable();

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ListenSocket->Bind(*SocketSubsystem->CreateInternetAddr(Endpoint.Address.Value, Endpoint.Port));
		ListenSocket->Listen(1);
		UE_LOG(LogTemp, Warning, TEXT("Listening2"));
	}
}

//*************
//TArray<float> states;
//states.Init(0.0, 5);
//for (int i = 0; i < states.Num(); i++) {
//	states[i] = (i + 1.5f);
//	FString TheFloatStr = FString::SanitizeFloat(states[i]);
//	TCHAR* serializedChar = TheFloatStr.GetCharArray().GetData();
//	int32 insize = FCString::Strlen(serializedChar);
//	int32 sent = 0;

//	bool successful = ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), insize, sent);
//} 

//************
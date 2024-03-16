// Fill out your copyright notice in the Description page of Project Settings.


#include "APIGateway/DynamoDBAPIGateway.h"
#include "AWSFunctionLibrary.h"
#include "AWSSubsystem.h"
#include "Auth/CognitoAuth.h"
#include "Interfaces/IHttpResponse.h"
#include <GenericPlatform/GenericPlatformHttp.h>
#include "..\..\Public\APIGateway\DynamoDBAPIGateway.h"

UDynamoDBAPIGateway::UDynamoDBAPIGateway()
{
	ApiUrl = UAWSFunctionLibrary::ReadFile("Urls/DynamoDBApiUrl.txt");

	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::UDynamoDBAPIGateway()"));
}

UDynamoDBAPIGateway::~UDynamoDBAPIGateway()
{
	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::~UDynamoDBAPIGateway()"));
}

void UDynamoDBAPIGateway::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::Initialize()"));
}

void UDynamoDBAPIGateway::Start(UAWSSubsystem* pSubsystem)
{
	Super::Start(pSubsystem);
	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::Start()"));
}

void UDynamoDBAPIGateway::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::Shutdown()"));
	Super::Shutdown();
}

////////////////////////////////////
// ProjectVersion
////////////////////////////////////

bool UDynamoDBAPIGateway::SendGetProjectVersionRequest(FGetProjectVersionSuccessEvent SuccessDelegate, FGetProjectVersionFailureEvent FailureDelegate)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetProjectVersion = pHttpModule->CreateRequest();
	GetProjectVersion->OnProcessRequestComplete().BindWeakLambda(this,
		[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
			OnGetProjectVersionResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
		});
	GetProjectVersion->SetURL(ApiUrl + "/version/TUKMetaCampus");
	GetProjectVersion->SetVerb("GET");
	GetProjectVersion->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetProjectVersionRequest() Complete"));
	return true;
}

void UDynamoDBAPIGateway::OnGetProjectVersionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetProjectVersionSuccessEvent SuccessDelegate, FGetProjectVersionFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if (true == BodyObject->HasField("Version")) {
					FString Version = BodyObject->GetStringField("Version");

					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetProjectVersionResponseReceived() Complete, Version: %s"), *Version);
					SuccessDelegate.ExecuteIfBound(Version);
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have Version field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetProjectVersionResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

////////////////////////////////////
// Info
////////////////////////////////////
bool UDynamoDBAPIGateway::SendGetMyInfoRequest(FGetMyInfoSuccessEvent SuccessDelegate, FGetMyInfoFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetMyInfo = pHttpModule->CreateRequest();
			GetMyInfo->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetMyInfoResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetMyInfo->SetURL(ApiUrl + "/info/myinfo");
			GetMyInfo->SetVerb("GET");
			GetMyInfo->SetHeader("Authorization", Token);
			GetMyInfo->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetMyInfoRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetMyInfoRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnGetMyInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyInfoSuccessEvent SuccessDelegate, FGetMyInfoFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if(true == BodyObject->HasField("PlayerInfo")){
					TSharedPtr<FJsonObject> PlayerInfoObj = BodyObject->GetObjectField("PlayerInfo");

					TSharedPtr<FJsonObject> PlayerSubObj = PlayerInfoObj->GetObjectField("PlayerSub");
					TSharedPtr<FJsonObject> PlayerNameObj = PlayerInfoObj->GetObjectField("PlayerName");
					TSharedPtr<FJsonObject> RoleObj = PlayerInfoObj->GetObjectField("Role");
					TSharedPtr<FJsonObject> CustomizeItemsObj = PlayerInfoObj->GetObjectField("CustomizeItems");

					FString PlayerSub = PlayerSubObj->GetStringField("S");
					FString PlayerName = PlayerNameObj->GetStringField("S");
					FString Role = RoleObj->GetStringField("S");
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyInfoResponseReceived() - Player sub : %s, Player name : %s, Role : %s"), *PlayerSub, *PlayerName, *Role);

					TArray<TSharedPtr<FJsonValue>> CustomizeItemsJsonArray = CustomizeItemsObj->GetArrayField("NS");
					TArray<int> CustomizeItemsArray;
					for (int i = 0; i < CustomizeItemsJsonArray.Num(); ++i) {
						if (CustomizeItemsJsonArray[i]->AsString() != "None") {
							int CustomizeItem = FCString::Atoi(*(CustomizeItemsJsonArray[i]->AsString()));
							CustomizeItemsArray.Add(CustomizeItem);
							UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyInfoResponseReceived() - Player name : %s, Customize item: %d"), *PlayerName, CustomizeItem);
						}
					}
					
					FPlayerInfo PlayerInfo;
					PlayerInfo.PlayerSub = PlayerSub;
					PlayerInfo.PlayerName = PlayerName;
					PlayerInfo.Role = Role;
					PlayerInfo.CustomizeItems = CustomizeItemsArray;

					SuccessDelegate.ExecuteIfBound(PlayerInfo);
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have player info field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetMyInfoResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendModifyMyCustomizeRequest(TArray<int> CustomizeItems, FModifyMyCustomizeSuccessEvent SuccessDelegate, FModifyMyCustomizeFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

			TArray<TSharedPtr<FJsonValue>> ModifyCustomizeItemsJsonArray;
			for (int i = 0; i < CustomizeItems.Num(); ++i) {
				ModifyCustomizeItemsJsonArray.Add(MakeShareable(new FJsonValueString(FString::FromInt(CustomizeItems[i]))));
			}
			RequestObj->SetArrayField("CustomizeItems", ModifyCustomizeItemsJsonArray);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyCustomize = pHttpModule->CreateRequest();
				ModifyCustomize->OnProcessRequestComplete().BindWeakLambda(this,
					[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
						OnModifyMyCustomizeResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
					});
				ModifyCustomize->SetURL(ApiUrl + "/info/myinfo/customize");
				ModifyCustomize->SetVerb("PUT");
				ModifyCustomize->SetHeader("Content-Type", "application/json");
				ModifyCustomize->SetHeader("Authorization", Token);
				ModifyCustomize->SetContentAsString(RequestBody);
				ModifyCustomize->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendModifyMyCustomizeRequest() Complete"));
				return true;
			}
			else {
				FailureReason = "Serialize failure";
			}
		}
		else {
			FailureReason = "AccessToken.Len or RefreshToken.Len is Zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendModifyMyCustomizeRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnModifyMyCustomizeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FModifyMyCustomizeSuccessEvent SuccessDelegate, FModifyMyCustomizeFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if(true == BodyObject->HasField("CustomizeItems")){
					auto CustomizeItemsObj = BodyObject->GetArrayField("CustomizeItems");
					TArray<int> CustomizeItems;

					for (int i = 0; i < CustomizeItemsObj.Num(); ++i) {
						if (CustomizeItemsObj[i]->AsString() != "None")
							CustomizeItems.Add(FCString::Atoi(*(CustomizeItemsObj[i]->AsString())));
					}

					SuccessDelegate.ExecuteIfBound(CustomizeItems);
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnModifyMyCustomizeResponseReceived() Success"));
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have customize items field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnModifyMyCustomizeResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendGetPlayerInfoRequest(FString PlayerSub, FGetPlayerInfoSuccessEvent SuccessDelegate, FGetPlayerInfoFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetPlayerInfo = pHttpModule->CreateRequest();
			GetPlayerInfo->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetPlayerInfoResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetPlayerInfo->SetURL(ApiUrl + "/info/" + PlayerSub);
			GetPlayerInfo->SetVerb("GET");
			GetPlayerInfo->SetHeader("Authorization", Token);
			GetPlayerInfo->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetPlayerInfoRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetPlayerInfoRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason, PlayerSub);
	return false;
}

void UDynamoDBAPIGateway::OnGetPlayerInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetPlayerInfoSuccessEvent SuccessDelegate, FGetPlayerInfoFailureEvent FailureDelegate)
{
	FString FailureReason;
	FString PlayerSub;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
					if (BodyObject->HasField("PlayerSub")) {
						PlayerSub = BodyObject->GetStringField("PlayerSub");
					}
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if(true == BodyObject->HasField("PlayerInfo")) {
					TSharedPtr<FJsonObject> PlayerInfoObj = BodyObject->GetObjectField("PlayerInfo");

					TSharedPtr<FJsonObject> PlayerSubObj = PlayerInfoObj->GetObjectField("PlayerSub");
					TSharedPtr<FJsonObject> PlayerNameObj = PlayerInfoObj->GetObjectField("PlayerName");
					TSharedPtr<FJsonObject> RoleObj = PlayerInfoObj->GetObjectField("Role");
					TSharedPtr<FJsonObject> CustomizeItemsObj = PlayerInfoObj->GetObjectField("CustomizeItems");

					PlayerSub = PlayerSubObj->GetStringField("S");
					FString PlayerName = PlayerNameObj->GetStringField("S");
					FString Role = RoleObj->GetStringField("S");
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetPlayerInfoResponseReceived() - Player sub : %s, Player name : %s, Role : %s"), *PlayerSub, *PlayerName, *Role);

					TArray<TSharedPtr<FJsonValue>> CustomizeItemsJsonArray = CustomizeItemsObj->GetArrayField("NS");
					TArray<int> CustomizeItemsArray;
					for (int i = 0; i < CustomizeItemsJsonArray.Num(); ++i) {
						if (CustomizeItemsJsonArray[i]->AsString() != "None") {
							int CustomizeItem = FCString::Atoi(*(CustomizeItemsJsonArray[i]->AsString()));
							CustomizeItemsArray.Add(CustomizeItem);
							UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetPlayerInfoResponseReceived() - Player name : %s, Customize item: %d"), *PlayerName, CustomizeItem);
						}
					}

					FPlayerInfo PlayerInfo;
					PlayerInfo.PlayerSub = PlayerSub;
					PlayerInfo.PlayerName = PlayerName;
					PlayerInfo.Role = Role;
					PlayerInfo.CustomizeItems = CustomizeItemsArray;

					SuccessDelegate.ExecuteIfBound(PlayerInfo);
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have player info field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetPlayerInfoResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason, PlayerSub);
}

////////////////////////////////////
// TimeTable
////////////////////////////////////
bool UDynamoDBAPIGateway::SendGetTimeTableRequest(FString StartTime, FString InstanceName, FGetTimeTableSuccessEvent SuccessDelegate, FGetTimeTableFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (false == InstanceName.IsEmpty())
			InstanceName = "/" + InstanceName;

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetTimeTable = pHttpModule->CreateRequest();
			GetTimeTable->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetTimeTableResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetTimeTable->SetURL(ApiUrl + "/timetable/" + FGenericPlatformHttp::UrlEncode(StartTime) + InstanceName);
			GetTimeTable->SetVerb("GET");
			GetTimeTable->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetTimeTableRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetTimeTableRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnGetTimeTableResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetTimeTableSuccessEvent SuccessDelegate, FGetTimeTableFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if(true == BodyObject->HasField("TimeTable")) {
					TArray<FTimeTable> TimeTables;
					TArray<TSharedPtr<FJsonValue>> TimeTableJsonArray = BodyObject->GetArrayField("TimeTable");

					for (int i = 0; i < TimeTableJsonArray.Num(); ++i) {
						TSharedPtr<FJsonObject> TimeTableObj = TimeTableJsonArray[i]->AsObject();

						TSharedPtr<FJsonObject> StartTimeObj = TimeTableObj->GetObjectField("StartTime");
						TSharedPtr<FJsonObject> InstanceNameObj = TimeTableObj->GetObjectField("InstanceName");
						TSharedPtr<FJsonObject> ContentNameObj = TimeTableObj->GetObjectField("ContentName");
						TSharedPtr<FJsonObject> LevelNameObj = TimeTableObj->GetObjectField("LevelName");
						TSharedPtr<FJsonObject> MaxPeopleObj = TimeTableObj->GetObjectField("MaxPeople");
						TSharedPtr<FJsonObject> RunningTimeObj = TimeTableObj->GetObjectField("RunningTime");
						TSharedPtr<FJsonObject> StudentsObj = TimeTableObj->GetObjectField("Students");

						FString StartTime = StartTimeObj->GetStringField("S");
						FString InstanceName = InstanceNameObj->GetStringField("S");
						FString ContentName = ContentNameObj->GetStringField("S");
						FString LevelName = LevelNameObj->GetStringField("S");
						int MaxPeople = MaxPeopleObj->GetNumberField("N");
						int RunningTime = RunningTimeObj->GetNumberField("N");

						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetTimeTableResponseReceived() - Start time : %s, Instance name : %s, Content name : %s, Level name : %s, Max people : %d, Running time : %d"), *StartTime, *InstanceName, *ContentName, *LevelName, MaxPeople, RunningTime);

						TArray<TSharedPtr<FJsonValue>> StudentsJsonArray = StudentsObj->GetArrayField("SS");
						TArray<FString> StudentsArray;
						for (int j = 0; j < StudentsJsonArray.Num(); ++j) {
							if (StudentsJsonArray[j]->AsString() != "None") {
								StudentsArray.Add(StudentsJsonArray[j]->AsString());
								UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetTimeTableResponseReceived() - Student sub : %s"), *StudentsJsonArray[j]->AsString());
							}
						}

						FTimeTable TimeTable;
						TimeTable.StartTime = StartTime;
						TimeTable.InstanceName = InstanceName;
						TimeTable.ContentName = ContentName;
						TimeTable.LevelName = LevelName;
						TimeTable.MaxPeople = MaxPeople;
						TimeTable.RunningTime = RunningTime;
						TimeTable.Students = StudentsArray;

						TimeTables.Add(TimeTable);
					}

					SuccessDelegate.ExecuteIfBound(TimeTables);
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have time table field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetTimeTableResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendAddToTimeTableKioskRequest(FString StartTime, FString LevelName, FAddToTimeTableKioskSuccessEvent SuccessDelegate, FAddToTimeTableKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("StartTime", StartTime);
			RequestObj->SetStringField("LevelName", LevelName);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> AddToTimeTableKiosk = pHttpModule->CreateRequest();
				AddToTimeTableKiosk->OnProcessRequestComplete().BindWeakLambda(this,
					[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
						OnAddToTimeTableKioskResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
					});
				AddToTimeTableKiosk->SetURL(ApiUrl + "/timetable");
				AddToTimeTableKiosk->SetVerb("POST");
				AddToTimeTableKiosk->SetHeader("Authorization", Token);
				AddToTimeTableKiosk->SetHeader("Content-Type", "application/json");
				AddToTimeTableKiosk->SetContentAsString(RequestBody);
				AddToTimeTableKiosk->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendAddToTimeTableKioskRequest() Complete"));
				return true;
			}
			FailureReason = "Serialize failure";
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendAddToTimeTableKioskRequest() Error, %s."), *FailureReason);

	FTimeTable TimeTable;
	FailureDelegate.ExecuteIfBound(FailureReason, TimeTable);
	return false;
}

void UDynamoDBAPIGateway::OnAddToTimeTableKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddToTimeTableKioskSuccessEvent SuccessDelegate, FAddToTimeTableKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (true == BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");

					if (true == BodyObject->HasField("TimeTable")) {
						TSharedPtr<FJsonObject> TimeTableObject = BodyObject->GetObjectField("TimeTable");

						FTimeTable TimeTable;

						TimeTable.StartTime = TimeTableObject->GetStringField("StartTime");
						TimeTable.InstanceName = TimeTableObject->GetStringField("InstanceName");
						TimeTable.ContentName = TimeTableObject->GetStringField("ContentName");
						TimeTable.LevelName = TimeTableObject->GetStringField("LevelName");
						TimeTable.MaxPeople = TimeTableObject->GetNumberField("MaxPeople");
						TimeTable.RunningTime = TimeTableObject->GetNumberField("RunningTime");

						TArray<TSharedPtr<FJsonValue>> StudentsObj = TimeTableObject->GetArrayField("Students");
						for (int i = 0; i < StudentsObj.Num(); i++) {
							if (StudentsObj[i]->AsString() != "None")
								TimeTable.Students.Add(StudentsObj[i]->AsString());
						}

						UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnAddToTimeTableKioskResponseReceived() Failure,\
						 StartTime: %s, InstanceName: %s, ContentName: %s, LevelName: %s, MaxPeople: %d, RunningTime : %d"),
							*TimeTable.StartTime, *TimeTable.InstanceName, *TimeTable.ContentName, *TimeTable.LevelName, TimeTable.MaxPeople, TimeTable.RunningTime);
						FailureDelegate.ExecuteIfBound(FailureReason, TimeTable);
						return;
					}
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else {
					FTimeTable TimeTable;

					TimeTable.StartTime = BodyObject->GetStringField("StartTime");
					TimeTable.InstanceName = BodyObject->GetStringField("InstanceName");
					TimeTable.ContentName = BodyObject->GetStringField("ContentName");
					TimeTable.LevelName = BodyObject->GetStringField("LevelName");
					TimeTable.MaxPeople = BodyObject->GetNumberField("MaxPeople");
					TimeTable.RunningTime = BodyObject->GetNumberField("RunningTime");

					TArray<TSharedPtr<FJsonValue>> StudentsObj = BodyObject->GetArrayField("Students");
					for (int i = 0; i < StudentsObj.Num(); i++) {
						if (StudentsObj[i]->AsString() != "None")
							TimeTable.Students.Add(StudentsObj[i]->AsString());
					}

					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnAddToTimeTableKioskResponseReceived() Complete,\
						 StartTime: %s, InstanceName: %s, ContentName: %s, LevelName: %s, MaxPeople: %d, RunningTime : %d"),
						*TimeTable.StartTime, *TimeTable.InstanceName, *TimeTable.ContentName, *TimeTable.LevelName, TimeTable.MaxPeople, TimeTable.RunningTime);
					SuccessDelegate.ExecuteIfBound(TimeTable);
					return;
				}
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	FTimeTable TimeTable;
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnAddToTimeTableKioskResponseReceived() Error, %s"), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason, TimeTable);
}

bool UDynamoDBAPIGateway::SendSignUpClassKioskRequest(FString StartTime, FString InstanceName, bool SignUp, FSignUpClassKioskSuccessEvent SuccessDelegate, FSignUpClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("StartTime", StartTime);
			RequestObj->SetStringField("InstanceName", InstanceName);
			RequestObj->SetBoolField("Action", SignUp);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> SignUpClass = pHttpModule->CreateRequest();
				SignUpClass->OnProcessRequestComplete().BindWeakLambda(this,
					[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
						OnSignUpClassKioskResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
					});
				SignUpClass->SetURL(ApiUrl + "/timetable");
				SignUpClass->SetVerb("PUT");
				SignUpClass->SetHeader("Authorization", Token);
				SignUpClass->SetHeader("Content-Type", "application/json");
				SignUpClass->SetContentAsString(RequestBody);
				SignUpClass->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendSignUpClassKioskRequest() Complete"));
				return true;
			}
			FailureReason = "Serialize failure";
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendSignUpClassKioskRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FSignUpClassKioskSuccessEvent SuccessDelegate, FSignUpClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if(true == BodyObject->HasField("Success")){
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Complete"));
					SuccessDelegate.ExecuteIfBound(true);
					return;
				}
				else {
					FailureReason = "Unknown error";
				}
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendCheckSignUpClassKioskRequest(FString StartTime, FString InstanceName, FCheckSignUpClassKioskSuccessEvent SuccessDelegate, FCheckSignUpClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (false == InstanceName.IsEmpty())
			InstanceName = "/" + InstanceName;

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CheckSignUpClass = pHttpModule->CreateRequest();
			CheckSignUpClass->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnCheckSignUpClassKioskResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			CheckSignUpClass->SetURL(ApiUrl + "/timetable/checksignup/" + FGenericPlatformHttp::UrlEncode(StartTime) + InstanceName);
			CheckSignUpClass->SetVerb("GET");
			CheckSignUpClass->SetHeader("Authorization", Token);
			CheckSignUpClass->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendCheckSignUpClassKioskRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendCheckSignUpClassKioskRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnCheckSignUpClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FCheckSignUpClassKioskSuccessEvent SuccessDelegate, FCheckSignUpClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if (true == BodyObject->HasField("Status")) {
					if (BodyObject->GetStringField("Status") == "Exist") {
						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Complete, Check status: Exist"));
						SuccessDelegate.ExecuteIfBound(true);
						return;
					}
					else if (BodyObject->GetStringField("Status") == "Null") {
						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Complete, Check status: Null"));
						SuccessDelegate.ExecuteIfBound(false);
						return;
					}
					else {
						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Error, Check status: Unknown"));
						FailureReason = "Check status is Unknown";
					}

				}
				else {
					FailureReason = "Unknown error";
				}
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnSignUpClassKioskResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendGetMyClassKioskRequest(FString StartTime, FGetMyClassKioskSuccessEvent SuccessDelegate, FGetMyClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetMyClass = pHttpModule->CreateRequest();
			GetMyClass->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetMyClassKioskResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetMyClass->SetURL(ApiUrl + "/timetable/myclass/" + FGenericPlatformHttp::UrlEncode(StartTime));
			GetMyClass->SetVerb("GET");
			GetMyClass->SetHeader("Authorization", Token);
			GetMyClass->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetMyClassKioskRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetMyClassKioskRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnGetMyClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyClassKioskSuccessEvent SuccessDelegate, FGetMyClassKioskFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObject = JsonObject->GetObjectField("body");
				if (BodyObject->HasField("Error")) {
					FailureReason = BodyObject->GetStringField("Error");
				}
				else if (true == BodyObject->HasField("error")) {
					FailureReason = BodyObject->GetStringField("error");
				}
				else if (true == BodyObject->HasField("MyClasses")) {
					TArray<FTimeTable> TimeTables;
					TArray<TSharedPtr<FJsonValue>> TimeTableJsonArray = BodyObject->GetArrayField("MyClasses");

					for (int i = 0; i < TimeTableJsonArray.Num(); ++i) {
						TSharedPtr<FJsonObject> TimeTableObj = TimeTableJsonArray[i]->AsObject();

						TSharedPtr<FJsonObject> StartTimeObj = TimeTableObj->GetObjectField("StartTime");
						TSharedPtr<FJsonObject> InstanceNameObj = TimeTableObj->GetObjectField("InstanceName");
						TSharedPtr<FJsonObject> ContentNameObj = TimeTableObj->GetObjectField("ContentName");
						TSharedPtr<FJsonObject> LevelNameObj = TimeTableObj->GetObjectField("LevelName");
						TSharedPtr<FJsonObject> MaxPeopleObj = TimeTableObj->GetObjectField("MaxPeople");
						TSharedPtr<FJsonObject> RunningTimeObj = TimeTableObj->GetObjectField("RunningTime");
						TSharedPtr<FJsonObject> StudentsObj = TimeTableObj->GetObjectField("Students");

						FString StartTime = StartTimeObj->GetStringField("S");
						FString InstanceName = InstanceNameObj->GetStringField("S");
						FString ContentName = ContentNameObj->GetStringField("S");
						FString LevelName = LevelNameObj->GetStringField("S");
						int MaxPeople = MaxPeopleObj->GetNumberField("N");
						int RunningTime = RunningTimeObj->GetNumberField("N");

						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyClassKioskResponseReceived() - Start time : %s, Instance name : %s, Content name : %s, Level name : %s, Max people : %d, Running time : %d"), *StartTime, *InstanceName, *ContentName, *LevelName, MaxPeople, RunningTime);

						TArray<TSharedPtr<FJsonValue>> StudentsJsonArray = StudentsObj->GetArrayField("SS");
						TArray<FString> StudentsArray;
						for (int j = 0; j < StudentsJsonArray.Num(); ++j) {
							if (StudentsJsonArray[j]->AsString() != "None") {
								StudentsArray.Add(StudentsJsonArray[j]->AsString());
								UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyClassKioskResponseReceived() - Student sub : %s"), *StudentsJsonArray[j]->AsString());
							}
						}

						FTimeTable TimeTable;
						TimeTable.StartTime = StartTime;
						TimeTable.InstanceName = InstanceName;
						TimeTable.ContentName = ContentName;
						TimeTable.LevelName = LevelName;
						TimeTable.MaxPeople = MaxPeople;
						TimeTable.RunningTime = RunningTime;
						TimeTable.Students = StudentsArray;

						TimeTables.Add(TimeTable);
					}

					SuccessDelegate.ExecuteIfBound(TimeTables);
					return;
				}
				else {
					FailureReason = "Unknown error";
				}
			}
			else {
				FailureReason = "JsonObject doesn't have body field";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetMyClassKioskResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

////////////////////////////////////
// AttendanceRecord
////////////////////////////////////
bool UDynamoDBAPIGateway::SendGetMyAttendanceRecordRequest(FGetMyAttendanceRecordSuccessEvent SuccessDelegate, FGetMyAttendanceRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetMyAttendanceRecord = pHttpModule->CreateRequest();
			GetMyAttendanceRecord->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetMyAttendanceRecordResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetMyAttendanceRecord->SetURL(ApiUrl + "/attendancerecord/myrecord");
			GetMyAttendanceRecord->SetVerb("GET");
			GetMyAttendanceRecord->SetHeader("Authorization", Token);
			GetMyAttendanceRecord->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetMyAttendanceRecordRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token length is zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetMyAttendanceRecordRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnGetMyAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyAttendanceRecordSuccessEvent SuccessDelegate, FGetMyAttendanceRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObj = JsonObject->GetObjectField("body");
				if (true == BodyObj->HasField("Error")) {
					FailureReason = BodyObj->GetStringField("Error");
				}
				else if (true == BodyObj->HasField("error")) {
					FailureReason = BodyObj->GetStringField("error");
				}
				else if (true == BodyObj->HasField("AttendanceRecords")) {
					TArray<FAttendanceRecord> AttendanceRecords;

					TArray<TSharedPtr<FJsonValue>> AttendanceRecordsJsonArray = BodyObj->GetArrayField("AttendanceRecords");
					for (int i = 0; i < AttendanceRecordsJsonArray.Num(); ++i) {
						TSharedPtr<FJsonObject> AttendanceRecordObj = AttendanceRecordsJsonArray[i]->AsObject();

						TSharedPtr<FJsonObject> PlayerSubObj = AttendanceRecordObj->GetObjectField("PlayerSub");
						TSharedPtr<FJsonObject> PlayerNameObj = AttendanceRecordObj->GetObjectField("PlayerName");
						TSharedPtr<FJsonObject> ContentNameObj = AttendanceRecordObj->GetObjectField("ContentName");
						TSharedPtr<FJsonObject> LevelNameObj = AttendanceRecordObj->GetObjectField("LevelName");
						TSharedPtr<FJsonObject> AttendanceListObj = AttendanceRecordObj->GetObjectField("AttendanceList");

						FString PlayerSub = PlayerSubObj->GetStringField("S");
						FString PlayerName = PlayerNameObj->GetStringField("S");
						FString ContentName = ContentNameObj->GetStringField("S");
						FString LevelName = LevelNameObj->GetStringField("S");
						TArray<TSharedPtr<FJsonValue>> AttendanceListJsonArray = AttendanceListObj->GetArrayField("SS");

						TArray<FString> AttendanceList;
						for (int j = 0; j < AttendanceListJsonArray.Num(); ++j) {
							if (AttendanceListJsonArray[i]->AsString() != "None")
								AttendanceList.Add(AttendanceListJsonArray[j]->AsString());
						}

						UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyAttendanceRecordResponseReceived(), Player name: %s, Content name: %s, Level name: %s"), *PlayerName, *ContentName, *LevelName);

						FAttendanceRecord AttendanceRecord;
						AttendanceRecord.PlayerSub = PlayerSub;
						AttendanceRecord.PlayerName = PlayerName;
						AttendanceRecord.ContentName = ContentName;
						AttendanceRecord.LevelName = LevelName;
						AttendanceRecord.AttendanceList = AttendanceList;
						AttendanceRecords.Add(AttendanceRecord);
					}

					SuccessDelegate.ExecuteIfBound(AttendanceRecords);
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetMyAttendanceRecordResponseReceived()"));
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have attendance records field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetMyAttendanceRecordResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendAddAttendanceRecordRequest(FString PlayerSub, FString LevelName, FString AttendanceDate, FAddAttendanceRecordSuccessEvent SuccessDelegate, FAddAttendanceRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("LevelName", LevelName);
			RequestObj->SetStringField("AttendanceDate", AttendanceDate);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> AddAtt = pHttpModule->CreateRequest();
				AddAtt->OnProcessRequestComplete().BindWeakLambda(this,
					[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
						OnAddAttendanceRecordResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
					});
				AddAtt->SetURL(ApiUrl + "/attendancerecord/" + PlayerSub);
				AddAtt->SetVerb("PUT");
				AddAtt->SetHeader("Content-Type", "application/json");
				AddAtt->SetHeader("Authorization", Token);
				AddAtt->SetContentAsString(RequestBody);
				AddAtt->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendAddAttendanceRecordRequest() Complete"));
				return true;
			}
			else {
				FailureReason = "Serialize failure";
			}
		}
		else {
			FailureReason = "Token.Len() is Zero";
		}
	}
	else {
		FailureReason = "Authorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendAddAttendanceRecordRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnAddAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddAttendanceRecordSuccessEvent SuccessDelegate, FAddAttendanceRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObj = JsonObject->GetObjectField("body");
				if (true == BodyObj->HasField("Error")) {
					FailureReason = BodyObj->GetStringField("Error");
				}
				else if (true == BodyObj->HasField("error")) {
					FailureReason = BodyObj->GetStringField("error");
				}
				else if(true == BodyObj->HasField("PlayerSub")) {
					FAttendanceRecord AttendanceRecord;

					FString PlayerSub = BodyObj->GetStringField("PlayerSub");
					FString LevelName = BodyObj->GetStringField("LevelName");
					FString PlayerName = BodyObj->GetStringField("PlayerName");
					FString ContentName = BodyObj->GetStringField("ContentName");

					TArray<FString> AttendanceList;
					TArray<TSharedPtr<FJsonValue>> AttendanceListJsonArray = BodyObj->GetArrayField("AttendanceList");
					for (int i = 0; i < AttendanceListJsonArray.Num(); ++i) {
						if (AttendanceListJsonArray[i]->AsString() != "None") {
							AttendanceList.Add(AttendanceListJsonArray[i]->AsString());
						}
					}

					AttendanceRecord.PlayerSub = PlayerSub;
					AttendanceRecord.PlayerName = PlayerName;
					AttendanceRecord.LevelName = LevelName;
					AttendanceRecord.ContentName = ContentName;
					AttendanceRecord.AttendanceList = AttendanceList;
					
					SuccessDelegate.ExecuteIfBound(AttendanceRecord);
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnAddAttendanceRecordResponseReceived() Success"));
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have player sub field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnAddAttendanceRecordResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}


bool UDynamoDBAPIGateway::SendGetVlcURLRequest(FGetVlcURLSuccessEvent SuccessDelegate, FGetVlcURLFailureEvent FailureDelegate)
{
	FString FailureReason;

	//if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		//FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		//if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetVlcURL = pHttpModule->CreateRequest();
			GetVlcURL->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetVlcURLResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetVlcURL->SetURL(ApiUrl + "/vlcurl");
			GetVlcURL->SetVerb("GET");
			//GetVlcURL->SetHeader("Authorization", Token);
			GetVlcURL->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendGetVlcURLRequest() Complete"));
			return true;
		//}
		//else {
		//	FailureReason = "Token length is zero";
		//}
	//}
	//else {
	//	FailureReason = "Get authorizer failed";
	//}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendGetVlcURLRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnGetVlcURLResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetVlcURLSuccessEvent SuccessDelegate, FGetVlcURLFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObj = JsonObject->GetObjectField("body");
				if (true == BodyObj->HasField("Error")) {
					FailureReason = BodyObj->GetStringField("Error");
				}
				else if (true == BodyObj->HasField("error")) {
					FailureReason = BodyObj->GetStringField("error");
				}
				else if (true == BodyObj->HasField("Url")) {
					FString Url = BodyObj->GetStringField("Url");
					bool IsLive = BodyObj->GetBoolField("IsLive");
					SuccessDelegate.ExecuteIfBound(Url, IsLive);
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::OnGetVlcURLResponseReceived() Success, Url: %s, IsLive: %s"), *Url, IsLive ? TEXT("true") : TEXT("false"));
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have url field";
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnGetVlcURLResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

bool UDynamoDBAPIGateway::SendAddAccessRecordRequest(FString PlayerSub, FString LevelName, FString AccessTime, FAddAccessRecordSuccessEvent SuccessDelegate, FAddAccessRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (pAWSSubsystem->GetAuthorizer() != nullptr) {
		FString Token = pAWSSubsystem->GetAuthorizer()->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);			
			RequestObj->SetStringField("LevelName", LevelName);
			RequestObj->SetStringField("AccessTime", AccessTime);
			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyCustomize = pHttpModule->CreateRequest();
				ModifyCustomize->OnProcessRequestComplete().BindWeakLambda(this,
					[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
						OnAddAccessRecordResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
					});
				ModifyCustomize->SetURL(ApiUrl + "/accessrecord/" + PlayerSub);
				ModifyCustomize->SetVerb("PUT");
				ModifyCustomize->SetHeader("Content-Type", "application/json");
				ModifyCustomize->SetHeader("Authorization", Token);
				ModifyCustomize->SetContentAsString(RequestBody);
				ModifyCustomize->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBAPIGateway::SendAddAccessRecordRequest() Complete"));
				return true;
			}
			else {
				FailureReason = "Serialize failure";
			}
		}
		else {
			FailureReason = "AccessToken.Len or RefreshToken.Len is Zero";
		}
	}
	else {
		FailureReason = "Get authorizer failed";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::SendAddAccessRecordRequest() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UDynamoDBAPIGateway::OnAddAccessRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddAccessRecordSuccessEvent SuccessDelegate, FAddAccessRecordFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObj = JsonObject->GetObjectField("body");
				if (true == BodyObj->HasField("Error")) {
					FailureReason = BodyObj->GetStringField("Error");
				}
				else if (true == BodyObj->HasField("error")) {
					FailureReason = BodyObj->GetStringField("error");
				}				
				else {
					SuccessDelegate.ExecuteIfBound();
					return;
				}
			}
			else if (true == JsonObject->HasField("message")) {
				FailureReason = JsonObject->GetStringField("message");
			}
			else {
				FailureReason = "JsonObject doesn't have body";
			}
		}
		else {
			FailureReason = "Deserialize failure";
		}
	}
	else {
		FailureReason = "bWasSuccessful is false";
	}

	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBAPIGateway::OnAddAccessRecordResponseReceived() Error, %s."), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}
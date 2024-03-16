// Fill out your copyright notice in the Description page of Project Settings.


#include "Database/DynamoDBSubsystem.h"

#include "AWSBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Authorize/AWSAuthorizeSubsystem.h"
#include "Database/DynamoDBCallbackInterface.h"
#include <string>
#include "Interfaces/IHttpResponse.h"


UDynamoDBSubsystem::UDynamoDBSubsystem()
{
	ApiUrl = UAWSBlueprintFunctionLibrary::ReadFile("Urls/ApiUrl.txt");
	pHttpModule = &FHttpModule::Get();
}

void UDynamoDBSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDynamoDBSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDynamoDBSubsystem::SendGetPlayerDataRequest(FString UserId)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;
	
	if(authorizeSubsystem != nullptr) {
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("playerSub", UserId);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetPlayerDataServer = pHttpModule->CreateRequest();
				GetPlayerDataServer->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetPlayerDataResponseReceived);
				GetPlayerDataServer->SetURL(ApiUrl + "/getplayerdataserver");
				GetPlayerDataServer->SetVerb("POST");
				GetPlayerDataServer->SetHeader("Authorization", Authorizer);
				GetPlayerDataServer->SetHeader("Content-Type", "application/json");
				GetPlayerDataServer->SetContentAsString(RequestBody);
				GetPlayerDataServer->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetPlayerDataRequest() Complete, UserId : %s"), *UserId);
				return true;
			}
			failureReason = "Serialize failure";
		}
		else
		{
			if (Authorizer.Len() > 0) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetPlayerData = pHttpModule->CreateRequest();
				GetPlayerData->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetPlayerDataResponseReceived);
				GetPlayerData->SetURL(ApiUrl + "/getplayerdata");
				GetPlayerData->SetVerb("GET");
				GetPlayerData->SetHeader("Authorization", Authorizer);
				GetPlayerData->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetPlayerDataRequest() Complete"));
				return true;
			}
			failureReason = "AccessToken.Len() is zero";
		}
	}
	else {
		failureReason = "Get Authorize Subsystem failed";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetPlayerDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendGetPlayerDataRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnGetPlayerDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance != nullptr)
	{
		if (bWasSuccessful) {
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
				if (JsonObject->HasField("playerData")) {
	
					TSharedPtr<FJsonObject> PlayerDataObj = JsonObject->GetObjectField("playerData");
					TSharedPtr<FJsonObject> UserIdObj = PlayerDataObj->GetObjectField("PlayerSub");
					TSharedPtr<FJsonObject> UserNameObj = PlayerDataObj->GetObjectField("PlayerId");

					FString UserId = UserIdObj->GetStringField("S");
					FString UserName = UserNameObj->GetStringField("S");
				
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() - UserId : %s, UserName: %s"), *UserId, *UserName);
				
					TSharedPtr<FJsonObject> RoleObj = PlayerDataObj->GetObjectField("Role");
					FString RoleStr = RoleObj->GetStringField("S");
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() - Username : %s, Role: %s"), *UserName, *RoleStr);

					TSharedPtr<FJsonObject> SpawnableObj = PlayerDataObj->GetObjectField("Spawnable");
					TArray<TSharedPtr<FJsonValue>> SpawnableJsonArray = SpawnableObj->GetArrayField("SS");
					TArray<FString> SpawnableArray;
					for (int i = 0; i < SpawnableJsonArray.Num(); ++i) {
						if (SpawnableJsonArray[i]->AsString() != "None") {
							SpawnableArray.Add(SpawnableJsonArray[i]->AsString());
							UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() - Username : %s, Spawnable: %s"), *UserName, *SpawnableJsonArray[i]->AsString());
						}
					}

					TSharedPtr<FJsonObject> InventoryObj = PlayerDataObj->GetObjectField("Inventory");
					TArray<TSharedPtr<FJsonValue>> InventoryJsonArray = InventoryObj->GetArrayField("SS");
					TArray<FString> InventoryArray;
					for (int i = 0; i < InventoryJsonArray.Num(); ++i) {
						if (InventoryJsonArray[i]->AsString() != "None") {
							InventoryArray.Add(InventoryJsonArray[i]->AsString());
							UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() - Username : %s, Item: %s"), *UserName, *InventoryJsonArray[i]->AsString());
						}
					}

					TSharedPtr<FJsonObject> AccessibleObj = PlayerDataObj->GetObjectField("Accessible");
					TArray<TSharedPtr<FJsonValue>> AccessibleJsonArray = AccessibleObj->GetArrayField("SS");
					TArray<FString> AccessibleArray;
					for (int i = 0; i < AccessibleJsonArray.Num(); ++i) {
						if (AccessibleJsonArray[i]->AsString() != "None") {
							AccessibleArray.Add(AccessibleJsonArray[i]->AsString());
							UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() - Username : %s, Accessible: %s"), *UserName, *AccessibleJsonArray[i]->AsString());
						}
					}
					
					if (GameInstance->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
						IDynamoDBCallbackInterface::Execute_ReceivedPlayerIdEvent(GameInstance, UserId);
						IDynamoDBCallbackInterface::Execute_ReceivedPlayerNameEvent(GameInstance, UserId, UserName);
						IDynamoDBCallbackInterface::Execute_ReceivedPlayerRoleEvent(GameInstance, UserId, RoleStr);
						IDynamoDBCallbackInterface::Execute_ReceivedPlayerInfoEvent(GameInstance, UserId, SpawnableArray, AccessibleArray, InventoryArray);
						UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() Complete, UserId : %s, UserName : %s, Role : %s."), *UserId, *UserName, *RoleStr);
						return;
					}
					failureReason = "ImplementsInterface() failure";
				}
				else {
					failureReason = "JsonObject doesn't have playerData field";
				}
			}
			else {
				failureReason = "Deserialize failure";
			}
		}
		else {
			failureReason = "bWasSuccessful is false";
		}
	}
	else {
		failureReason = "GameInstance is nullptr";
	}
	
	if (GameInstance->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetPlayerDataFailureEvent(GameInstance, failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnGetPlayerDataResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendGetPlayerCustomizeDataRequest(FString UserId)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;

	if(authorizeSubsystem != nullptr){
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("playerSub", UserId);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetPlayerDataServer = pHttpModule->CreateRequest();
				GetPlayerDataServer->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetPlayerCustomizeDataResponseReceived);
				GetPlayerDataServer->SetURL(ApiUrl + "/getplayercustomizedataserver");
				GetPlayerDataServer->SetVerb("POST");
				GetPlayerDataServer->SetHeader("Authorization", Authorizer);
				GetPlayerDataServer->SetHeader("Content-Type", "application/json");
				GetPlayerDataServer->SetContentAsString(RequestBody);
				GetPlayerDataServer->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetPlayerCustomizeDataRequest() Complete, UserId : %s."), *UserId);
				return true;
			}
			failureReason = "Serialize failure";
		}
		else
		{
			if (Authorizer.Len() > 0) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetPlayerCustomizeData = pHttpModule->CreateRequest();
				GetPlayerCustomizeData->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetPlayerCustomizeDataResponseReceived);
				GetPlayerCustomizeData->SetURL(ApiUrl + "/getplayercustomizedata");
				GetPlayerCustomizeData->SetVerb("GET");
				GetPlayerCustomizeData->SetHeader("Authorization", Authorizer);
				GetPlayerCustomizeData->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetPlayerCustomizeDataRequest() Complete."));
				return true;
			}
			failureReason = "AccessToken.Len() is Zero";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetPlayerCustomizeDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendGetPlayerCustomizeDataRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnGetPlayerCustomizeDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance != nullptr)
	{
		if (bWasSuccessful) {
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
				if (JsonObject->HasField("playerData")) {
					TSharedPtr<FJsonObject> PlayerDataObj = JsonObject->GetObjectField("playerData");
					TSharedPtr<FJsonObject> UserIdObj = PlayerDataObj->GetObjectField("PlayerSub");
					TSharedPtr<FJsonObject> UserNameObj = PlayerDataObj->GetObjectField("PlayerId");

					FString UserId = UserIdObj->GetStringField("S");
					FString UserName = UserNameObj->GetStringField("S");

					TSharedPtr<FJsonObject> CustomizeObj = PlayerDataObj->GetObjectField("Items");
					TArray<TSharedPtr<FJsonValue>> CustomizeJsonArray = CustomizeObj->GetArrayField("NS");
					TArray<int> CustomizeArray;
					for (int i = 0; i < CustomizeJsonArray.Num(); ++i) {
						CustomizeArray.Add(CustomizeJsonArray[i]->AsNumber());
					}

					if (GameInstance->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
						IDynamoDBCallbackInterface::Execute_ReceivedGetPlayerCustomizeDataSuccessEvent(GameInstance, UserId, CustomizeArray);
					}
					UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetPlayerCustomizeDataResponseReceived() Success, UserId : %s, UserName : %s"), *UserId, *UserName);
					return;
				}
				else if (JsonObject->HasField("error")) {
					failureReason = JsonObject->GetStringField("error");
					if (JsonObject->HasField("playerSub")) {
						SendGetPlayerCustomizeDataRequest(JsonObject->GetStringField("playerSub"));
					}
				}
				else
				{
					failureReason = "JsonObject doesn't have playerData field";
				}
			}
			else
			{
				failureReason = "Deserialize failure";
			}
		}
		else
		{
			failureReason = "bWasSuccessful is false";
		}
	}
	else
	{
		failureReason = "GameInstance is nullptr";
	}
	
	if (GameInstance->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetPlayerCustomizeDataFailureEvent(GameInstance, failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnGetPlayerCustomizeDataResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendModifyPlayerCustomizeDataRequest(FString UserId, TArray<int> CustomizeItems)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;
	
	if(authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			failureReason = "This function can be used in only client";
		}
		else
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

			TArray<TSharedPtr<FJsonValue>> itemsArray;
			for (int i = 0; i < CustomizeItems.Num(); ++i) {
				itemsArray.Add(MakeShareable(new FJsonValueNumber(CustomizeItems[i])));
			}
			RequestObj->SetArrayField("items", itemsArray);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyPlayerCustomize = pHttpModule->CreateRequest();
				//ModifyPlayerCustomize->OnProcessRequestComplete().BindUObject(this, &UAWSConnectorClient::OnModifyPlayerCustomizeResponseReceived);
				ModifyPlayerCustomize->SetURL(ApiUrl + "/modifyplayercustomize");
				ModifyPlayerCustomize->SetVerb("POST");
				ModifyPlayerCustomize->SetHeader("Authorization", Authorizer);
				ModifyPlayerCustomize->SetHeader("Content-Type", "application/json");
				ModifyPlayerCustomize->SetContentAsString(RequestBody);
				ModifyPlayerCustomize->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendModifyPlayerCustomizeDataRequest() Complete"));
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendModifyPlayerCustomizeDataRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnModifyPlayerCustomizeDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	
}

bool UDynamoDBSubsystem::SendGetLectureDataRequest(FString LectureCode)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;

	if (authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if (true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("lectureCode", LectureCode);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetLectureDataServer = pHttpModule->CreateRequest();
				GetLectureDataServer->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetLectureDataResponseReceived);
				GetLectureDataServer->SetURL(ApiUrl + "/getlecturedataserver");
				GetLectureDataServer->SetVerb("POST");
				GetLectureDataServer->SetHeader("Authorization", Authorizer);
				GetLectureDataServer->SetHeader("Content-Type", "application/json");
				GetLectureDataServer->SetContentAsString(RequestBody);
				GetLectureDataServer->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetLectureDataRequest() Complete, LectureCode : %s."), *LectureCode);
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
		else
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("lectureCode", LectureCode);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetLectureData = pHttpModule->CreateRequest();
				GetLectureData->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetLectureDataResponseReceived);
				GetLectureData->SetURL(ApiUrl + "/getlecturedata");
				GetLectureData->SetVerb("POST");
				GetLectureData->SetHeader("Authorization", Authorizer);
				GetLectureData->SetHeader("Content-Type", "application/json");
				GetLectureData->SetContentAsString(RequestBody);
				GetLectureData->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetLectureDataRequest() Complete, LectureCode : %s."), *LectureCode);
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetLectureDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendGetLectureDataRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnGetLectureDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,	bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("lectureData")) {
				TSharedPtr<FJsonObject> LectureDataObj = JsonObject->GetObjectField("lectureData");

				TSharedPtr<FJsonObject> LectureCodeObj = LectureDataObj->GetObjectField("LectureCode");
				FString LectureCode = LectureCodeObj->GetStringField("S");

				TSharedPtr<FJsonObject> schoolObj = LectureDataObj->GetObjectField("SchoolName");
				FString SchoolName = schoolObj->GetStringField("S");

				TSharedPtr<FJsonObject> departmentObj = LectureDataObj->GetObjectField("DepartmentName");
				FString DepartmentName = departmentObj->GetStringField("S");

				TSharedPtr<FJsonObject> lectureObj = LectureDataObj->GetObjectField("LectureName");
				FString LectureName = lectureObj->GetStringField("S");

				TSharedPtr<FJsonObject> ContentsObj = LectureDataObj->GetObjectField("Contents");
				TArray<TSharedPtr<FJsonValue>> ContentsArray = ContentsObj->GetArrayField("SS");
				TArray<FString> Contents;
				for (int i = 0; i < ContentsArray.Num(); ++i) {
					if (ContentsArray[i]->AsString() != "None")
						Contents.Add(ContentsArray[i]->AsString());
				}

				TSharedPtr<FJsonObject> ProfessorObj = LectureDataObj->GetObjectField("ProfessorSub");
				FString ProfessorSub = ProfessorObj->GetStringField("S");

				TSharedPtr<FJsonObject> StudentsObj = LectureDataObj->GetObjectField("StudentsSub");
				TArray<TSharedPtr<FJsonValue>> StudentsArray = StudentsObj->GetArrayField("SS");
				TArray<FString> Students;
				for (int i = 0; i < StudentsArray.Num(); ++i) {
					if (StudentsArray[i]->AsString() != "None")
						Students.Add(StudentsArray[i]->AsString());
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedGetLectureDataSuccessEvent(GetGameInstance(), LectureCode, SchoolName, DepartmentName, LectureName, Contents, ProfessorSub, Students);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetLectureDataResponseReceived() Complete, LectureCode : %s, SchoolName : %s, DepartmentName : %s, LectureName : %s, ProfessorSub : %s"), 
							*LectureCode, *SchoolName, *DepartmentName, *LectureName, *ProfessorSub);
				return;
			}
			else
			{
				failureReason = "JsonObject doesn't have lectureData field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetLectureDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnGetLectureDataResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendGetTimeTableDataRequest(FDateTime date, FString Classroom)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;

	if (authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if (true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

			RequestObj->SetStringField("year", FString::FromInt(date.GetYear()));
			RequestObj->SetStringField("month", FString::FromInt(date.GetMonth()));
			RequestObj->SetStringField("day", FString::FromInt(date.GetDay()));
			RequestObj->SetStringField("hour", FString::FromInt(date.GetHour()));
			RequestObj->SetStringField("minute", FString::FromInt(date.GetMinute()));
			RequestObj->SetStringField("classroom", Classroom);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetAttendanceRecord = pHttpModule->CreateRequest();
				GetAttendanceRecord->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetTimeTableDataResponseReceived);
				GetAttendanceRecord->SetURL(ApiUrl + "/gettimetableserver");
				GetAttendanceRecord->SetVerb("POST");
				GetAttendanceRecord->SetHeader("Authorization", Authorizer);
				GetAttendanceRecord->SetHeader("Content-Type", "application/json");
				GetAttendanceRecord->SetContentAsString(RequestBody);
				GetAttendanceRecord->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetTimeTableDataRequest() Complete, %s, %s, %s, %s, %s, %s"), 
					*FString::FromInt(date.GetYear()), *FString::FromInt(date.GetMonth()), *FString::FromInt(date.GetDay()), *FString::FromInt(date.GetHour()), *FString::FromInt(date.GetMinute()), *Classroom);
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
		else
		{
			failureReason = "This function can be used in only server";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetTimeTableDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendGetTimeTableDataRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnGetTimeTableDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	FString failureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("reservation")) {
				TSharedPtr<FJsonObject> reservationObj = JsonObject->GetObjectField("reservation");
				TSharedPtr<FJsonObject> classroomObj = reservationObj->GetObjectField("Classroom");
				FString classroomStr = classroomObj->GetStringField("S");

				TSharedPtr<FJsonObject> contentObj = reservationObj->GetObjectField("Content");
				FString contentStr = contentObj->GetStringField("S");

				TSharedPtr<FJsonObject> startTimeObj = reservationObj->GetObjectField("StartTime");
				FString startTimeStr = startTimeObj->GetStringField("S");
				TArray<FString> startTimesArray;
				const TCHAR* spliters[] = {TEXT("-")};
				TArray<FString> startTimeHMArray;
				const TCHAR* hmSpliters[] = { TEXT(":") };
				startTimeStr.ParseIntoArray(startTimesArray, spliters, true);
				startTimesArray[3].ParseIntoArray(startTimeHMArray, hmSpliters, true);
				FDateTime startDate(FCString::Atoi(*startTimesArray[0]), FCString::Atoi(*startTimesArray[1]), FCString::Atoi(*startTimesArray[2]), FCString::Atoi(*startTimeHMArray[0]),
					FCString::Atoi(*startTimeHMArray[1]), 0, 0);
				
				TSharedPtr<FJsonObject> studentsObj = reservationObj->GetObjectField("StudentsSub");
				TArray<TSharedPtr<FJsonValue>> studentsJsonArray = studentsObj->GetArrayField("SS");
				TArray<FString> studentsStr;

				for(int i=0;i<studentsJsonArray.Num();++i)
				{
					if(studentsJsonArray[i]->AsString() != "None")
					{
						studentsStr.Add(studentsJsonArray[i]->AsString());
					}
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedGetTimeTableDataSuccessEvent(GetGameInstance(), startDate, classroomStr, contentStr, studentsStr);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetTimeTableDataResponseReceived() Complete"));
				return;
			}
			else if (JsonObject->HasField("error"))
			{
				failureReason = JsonObject->GetStringField("error");
			}
			else
			{
				failureReason = "JsonObject doesn't have reservation field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetTimeTableDataFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnGetTimeTableDataResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendGetAttendanceRecordRequest(FString studentSub)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;

	if (authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if (true == GetGameInstance()->IsDedicatedServerInstance())
		{
			failureReason = "This function can be used in only client";
		}
		else
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

			RequestObj->SetStringField("playerSub", studentSub);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetAttendanceRecord = pHttpModule->CreateRequest();
				GetAttendanceRecord->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnGetAttendanceRecordResponseReceived);
				GetAttendanceRecord->SetURL(ApiUrl + "/getattendancerecord");
				GetAttendanceRecord->SetVerb("POST");
				GetAttendanceRecord->SetHeader("Authorization", Authorizer);
				GetAttendanceRecord->SetHeader("Content-Type", "application/json");
				GetAttendanceRecord->SetContentAsString(RequestBody);
				GetAttendanceRecord->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendGetAttendanceRecordRequest() Complete, player sub: %s"), *studentSub);
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetAttendanceRecordFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendGetAttendanceRecordRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnGetAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerData")) {
				TSharedPtr<FJsonObject> playerDataObj = JsonObject->GetObjectField("playerData");
				TSharedPtr<FJsonObject> studentSubObj = playerDataObj->GetObjectField("PlayerSub");
				TSharedPtr<FJsonObject> studentNicknameObj = playerDataObj->GetObjectField("PlayerId");

				FString studentSubStr = studentSubObj->GetStringField("S");
				FString studentNicknameStr = studentNicknameObj->GetStringField("S");

				TArray<FString> records[5][3];// 1, records2, records3;

				FString contentsName;

				TArray<FString> contentsNames;
				contentsNames.Add("PhysicsTrain");
				contentsNames.Add("PhysicsSlope");
				contentsNames.Add("Electromagnetics");
				contentsNames.Add("Semiconductor");
				contentsNames.Add("ElectricCar");
				
				for (int contentCode = 0; contentCode < contentsNames.Num(); ++contentCode) {
					if (contentCode == 0 || contentCode == 1) {
						TSharedPtr<FJsonObject> attendanceObj = playerDataObj->GetObjectField(contentsNames[contentCode]);
						TArray<TSharedPtr<FJsonValue>> attendanceArray = attendanceObj->GetArrayField("SS");

						for (int i = 0; i < attendanceArray.Num(); ++i) {
							if (attendanceArray[i]->AsString() != "None") {
								records[contentCode][0].Add(attendanceArray[i]->AsString());
							}
						}
					}
					else {
						for (int i = 0; i < 3; ++i) {
							TSharedPtr<FJsonObject> attendanceObj = playerDataObj->GetObjectField(contentsNames[contentCode] + FString::FromInt(i + 1));
							TArray<TSharedPtr<FJsonValue>> attendanceArray = attendanceObj->GetArrayField("SS");

							for (int j = 0; j < attendanceArray.Num(); ++j) {
								if (attendanceArray[j]->AsString() != "None") {
									records[contentCode][i].Add(attendanceArray[j]->AsString());
								}
							}
						}
					}
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedGetAttendanceRecordSuccessEvent(GetGameInstance(), studentSubStr, studentNicknameStr,
						records[0][0], records[1][0],
						records[2][0], records[2][1], records[2][2],
						records[3][0], records[3][1], records[3][2],
						records[4][0], records[4][1], records[4][2]);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnGetAttendanceRecordResponseReceived() Complete"));
				return;
			}
			else if (JsonObject->HasField("error"))
			{
				failureReason = JsonObject->GetStringField("error");
			}
			else
			{
				failureReason = "JsonObject doesn't have playerData field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedGetAttendanceRecordFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnGetAttendanceRecordResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendModifyAttendanceRecordRequest(FString StudentSub, FString LectureStartedTime, FString Content)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;

	if (authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if (true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

			RequestObj->SetStringField("playerSub", StudentSub);
			RequestObj->SetStringField("newTime", LectureStartedTime);
			RequestObj->SetStringField("lecture", Content);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyAttendanceRecord = pHttpModule->CreateRequest();
				ModifyAttendanceRecord->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnModifyAttendanceRecordResponseReceived);
				ModifyAttendanceRecord->SetURL(ApiUrl + "/modifyattendancerecord");
				ModifyAttendanceRecord->SetVerb("POST");
				ModifyAttendanceRecord->SetHeader("Authorization", Authorizer);
				ModifyAttendanceRecord->SetHeader("Content-Type", "application/json");
				ModifyAttendanceRecord->SetContentAsString(RequestBody);
				ModifyAttendanceRecord->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendModifyAttendanceRecordRequest() Complete, player sub: %s, time: %s, content: %s"), *StudentSub, *LectureStartedTime, *Content);
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}
		}
		else
		{
			failureReason = "This function can be used in only server";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModifyAttendanceRecordFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendModifyAttendanceRecordRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnModifyAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerSub")) {
				FString studentSubStr = JsonObject->GetStringField("playerSub");

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedModifyAttendanceRecordSuccessEvent(GetGameInstance(), studentSubStr);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnModifyAttendanceRecordResponseReceived() Complete, student sub: %s"), *studentSubStr);
				return;
			}
			else if (JsonObject->HasField("error"))
			{
				failureReason = JsonObject->GetStringField("error");
			}
			else
			{
				failureReason = "JsonObject doesn't have playerSub field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}

	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModifyAttendanceRecordFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnModifyAttendanceRecordResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendModifyPlayerSpawnableRequest(FString UserId, TArray<FString> AddSpawnables, TArray<FString> DeleteSpawnables)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;
    	
	if(authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("playerSub", UserId);

			TArray<TSharedPtr<FJsonValue>> addArray;
			for (int i = 0; i < AddSpawnables.Num(); ++i) {
				addArray.Add(MakeShareable(new FJsonValueString(AddSpawnables[i])));
			}
			RequestObj->SetArrayField("addList", addArray);

			TArray<TSharedPtr<FJsonValue>> deleteArray;
			for (int i = 0; i < DeleteSpawnables.Num(); ++i) {
				deleteArray.Add(MakeShareable(new FJsonValueString(DeleteSpawnables[i])));
			}
			RequestObj->SetArrayField("deleteList", deleteArray);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyPlayerContents = pHttpModule->CreateRequest();
				ModifyPlayerContents->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnModifyPlayerSpawnableResponseReceived);
				ModifyPlayerContents->SetURL(ApiUrl + "/modifyplayerspawnable");
				ModifyPlayerContents->SetVerb("POST");
				ModifyPlayerContents->SetHeader("Authorization", Authorizer);
				ModifyPlayerContents->SetHeader("Content-Type", "application/json");
				ModifyPlayerContents->SetContentAsString(RequestBody);
				ModifyPlayerContents->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendModifyPlayerSpawnableRequest() Complete, UserId : %s, "), *UserId);
				for (int i = 0; i < AddSpawnables.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("AddSpawnable %d : %s, "), i, *AddSpawnables[i]);
				}
				for (int i = 0; i < DeleteSpawnables.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("DeleteSpawnable %d : %s, "), i, *DeleteSpawnables[i]);
				}
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}			
		}
		else
		{
			failureReason = "This function can be used in only server";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}
    	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerSpawnableFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendModifyPlayerSpawnableRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnModifyPlayerSpawnableResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerSub")) {
				FString playerId = JsonObject->GetStringField("playerSub");

				TArray<TSharedPtr<FJsonValue>> spawnableArray = JsonObject->GetArrayField("spawnable");
				TArray<FString> spawnables;
				for (int i = 0; i < spawnableArray.Num(); ++i) {
					if (spawnableArray[i]->AsString() != "None")
						spawnables.Add(spawnableArray[i]->AsString());
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerSpawnableSuccessEvent(GetGameInstance(), playerId, spawnables);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnModifyPlayerSpawnableResponseReceived() Complete, playerSub : %s, "), *playerId);
				for (int i = 0; i < spawnables.Num(); ++i){
					UE_LOG(LogTemp, Log, TEXT("Spawnable %d : %s, "), i, *spawnables[i]);
				}
				return;
			}
			else
			{
				failureReason = "JsonObject doesn't have playerSub field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerSpawnableFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnModifyPlayerSpawnableResponseReceived() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendModifyPlayerAccessibleRequest(FString UserId, TArray<FString> AddAccessibles, TArray<FString> DeleteAccessibles)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;
    	
	if(authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("playerSub", UserId);

			TArray<TSharedPtr<FJsonValue>> addArray;
			for (int i = 0; i < AddAccessibles.Num(); ++i) {
				addArray.Add(MakeShareable(new FJsonValueString(AddAccessibles[i])));
			}
			RequestObj->SetArrayField("addList", addArray);

			TArray<TSharedPtr<FJsonValue>> deleteArray;
			for (int i = 0; i < DeleteAccessibles.Num(); ++i) {
				deleteArray.Add(MakeShareable(new FJsonValueString(DeleteAccessibles[i])));
			}
			RequestObj->SetArrayField("deleteList", deleteArray);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyPlayerContents = pHttpModule->CreateRequest();
				ModifyPlayerContents->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnModifyPlayerAccessibleResponseReceived);
				ModifyPlayerContents->SetURL(ApiUrl + "/modifyplayeraccessible");
				ModifyPlayerContents->SetVerb("POST");
				ModifyPlayerContents->SetHeader("Authorization", Authorizer);
				ModifyPlayerContents->SetHeader("Content-Type", "application/json");
				ModifyPlayerContents->SetContentAsString(RequestBody);
				ModifyPlayerContents->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendModifyPlayerAccessibleRequest() Complete, PlayerId : %s, "), *UserId);
				for (int i = 0; i < AddAccessibles.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("AddAccessibles %d : %s, "), i, *AddAccessibles[i]);
				}
				for (int i = 0; i < DeleteAccessibles.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("DeleteAccessibles %d : %s, "), i, *DeleteAccessibles[i]);
				}
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}			
		}
		else
		{
			failureReason = "This function can be used in only server";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}
    	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerAccessibleFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendModifyPlayerAccessibleRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnModifyPlayerAccessibleResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerSub")) {
				FString playerId = JsonObject->GetStringField("playerSub");

				TArray<TSharedPtr<FJsonValue>> accessibleArray = JsonObject->GetArrayField("accessible");
				TArray<FString> accessibles;
				for (int i = 0; i < accessibleArray.Num(); ++i) {
					if (accessibleArray[i]->AsString() != "None")
						accessibles.Add(accessibleArray[i]->AsString());
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerAccessibleSuccessEvent(GetGameInstance(), playerId, accessibles);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnModifyPlayerAccessibleResponseReceived() Complete, playerSub : %s, "), *playerId);
				for (int i = 0; i < accessibles.Num(); ++i){
					UE_LOG(LogTemp, Log, TEXT("Accessible %d : %s, "), i, *accessibles[i]);
				}
				return;
			}
			else
			{
				failureReason = "JsonObject doesn't have playerSub field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerAccessibleFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::ReceivedModfiyPlayerAccessibleFailureEvent() Error, %s"), *failureReason);
}

bool UDynamoDBSubsystem::SendModifyPlayerInventoryRequest(FString UserId, TArray<FString> AddItems, TArray<FString> DeleteItems)
{
	FString failureReason;
	auto authorizeSubsystem = GetGameInstance()->GetSubsystem<UAWSAuthorizeSubsystem>();
	FString Authorizer;
    	
	if(authorizeSubsystem != nullptr)
	{
		Authorizer = authorizeSubsystem->GetAuthorization();
		if(true == GetGameInstance()->IsDedicatedServerInstance())
		{
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("playerSub", UserId);

			TArray<TSharedPtr<FJsonValue>> addItemsArray;
			for (int i = 0; i < AddItems.Num(); ++i) {
				addItemsArray.Add(MakeShareable(new FJsonValueString(AddItems[i])));
			}
			RequestObj->SetArrayField("addItem", addItemsArray);

			TArray<TSharedPtr<FJsonValue>> deleteItemsArray;
			for (int i = 0; i < DeleteItems.Num(); ++i) {
				deleteItemsArray.Add(MakeShareable(new FJsonValueString(DeleteItems[i])));
			}
			RequestObj->SetArrayField("deleteItem", deleteItemsArray);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ModifyPlayerContents = pHttpModule->CreateRequest();
				ModifyPlayerContents->OnProcessRequestComplete().BindUObject(this, &UDynamoDBSubsystem::OnModifyPlayerInventoryResponseReceived);
				ModifyPlayerContents->SetURL(ApiUrl + "/modifyplayerinventory");
				ModifyPlayerContents->SetVerb("POST");
				ModifyPlayerContents->SetHeader("Authorization", Authorizer);
				ModifyPlayerContents->SetHeader("Content-Type", "application/json");
				ModifyPlayerContents->SetContentAsString(RequestBody);
				ModifyPlayerContents->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::SendModifyPlayerInventoryRequest() Complete, PlayerId : %s, "), *UserId);
				for (int i = 0; i < AddItems.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("AddItem %d : %s, "), i, *AddItems[i]);
				}
				for (int i = 0; i < DeleteItems.Num(); ++i) {
					UE_LOG(LogTemp, Log, TEXT("DeleteItem %d : %s, "), i, *DeleteItems[i]);
				}
				return true;
			}
			else
			{
				failureReason = "Serialize failure";
			}			
		}
		else
		{
			failureReason = "This function can be used in only server";
		}
	}
	else
	{
		failureReason = "Get Authorize Subsystem failure";
	}
    	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerInventoryFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::SendModifyPlayerInventoryRequest() Error, %s"), *failureReason);
	return false;
}

void UDynamoDBSubsystem::OnModifyPlayerInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerSub")) {
				FString playerId = JsonObject->GetStringField("playerSub");

				TArray<TSharedPtr<FJsonValue>> inventoryArray = JsonObject->GetArrayField("inventory");
				TArray<FString> inventory;
				for (int i = 0; i < inventoryArray.Num(); ++i) {
					if (inventoryArray[i]->AsString() != "None")
						inventory.Add(inventoryArray[i]->AsString());
				}

				if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
					IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerInventorySuccessEvent(GetGameInstance(), playerId, inventory);
				}
				UE_LOG(LogTemp, Log, TEXT("UDynamoDBSubsystem::OnModifyPlayerInventoryResponseReceived() Complete, playerSub : %s, "), *playerId);
				for (int i = 0; i < inventory.Num(); ++i){
					UE_LOG(LogTemp, Log, TEXT("Item %d : %s, "), i, *inventory[i]);
				}
				return;
			}
			else
			{
				failureReason = "JsonObject doesn't have playerSub field";
			}
		}
		else
		{
			failureReason = "Deserialize failure";
		}
	}
	else
	{
		failureReason = "bWasSuccessful is false";
	}
	
	if (GetGameInstance()->GetClass()->ImplementsInterface(UDynamoDBCallbackInterface::StaticClass())) {
		IDynamoDBCallbackInterface::Execute_ReceivedModfiyPlayerInventoryFailureEvent(GetGameInstance(), failureReason);
	}
	UE_LOG(LogTemp, Warning, TEXT("UDynamoDBSubsystem::OnModifyPlayerInventoryResponseReceived() Error, %s"), *failureReason);
}

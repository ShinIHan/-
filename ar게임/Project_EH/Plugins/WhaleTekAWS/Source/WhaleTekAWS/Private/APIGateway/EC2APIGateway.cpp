// Fill out your copyright notice in the Description page of Project Settings.


#include "APIGateway/EC2APIGateway.h"
#include "AWSFunctionLibrary.h"
#include "AWSSubsystem.h"
#include "Auth/CognitoAuth.h"
#include "Interfaces/IHttpResponse.h"
#include "..\..\Public\APIGateway\EC2APIGateway.h"

UEC2APIGateway::UEC2APIGateway()
{
	ApiUrl = UAWSFunctionLibrary::ReadFile("Urls/EC2ApiUrl.txt");

	UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::UEC2APIGateway()"));
}

UEC2APIGateway::~UEC2APIGateway()
{
	UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::~UEC2APIGateway()"));
}

void UEC2APIGateway::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::Initialize()"));
}

void UEC2APIGateway::Start(UAWSSubsystem* pSubsystem)
{
	Super::Start(pSubsystem);
	UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::Start()"));
}

void UEC2APIGateway::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::Shutdown()"));
	Super::Shutdown();
}

bool UEC2APIGateway::SendGetEC2StateTagNameRequest(FString TagName, FGetEC2StateTagNameSuccessEvent SuccessDelegate, FGetEC2StateTagNameFailureEvent FailureDelegate)
{
	FString FailureReason;
	UAuth* Auth = pAWSSubsystem->GetAuthorizer();

	if (Auth == nullptr) {
		FailureReason = "Authorizer is nullptr";
	}
	else {
		FString Token = Auth->GetAuthorization();

		if (Token.Len() > 0) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetEc2State = pHttpModule->CreateRequest();
			GetEc2State->OnProcessRequestComplete().BindWeakLambda(this,
				[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
					OnGetEC2StateTagNameResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
				});
			GetEc2State->SetURL(ApiUrl + "/getec2/" + TagName);
			GetEc2State->SetVerb("GET");
			GetEc2State->SetHeader("Authorization", Token);
			GetEc2State->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::SendGetEc2StateTagNameRequest() Complete"));
			return true;
		}
		else {
			FailureReason = "Token.Len() is zero";
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UEC2APIGateway::SendGetEc2StateTagNameRequest() Error, %s"), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UEC2APIGateway::OnGetEC2StateTagNameResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetEC2StateTagNameSuccessEvent SuccessDelegate, FGetEC2StateTagNameFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("body")) {
				TSharedPtr<FJsonObject> BodyObj = JsonObject->GetObjectField("body");
				if (BodyObj->HasField("Error")) {
					FailureReason = BodyObj->GetStringField("Error");
				}
				else {
					TArray<TSharedPtr<FJsonValue>> InstancesInfoArrobj = BodyObj->GetArrayField("InstancesInfo");
					TArray<FEC2Info> Ec2Infos;
					for (int i = 0; i < InstancesInfoArrobj.Num(); ++i) {
						TSharedPtr<FJsonObject> instancesObj = InstancesInfoArrobj[i]->AsObject();
						TArray<TSharedPtr<FJsonValue>> instanceArrObj = instancesObj->GetArrayField("Instances");
						TSharedPtr<FJsonObject> instanceObj = instanceArrObj[0]->AsObject();

						int EC2State = instanceObj->GetObjectField("State")->GetIntegerField("Code");
						if (EC2State != 48 && EC2State != 32) {
							FEC2Info temp;
							if (EC2State == 0) {
								temp.State = "Pending";
							}
							else if (EC2State == 16) {
								temp.State = "running";
							}
							else if (EC2State == 64) {
								temp.State = "stopping";
							}
							else if (EC2State == 80) {
								temp.State = "stopped";
							}
							temp.InstanceId = instanceObj->GetStringField("InstanceId");
							temp.PublicAddress = instanceObj->GetStringField("PublicIpAddress");//
							TArray<TSharedPtr<FJsonValue>> TagObj = instanceObj->GetArrayField("Tags");
							for (int j = 0; j < TagObj.Num(); ++j) {
								FString Key = TagObj[j]->AsObject()->GetStringField("Key");
								FString Value = TagObj[j]->AsObject()->GetStringField("Value");
								FTag TempTag;
								TempTag.Key = Key;
								TempTag.Value = Value;
								temp.Tags.Add(TempTag);
							}
							Ec2Infos.Add(temp);
						}
					}
					SuccessDelegate.ExecuteIfBound(Ec2Infos);
					UE_LOG(LogTemp, Log, TEXT("UEC2APIGateway::OnGetEc2StateResponseReceived() Success"));
					return;
				}
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

	UE_LOG(LogTemp, Warning, TEXT("UEC2APIGateway::OnGetEc2StateTagNameResponseReceived() Error, %s"), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}
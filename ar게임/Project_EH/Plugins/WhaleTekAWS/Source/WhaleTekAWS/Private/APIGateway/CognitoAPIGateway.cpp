// Fill out your copyright notice in the Description page of Project Settings.


#include "APIGateway/CognitoAPIGateway.h"
#include "AWSFunctionLibrary.h"
#include "AWSSubsystem.h"
#include "Auth/CognitoAuth.h"
#include "Interfaces/IHttpResponse.h"

UCognitoAPIGateway::UCognitoAPIGateway()
{
	ApiUrl = UAWSFunctionLibrary::ReadFile("Urls/CognitoApiUrl.txt");

	UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::UCognitoAPIGateway()"));
}

UCognitoAPIGateway::~UCognitoAPIGateway()
{
	UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::~UCognitoAPIGateway()"));
}

void UCognitoAPIGateway::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::Initialize()"));
}

void UCognitoAPIGateway::Start(UAWSSubsystem* pSubsystem)
{
	Super::Start(pSubsystem);
	UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::Start()"));
}

void UCognitoAPIGateway::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::Shutdown()"));
	Super::Shutdown();
}

bool UCognitoAPIGateway::SendInvalidateTokensRequest()
{
	UAuth* Auth = pAWSSubsystem->GetAuthorizer();
	
	if (Auth == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::SendInvalidateTokensRequest() Error, Authorizer is nullptr."));
	}
	else {
		UCognitoAuth* CognitoAuthorizer = Cast<UCognitoAuth>(Auth);

		if (CognitoAuthorizer != nullptr) {
			if (CognitoAuthorizer->GetAuthorization().Len() > 0) {
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InvalidateTokens = pHttpModule->CreateRequest();
				InvalidateTokens->SetURL(ApiUrl + "/invalidatetokens");
				InvalidateTokens->SetVerb("GET");
				InvalidateTokens->SetHeader("Authorization", CognitoAuthorizer->GetAuthorization());
				InvalidateTokens->ProcessRequest();
				UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::SendInvalidateTokensRequest() Complete"));
				return true;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::SendInvalidateTokensRequest() Error, AccessToken.Len() is zero."));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::SendInvalidateTokensRequest() Error, Cognito authorizer cast failed."));
		}
	}
	
	return false;
}

bool UCognitoAPIGateway::SendExchangeCodeForTokensRequest(FText ChangedUrl, FExchangeCodeForTokensSuccessEvent SuccessDelegate, FExchangeCodeForTokensFailureEvent FailureDelegate)
{
	FString BrowserUrl = ChangedUrl.ToString();
	FString Url;
	FString QueryParameters;

	FString FailureReason;

	UAuth* Auth = pAWSSubsystem->GetAuthorizer();
	
	if (Auth == nullptr) {
		FailureReason = "Authorizer is nullptr";
	}
	else {
		UCognitoAuth* CognitoAuth = Cast<UCognitoAuth>(Auth);
		if (CognitoAuth == nullptr) {
			FailureReason = "Cognito authorizer cast failed";
		}
		else {
			FString CallbackUrl = CognitoAuth->GetCallbackUrl();
			if (BrowserUrl.Split("?", &Url, &QueryParameters)) {
				if (Url.Equals(CallbackUrl)) {
					FString ParameterName;
					FString ParameterValue;

					if (QueryParameters.Split("=", &ParameterName, &ParameterValue)) {
						if (ParameterName.Equals("code")) {
							ParameterValue = ParameterValue.Replace(*FString("#"), *FString(""));

							TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
							RequestObj->SetStringField("Code", ParameterValue);

							FString RequestBody;
							TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

							if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
								TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ExchangeCodeForTokens = pHttpModule->CreateRequest();
								ExchangeCodeForTokens->OnProcessRequestComplete().BindWeakLambda(this,
									[this, SuccessDelegate, FailureDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
										OnExchangeCodeForTokensResponseReceived(Request, Response, bWasSuccessful, SuccessDelegate, FailureDelegate);
									});
								ExchangeCodeForTokens->SetURL(ApiUrl + "/exchangecodefortokens");
								ExchangeCodeForTokens->SetVerb("POST");
								ExchangeCodeForTokens->SetHeader("Content-Type", "application/json");
								ExchangeCodeForTokens->SetContentAsString(RequestBody);
								ExchangeCodeForTokens->ProcessRequest();
								UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::SendExchangeCodeForTokensRequest() Complete"));
								return true;
							}
							else {
								FailureReason = "Serialize failure";
							}
						}
						else {
							FailureReason = "code parameter is not included";
						}
					}
					else {
						FailureReason = "Parameter split failure";
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::SendExchangeCodeForTokensRequest() Error, %s"), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
	return false;
}

void UCognitoAPIGateway::OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FExchangeCodeForTokensSuccessEvent SuccessDelegate, FExchangeCodeForTokensFailureEvent FailureDelegate)
{
	FString FailureReason;

	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (true == JsonObject->HasField("body")) {
				auto BodyObject = JsonObject->GetObjectField("body");

				if (BodyObject->HasField("access_token") && BodyObject->HasField("id_token") && BodyObject->HasField("refresh_token")) {
					const FString NewAccessToken = BodyObject->GetStringField("access_token");
					const FString NewIdToken = BodyObject->GetStringField("id_token");
					const FString NewRefreshToken = BodyObject->GetStringField("refresh_token");
					UCognitoAuth* CognitoAuthorizer = Cast<UCognitoAuth>(pAWSSubsystem->GetAuthorizer());
					CognitoAuthorizer->SetCognitoTokens(NewAccessToken, NewIdToken, NewRefreshToken);
					UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::OnExchangeCodeForTokensResponseReceived() Complete, Access token : %s, Id token : %s, Refresh token : %s"), *NewAccessToken, *NewIdToken, *NewRefreshToken);

					SuccessDelegate.ExecuteIfBound();
					return;
				}
				else {
					FailureReason = "JsonObject doesn't have token field";
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

	UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::OnExchangeCodeForTokensResponseReceived() Error, %s"), *FailureReason);
	FailureDelegate.ExecuteIfBound(FailureReason);
}

void UCognitoAPIGateway::SendRetrieveNewTokensRequest()
{
	FString FailureReason;

	UAuth* Auth = pAWSSubsystem->GetAuthorizer();
	
	if (Auth == nullptr) {
		FailureReason = "Authorizer is nullptr";
	}
	else {
		UCognitoAuth* CognitoAuthorizer = Cast<UCognitoAuth>(Auth);

		if (CognitoAuthorizer == nullptr) {
			FailureReason = "Cognito authorizer cast failed";
		}
		else
		{
			if (CognitoAuthorizer->GetAuthorization().Len() > 0 && CognitoAuthorizer->GetRefreshToken().Len() > 0) {
				TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
				RequestObj->SetStringField("RefreshToken", CognitoAuthorizer->GetRefreshToken());

				FString RequestBody;
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

				if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
					TSharedRef<IHttpRequest, ESPMode::ThreadSafe> RetrieveNewTokens = pHttpModule->CreateRequest();
					RetrieveNewTokens->OnProcessRequestComplete().BindUObject(this, &UCognitoAPIGateway::OnRetrieveNewTokensResponseReceived);
					RetrieveNewTokens->SetURL(ApiUrl + "/retrievenewtokens");
					RetrieveNewTokens->SetVerb("PUT");
					RetrieveNewTokens->SetHeader("Content-Type", "application/json");
					RetrieveNewTokens->SetHeader("Authorization", CognitoAuthorizer->GetAuthorization());
					RetrieveNewTokens->SetContentAsString(RequestBody);
					RetrieveNewTokens->ProcessRequest();
					UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::SendRetrieveNewTokensRequest() Complete, Refresh token: %s"), *(CognitoAuthorizer->GetRefreshToken()));
					return;
				}
				else {
					pAWSSubsystem->GetGameInstance()->GetTimerManager().SetTimer(CognitoAuthorizer->RetrieveNewTokensHandle, this, &UCognitoAPIGateway::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
					FailureReason = "Serialize failure";
				}
			}
			else {
				FailureReason = "AccessToken.Len or RefreshToken.Len is Zero, Access token: " + CognitoAuthorizer->GetAuthorization() + ", Refresh token: " + CognitoAuthorizer->GetRefreshToken();
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::SendRetrieveNewTokensRequest() Error, %s"), *FailureReason);
}

void UCognitoAPIGateway::OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString FailureReason;
	UAuth* Auth = pAWSSubsystem->GetAuthorizer();

	if (Auth == nullptr) {
		FailureReason = "Authorizer is nullptr";
	}
	else {
		UCognitoAuth* CognitoAuthorizer = Cast<UCognitoAuth>(Auth);

		if (CognitoAuthorizer == nullptr) {
			FailureReason = "Cognito authorizer cast failed";
		}
		else
		{
			if (bWasSuccessful) {
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
					if (true == JsonObject->HasField("body")) {
						auto BodyObject = JsonObject->GetObjectField("body");

						if (BodyObject->HasField("AccessToken") && BodyObject->HasField("IdToken")) {
							CognitoAuthorizer->SetCognitoTokens(BodyObject->GetStringField("AccessToken"), BodyObject->GetStringField("IdToken"), CognitoAuthorizer->GetRefreshToken());
							UE_LOG(LogTemp, Log, TEXT("UCognitoAPIGateway::OnRetrieveNewTokensResponseReceived() Complete, Access token : %s, Id token : %s, Refresh token: %s"), *(BodyObject->GetStringField("AccessToken")), *(BodyObject->GetStringField("IdToken")), *(CognitoAuthorizer->GetRefreshToken()));
							return;
						}
						else {
							FailureReason = "JsonObject doesn't have access token or id token field";
						}
					}
					else {
						FailureReason = "JsonObject doesn't have body field";
					}
				}
				else {
					pAWSSubsystem->GetGameInstance()->GetTimerManager().SetTimer(CognitoAuthorizer->RetrieveNewTokensHandle, this, &UCognitoAPIGateway::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
					FailureReason = "Deserialize failure";
				}
			}
			else {
				pAWSSubsystem->GetGameInstance()->GetTimerManager().SetTimer(CognitoAuthorizer->RetrieveNewTokensHandle, this, &UCognitoAPIGateway::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
				FailureReason = "bWasSuccessful is false";
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("UCognitoAPIGateway::OnRetrieveNewTokensResponseReceived() Error, %s"), *FailureReason);
}
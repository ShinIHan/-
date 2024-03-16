// Fill out your copyright notice in the Description page of Project Settings.


#include "Authorize/CognitoAuthorizer.h"
#include "Authorize/AuthorizerCallbackInterface.h"
#include "AWSBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "IWebBrowserCookieManager.h"
#include "IWebBrowserSingleton.h"
#include "WebBrowserModule.h"
#include "Database/DynamoDBSubsystem.h"
#include "Interfaces/IHttpResponse.h"

UCognitoAuthorizer::UCognitoAuthorizer()
{
	LoginUrl = UAWSBlueprintFunctionLibrary::ReadFile("Urls/LoginUrl.txt");
	CallbackUrl = UAWSBlueprintFunctionLibrary::ReadFile("Urls/CallbackUrl.txt");

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::UCognitoAuthorizer()"));
}

UCognitoAuthorizer::~UCognitoAuthorizer()
{
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::~UCognitoAuthorizer()"));
}

void UCognitoAuthorizer::Initialize()
{
	Super::Initialize();

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::Initialize()"));
}

void UCognitoAuthorizer::Start(UGameInstance* GameInstance)
{
	Super::Start(GameInstance);
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::Start()"));
}

void UCognitoAuthorizer::Shutdown()
{
	pGameInstance->GetWorld()->GetTimerManager().ClearTimer(RetrieveNewTokensHandle);

	if(AccessToken.Len() > 0)
		SendInvalidateTokensRequest();

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::Shutdown()"));
	
	Super::Shutdown();
}

FString UCognitoAuthorizer::GetLoginUrlAndClearCookie()
{
	IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();

	if (WebBrowserSingleton != nullptr) {
		const TOptional<FString> DefaultContext;
		const TSharedPtr<IWebBrowserCookieManager> CookieManager = WebBrowserSingleton->GetCookieManager(DefaultContext);
		if (CookieManager.IsValid()) {
			CookieManager->DeleteCookies();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::GetLoginUrlAndClearCookie(), LoginUrl: %s"), *LoginUrl);
	return LoginUrl;
}

void UCognitoAuthorizer::SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken)
{
	AccessToken = NewAccessToken;
	IdToken = NewIdToken;
	RefreshToken = NewRefreshToken;

	if(pGameInstance != nullptr)
		pGameInstance->GetWorld()->GetTimerManager().SetTimer(RetrieveNewTokensHandle, this, &UCognitoAuthorizer::SendRetrieveNewTokensRequest, 1.0f, false, 3300.0f);
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::SetCognitoTokens(), Access token: %s, Id token: %s, Refresh token: %s"), *AccessToken, *IdToken, *RefreshToken);
}

bool UCognitoAuthorizer::SendInvalidateTokensRequest()
{
	if (AccessToken.Len() > 0) {
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InvalidateTokens = GetHttpModule()->CreateRequest();
		InvalidateTokens->SetURL(GetApiUrl() + "/invalidatetokens");
		InvalidateTokens->SetVerb("GET");
		InvalidateTokens->SetHeader("Authorization", AccessToken);
		InvalidateTokens->ProcessRequest();
		UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::SendInvalidateTokensRequest() Complete"));
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UCognitoAuthorizer::SendInvalidateTokensRequest() Error, AccessToken.Len() is zero."));
	return false;
}

void UCognitoAuthorizer::SendRetrieveNewTokensRequest()
{
	FString failureReason;
	
	if (AccessToken.Len() > 0 && RefreshToken.Len() > 0) {
		TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
		RequestObj->SetStringField("refreshToken", RefreshToken);

		FString RequestBody;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

		if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> RetrieveNewTokens = GetHttpModule()->CreateRequest();
			RetrieveNewTokens->OnProcessRequestComplete().BindUObject(this, &UCognitoAuthorizer::OnRetrieveNewTokensResponseReceived);
			RetrieveNewTokens->SetURL(GetApiUrl() + "/retrievenewtokens");
			RetrieveNewTokens->SetVerb("POST");
			RetrieveNewTokens->SetHeader("Content-Type", "application/json");
			RetrieveNewTokens->SetHeader("Authorization", AccessToken);
			RetrieveNewTokens->SetContentAsString(RequestBody);
			RetrieveNewTokens->ProcessRequest();
			UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::SendRetrieveNewTokensRequest() Complete, Refresh token: %s"), *RefreshToken);
			return;
		}
		else {
			pGameInstance->GetWorld()->GetTimerManager().SetTimer(RetrieveNewTokensHandle, this, &UCognitoAuthorizer::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
			failureReason = "Serialize failure";
		}
	}
	else {
		failureReason = "AccessToken.Len or RefreshToken.Len is Zero, Access token: " + AccessToken + ", Refresh token: " + RefreshToken;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UCognitoAuthorizer::SendRetrieveNewTokensRequest() Error, %s"), *failureReason);
}

void UCognitoAuthorizer::OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("accessToken") && JsonObject->HasField("idToken")) {
				SetCognitoTokens(JsonObject->GetStringField("accessToken"), JsonObject->GetStringField("idToken"), RefreshToken);
				UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::OnRetrieveNewTokensResponseReceived() Complete, Access token : %s, Id token : %s, Refresh token: %s"), *(JsonObject->GetStringField("accessToken")), *(JsonObject->GetStringField("idToken")), *RefreshToken);
				return;
			}
			else {
				failureReason = "JsonObject doesn't have accessToken or idToken field";
			}
		}
		else {
			pGameInstance->GetWorld()->GetTimerManager().SetTimer(RetrieveNewTokensHandle, this, &UCognitoAuthorizer::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
			failureReason = "Deserialize failure";
		}
	}
	else {
		pGameInstance->GetWorld()->GetTimerManager().SetTimer(RetrieveNewTokensHandle, this, &UCognitoAuthorizer::SendRetrieveNewTokensRequest, 1.0f, false, 30.0f);
		failureReason = "bWasSuccessful is false";
	}
	UE_LOG(LogTemp, Warning, TEXT("UCognitoAuthorizer::OnRetrieveNewTokensResponseReceived() Error, %s"), *failureReason);
}

bool UCognitoAuthorizer::SendExchangeCodeForTokensRequest(FText ChangedUrl)
{
	FString BrowserUrl = ChangedUrl.ToString();
	FString Url;
	FString QueryParameters;

	FString failureReason;
	
	if (BrowserUrl.Split("?", &Url, &QueryParameters)) {
		if (Url.Equals(CallbackUrl)) {
			FString ParameterName;
			FString ParameterValue;

			if (QueryParameters.Split("=", &ParameterName, &ParameterValue)) {
				if (ParameterName.Equals("code")) {
					ParameterValue = ParameterValue.Replace(*FString("#"), *FString(""));

					TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
					RequestObj->SetStringField(ParameterName, ParameterValue);

					FString RequestBody;
					TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

					if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
						TSharedRef<IHttpRequest, ESPMode::ThreadSafe> ExchangeCodeForTokens = GetHttpModule()->CreateRequest();
						ExchangeCodeForTokens->OnProcessRequestComplete().BindUObject(this, &UCognitoAuthorizer::OnExchangeCodeForTokensResponseReceived);
						ExchangeCodeForTokens->SetURL(GetApiUrl() + "/exchangecodefortokens");
						ExchangeCodeForTokens->SetVerb("POST");
						ExchangeCodeForTokens->SetHeader("Content-Type", "application/json");
						ExchangeCodeForTokens->SetContentAsString(RequestBody);
						ExchangeCodeForTokens->ProcessRequest();
						UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::SendExchangeCodeForTokensRequest() Complete"));
						return true;
					}
					else {
						failureReason = "Serialize failure";
					}
				}
				else {
					failureReason = "code parameter is not included";
				}
			}
			else {
				failureReason = "Parameter split failure";
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("UCognitoAuthorizer::SendExchangeCodeForTokensRequest() Error, %s"), *failureReason);
	if(false == failureReason.IsEmpty()){
		if (pGameInstance->GetClass()->ImplementsInterface(UAuthorizerCallbackInterface::StaticClass()))
			IAuthorizerCallbackInterface::Execute_ReceivedTokenCreateFailureEvent(pGameInstance, failureReason);
	}
	return false;
}

void UCognitoAuthorizer::OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString failureReason;
	
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("access_token") && JsonObject->HasField("id_token") && JsonObject->HasField("refresh_token")) {
				const FString NewAccessToken = JsonObject->GetStringField("access_token");
				const FString NewIdToken = JsonObject->GetStringField("id_token");
				const FString NewRefreshToken = JsonObject->GetStringField("refresh_token");
				SetCognitoTokens(NewAccessToken, NewIdToken, NewRefreshToken);
				UE_LOG(LogTemp, Log, TEXT("UCognitoAuthorizer::OnExchangeCodeForTokensResponseReceived() Complete, Access token : %s, Id token : %s, Refresh token : %s"), *NewAccessToken, *NewIdToken, *NewRefreshToken);

				auto dbSubSystem = pGameInstance->GetSubsystem<UDynamoDBSubsystem>();
				if(dbSubSystem != nullptr) {
					dbSubSystem->SendGetPlayerDataRequest();
				}

				if(pGameInstance->GetClass()->ImplementsInterface(UAuthorizerCallbackInterface::StaticClass()))
					IAuthorizerCallbackInterface::Execute_ReceivedTokenCreateSuccessEvent(pGameInstance);
				return;
			}
			else {
				failureReason = "JsonObject doesn't have tokenField";
			}
		}
		else {
			failureReason = "Deserialize failure";
		}
	}
	else {
		failureReason = "bWasSuccessful is false";
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UCognitoAuthorizer::OnExchangeCodeForTokensResponseReceived() Error, %s"), *failureReason);
	if(pGameInstance->GetClass()->ImplementsInterface(UAuthorizerCallbackInterface::StaticClass()))
		IAuthorizerCallbackInterface::Execute_ReceivedTokenCreateFailureEvent(pGameInstance, failureReason);
}

FString UCognitoAuthorizer::GetAccessToken() const
{
	return AccessToken;
}

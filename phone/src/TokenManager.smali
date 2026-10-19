# Byte-level ground truth (smali) from classes2.dex
# Java best-effort: src-java/club/sjrc/app/manager/TokenManager.java

.class public final Lclub/sjrc/app/manager/TokenManager;
.super Ljava/lang/Object;
.source "TokenManager.kt"

# Lclub/sjrc/app/manager/TokenManager;
.field public static final INSTANCE:Lclub/sjrc/app/manager/TokenManager;
# Ljava/lang/String;
.field public static accessToken:Ljava/lang/String;
# Ljava/lang/String;
.field public static email:Ljava/lang/String;
# J
.field public static lastSavedTime:J
# Ljava/lang/String;
.field public static refreshToken:Ljava/lang/String;
# Lclub/sjrc/app/net/entity/UserInfoEntity;
.field public static userInfoEntity:Lclub/sjrc/app/net/entity/UserInfoEntity;
# Ljava/lang/String;
.field public static version:Ljava/lang/String;

.method public static <clinit>()V
    .registers 1   # 0 in
    new-instance v0, Lclub/sjrc/app/manager/TokenManager;
    invoke-direct v0, Lclub/sjrc/app/manager/TokenManager;-><init>()V
    sput-object  v0, Lclub/sjrc/app/manager/TokenManager;->INSTANCE Lclub/sjrc/app/manager/TokenManager;
    const-string v0, ""
    sput-object  v0, Lclub/sjrc/app/manager/TokenManager;->email Ljava/lang/String;
    const-string v0, "SUBSCRIPTION"
    sput-object  v0, Lclub/sjrc/app/manager/TokenManager;->version Ljava/lang/String;
    return-void  
.end method

.method public <init>()V
    .registers 1   # 1 in
    invoke-direct v0, Ljava/lang/Object;-><init>()V
    return-void  
.end method

.method public final getEmail()Ljava/lang/String;
    .registers 2   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->email Ljava/lang/String;
    return-object v0
.end method

.method public final getRefreshToken()Ljava/lang/String;
    .registers 2   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->refreshToken Ljava/lang/String;
    if-nez       v0, +004h
    const-string v0, ""
    return-object v0
.end method

.method public final getSjrcVersion()Ljava/lang/String;
    .registers 2   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->version Ljava/lang/String;
    return-object v0
.end method

.method public final getToken()Ljava/lang/String;
    .registers 2   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->accessToken Ljava/lang/String;
    if-nez       v0, +004h
    const-string v0, ""
    return-object v0
.end method

.method public final getUserInfo()Lclub/sjrc/app/net/entity/UserInfoEntity;
    .registers 2   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->userInfoEntity Lclub/sjrc/app/net/entity/UserInfoEntity;
    return-object v0
.end method

.method public final isFullVersion()Z
    .registers 3   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->version Ljava/lang/String;
    const-string v1, "SUBSCRIPTION_REFERENCE"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v0
    return       v0
.end method

.method public final isLogin()Z
    .registers 3   # 1 in
    sget-object  v0, Lclub/sjrc/app/manager/TokenManager;->accessToken Ljava/lang/String;
    const/4      v1, 1
    if-eqz       v0, +00bh
    invoke-interface v0, Ljava/lang/CharSequence;->length()I
    move-result  v0
    if-nez       v0, +003h
    goto         +3h
    const/4      v0, 0
    goto         +2h
    move         v0, v1
    xor-int/2addr v0, v1
    return       v0
.end method

.method public final logout()V
    .registers 2   # 1 in
    const/4      v0, 0
    sput-object  v0, Lclub/sjrc/app/manager/TokenManager;->accessToken Ljava/lang/String;
    sput-object  v0, Lclub/sjrc/app/manager/TokenManager;->refreshToken Ljava/lang/String;
    return-void  
.end method

.method public final updateSavedToken(Ljava/lang/String; Ljava/lang/String;)V
    .registers 5   # 3 in
    sput-object  v3, Lclub/sjrc/app/manager/TokenManager;->accessToken Ljava/lang/String;
    sput-object  v4, Lclub/sjrc/app/manager/TokenManager;->refreshToken Ljava/lang/String;
    invoke-static Ljava/lang/System;->currentTimeMillis()J
    move-result-wide v0
    sput-wide    v0, Lclub/sjrc/app/manager/TokenManager;->lastSavedTime J
    invoke-virtual v2, v3, Lclub/sjrc/app/manager/TokenManager;->updateTokenInfo(Ljava/lang/String;)V
    return-void  
.end method

.method public final updateToken()V
    .registers 8   # 1 in
    invoke-static Ljava/lang/System;->currentTimeMillis()J
    move-result-wide v0
    sget-wide    v2, Lclub/sjrc/app/manager/TokenManager;->lastSavedTime J
    sub-long/2addr v0, v2
    const-wide/32 v2, 600000
    cmp-long     v0, v0, v2
    if-gez       v0, +003h
    return-void  
    sget-object  v1, Lkotlinx/coroutines/GlobalScope;->INSTANCE Lkotlinx/coroutines/GlobalScope;
    const/4      v2, 0
    const/4      v3, 0
    new-instance v4, Lclub/sjrc/app/manager/TokenManager$updateToken$1;
    const/4      v0, 0
    invoke-direct v4, v0, Lclub/sjrc/app/manager/TokenManager$updateToken$1;-><init>(Lkotlin/coroutines/Continuation;)V
    const/4      v5, 3
    const/4      v6, 0
    invoke-static/range v1 ... v6, Lkotlinx/coroutines/BuildersKt;->launch$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method public final updateTokenInfo(Ljava/lang/String;)V
    .registers 4   # 2 in
    sget-object  v0, Lclub/sjrc/app/utils/JWTUtils;->INSTANCE Lclub/sjrc/app/utils/JWTUtils;
    invoke-virtual v0, v3, Lclub/sjrc/app/utils/JWTUtils;->decoded(Ljava/lang/String;)Ljava/lang/String;
    move-result-object v3
    new-instance v0, Lorg/json/JSONObject;
    invoke-direct v0, v3, Lorg/json/JSONObject;-><init>(Ljava/lang/String;)V
    const-string v3, "email"
    invoke-virtual v0, v3, Lorg/json/JSONObject;->has(Ljava/lang/String;)Z
    move-result  v1
    if-eqz       v1, +010h
    invoke-virtual v0, v3, Lorg/json/JSONObject;->getString(Ljava/lang/String;)Ljava/lang/String;
    move-result-object v3
    invoke-static v3, Lkotlin/text/StringsKt;->trim(Ljava/lang/CharSequence;)Ljava/lang/CharSequence;
    move-result-object v3
    invoke-virtual v3, Ljava/lang/Object;->toString()Ljava/lang/String;
    move-result-object v3
    sput-object  v3, Lclub/sjrc/app/manager/TokenManager;->email Ljava/lang/String;
    const-string/jumbo v3, version
    invoke-virtual v0, v3, Lorg/json/JSONObject;->has(Ljava/lang/String;)Z
    move-result  v1
    if-eqz       v1, +010h
    invoke-virtual v0, v3, Lorg/json/JSONObject;->getString(Ljava/lang/String;)Ljava/lang/String;
    move-result-object v3
    invoke-static v3, Lkotlin/text/StringsKt;->trim(Ljava/lang/CharSequence;)Ljava/lang/CharSequence;
    move-result-object v3
    invoke-virtual v3, Ljava/lang/Object;->toString()Ljava/lang/String;
    move-result-object v3
    sput-object  v3, Lclub/sjrc/app/manager/TokenManager;->version Ljava/lang/String;
    return-void  
.end method

.method public final updateUserInfoEntity(Lclub/sjrc/app/net/entity/UserInfoEntity;)V
    .registers 2   # 2 in
    sput-object  v1, Lclub/sjrc/app/manager/TokenManager;->userInfoEntity Lclub/sjrc/app/net/entity/UserInfoEntity;
    return-void  
.end method


# Byte-level ground truth (smali) from classes2.dex
# Java best-effort: src-java/club/sjrc/app/net/interceptor/ParameterInterceptor.java

.class public final Lclub/sjrc/app/net/interceptor/ParameterInterceptor;
.super Ljava/lang/Object;
.implements Lokhttp3/Interceptor;
.source "ParameterInterceptor.kt"

.method public <init>()V
    .registers 1   # 1 in
    invoke-direct v0, Ljava/lang/Object;-><init>()V
    return-void  
.end method

.method public intercept(Lokhttp3/Interceptor$Chain;)Lokhttp3/Response;
    .registers 9   # 2 in
    invoke-interface v8, Lokhttp3/Interceptor$Chain;->request()Lokhttp3/Request;
    move-result-object v0
    invoke-virtual v0, Lokhttp3/Request;->url()Lokhttp3/HttpUrl;
    move-result-object v1
    invoke-virtual v1, Lokhttp3/HttpUrl;->newBuilder()Lokhttp3/HttpUrl$Builder;
    move-result-object v1
    invoke-virtual v1, Lokhttp3/HttpUrl$Builder;->build()Lokhttp3/HttpUrl;
    move-result-object v1
    invoke-virtual v0, Lokhttp3/Request;->newBuilder()Lokhttp3/Request$Builder;
    move-result-object v2
    invoke-virtual v2, v1, Lokhttp3/Request$Builder;->url(Lokhttp3/HttpUrl;)Lokhttp3/Request$Builder;
    move-result-object v1
    sget-object  v2, Lclub/sjrc/app/manager/TokenManager;->INSTANCE Lclub/sjrc/app/manager/TokenManager;
    invoke-virtual v2, Lclub/sjrc/app/manager/TokenManager;->getToken()Ljava/lang/String;
    move-result-object v2
    invoke-interface v2, Ljava/lang/CharSequence;->length()I
    move-result  v3
    const/4      v4, 0
    if-lez       v3, +004h
    const/4      v3, 1
    goto         +2h
    move         v3, v4
    if-eqz       v3, +043h
    const-string/jumbo v3, x-auth-schema
    invoke-virtual v0, v3, Lokhttp3/Request;->header(Ljava/lang/String;)Ljava/lang/String;
    move-result-object v0
    new-instance v3, Ljava/lang/StringBuilder;
    invoke-direct v3, Ljava/lang/StringBuilder;-><init>()V
    const-string v5, "header = "
    invoke-virtual v3, v5, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    const-string v3, "CLIENT"
    const/4      v5, 2
    const/4      v6, 0
    invoke-static v0, v3, v4, v5, v6, Lkotlin/text/StringsKt;->equals$default(Ljava/lang/String; Ljava/lang/String; Z I Ljava/lang/Object;)Z
    move-result  v0
    if-eqz       v0, +025h
    const-string v0, "authorization"
    new-instance v3, Ljava/lang/StringBuilder;
    invoke-direct v3, Ljava/lang/StringBuilder;-><init>()V
    const-string v4, "Bearer "
    invoke-virtual v3, v4, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, v2, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;
    move-result-object v3
    invoke-virtual v1, v0, v3, Lokhttp3/Request$Builder;->addHeader(Ljava/lang/String; Ljava/lang/String;)Lokhttp3/Request$Builder;
    new-instance v0, Ljava/lang/StringBuilder;
    invoke-direct v0, Ljava/lang/StringBuilder;-><init>()V
    const-string v3, "authorization = "
    invoke-virtual v0, v3, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v0, v2, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    const-string/jumbo v0, x-current-locale
    sget-object  v2, Lclub/sjrc/app/manager/LanguageManage;->INSTANCE Lclub/sjrc/app/manager/LanguageManage;
    invoke-static Lcom/blankj/utilcode/util/ActivityUtils;->getTopActivity()Landroid/app/Activity;
    move-result-object v3
    invoke-virtual v2, v3, Lclub/sjrc/app/manager/LanguageManage;->getCurrentLanguageHeaderStr(Landroid/content/Context;)Ljava/lang/String;
    move-result-object v2
    invoke-virtual v1, v0, v2, Lokhttp3/Request$Builder;->addHeader(Ljava/lang/String; Ljava/lang/String;)Lokhttp3/Request$Builder;
    const-string/jumbo v0, x-current-platform
    const-string v2, "android"
    invoke-virtual v1, v0, v2, Lokhttp3/Request$Builder;->addHeader(Ljava/lang/String; Ljava/lang/String;)Lokhttp3/Request$Builder;
    const-string/jumbo v0, version
    const-string v2, "2.9.0"
    invoke-virtual v1, v0, v2, Lokhttp3/Request$Builder;->addHeader(Ljava/lang/String; Ljava/lang/String;)Lokhttp3/Request$Builder;
    invoke-virtual v1, Lokhttp3/Request$Builder;->build()Lokhttp3/Request;
    move-result-object v0
    invoke-interface v8, v0, Lokhttp3/Interceptor$Chain;->proceed(Lokhttp3/Request;)Lokhttp3/Response;
    move-result-object v8
    return-object v8
    move-exception v8
    invoke-virtual v8, Ljava/lang/Throwable;->printStackTrace()V
    new-instance v8, Ljava/io/IOException;
    invoke-direct v8, Ljava/io/IOException;-><init>()V
    throw        v8
.end method


# Byte-level ground truth (smali) from classes4.dex
# Java best-effort: src-java/com/sjrc/baseapp/net/RetrofitClient.java

.class public final Lcom/sjrc/baseapp/net/RetrofitClient;
.super Ljava/lang/Object;
.source "RetrofitClient.kt"

# Ljava/lang/String;
.field public static BASE_URL:Ljava/lang/String;
# Lcom/sjrc/baseapp/net/RetrofitClient;
.field public static final INSTANCE:Lcom/sjrc/baseapp/net/RetrofitClient;
# Ljava/util/ArrayList;
.field public static final list:Ljava/util/ArrayList;
# Lkotlin/Lazy;
.field public static final retrofit$delegate:Lkotlin/Lazy;

.method public static synthetic $r8$lambda$53M_kTasGRcpHFsMLC88MYIyPPY(Ljava/lang/String;)V
    .registers 1   # 1 in
    invoke-static v0, Lcom/sjrc/baseapp/net/RetrofitClient;->getOkClient$lambda$0(Ljava/lang/String;)V
    return-void  
.end method

.method public static <clinit>()V
    .registers 1   # 0 in
    new-instance v0, Lcom/sjrc/baseapp/net/RetrofitClient;
    invoke-direct v0, Lcom/sjrc/baseapp/net/RetrofitClient;-><init>()V
    sput-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->INSTANCE Lcom/sjrc/baseapp/net/RetrofitClient;
    new-instance v0, Ljava/util/ArrayList;
    invoke-direct v0, Ljava/util/ArrayList;-><init>()V
    sput-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->list Ljava/util/ArrayList;
    const-string v0, ""
    sput-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->BASE_URL Ljava/lang/String;
    sget-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient$retrofit$2;->INSTANCE Lcom/sjrc/baseapp/net/RetrofitClient$retrofit$2;
    invoke-static v0, Lkotlin/LazyKt;->lazy(Lkotlin/jvm/functions/Function0;)Lkotlin/Lazy;
    move-result-object v0
    sput-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->retrofit$delegate Lkotlin/Lazy;
    return-void  
.end method

.method public <init>()V
    .registers 1   # 1 in
    invoke-direct v0, Ljava/lang/Object;-><init>()V
    return-void  
.end method

.method public static final synthetic access$getBASE_URL$p()Ljava/lang/String;
    .registers 1   # 0 in
    sget-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->BASE_URL Ljava/lang/String;
    return-object v0
.end method

.method public static final synthetic access$getOkClient(Lcom/sjrc/baseapp/net/RetrofitClient;)Lokhttp3/OkHttpClient;
    .registers 1   # 1 in
    invoke-virtual v0, Lcom/sjrc/baseapp/net/RetrofitClient;->getOkClient()Lokhttp3/OkHttpClient;
    move-result-object v0
    return-object v0
.end method

.method public static final getOkClient$lambda$0(Ljava/lang/String;)V
    .registers 3   # 1 in
    const-string v0, "message"
    invoke-static v2, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    new-instance v0, Ljava/lang/StringBuilder;
    invoke-direct v0, Ljava/lang/StringBuilder;-><init>()V
    const-string v1, "log: "
    invoke-virtual v0, v1, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v0, v2, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    return-void  
.end method

.method public final addInterceptor(Lokhttp3/Interceptor;)V
    .registers 3   # 2 in
    const-string v0, "interceptor"
    invoke-static v2, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    sget-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->list Ljava/util/ArrayList;
    invoke-virtual v0, v2, Ljava/util/ArrayList;->add(Ljava/lang/Object;)Z
    return-void  
.end method

.method public final getList()Ljava/util/ArrayList;
    .registers 2   # 1 in
    sget-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->list Ljava/util/ArrayList;
    return-object v0
.end method

.method public final getOkClient()Lokhttp3/OkHttpClient;
    .registers 5   # 1 in
    new-instance v0, Lokhttp3/OkHttpClient$Builder;
    invoke-direct v0, Lokhttp3/OkHttpClient$Builder;-><init>()V
    sget-object  v1, Ljava/util/concurrent/TimeUnit;->SECONDS Ljava/util/concurrent/TimeUnit;
    const-wide/16 v2, 60
    invoke-virtual v0, v2, v3, v1, Lokhttp3/OkHttpClient$Builder;->callTimeout(J Ljava/util/concurrent/TimeUnit;)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    invoke-virtual v0, v2, v3, v1, Lokhttp3/OkHttpClient$Builder;->connectTimeout(J Ljava/util/concurrent/TimeUnit;)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    invoke-virtual v0, v2, v3, v1, Lokhttp3/OkHttpClient$Builder;->readTimeout(J Ljava/util/concurrent/TimeUnit;)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    invoke-virtual v0, v2, v3, v1, Lokhttp3/OkHttpClient$Builder;->writeTimeout(J Ljava/util/concurrent/TimeUnit;)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    const/4      v1, 1
    invoke-virtual v0, v1, Lokhttp3/OkHttpClient$Builder;->retryOnConnectionFailure(Z)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    const/4      v1, 0
    invoke-virtual v0, v1, Lokhttp3/OkHttpClient$Builder;->followRedirects(Z)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    new-instance v1, Lokhttp3/logging/HttpLoggingInterceptor;
    new-instance v2, Lcom/sjrc/baseapp/net/RetrofitClient$$ExternalSyntheticLambda0;
    invoke-direct v2, Lcom/sjrc/baseapp/net/RetrofitClient$$ExternalSyntheticLambda0;-><init>()V
    invoke-direct v1, v2, Lokhttp3/logging/HttpLoggingInterceptor;-><init>(Lokhttp3/logging/HttpLoggingInterceptor$Logger;)V
    sget-object  v2, Lokhttp3/logging/HttpLoggingInterceptor$Level;->BODY Lokhttp3/logging/HttpLoggingInterceptor$Level;
    invoke-virtual v1, v2, Lokhttp3/logging/HttpLoggingInterceptor;->setLevel(Lokhttp3/logging/HttpLoggingInterceptor$Level;)Lokhttp3/logging/HttpLoggingInterceptor;
    move-result-object v1
    invoke-virtual v0, v1, Lokhttp3/OkHttpClient$Builder;->addInterceptor(Lokhttp3/Interceptor;)Lokhttp3/OkHttpClient$Builder;
    move-result-object v0
    sget-object  v1, Lcom/sjrc/baseapp/net/RetrofitClient;->list Ljava/util/ArrayList;
    invoke-interface v1, Ljava/lang/Iterable;->iterator()Ljava/util/Iterator;
    move-result-object v1
    invoke-interface v1, Ljava/util/Iterator;->hasNext()Z
    move-result  v2
    if-eqz       v2, +00ch
    invoke-interface v1, Ljava/util/Iterator;->next()Ljava/lang/Object;
    move-result-object v2
    check-cast   v2, Lokhttp3/Interceptor;
    invoke-virtual v0, v2, Lokhttp3/OkHttpClient$Builder;->addInterceptor(Lokhttp3/Interceptor;)Lokhttp3/OkHttpClient$Builder;
    goto         -fh
    invoke-virtual v0, Lokhttp3/OkHttpClient$Builder;->build()Lokhttp3/OkHttpClient;
    move-result-object v0
    return-object v0
.end method

.method public final getRetrofit()Lretrofit2/Retrofit;
    .registers 3   # 1 in
    sget-object  v0, Lcom/sjrc/baseapp/net/RetrofitClient;->retrofit$delegate Lkotlin/Lazy;
    invoke-interface v0, Lkotlin/Lazy;->getValue()Ljava/lang/Object;
    move-result-object v0
    const-string v1, "getValue(...)"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->checkNotNullExpressionValue(Ljava/lang/Object; Ljava/lang/String;)V
    check-cast   v0, Lretrofit2/Retrofit;
    return-object v0
.end method

.method public final getService(Ljava/lang/Class;)Ljava/lang/Object;
    .registers 3   # 2 in
    const-string v0, "serviceClass"
    invoke-static v2, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    invoke-virtual v1, Lcom/sjrc/baseapp/net/RetrofitClient;->getRetrofit()Lretrofit2/Retrofit;
    move-result-object v0
    invoke-virtual v0, v2, Lretrofit2/Retrofit;->create(Ljava/lang/Class;)Ljava/lang/Object;
    move-result-object v2
    return-object v2
.end method

.method public final init(Ljava/lang/String;)V
    .registers 3   # 2 in
    const-string v0, "baseUrl"
    invoke-static v2, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    sput-object  v2, Lcom/sjrc/baseapp/net/RetrofitClient;->BASE_URL Ljava/lang/String;
    return-void  
.end method


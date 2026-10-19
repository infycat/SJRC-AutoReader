# Byte-level ground truth (smali) from classes4.dex
# Java best-effort: src-java/com/middle/network/BodyBuilder.java

.class public final Lcom/middle/network/BodyBuilder;
.super Ljava/lang/Object;
.source "BodyBuilder.kt"

# Lcom/middle/network/BodyBuilder$Companion;
.field public static final Companion:Lcom/middle/network/BodyBuilder$Companion;
# Ljava/util/Map;
.field private final map:Ljava/util/Map;

.method public static <clinit>()V
    .registers 2   # 0 in
    new-instance v0, Lcom/middle/network/BodyBuilder$Companion;
    const/4      v1, 0
    invoke-direct v0, v1, Lcom/middle/network/BodyBuilder$Companion;-><init>(Lkotlin/jvm/internal/DefaultConstructorMarker;)V
    sput-object  v0, Lcom/middle/network/BodyBuilder;->Companion Lcom/middle/network/BodyBuilder$Companion;
    return-void  
.end method

.method public <init>()V
    .registers 2   # 1 in
    invoke-direct v1, Ljava/lang/Object;-><init>()V
    new-instance v0, Ljava/util/HashMap;
    invoke-direct v0, Ljava/util/HashMap;-><init>()V
    iput-object  v0, v1, Lcom/middle/network/BodyBuilder;->map Ljava/util/Map;
    return-void  
.end method

.method public final put(Ljava/lang/String; Ljava/lang/Object;)Lcom/middle/network/BodyBuilder;
    .registers 4   # 3 in
    const-string v0, "key"
    invoke-static v2, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    const-string v0, "obj"
    invoke-static v3, v0, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object; Ljava/lang/String;)V
    iget-object  v0, v1, Lcom/middle/network/BodyBuilder;->map Ljava/util/Map;
    invoke-interface v0, v2, v3, Ljava/util/Map;->put(Ljava/lang/Object; Ljava/lang/Object;)Ljava/lang/Object;
    return-object v1
.end method

.method public final toBody()Lokhttp3/RequestBody;
    .registers 3   # 1 in
    sget-object  v0, Lcom/middle/network/BodyBuilder;->Companion Lcom/middle/network/BodyBuilder$Companion;
    iget-object  v1, v2, Lcom/middle/network/BodyBuilder;->map Ljava/util/Map;
    invoke-virtual v0, v1, Lcom/middle/network/BodyBuilder$Companion;->mapToRequestBody(Ljava/util/Map;)Lokhttp3/RequestBody;
    move-result-object v0
    return-object v0
.end method


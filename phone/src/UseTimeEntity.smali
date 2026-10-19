# Byte-level ground truth (smali) from classes2.dex
# Java best-effort: src-java/club/sjrc/app/net/entity/UseTimeEntity.java

.class public final Lclub/sjrc/app/net/entity/UseTimeEntity;
.super Ljava/lang/Object;
.source "ReadTimeEntity.kt"

# Ljava/lang/String;
.field private final sessionId:Ljava/lang/String;
# I
.field private final totalQuota:I
# I
.field private final usedQuota:I

.method public <init>(Ljava/lang/String; I I)V
    .registers 4   # 4 in
    invoke-direct v0, Ljava/lang/Object;-><init>()V
    iput-object  v1, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    iput         v2, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    iput         v3, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    return-void  
.end method

.method public static synthetic copy$default(Lclub/sjrc/app/net/entity/UseTimeEntity; Ljava/lang/String; I I I Ljava/lang/Object;)Lclub/sjrc/app/net/entity/UseTimeEntity;
    .registers 6   # 6 in
    and-int/lit8 v5, v4, 1
    if-eqz       v5, +004h
    iget-object  v1, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    and-int/lit8 v5, v4, 2
    if-eqz       v5, +004h
    iget         v2, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    and-int/lit8 v4, v4, 4
    if-eqz       v4, +004h
    iget         v3, v0, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    invoke-virtual v0, v1, v2, v3, Lclub/sjrc/app/net/entity/UseTimeEntity;->copy(Ljava/lang/String; I I)Lclub/sjrc/app/net/entity/UseTimeEntity;
    move-result-object v0
    return-object v0
.end method

.method public final component1()Ljava/lang/String;
    .registers 2   # 1 in
    iget-object  v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    return-object v0
.end method

.method public final component2()I
    .registers 2   # 1 in
    iget         v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    return       v0
.end method

.method public final component3()I
    .registers 2   # 1 in
    iget         v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    return       v0
.end method

.method public final copy(Ljava/lang/String; I I)Lclub/sjrc/app/net/entity/UseTimeEntity;
    .registers 5   # 4 in
    new-instance v0, Lclub/sjrc/app/net/entity/UseTimeEntity;
    invoke-direct v0, v2, v3, v4, Lclub/sjrc/app/net/entity/UseTimeEntity;-><init>(Ljava/lang/String; I I)V
    return-object v0
.end method

.method public equals(Ljava/lang/Object;)Z
    .registers 6   # 2 in
    const/4      v0, 1
    if-ne        v4, v5, +003h
    return       v0
    instance-of  v1, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;
    const/4      v2, 0
    if-nez       v1, +003h
    return       v2
    check-cast   v5, Lclub/sjrc/app/net/entity/UseTimeEntity;
    iget-object  v1, v4, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    iget-object  v3, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    invoke-static v1, v3, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v1
    if-nez       v1, +003h
    return       v2
    iget         v1, v4, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    iget         v3, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    if-eq        v1, v3, +003h
    return       v2
    iget         v1, v4, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    iget         v5, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    if-eq        v1, v5, +003h
    return       v2
    return       v0
.end method

.method public final getSessionId()Ljava/lang/String;
    .registers 2   # 1 in
    iget-object  v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    return-object v0
.end method

.method public final getTotalQuota()I
    .registers 2   # 1 in
    iget         v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    return       v0
.end method

.method public final getUsedQuota()I
    .registers 2   # 1 in
    iget         v0, v1, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    return       v0
.end method

.method public hashCode()I
    .registers 3   # 1 in
    iget-object  v0, v2, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    invoke-virtual v0, Ljava/lang/String;->hashCode()I
    move-result  v0
    mul-int/lit8 v0, v0, 31
    iget         v1, v2, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    invoke-static v1, Ljava/lang/Integer;->hashCode(I)I
    move-result  v1
    add-int/2addr v0, v1
    mul-int/lit8 v0, v0, 31
    iget         v1, v2, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    invoke-static v1, Ljava/lang/Integer;->hashCode(I)I
    move-result  v1
    add-int/2addr v0, v1
    return       v0
.end method

.method public final isRemainTimeLessThan20()Z
    .registers 4   # 1 in
    iget         v0, v3, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    iget         v1, v3, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    sub-int      v1, v0, v1
    int-to-float v1, v1
    int-to-float v0, v0
    const        v2, 1045220557
    mul-float/2addr v0, v2
    cmpg-float   v0, v1, v0
    if-gtz       v0, +004h
    const/4      v0, 1
    goto         +2h
    const/4      v0, 0
    return       v0
.end method

.method public final isUserQuotaValid()Z
    .registers 3   # 1 in
    iget         v0, v2, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    iget         v1, v2, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    if-lt        v0, v1, +004h
    const/4      v0, 1
    goto         +2h
    const/4      v0, 0
    return       v0
.end method

.method public toString()Ljava/lang/String;
    .registers 6   # 1 in
    iget-object  v0, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->sessionId Ljava/lang/String;
    iget         v1, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->totalQuota I
    iget         v2, v5, Lclub/sjrc/app/net/entity/UseTimeEntity;->usedQuota I
    new-instance v3, Ljava/lang/StringBuilder;
    invoke-direct v3, Ljava/lang/StringBuilder;-><init>()V
    const-string v4, "UseTimeEntity(sessionId="
    invoke-virtual v3, v4, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    const-string v0, ", totalQuota="
    invoke-virtual v3, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, v1, Ljava/lang/StringBuilder;->append(I)Ljava/lang/StringBuilder;
    const-string v0, ", usedQuota="
    invoke-virtual v3, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, v2, Ljava/lang/StringBuilder;->append(I)Ljava/lang/StringBuilder;
    const-string v0, ")"
    invoke-virtual v3, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v3, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;
    move-result-object v0
    return-object v0
.end method


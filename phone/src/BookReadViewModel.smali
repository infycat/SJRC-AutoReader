# Byte-level ground truth (smali) from classes2.dex
# Java best-effort: src-java/club/sjrc/app/business/book/viewModel/BookReadViewModel.java

.class public final Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;
.super Landroidx/lifecycle/ViewModel;
.source "BookReadViewModel.kt"

# Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$Companion;
.field public static final Companion:Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$Companion;
    = None
# J
.field public static final READ_USER_PERIOD:J
    = 3
# Lkotlin/Lazy;
.field private final atomicShowLessTimeDialog$delegate:Lkotlin/Lazy;
# I
.field private bookId:I
# Ljava/lang/String;
.field private bookSessionId:Ljava/lang/String;
# Lkotlin/Lazy;
.field private final countDownTimerUtils$delegate:Lkotlin/Lazy;
# I
.field private errorTime:I
# Landroidx/lifecycle/MutableLiveData;
.field private readStatusLiveData:Landroidx/lifecycle/MutableLiveData;
# Ljava/lang/String;
.field private version:Ljava/lang/String;

.method public static synthetic $r8$lambda$73u6dLsqBFbrL3LX9Vxh75U6kn4(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; J)V
    .registers 3   # 3 in
    invoke-static v0, v1, v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->startCountDown$lambda$0(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; J)V
    return-void  
.end method

.method public static synthetic $r8$lambda$7Dhn1l89Gn5tVdJrVJg1hGOd9kQ()V
    .registers 0   # 0 in
    invoke-static Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->startCountDown$lambda$1()V
    return-void  
.end method

.method public static <clinit>()V
    .registers 2   # 0 in
    new-instance v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$Companion;
    const/4      v1, 0
    invoke-direct v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$Companion;-><init>(Lkotlin/jvm/internal/DefaultConstructorMarker;)V
    sput-object  v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->Companion Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$Companion;
    return-void  
.end method

.method public <init>()V
    .registers 2   # 1 in
    invoke-direct v1, Landroidx/lifecycle/ViewModel;-><init>()V
    sget-object  v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$countDownTimerUtils$2;->INSTANCE Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$countDownTimerUtils$2;
    invoke-static v0, Lkotlin/LazyKt;->lazy(Lkotlin/jvm/functions/Function0;)Lkotlin/Lazy;
    move-result-object v0
    iput-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->countDownTimerUtils$delegate Lkotlin/Lazy;
    new-instance v0, Landroidx/lifecycle/MutableLiveData;
    invoke-direct v0, Landroidx/lifecycle/MutableLiveData;-><init>()V
    iput-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readStatusLiveData Landroidx/lifecycle/MutableLiveData;
    sget-object  v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$atomicShowLessTimeDialog$2;->INSTANCE Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$atomicShowLessTimeDialog$2;
    invoke-static v0, Lkotlin/LazyKt;->lazy(Lkotlin/jvm/functions/Function0;)Lkotlin/Lazy;
    move-result-object v0
    iput-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->atomicShowLessTimeDialog$delegate Lkotlin/Lazy;
    const-string v0, ""
    iput-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    return-void  
.end method

.method public static final synthetic access$getAtomicShowLessTimeDialog(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;)Ljava/util/concurrent/atomic/AtomicBoolean;
    .registers 1   # 1 in
    invoke-direct v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->getAtomicShowLessTimeDialog()Ljava/util/concurrent/atomic/AtomicBoolean;
    move-result-object v0
    return-object v0
.end method

.method public static final synthetic access$getBookId$p(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;)I
    .registers 1   # 1 in
    iget         v0, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->bookId I
    return       v0
.end method

.method public static final synthetic access$getBookSessionId$p(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;)Ljava/lang/String;
    .registers 1   # 1 in
    iget-object  v0, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->bookSessionId Ljava/lang/String;
    return-object v0
.end method

.method public static final synthetic access$getErrorTime$p(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;)I
    .registers 1   # 1 in
    iget         v0, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->errorTime I
    return       v0
.end method

.method public static final synthetic access$setBookSessionId$p(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; Ljava/lang/String;)V
    .registers 2   # 2 in
    iput-object  v1, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->bookSessionId Ljava/lang/String;
    return-void  
.end method

.method public static final synthetic access$setErrorTime$p(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; I)V
    .registers 2   # 2 in
    iput         v1, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->errorTime I
    return-void  
.end method

.method private final getAtomicShowLessTimeDialog()Ljava/util/concurrent/atomic/AtomicBoolean;
    .registers 2   # 1 in
    iget-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->atomicShowLessTimeDialog$delegate Lkotlin/Lazy;
    invoke-interface v0, Lkotlin/Lazy;->getValue()Ljava/lang/Object;
    move-result-object v0
    check-cast   v0, Ljava/util/concurrent/atomic/AtomicBoolean;
    return-object v0
.end method

.method private final getCountDownTimerUtils()Lclub/sjrc/app/utils/CountDownTimerUtils;
    .registers 2   # 1 in
    iget-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->countDownTimerUtils$delegate Lkotlin/Lazy;
    invoke-interface v0, Lkotlin/Lazy;->getValue()Ljava/lang/Object;
    move-result-object v0
    check-cast   v0, Lclub/sjrc/app/utils/CountDownTimerUtils;
    return-object v0
.end method

.method public static synthetic readPos$default(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; I Ljava/lang/String; Ljava/lang/Integer; F I Ljava/lang/Object;)V
    .registers 8   # 7 in
    and-int/lit8 v7, v6, 2
    const/4      v0, 0
    if-eqz       v7, +003h
    move-object  v3, v0
    and-int/lit8 v6, v6, 4
    if-eqz       v6, +003h
    move-object  v4, v0
    invoke-virtual v1, v2, v3, v4, v5, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readPos(I Ljava/lang/String; Ljava/lang/Integer; F)V
    return-void  
.end method

.method private final readRecordReading()V
    .registers 7   # 1 in
    invoke-static v6, Landroidx/lifecycle/ViewModelKt;->getViewModelScope(Landroidx/lifecycle/ViewModel;)Lkotlinx/coroutines/CoroutineScope;
    move-result-object v0
    const/4      v1, 0
    const/4      v2, 0
    new-instance v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readRecordReading$1;
    const/4      v4, 0
    invoke-direct v3, v6, v4, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readRecordReading$1;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; Lkotlin/coroutines/Continuation;)V
    const/4      v4, 3
    const/4      v5, 0
    invoke-static/range v0 ... v5, Lclub/sjrc/app/extensions/CoroutineExtentionsKt;->launchSafely$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method private final readTimeEnd()V
    .registers 7   # 1 in
    sget-object  v0, Lkotlinx/coroutines/GlobalScope;->INSTANCE Lkotlinx/coroutines/GlobalScope;
    const/4      v1, 0
    const/4      v2, 0
    new-instance v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeEnd$1;
    const/4      v4, 0
    invoke-direct v3, v6, v4, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeEnd$1;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; Lkotlin/coroutines/Continuation;)V
    const/4      v4, 3
    const/4      v5, 0
    invoke-static/range v0 ... v5, Lkotlinx/coroutines/BuildersKt;->launch$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method private final readTimeStart()V
    .registers 7   # 1 in
    invoke-static v6, Landroidx/lifecycle/ViewModelKt;->getViewModelScope(Landroidx/lifecycle/ViewModel;)Lkotlinx/coroutines/CoroutineScope;
    move-result-object v0
    const/4      v1, 0
    const/4      v2, 0
    new-instance v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeStart$1;
    const/4      v4, 0
    invoke-direct v3, v6, v4, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeStart$1;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; Lkotlin/coroutines/Continuation;)V
    const/4      v4, 3
    const/4      v5, 0
    invoke-static/range v0 ... v5, Lclub/sjrc/app/extensions/CoroutineExtentionsKt;->launchSafely$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method private final readTimeUsed()V
    .registers 10   # 1 in
    iget-object  v0, v9, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->bookSessionId Ljava/lang/String;
    if-eqz       v0, +00bh
    invoke-interface v0, Ljava/lang/CharSequence;->length()I
    move-result  v0
    if-nez       v0, +003h
    goto         +3h
    const/4      v0, 0
    goto         +2h
    const/4      v0, 1
    if-eqz       v0, +003h
    return-void  
    iget         v0, v9, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->errorTime I
    const-wide/16 v1, 3
    if-lez       v0, +004h
    int-to-long  v3, v0
    mul-long/2addr v1, v3
    invoke-static v9, Landroidx/lifecycle/ViewModelKt;->getViewModelScope(Landroidx/lifecycle/ViewModel;)Lkotlinx/coroutines/CoroutineScope;
    move-result-object v3
    const/4      v4, 0
    const/4      v5, 0
    new-instance v6, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeUsed$1;
    const/4      v0, 0
    invoke-direct v6, v9, v1, v2, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readTimeUsed$1;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; J Lkotlin/coroutines/Continuation;)V
    const/4      v7, 3
    const/4      v8, 0
    invoke-static/range v3 ... v8, Lclub/sjrc/app/extensions/CoroutineExtentionsKt;->launchSafely$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method private static final startCountDown$lambda$0(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; J)V
    .registers 3   # 3 in
    invoke-virtual v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->handReadTimeUse()V
    return-void  
.end method

.method private static final startCountDown$lambda$1()V
    .registers 0   # 0 in
    return-void  
.end method

.method public final getReadStatusLiveData()Landroidx/lifecycle/MutableLiveData;
    .registers 2   # 1 in
    iget-object  v0, v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readStatusLiveData Landroidx/lifecycle/MutableLiveData;
    return-object v0
.end method

.method public final handReadTimeEnd()V
    .registers 3   # 1 in
    iget-object  v0, v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    const-string v1, "SUBSCRIPTION"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v1
    if-eqz       v1, +006h
    invoke-direct v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readTimeEnd()V
    goto         +ch
    const-string v1, "REFERENCE"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v0
    if-eqz       v0, +005h
    invoke-virtual v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readRecordEnd()V
    return-void  
.end method

.method public final handReadTimeStart()V
    .registers 3   # 1 in
    iget-object  v0, v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    const-string v1, "SUBSCRIPTION"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v0
    if-eqz       v0, +005h
    invoke-direct v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readTimeStart()V
    return-void  
.end method

.method public final handReadTimeUse()V
    .registers 3   # 1 in
    iget-object  v0, v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    const-string v1, "SUBSCRIPTION"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v1
    if-eqz       v1, +006h
    invoke-direct v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readTimeUsed()V
    goto         +ch
    const-string v1, "REFERENCE"
    invoke-static v0, v1, Lkotlin/jvm/internal/Intrinsics;->areEqual(Ljava/lang/Object; Ljava/lang/Object;)Z
    move-result  v0
    if-eqz       v0, +005h
    invoke-direct v2, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readRecordReading()V
    return-void  
.end method

.method public final readPos(I Ljava/lang/String; Ljava/lang/Integer; F)V
    .registers 15   # 5 in
    invoke-static v10, Landroidx/lifecycle/ViewModelKt;->getViewModelScope(Landroidx/lifecycle/ViewModel;)Lkotlinx/coroutines/CoroutineScope;
    move-result-object v0
    const/4      v1, 0
    const/4      v2, 0
    new-instance v9, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readPos$1;
    const/4      v8, 0
    move-object  v3, v9
    move-object  v4, v12
    move-object  v5, v13
    move         v6, v14
    move         v7, v11
    invoke-direct/range v3 ... v8, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readPos$1;-><init>(Ljava/lang/String; Ljava/lang/Integer; F I Lkotlin/coroutines/Continuation;)V
    const/4      v4, 3
    const/4      v5, 0
    invoke-static/range v0 ... v5, Lclub/sjrc/app/extensions/CoroutineExtentionsKt;->launchSafely$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method public final readRecordEnd()V
    .registers 7   # 1 in
    sget-object  v0, Lkotlinx/coroutines/GlobalScope;->INSTANCE Lkotlinx/coroutines/GlobalScope;
    const/4      v1, 0
    const/4      v2, 0
    new-instance v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readRecordEnd$1;
    const/4      v4, 0
    invoke-direct v3, v6, v4, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$readRecordEnd$1;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel; Lkotlin/coroutines/Continuation;)V
    const/4      v4, 3
    const/4      v5, 0
    invoke-static/range v0 ... v5, Lkotlinx/coroutines/BuildersKt;->launch$default(Lkotlinx/coroutines/CoroutineScope; Lkotlin/coroutines/CoroutineContext; Lkotlinx/coroutines/CoroutineStart; Lkotlin/jvm/functions/Function2; I Ljava/lang/Object;)Lkotlinx/coroutines/Job;
    return-void  
.end method

.method public final setReadStatusLiveData(Landroidx/lifecycle/MutableLiveData;)V
    .registers 2   # 2 in
    iput-object  v1, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->readStatusLiveData Landroidx/lifecycle/MutableLiveData;
    return-void  
.end method

.method public final startCountDown()V
    .registers 4   # 1 in
    iget-object  v0, v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    new-instance v1, Ljava/lang/StringBuilder;
    invoke-direct v1, Ljava/lang/StringBuilder;-><init>()V
    const-string v2, "startCountDown version is -> "
    invoke-virtual v1, v2, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v1, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-direct v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->getCountDownTimerUtils()Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    invoke-virtual v0, Lclub/sjrc/app/utils/CountDownTimerUtils;->cancel()V
    invoke-direct v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->getCountDownTimerUtils()Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    const-wide   v1, 9999999000
    invoke-virtual v0, v1, v2, Lclub/sjrc/app/utils/CountDownTimerUtils;->setMillisInFuture(J)Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    const-wide/16 v1, 3000
    invoke-virtual v0, v1, v2, Lclub/sjrc/app/utils/CountDownTimerUtils;->setCountDownInterval(J)Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    new-instance v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$$ExternalSyntheticLambda0;
    invoke-direct v1, v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$$ExternalSyntheticLambda0;-><init>(Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;)V
    invoke-virtual v0, v1, Lclub/sjrc/app/utils/CountDownTimerUtils;->setTickDelegate(Lclub/sjrc/app/utils/CountDownTimerUtils$TickDelegate;)Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    new-instance v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$$ExternalSyntheticLambda1;
    invoke-direct v1, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel$$ExternalSyntheticLambda1;-><init>()V
    invoke-virtual v0, v1, Lclub/sjrc/app/utils/CountDownTimerUtils;->setFinishDelegate(Lclub/sjrc/app/utils/CountDownTimerUtils$FinishDelegate;)Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    invoke-virtual v0, Lclub/sjrc/app/utils/CountDownTimerUtils;->start()V
    return-void  
.end method

.method public final stopCountDown()V
    .registers 4   # 1 in
    iget-object  v0, v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    new-instance v1, Ljava/lang/StringBuilder;
    invoke-direct v1, Ljava/lang/StringBuilder;-><init>()V
    const-string v2, "stopCountDown "
    invoke-virtual v1, v2, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-virtual v1, v0, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    invoke-direct v3, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->getCountDownTimerUtils()Lclub/sjrc/app/utils/CountDownTimerUtils;
    move-result-object v0
    invoke-virtual v0, Lclub/sjrc/app/utils/CountDownTimerUtils;->cancel()V
    return-void  
.end method

.method public final updateBookId(I)V
    .registers 2   # 2 in
    iput         v1, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->bookId I
    return-void  
.end method

.method public final updateVersion(Ljava/lang/String;)V
    .registers 2   # 2 in
    iput-object  v1, v0, Lclub/sjrc/app/business/book/viewModel/BookReadViewModel;->version Ljava/lang/String;
    return-void  
.end method


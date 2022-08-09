= 並行実行制御


並行実行制御は英語では Concurrency control (CC) といいます。
並行実行制御とは、並行/並列にトランザクションを実行することを前提として、
Isolation (ACID の I) を担保するために必要な処理です。
Isolation は独立性とか分離性などと訳されますが、
要はトランザクション同士の実行が混ざらない性質を言います。
混ざらないとは何かについて厳密に考え始めると、Serializability の話になります。
Serializability については後述します。

並行実行制御を行う手法を CC protocol と呼びます。
CC protocol の仕事は、各トランザクションが要求する Read/write 実行の排他制御したり、
(Multi-version CC protocol の場合は)指定されたレコードのどの Version を読むかを決めたり、
Commit 処理時に Isolation が守られているかチェックをしてダメなら System abort させたりする処理などです。
具体的にどのようなデータ構造とアルゴリズムを用いるかは CC protocol によって異なります。


== 直列実行ではもったいない

トランザクションを直列にひとつずつ実行すれば Isolation は完璧です。
昔は直列で良かったんじゃないかと思われそうですが、
CPU がひとつしかなくても HDD が遅かったため、IO 実行中に CPU がヒマしているのは
リソースがもったいないわけで、並行実行したいというモチベーションはありました。
現代はひとつのサーバだけ見ても CPU コアがたくさんあり、メモリも潤沢で、
永続ストレージも Flash memory やら NVRAM やら高速な選択肢が豊富になりました。
現代のアーキテクチャにおいて直列実行しかできないのでは、
昔にも増してリソースがもったいないということになります。


== 並行実行制御の難しさ

直感的には、まったく異なるレコードにアクセスするなら並列に実行しても
何の問題もなさそうじゃないか、と思います。はい、そのとおりです。
では、アクセスするレコード集合(ここでは Read/write set と呼びます)
がトランザクション実行前に把握できるでしょうか。
実は、限られたものを除けば、難しいです。
仮に One-shot トランザクションのように外部とのやりとりが途中でないようなトランザクションに限ったとしても、
一般に、Read/write set はデータベースの状態に依存して決まります。
トランザクションロジックは、
Database 状態と入力を引数にとり、変更後の Database 状態と出力を返り値とする
副作用のない関数、
@<tt>{function do_transaction(before_database, input)} @<m>{\rightarrow} @<tt>{(after_database, output)} と解釈できます。
@<tt>{do_transaction} の中身を静的解析 (@<tt>{before_database} なしで分かる情報を得るという意味です)
できたとしても、限られたケース以外では Read/write set を決定するのは無理ですね。
限られたケースとは、@<tt>{before_database} の状態に依存せずに Read/write set が確定する場合です。
例えば @<tt>{do_transaction} 内に条件分岐があり、それによって Read/write set が変わるとしたら、
実行前に確定させるのは無理です。

もし @<tt>{before_database} も使えれば Read/write set はトランザクション実行開始前に
ほぼ把握できるはずだ、と思ったあなた、それは正しいです。
しかし、@<tt>{before_database} を使うということは、
トランザクションロジックを実行してみることと多くの場合同じではないでしょうか。
グレーゾーンはありますが、典型的には、@<tt>{before_database} に依存しないトランザクションロジックであれば、
トランザクション開始時に Read/write set を確定させることができます。
そのような仮定の元で動作する DBMS を Deterministic DBMS と呼びます。
読んだデータに基づく条件分岐や @<tt>{join} 操作など、Deterministic DBMS の仮定が成り立たない
ロジックはすぐに思いつきますので、Deterministic DBMS の適用範囲は相対的に狭くなります。
Deterministic DBMS ではない、すなわち、トランザクション開始時には
Read/write set が不明であるとの立場をとる DBMS を Non-deterministic DBMS と呼びます。
静的解析だとか過去のワークロードから Read/write set を推定などする手法も、
広い意味での Non-derministic DBMS に分類することにしましょう。

本書は Non-deterministic DBMS を作ることを想定します。
Non-deterministic DBMS の CC protocol は
@<tt>{do_transaction} を実行しながら Isolation を担保するために頑張る必要があります。


== Serializability

Serializablility と日本語では「直列化可能性」と訳されます。
トランザクションの理論では、各トランザクションは有限の長さの Read/write オペレーション列を持っています。
CC を処理する機構を Scheduler とよび、Scheduler は複数のトランザクションの
オペレーション同士を適切に順序づけたり、
Multi-version の場合は Read-from 関係を決定したりします。
Scheduler が生成したオペレーション順序(や Read-from 関係)を Schedule もしくは History といいます。

ここで、Mono-version model および Multi-version model について説明します。
Mono-version model は、各 Record がひとつしか Version を保持しない、という仮定を置き、
Write operation は前の Version を上書きしてそれ以降読めなくすること、
Read operation はそのとき存在する Version を読むこととする、理論上のデータベースとその処理モデルです。
すなわち Mono-version model は、オペレーション順序から Reads-from 関係が一意に決まるモデルで、
具体的には直前に同一 Record を書いたオペレーションが Reads-from 関係の対象となります。
よって、Mono-version model においては、オペレーション順序情報を Schedule として扱います。
一方で、Multi-version model は、各 Record につき、無限の Version を持てるという仮定を置き、
Write operation は新しい Version を生成し、
Read operation は過去に書かれたどんな Version でも読めるとする、理論上のモデルです。
現実にはもちろん無限には持てませんが。
Multi-version model は、Reads-from 関係が重要であり、その情報を Schedule として扱います。
CC protocol は Mono-version もしくは Multi-version のいずれかの世界を想定して設計されます。

ある Schedule が Serializable であるとは、
同じトランザクション集合を直列に実行した(Trivial schedule と呼びます。
Trivial schedule における Reads-from 関係は Mono-version model の考え方を用いて
直前に書かれたものを読むものとします) のと「等価」であることと定義されます。
トランザクションの数が @<m>{N} 個あれば、Trivial schedule は
トランザクションの並べ方の数すなわち @<m>{N!} 個存在しますが、
そのなかに与えられた Schedule と「等価」なものが存在すれば良いわけです。

「等価」とは何でしょうか。これには複数の考え方があります。


=== View Serializability

まず取りあげるのが、View が同じなら「等価」とみなす考え方です。
View とは、各トランザクションが、どのトランザクションの書いた値を読んだか、
すなわち Reads-from 関係を指します。
ある Schedule について、View が同じ Trivial schedule が存在するとき、
その Schedule は View-serializable であるといい、
View-serializable な Schedule からなる集合を VSR といいます。
Multi-version schedule の場合は Multi-version view serializable および MVSR と呼ばれます。
MVSR はこれまで考えられている中で一番広い Serializable な Schedule 空間です。
しかし、MVSR/VSR には扱いづらい点があります。
それは View 等価である Trivial schedule を同定するのが難しいとことです。
理論上では、与えられた Schedule が (Multi-version) view serializable であるかどうかを決定する問題は NP-complete です。
ただし、次々に実行されるトランザクションを処理していくオンラインスケジューラであれば、
この理論に基づく Protocol が必ずしも非現実的とは限りません。


=== Conflict Serializability

MVSR や VSR が難しいとすると、
現実的な CC protocol が作れる良い性質はないだろうかという話になります。
そのような性質は、あります。それは、競合関係(Conflicts)が同じなら「等価」とみなす考え方です。
具体的には、同一レコードにアクセスするトランザクションの関係のうち、
片方が Write をするものを競合関係と定義します。
競合関係は、オペレーションの実行順序を考慮して、
Write-read (w-r)、Write-write (w-w)、Read-write (r-w) の 3 つです。
これらは、それぞれ Flow dependency、Output dependency、Anti-dependency、とも呼ばれます。
Read-read (r-r) は競合とはみなしません。

ある Schedule と競合関係が同じ Trival schedule が存在するとき、
その Schedule は Conflict serializable であるといい、
Conflict serializable な Schedule からなる集合を CSR といいます。
CSR に含まれる Schedule は VSR や MVSR にもまた含まれます。
任意の Mono-version schedule について、
Conflict serializable であれば、View serializable であることが導けます。

CSR は原則として Mono-version model を前提としますが、
Multi-version model においても、Reads-from 関係を Write-read 関係とみなし、
上書きに相当する関係を Write-write とみなし、それに準じて Read-write 関係も定義することで、
同様に扱うことができます。
Conflict serializable は競合等価となる候補の Trivial scheudler が一意に決まるので、
判定問題が NP-complete ではなく P に属します。
よって、比較的扱いやすいです。


====[column] Multi-version model と Mono-version model の関係について

Serializability についての理論研究は Mono-version model から始まり、
後に Multi-version model に拡張されました。
Mono-version schedule では各トランザクションがどの Record を Read/write したか、という情報すなわち
Operation 集合、およびそれらの全順序 (半順序として扱う流儀もあります) すなわち Operation order を考え、
Reads-from 関係 (Version function という写像で表現することもあります)は
Operation order から一意に決まる (Standard version function と呼ばれます)
ものとして議論されていました。
一方で、Multi-version model の世界になると、Reads-from 関係こそが Serializability を判定するための
主な情報で、Operation order は必ずしも必要ではなくなりますが、
Mono-version model を拡張したという歴史的経緯でそれが残ってしまっています。

本来 Mono-version model は Multi-version model に含まれます。
よって、Operation order がない状況で Serializability や Recoverability などの議論をすべきです。
その端緒となる「Multiversion View Serializability の簡潔な定義」@<fn>{simple_mvsr_definition}
という記事を書きました。その記事には、
本来必要な Order とは Operation order ではなく、Transaction order であるということが書いてあります。
Trivial schedule も Transaction order を用いて定義します。

====[/column]

//footnote[simple_mvsr_definition][https://qiita.com/starpoz/items/266ab514bbc308d438a6]


=== Anomaly

(Multi-version) view serializable ではないということは、
View 等価な Trivial schedule がひとつも存在しないことを意味します。
ということは、どんな Trivial schedule を持ってきても、
読むべき Version を読めていない Read operation が存在するということです。
この Read operation の View は正常ではない、すなわち異常です。
これを Anomaly と呼び、Dirty read とか、Lost update など、典型的パターンには名前がついています。
しかし、名前がついている Anomaly を列挙していけば、View の異常を全て網羅できるとは考えない方が良いでしょう。
詳細が気になる人は、「いろんなAnomaly」@<fn>{footnote_various_anomaly} という記事がありますので参考にしてください。

//footnote[footnote_various_anomaly][いろんなAnomaly: https://qiita.com/kumagi/items/5ef5e404546736ebac49]



== CC protocol の分類

CC procotol は大きく分けて 4 種類あります。
2PL 系、TO 系、SI 系、Graph 系です。ひとつずつ説明します。


=== 2PL 系 CC protocol

2PL は Two-Phase Locking の略です。
MySQL InnoDB や Google Spanner などで採用されています。
名前の通り、各レコードを Lock (典型的には Reader-writer lock) を使って排他する方式です。
典型的な 2PL およびその亜種はレコード毎に Mutex object を用意し、Reader-writer lock を用いて排他制御を行います。
あるトランザクションがアクセスする Record は必ず Read ロックまたは Write ロックを取って、
他のトランザクションが触れないようにします。
Read ロック同士は共存できます。つまり、競合関係にあるトランザクション同士は排他制御されるというわけです。
2PL のルールは 1 つだけです。
トランザクションの実行はロックの成長(Growing)フェーズと縮退(Shrinking)フェーズが
それぞれひとつだけ存在することが求められます。
トランザクション実行中にひとつでも Unlock したらそれ以降 Lock はできません、ということです。
2PL を使って生成できる Schedule は理論上 CSR と等しいです。
2PL は Mono-version model 前提のプロトコルで、
「最新」のデータ以外を読むことは想定されません。
似た名前の 2V2PL とか MV2PL などという Multi-version のプロトコルが(少なくとも研究としては)あるようですが、
別のプロトコルです。

Read locking をする代わりに楽観的に読む OCC (Optimistic Concurrency Protocol) も、
広義には 2PL に含まれます。
OCC は Commit 処理のときに、Read set の内容を検証 (Verify) することで、
読んでから Verify までの間に他のトランザクションによって更新されなかったかどうかをチェックします。
もし更新されていなかったら結果としてその期間 Read lock していたのと同じ効果を得られたと解釈できます。
Write については Commit 処理まで遅らせたりはしますが、Lock します。
OCC にも Growing phase と Shrinking phase は存在します。
2013 年に研究論文として発表された Silo というプロトコルが、洗練されています。


=== TO 系 CC protocol

TO とは Timestamp ordering の略です。
CochroachDB などで採用されています。
Timestamp ordering の基本は、トランザクション開始時に、
ユニークな Timestamp を付与し、その Timestamp order を Serializable order とするように
実行する方式です。
@<m>{ts(t)} はトランザクション @<m>{t} に付与された Timestamp を表すとします。
トランザクション @<m>{t_1} が @<m>{t_2} の書いた @<m>{x} を読むときは、
@<m>{ts(t_2) < ts(t_1)} を満たす必要があります。自分より新しい Timestamp のついているトランザクションの
書き込んだ値を読んではいけないということです。
また、@<m>{ts(t_2) < ts(t_3) < ts(t_1)} となるような @<m>{x} を書いた @<m>{t_3} が存在しないようにする必要があります。
これは、タイミングやプロトコルの詳細次第ではありますが、
@<m>{t_3} 側を排除するか、@<m>{t_1} 側を排除するか、どちらかによって達成されます。
TO は Mono-version model の制約の元で設計することもあるし、
Multi-version model 用のもの (MVTO と呼ばれます) もあります。


=== SI 系 CC protocol

SI とは Snapshot isolation の略です。
Oracle、PostgreSQL、TiDB などで採用されています。
Snapshot isolation とは、トランザクション開始時に確定している Snapshot を読むことにして、
書くときは Write-write の競合のみを排除するという方式です。
残念ながら素の SI protocol は Serializable ではありません。
SI に専用の検証器 (Certifier) を組み合わせて Serializable にする取り組みが、
SSI (Serializable Snapshot Isolation) や SSN (Serial Safety Net) などの手法で提案されています。
検証器の仕事は、SI では通るけど Serializable にならないようなトランザクションを選んで排除することです。
多くの場合、Anti-dependency (r-w 競合) を記録し、その情報に基いて
リスクのある構造を検知することで対象トランザクションを排除します。


=== Graph 系 CC protocol

実用 DBMS では見たことがありませんが、
トランザクションを Vertex、その依存関係や制約を edge とする
Graph 構造をメンテナンスしながら、処理を進めるプロトコルです。

Conflicts に基づくのであれば、対応する Conflict graph という構造を、
View に基づくのであれば、MVSG という Graph 構造を使います。
どちらも、循環(Cycle)存在しないように Graph を構成できれば Serializable なので、
循環になる(なりそうな)トランザクションを排除 (Abort) させて処理を進めます。
Graph 構造のメンテナンスはオーバーヘッドの大きさが懸念されたり、
古い  Vertex の GC など、難しさがあります。


== S2PL プロトコル

CC protocol の具体例として、長らくデファクトスタンダードとして使われてきた
S2PL (Strict two-phase locking) について紹介します。

S2PL プロトコルは、2PL に従いますが、Write lock の解放を Commit 完了後に行う制約を追加で守る必要のあるプロトコルです。
これにより、S2PL は(適切に WAL 手法と連携する必要はありますが) Strictness も満たします。
もっと強い制約を要求する SS2PL (strong strict two-phase locking) というプロトコルは、
Write lock だけでなく Read lock の解放を Commit 完了後に行う制約を守る必要のあるプロトコルです。
SS2PL は Rigorousness も満たします。
つまり、2PL に対して、S2PL や SS2PL は 永続化 (WAL) のことも考慮されているプロトコルといえます。

なお、先に紹介した Silo は、CC protocol の仕事と Logging の永続化を分離しており
(元々は Early Lock Release という名前で議論された手法の本質を、Silo は踏襲したと解釈して良いでしょう)、
永続化の遅延がスループットに影響を与えないような工夫をしながらも Strong recoverability 相当の保証を実現します。
CC protocol と Logging の分離が今後の標準的な手法になるのは間違いないと私は思います。



== Serializable ではないプロトコル

世の中の DBMS 実装においては、
性能が出ないなどの理由で、Isolation の性質を完全には満たさない、
すなわち Serializable とはいえず、ワークロードによっては Anomaly が発生してしまう
CC protocol が多くの場所で使われています。
例えば、Read committed と呼ばれるプロトロルです。SI もそうでしたね。
Read committed は Read lock について 2PL のルールを満たさない S2PL を指すことがほとんどです。
アプリケーションによってはそれらのプロトコルでも問題とならないケースは確かにあると思いますが、
アプリケーションの設計者が問題が起きないように注意深く検討するべきなのは言うまでもありません。


== CC protocol の実装について

CC protocol はトランザクションの並行/並列実行を前提としますので、
インデクスも並列アクセスに対応しているデータ構造を使う必要があります。
2PL で使われる Reader-writer lock は CC のためのレコードアクセスの排他制御であって、
インデクスを構成するデータ構造への並列アクセスには専用の排他制御が必要になります。

CC プロトコル以外の実装を極力サボってプロトタイプを作りたい場合は、
ごくごく単純なデータベースとしてレコードの配列を用意して配列インデクスを Key と見做せば
データ構造専用の排他制御が不要で、とりあえず並列に動かすことはできます。
ただし、Key は @<m>{\\{0, 1, ..., N - 1\\\}} で固定ですし、
Insert や Delete 操作にも対応できませんので、
ごくごく単純なベンチマーク(YCSB など)しか実行できません。
Thread-unsafe なデータ構造を使いながら、並行実行させるためには、
例えば User-level thread (Green thread とも呼ばれます) が使えます。
C++ だと素朴にはできないのですが(coroutine のサポートがなされようとしているので期待)、
非同期実行の仕組みがあれば可能です。
Thread-safe なデータ構造を使ってしまえば、並列実行ができます。
自作するのは大変なので、既存の Concurrent hash table や Concurrent balanced tree を使うのも手ですね。


== その他の話題

これだけだと実用的なプロトコルには足りないのですが、キーワードのみを書いておきます。
気になった人は是非調べてみてください。

 * Deadlock prevension
 * Phantom protection
 * Early lock release
 * Starvation
 * Garbage collection (for multi-version CC protocol)
 * External consistency (non-stale reads)

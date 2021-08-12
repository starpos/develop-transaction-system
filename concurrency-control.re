= 並行実行制御


並行実行制御は英語では concurrency control (CC) といいます。
並行実行制御とは、並行/並列にトランザクションを実行することを前提として、
isolation (ACID の I) を担保するために必要な処理です。
Isolation は独立性とか分離性などと訳されますが、
要はトランザクション同士の実行が混ざらない性質を言います。
混ざらないとは何かについて厳密に考え始めると、serializability の話になります。
Serializability については後述します。

並行実行制御を行う手法を CC protocol と呼びます。
CC protocol の仕事は、各トランザクションが要求する read/write 実行の排他制御したり、
(multi-version CC protocol の場合は)指定されたレコードのどの version を読むかを決めたり、
commit 処理時に isolation が守られているかチェックをしてダメなら abort させたりする処理などです。
具体的にどのようなデータ構造とアルゴリズムを用いるかは CC protocol によって異なります。


== 直列実行ではもったいない

トランザクションを直列にひとつずつ実行すれば isolation は完璧です。
昔は直列で良かったんじゃないかと思われそうですが、
CPU がひとつしかなくても HDD が遅かったため、IO 実行中に CPU がヒマしているのは
リソースがもったいないわけで、並行実行したいというモチベーションはありました。
現代はひとつのサーバだけ見ても CPU コアがたくさんあり、メモリも潤沢で、
永続ストレージも flash memory やら NVRAM やら高速な選択肢が豊富になりました。
現代のアーキテクチャにおいて直列実行しかできないのでは、
昔にも増してリソースがもったいないということになります。


== 並行実行制御の難しさ

直感的には、まったく異なるレコードにアクセスするなら並列に実行しても
何の問題もなさそうじゃないか、と思います。はい、そのとおりです。
では、アクセスするレコード集合(ここでは read/write set と呼びます)
がトランザクション実行前に把握できるでしょうか。
実は、限られたものを除けば、難しいです。
仮に one-shot トランザクションのように外部とのやりとりが途中でないようなトランザクションに限ったとしても、
一般に、read/write set はデータベースの状態に依存して決まります。
トランザクションロジックは、
database 状態と入力を引数にとり、変更後の database 状態と出力を返り値とする
副作用のない関数、
@<tt>{function do_transaction(before_database, input) -> (after_database, output)} と解釈できます。
@<tt>{do_transaction} の中身を静的解析 (@<tt>{before_database} なしで分かる情報を得るという意味です)
できたとしても、限られたケース以外では read/write set を決定するのは無理ですね。
限られたケースとは、@<tt>{before_database} の状態に依存せずに read/write set が確定する場合です。
例えば @<tt>{do_transaction} 内に条件分岐があり、それによって read/write set が変わるとしたら、
実行前に確定させるのは無理です。

もし @<tt>{before_database} も使えれば read/write set はトランザクション実行開始前に
ほぼ把握できるはずだ、と思ったあなた、それは正しいです。
しかし、@<tt>{before_database} を使うということは、
トランザクションロジックを実行してみることと多くの場合同じではないでしょうか。
グレーゾーンはありますが、典型的には、@<tt>{before_database} に依存しないトランザクションロジックであれば、
トランザクション開始時に read/write set を確定させることができます。
そのような仮定の元で動作する DBMS を deterministic DBMS と呼びます。
読んだデータに基づく条件分岐や join 操作など、deterministic DBMS の仮定が成り立たない
ロジックはすぐに思いつきますので、deterministic DBMS の適用範囲は相対的に狭くなります。
Deterministic DBMS ではない、すなわち、トランザクション開始時には
read/write set が不明であるとの立場をとる DBMS を non-deterministic DBMS と呼びます。
静的解析だとか過去のワークロードから read/write set を推定などする手法も、
広い意味での non-derministic DBMS に分類することにしましょう。

本書は non-deterministic DBMS を作ることを想定します。
Non-deterministic DBMS の CC protocol は
@<tt>{do_transaction} を実行しながら isolation を担保するために頑張る必要があります。


== Serializability

Serializablility とは日本語では「直列化可能性」と訳されます。
トランザクションの理論では、各トランザクションは有限の長さの read/write オペレーション列を持っています。
CC を処理する機構を scheduler とよび、scheduler は複数のトランザクションの
オペレーション同士を適切に順序づけたり、
multi-version の場合は read-from 関係を決定したりします。
Scheduler が生成したオペレーション列(や read-from 関係)を schedule といいます。

ここで、mono-version schedule および multi-version schedule について説明します。
Mono-version schedule は、各 record がひとつしか version を保持しない、という仮定を置き、
write operation は前の version を上書きしてそれ以降読めなくすること、
read operation はそのとき存在する version を読むこととする、理論上の世界です。
すなわち mono-version schedule は、オペレーション列から reads-from 関係が一意に決まるモデルで、
具体的には直前に同一 record を書いたオペレーションが対象となります。
一方で、multi-version は、各 record につき、無限の version を持てるという仮定を置き、
write operation は新しい version を生成し、
read operation は過去に書かれたどんな version でも読めるとする、理論上の世界です。
現実にはもちろん無限には持てませんが。
CC protocol は mono-version もしくは multi-version のいずれかの世界を仮定して設計されます。

ある schedule が serializable であるとは、
同じトランザクション集合を直列に実行したことを表す schedule (serial mono-version schedule といいます)
と「等価」であることと定義されます。
トランザクションの数が @<m>{N} 個あれば、serial mono-version schedule は
トランザクションの並べ方の数すなわち @<m>{N!} 個存在しますが、
そのなかに与えられた schedule と「等価」であるものが存在すれば良いわけです。

「等価」とは何でしょうか。これには複数の考え方があります。


=== View Serializability

まず取りあげるのが、view が同じなら「等価」とみなす考え方です。
View とは、各トランザクションが、どのトランザクションの書いた値を読んだか、
すなわち reads-from 関係の集合を指します。
ある schedule について、view が同じ serial mono-version schedule が存在するとき
その schedule は view-serializable であるといい、
view-serializable な schedule からなる集合を VSR といいます。
これが、multi-version schedule の場合は multi-version view serializable および MVSR となるのですが、
view 等価となる対象は、serial mono-version schedule である必要があります。
MVSR はこれまで考えられている中で一番広い serializable な schedule 空間です。
しかし、MVSR/VSR には扱いづらい点があります。
それは view 等価である serial monoversion schedule を探すのが難しいということです。
理論上では、与えられた schedule が view serializable であるかどうかを決定する問題は NP-complete です。
ただし、次々に実行されるトランザクションを処理していくオンラインスケジューラであれば、
この理論に基く protocol が必ずしも非現実的とは限りません。


=== Conflict Serializability

MVSR や VSR が難しいとすると、
現実的な CC protocol が作れる良い性質はないだろうかという話になります。
そのような性質は、あります。それは、競合関係(conflicts)が同じなら「等価」とみなす考え方です。
具体的には、同一レコードにアクセスするトランザクションの関係のうち、
片方が write をするものを競合関係と定義します。
競合関係は、オペレーションの実行順序を考慮して、
write-read (w-r)、write-write (w-w)、read-write (r-w) の 3 つです。
これらは、それぞれ flow dependency、output dependency、anti-dependency、とも呼ばれます。
Read-read (r-r) は競合とはみなしません。
ある schedule と競合関係が同じ serial mono-version schedule が存在するとき、
その schedule は conflict serializable であるといい、
conflict serializable な schedule からなる集合を CSR といいます。

CSR は mono-version schedule を前提とします。
CSR に含まれる schedule は VSR や MVSR にもまた含まれます。
競合関係は view についてなにも言及していないですが、
任意の mono-version schedule について、
serial mono-versionh schedule と競合等価であれば、view 等価であることが導けます。

Multi-version schedule で CSR に近いものがあるとすれば、
view の等価性に加えて w-w の競合関係のみについて等価性を要求する schedule 空間です。
これは論文@<fn>{footnote_dmvsr}では DMVSR と呼ばれています。
CSR もそうですが、DMVSR は schedule 空間に属するかどうかの決定問題が NP-complete ではなく P に属します。
何故なら、検証対象の serial mono-version schedule がひとつしか存在しないからです。

//footnote[footnote_dmvsr][On Concurrency Control by Multiple Versions. Christos H. Papadimitriou and Paris C. Kanellakis. 1984. https://dl.acm.org/citation.cfm?id=318588]


====[column] Multi-version schedule と mono-version schedule について

歴史的理由から、serializability についての理論研究は mono-version schedule
から始まり、後に multi-version に拡張されました。
Mono-version schedule では各トランザクションがどの record を write/read したか、という情報すなわち
operation 集合、およびそれらの全順序 (半順序として扱う流儀もあります) すなわち operation order を考え、
reads-from 関係 (version function という写像で表現することもあります)は
operation order から一意に決まる (standard version function と呼ばれます)
ものとして議論されていました。
一方で、multi-version schedule の世界になると、reads-from こそが serializability を判定するための
主な情報で、operation order は必ずしも必要ではなくなりますが、
mono-version を拡張したという歴史的理由でそれが残ってしまっていると私は考えます。

本来 mono-version schedule は multi-version schedule の部分集合と見做せます。
よって、operation order がなくても、(multi-version) schedule を定義したり、
serializability の議論はできるはずで、mono-version schedule というのは、
そのような schedule に operation order という制約を加えることによって構成する、
という整理ができるはずです。
その触りとなる「Multiversion View Serializability の簡潔な定義」@<fn>{simple_mvsr_definition}
という記事を書きました。その記事には、
本来必要な order とは operation order ではなく、transaction order であるということが書いてあります。

====[/column]

//footnote[simple_mvsr_definition][https://qiita.com/starpoz/items/266ab514bbc308d438a6]


=== Anomaly

View serializable ではないということは、
view 等価な serial mono-version schedule が存在しないことを意味します。
ということは、どんな serial mono-version schedule を持ってきても、
読むべき version を読めていない read operation が存在するということです。
この read operation の view は正常ではない、すなわち異常です。
これを anomaly と呼び、dirty read とか、lost update など、典型的パターンには名前がついています。
名前がついている anomaly だけで view の異常を全て網羅できるとは考えない方が良いでしょう。
詳細が気になる人は、「いろんなAnomaly」@<fn>{footnote_various_anomaly} という記事がありますので参考にしてください。

//footnote[footnote_various_anomaly][いろんなAnomaly: https://qiita.com/kumagi/items/5ef5e404546736ebac49]



== CC protocol の分類

CC procotol は大きく分けて 4 種類あります。
2PL 系、TO 系、SI 系、Graph 系です。ひとつずつ説明します。


=== 2PL 系 protocol

2PL は Two-Phase Locking の略です。
MySQL InnoDB や Google Spanner などで採用されています。
名前の通り、各レコードを lock (典型的には reader-writer lock) を使って排他する方式です。
2PL およびその亜種はレコード毎に mutex object を用意し、reader-writer lock を用いて排他制御を行います。
アクセスする record は必ず read または write ロックを取って、
他のトランザクションが触れないようにします。
Read ロック同士は共存できます。つまり、競合関係にあるトランザクション同士は排他制御されるというわけです。
2PL のルールは 1 つだけです。
トランザクションの実行はロックの成長(growing)フェーズと縮退(shrinking)フェーズが
それぞれひとつだけ存在することが求められます。
トランザクション実行中にひとつでも unlock したらそれ以降 lock はできません、ということです。
2PL を使って生成できる schedule は理論上 CSR と等しいです。
2PL は mono-version schedule 前提のプロトコルで、
「最新」のデータ以外を読むことは想定されません。
似た名前の 2V2PL とか MV2PL などという multi-version のプロトコルが(少なくとも研究としては)あるようですが、
別のプロトコルです。

Read locking をする代わりに楽観的に読む OCC (Optimistic Concurrency Protocol) も、
広義には 2PL に含まれます。
OCC は commit 処理のときに、read set の内容を検証 (verify) することで、
読んでから verify までの間に他のトランザクションによって更新されなかったかどうかをチェックします。
もし更新されていなかったら結果としてその期間 read lock していたのと同じ効果を得られたと解釈できます。
Write については commit 処理まで遅らせたりはしますが、lock します。
OCC にも growing phase と shrinking phase は存在します。
2013 年に研究論文として発表された Silo というプロトコルが、洗練されています。


=== TO 系 protocol

TO とは Timestamp Ordering の略です。
CochroachDB などで採用されています。
Timestamp Ordering の基本は、トランザクション開始時に、
ユニークな timestamp を付与し、その timestamp order を serializable order とするように
実行する方式です。
@<m>{ts(t)} はトランザクション @<m>{t} に付与された timestamp を表すとします。
トランザクション @<m>{t_1} が @<m>{t_2} の書いた @<m>{x} を読むときは、
@<m>{ts(t_2) < ts(t_1)} を満たす必要があります。自分より新しい timestamp のついているトランザクションの
書き込んだ値を読んではいけないということです。
また、@<m>{ts(t_2) < ts(t_3) < ts(t_1)} となるような @<m>{x} を書いた @<m>{t_3} が存在しないようにする必要があります。
これは、タイミングやプロトコルの詳細次第ではありますが、
@<m>{t_3} 側を排除するか、@<m>{t_1} 側を排除するか、どちらかによって達成されます。
TO は mono-version の制約をつけることもできるし、つけない (MVTO と呼ばれます) こともできます。


=== SI 系 protocol

SI とは Snapshot Isolation の略です。
Oracle、PostgreSQL、TiDB などで採用されています。
Snapshot Isolation とは、トランザクション開始時に確定している snapshot を読むことにして、
書くときは write-write の競合のみを排除するという方式です。
残念ながら素の SI protocol は serializable ではありません。
SI に専用の検証器 (certifier) を組み合わせて serializable にする取り組みが、
SSI (Serializable Snapshot Isolation) や SSN (Serial Safety Net) などの名前で行われています。
検証器の仕事は、SI では通るけど serializable にならないようなトランザクションを選んで排除することです。
多くの場合、anti-dependency (r-w 競合) を記録し、その情報に基いて
リスクのある構造を検知することで対象トランザクションを排除します。


=== Graph 系 protocol

実用 DBMS では見たことがありませんが、
トランザクションを vertex、その依存関係や制約を edge とする
graph 構造をメンテナンスしながら、処理を進めるプロトコルです。

Conflict serializability に基づくのであれば、conflict graph という構造を、
Multi-version view serializability であれば、multi-version serialization graph という構造を使います。
どちらも、循環(cycle)存在しないように graph を構成できれば serializable なので、
循環になる(なりそうな)トランザクションを排除 (abort) させて処理を進めます。
Graph 構造のメンテナンスはオーバーヘッドの大きさが懸念されたり、
古い vertex の GC など、難しさがあると思います。


== S2PL プロトコル

CC protocol の具体例として、長らくデファクトスタンダードとして使われてきた
S2PL (Strict two-phase locking) について紹介します。

S2PL プロトコルは、2PL に従いますが、write lock の解放を commit 完了後に行う制約を追加で守る必要のあるプロトコルです。
これにより、S2PL は(適切に WAL 手法と連携する必要はありますが) strictness も満たします。
もっと強い制約を要求する SS2PL (strong strict two-phase locking) というプロトコルは、
write lock だけでなく read lock の解放を commit 完了後に行う制約を守る必要のあるプロトコルです。
SS2PL は rigorousness も満たします。
つまり、2PL に対して、S2PL や SS2PL は 永続化 (WAL) のことも考慮されているプロトコルといえます。




== Serializable ではないプロトコル

世の中の DBMS 実装においては、
性能が出ないなどの理由で、isolation の性質を完全には満たさない、
すなわち serializable とはいえず、ワークロードによっては anomaly が発生してしまう
CC protocol が多くの場所で使われています。
例えば、read committed と呼ばれるプロトロルです。SI もそうでしたね。
Read committed は read lock について 2PL のルールを満たさない S2PL を指すことがほとんどです。
アプリケーションによってはそれらのプロトコルでも問題とならないケースは確かにあると思いますが、
アプリケーションの設計者が問題が起きないように注意深く検討するべきなのは言うまでもありません。


== CC protocol の実装について

CC protocol はトランザクションの並行/並列 実行を前提としますので、
インデクスも並列アクセスに対応しているデータ構造を使う必要があります。
2PL で使われる reader-writer lock はCC のためのレコードアクセスの排他制御であって、
インデクスを構成するデータ構造への並列アクセスには専用の排他制御が必要になります。

CC プロトコル以外の実装を極力サボってプロトタイプを作りたい場合は、
ごくごく単純なデータベースとしてレコードの配列を用意して配列インデクスを key と見做せば
データ構造専用の排他制御が不要で、とりあえず並列に動かすことはできます。
ただし、key は @<m>{\\{0, 1, ..., N - 1\\\}} で固定ですし、
insert や delete 操作にも対応できませんので、
ごくごく単純なベンチマーク(YCSB など)しか実行できません。
Thread-unsafe なデータ構造を使いながら、並行実行させるためには、
例えば user-level thread (green thread とも呼ばれます) が使えます。
C++ だと素朴にはできないのですが(coroutine のサポートがなされようとしているので期待)、
非同期実行の仕組みがあれば可能です。
Thread-safe なデータ構造を使ってしまえば、並列実行ができます。
自作するのは大変なので、既存の concurrent hash table や concurrent balanced tree を使うのも手ですね。


== その他の話題

これだけだと実用的なプロトコルには足りないのですが、キーワードのみを書いておきます。
気になった人は是非調べてみてください。

 * Deadlock prevension
 * Phantom protection
 * Early lock release
 * Starvation
 * Garbage collection (for multi-version CC protocol)
 * External consistency (non-stale reads)

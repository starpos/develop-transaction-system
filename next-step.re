= DBMS を学ぶためのリファレンス設計 応用


前章「DBMS を学ぶためのリファレンス設計 基本」に従ってプロトタイプを作り、
動かしたりテストしたりして満足し、
次のステップに進みたいと思った方は、
楽しい並行実行制御やその他の改善を実装するための準備をしていきましょう。
本章では、向かうべき方向について概要のみを述べます。
詳しくは、キーワードを頼りにご自分で調べてみてください。
場合によっては専門の教科書や学術論文を読まないといけないこともあるでしょう。


== 向かうべき複数の方向性

まず、皆さんの興味がどの領域にあるか、勉強したい、取り組んでみたい、深めてみたいのはどこか、
を考えながら進めていただくのがが良いと思います。
@<chapref>{basic-design}では並行実行制御ができてませんから、
それをやってみたい、という方は多いと思います。
そのようなニーズを想定して本章では並行実行制御についての話を主に取り上げますが、
データ構造や、永続化、異なる前提や環境でのトランザクション処理、
分析処理、など、様々な発展的テーマがあります。

実用としての DBMS においても、まだまだ課題として解決しなければならないものが
ありますので、あなたがそれを解決してくれる人になるかも知れません。
もしそうなったら、本書を書いた人間として冥利に尽きます。


=={sec-direction-for-concurrency-control} 並行実行制御の方針

並行実行制御の設計、実装にどうやって取り組んでいくかについての方針を示します。
主に、シングルスレッドでやるかマルチスレッドでやるか、2 つの選択肢があります。
その後、設計例として、S2PL を取り上げるのと、発展的話題として、現代の
In-memory DBMS 向けの並行実行制御プロトコルについて紹介します。


=== シングルスレッド vs マルチスレッド

@<chap>{basic-design}ではシングルスレッドで並行実行制御ナシという設計でした。
次のステップとして、シングルスレッドで並行実行制御アリ、という設計が考えられます。
CPU コアはひとつしか使わないけれど、IO や他の処理待ちの間は
別のトランザクションなどを実行することを意味します。
具体的にはいわゆるユーザースレッド(グリーンスレッドと言ったりもします)を使って並行処理を行うシステムです。
並列(Parallel)ではなくて並行(Concurrent)という言葉を使っているところが、昔はまさにそうしていた
ことの状況証拠ではないかと思っています。

CPU コアがたくさん使える現代において、
並行実行制御をするけれど、CPU コアはひとつしか使わない縛りをする意味が、
性能を目的とした場合はあまりありません。CPU コアをたくさん使った方たくさん処理できるはずですから。
皆さんには、最終的にはマルチスレッド動作する、スケーラブルなシステムを目指して欲しいなと思っています。

シングルスレッドで並行実行制御アリの設計を採用する利点があるとすれば、
それは皆さんのような学習者にとってのものです。
なぜなら、マルチスレッドに比べて設計実装の難易度が低いからです。
シングルスレッド、すなわち DBMS が CPU コアをひとつしか使わずに動作するため、
メモリに存在するデータ構造についての排他制御が必要なくなります。
一方、トランザクションは並行に動くため、並行実行制御は必要となります。
この設計でも、例えば素朴な 2PL を実装し、デッドロックなどを引き起こすことはできます。

マルチスレッドで並行実行制御アリの場合、データ構造の排他が必要になってきますので、
これがひとつのハードルになると思います。詳細は、データ構造の節で述べます。


=== シングルスレッドで並行実行制御

最初にものすごくお手軽な方法を紹介します。
それは、複数のトランザクションをインターリーブ実行させるということです。
具体例として疑似コードを以下に示します。

//list[interleaved_execution_example][]{
void interleaved_execution() {
    Transaction t1;
    Transaction t2;

    t1.read(x)
    t2.read(y)
    t1.write(y)
    t2.write(y);

    t1.commit();
    t2.commit();
}
//}

交互にオペレーションを実行するように書き下すだけです。
テストコードとしてならこのような方法で十分でしょう。

任意のトランザクションをインターリーブ実行、となるとハードルが少し上がります。
例えば、使っているプログラミング言語が非同期処理(@<tt>{async/await} とか、コルーチンなど)を
サポートしていれば、それを使うのが手っ取り早いです。
トランザクション内のひとつひとつのオペレーションを実行単位として、
それをがインターリーブ実行されるように制御します。
グリーンスレッドを使えるならば、各グリーンスレッドでひとつのトランザクションを実行し、
オペレーション完了毎に他のグリーンスレッドに処理を明け渡すような挙動となります。

C++ の場合、2021 年現在コルーチンのサポートは限定的なようですが、
チャレンジしてみるのもありかも知れません。
コルーチンを使わない場合、例えば、各トランザクションは異なるカーネルスレッドで実行するけれども、
同時に動かせるオペレーションはひとつだけで、
オペレーション処理が終わったら、他のトランザクションに処理を譲る仕組みを、
@<tt>{mutex} と @<tt>{condition_variable} を用いて作る方法が考えられます。


=== マルチスレッドで並行実行制御

複数のカーネルスレッドで、それぞれトランザクションを実行する方法です。
複数トランザクションで共有するデータ構造、特にインデクス構造が
スレッドセーフである必要があります。
ひとつの選択肢としては、スレッドアンセーフなインデクス構造へのアクセスをまるごと
排他してしまう手段が考えられます。もちろん、これでは性能はスケールしません。

ごくごく単純なプロトタイプレベルでよければ、Insert も Delete もなしで、
Update と Read のみのアクセスを前提にし、
インデクス構造もなし、ただの固定配列 (Fixed array、Key は Array index) を
インメモリ DB とする案があります。
これは、データ構造の排他をせずに並行実行制御をマルチスレッドで動かす手っ取り早い方法です。

スレッドセーフでスケーラブルなインデクス構造、
いわゆる Concurrent index を採用するのもひとつの手でしょう。
これを自作したり、新しい手法を探求するのもひとつの深め方です。
Concurrent index については後で説明します。

インデクスを使ってレコードデータに辿りついたら、
その上でもスレッドセーフな挙動をするように心掛ける必要があります。
必要ならば、Mutex lock などでクリティカルセクションを設けましょう。
もっと並列性を高めたいならば、頑張る必要があるでしょう。
たとえば、MVCC を作るのであれば、レコードは複数のバージョンから構成されています。
あるレコードへのアクセスに対して、まるまるクリティカルセクションで守るのか、
もっと細粒度に頑張るのか、という設計の選択肢があるということです。
細粒度の Locking だけでなく、Mutex-free なデータ構造やアルゴリズムも検討されるべきでしょう。




=== S2PL の設計例

ここでは S2PL の設計例を示します。
まず、ロッキングプロトコルを用意します。ごくごく単純な Reader-writer lock で構いませんし、
Reader (shared) lock をサポートしていないただの Lock でも構いません。
並行実行できる機会は減りますが、Reader lock を通常の Lock で代用できます。

Record 毎に Mutex object を用意します。
ロックテーブル(ロックを管理するためのデータ構造)を用意するアーキテクチャもありますが、
簡単なのは、Record 毎に Mutex object を用意するアプローチだと思います。
Record を格納する構造体の中に確保しても良いですし、ポインタのみ保持して、
別途確保された Mutex object を指しても良いです。

S2PL は元々ページモデルという理論を前提として Serializability の議論がなされています。
そしてページモデルは Record(=ページ) の Insert/delete を考慮していないモデルです。
S2PL は Serializable (CSR) スケジューラなのですが、
Insert/delete 操作が存在する世界では、Phantom problem と呼ばれる一貫性の問題が存在します。
Phantom problem は、Range scan と Insert/delete の順序が前後することで
読めるべき Record が読めていなかったり、読むべきでない Record が読めてしまったりする問題です。
Mono-version を前提とする場合、多くの設計では追加の排他制御を行うことで対処します。
Range (範囲) を排他するためには、Predicate locking (条件)
という仕組みを使う場合と、インデクス構造を使って Range 相当のオブジェクト(たとえば
B+tree の Leaf node) を Lock する方法があります。
MySQL InnoDB が採用している Next key lock という手段もあります。

素朴な S2PL には Deadlock という問題があります。
例えば Record A @<m>{\rightarrow} Record B という順番で Lock するトランザクションと
Record B @<m>{\rightarrow} Record A という順番で Lock するトランザクションを同時に動かすと、
簡単にお互いがお互いをロック解放を永遠に待ち続けてしまう現象が発生します。これが Deadlock の例です。
Deadlock の発生は困るのでなんとかしたいと思った方は、
Deadlock avoidance とか Deadlock prevension というキーワードで調べてみてください。
一番単純なのは @<tt>{nowait} と呼ばれる Deadlock prevention 手法です。

最後に、コミットプロトコルです。
S2PL は Write lock を WAL 永続化の後に外す必要があります。
Read lock も WAL 永続化の後にはずす場合は SS2PL となります。
並行実行制御と永続化を非同期にやる場合は、この限りではありません。
興味のある方は Early Lock Release というキーワードで調べてみてください。



=== 並行実行制御の探求

並行実行制御には色々な方式があります。
理論から勉強するには、現状、

 * Transactional Information Systems: Theory, Algorithms, and the Practice of Concurrency Control and Recovery.
 ** 著者: Gerhard Weikum and Gottfried Vossen
 ** 出版社: Morgan Kaufmann
 ** 出版年: 2001年

という本を読むしかないと思います。
また、ひとつひとつの CC プロトコルをきちんと理解したいのであれば、
この本の参考文献に挙げられている元論文もあたった方が良いです。
他の本だと 2PL のみが紹介されていることが多いと思います。

2001 年以降の並行実行制御の研究について、特に Many-core アーキテクチャでのインメモリ DBMS
向けのものについてのキーワードをいくつか挙げておきます:

 * 2PL 系
 ** Silo (2013)
 ** FOEDUS (2015)
 ** MOCC (Mostly Optimistic Concurrency Control, 2016)
 * SI 系
 ** SSN (Sefial Safety Net, 2015)
 * TO 系
 ** TicToc (2016)
 ** Cicada (2017)

それぞれ対応する論文がありますので、興味のある方は読んでみてください。
以下、列挙します。

 * Speedy transactions in multicore in-memory databases
 ** @<href>{https://dl.acm.org/doi/10.1145/2517349.2522713}
 ** SOSP2013
 ** Silo 論文
 * FOEDUS: OLTP Engine for a Thousand Cores and NVRAM
 ** @<href>{https://dl.acm.org/doi/10.1145/2723372.2746480}
 ** SIGMOD2015
 ** FOEDUS 論文
 * Mostly-optimistic concurrency control for highly contended dynamic workloads on a thousand cores
 ** @<href>{https://dl.acm.org/doi/10.14778/3015274.3015276}
 ** PVLDB2016
 ** MOCC 論文
 * The Serial Safety Net: Efficient Concurrency Control on Modern Hardware
 ** @<href>{https://dl.acm.org/doi/10.1145/2771937.2771949}
 ** DaMoN2015
 ** SSN 論文
 * TicToc: Time Traveling Optimistic Concurrency Control
 ** @<href>{https://dl.acm.org/doi/10.1145/2882903.2882935}
 ** SIGMOD2016
 ** TicToc 論文
 * Cicada: Dependably Fast Multi-Core In-Memory Transactions
 ** @<href>{https://dl.acm.org/doi/10.1145/3035918.3064015}
 ** SIGMOD2017
 ** Cicada 論文


分散 CC についての議論もあります。論文だけ列挙しておきます。

 * Calvin: fast distributed transactions for partitioned database systems
 ** @<href>{https://dl.acm.org/doi/10.1145/2213836.2213838}
 ** SIGMOD2012
 * Ocean vista: gossip-based visibility control for speedy geo-distributed transactions
 ** @<href>{https://dl.acm.org/doi/10.14778/3342263.3342627}
 ** PVDLB2019
 * Aria: a fast and practical deterministic OLTP database
 ** @<href>{https://dl.acm.org/doi/10.14778/3407790.3407808}
 ** PVLDB2019


== データ構造の方針

まず、データ構造の排他制御について説明し、
その後 Concurrent Index と Garbage Collection の話題を取り上げます。


=== データ構造の排他制御

DBMS をマルチスレッド動作させようとすると、データ構造に排他制御が必要になります。
典型的には、Lock (Record lock と区別するために Latch という言葉を使う人もいます) を用いて排他制御します。
最近は、Mutex-free なデータ構造を使うことも珍しくなくなってきたと思います。
Mutex-free アルゴリズムは、Lock によって守られたクリティカルセクションの実行を前提としたアルゴリズムではなく、
主に CPU が用意している Atomic 命令を用いてクリティカルセクションなしで並列処理を行うアルゴリズムを指します。
Mutex-free アルゴリズムは、GC (Garbage collection) の仕組みとセットになるので、
設計、実装難易度は高めだと思います。すぐに Race condition が起きたり、
GC を自前でやっている場合はメモリについてのトラブル(Dangling pointer や Double free)に
悩まされたりします。

排他そのものも興味深いトピックだと思います。
Locking ひとつとっても、MCS lock、CLH lock、Ticket lock、などなど、様々な手法が存在します。


=== Concurrent Index の探求

Concurrent Hash Index は、要件にもよりますが、比較的 Scalable なものが作りやすいと思います。
なぜなら、Hash 関数で Bucket を選択するところは、排他等が不要だからです。
Rehash などのことを考え始めると、難しくなってくると思います。
メインメモリだけの話なら、Java だと Concurrent Hash Map が標準で使えますし、
C++ だと Intel TBB のものがあったりします。他にもあると思いますので探してみてください。
もちろん自分で作るぜ、というのもありです。

Concurrent Tree Index も、様々なアプローチがあります。
古き良き Concurrent tree index といえば、B+tree です。
B+tree は細粒度の排他制御のために Intention lock と呼ばれる手法を採用しています。
B+tree は Concurrent index としての長い歴史がありますので、勉強がてら実装してみるのも良いでしょう。
ただ、B+tree は In-memory DBMS 向けではあまり使われていないと思います。
最近の手法ですと、Masstree や BwTree、その他色々あるようです。
私は読んでませんが 2018 年の論文で In-memory index の評価論文があるので、参考にしてください。

 * A Comprehensive Performance Evaluation of Modern in-Memory Indices
 ** @<href>{https://www.comp.nus.edu.sg/~dbsystem/download/xie-icde18-paper.pdf}
 ** Zhongle Xie, Qingchao Cai, Gang Chen, Rui Mao, Meihui Zhang
 ** ICDE2018



=== Garbage Collection

Mutex-free なインデクス構造を使う場合、GC が必要だと言いました。
Java などの GC が前提の言語で開発している場合、自分で GC を実装する必要がないので楽できます。
メモリ管理を自分で行う必要がある C++ などでは、
QSBR (Quiescent state based reclamation) や EBR (Epoch-based reclamation) という手法があります。

インデクスとは別に、MVCC の場合、複数の Version を管理しますから、ナイーブには、「古い」Version を
回収して再利用する必要があります。原則としては、生きているもしくは未来のトランザクションが
絶対にアクセスしない Version は捨てて良いです。
ただ、一般に、管理する Version が増えると、メインメモリが枯渇したり、性能に悪影響が
出ますので、いかに Version を減らすか、急いで GC するか、という視点が必要になります。
Version を減らすアプローチは、並行実行制御手法にまで踏み込んで、Read できる Version
が減るような工夫が必要かも知れません。

Mono-version だとしても、アップデートの戦略によっては GC が必要かも知れません。
たとえば、対象のレコードの内容が格納されているメモリ領域を上書きするのではなく、
Copy-on-write によって、新しい内容を別メモリ領域に用意し、ポインタの書き換えでアップデートを実現する場合、
古い内容を格納しているメモリは不要になり GC が必要になります。
すぐには消せません。なぜなら、古いデータに触るトランザクションがいるかも知れないからです。
また、アップデートによって内容サイズが増えるけれども、上書きするには容量が足りない場合なども、
同様の手当てが必要になると思います。



=={sec-direction-for-durability} 永続化の方針

永続化について、主に 3 つの視点で深められると思います。
それは、WAL、Crash recovery、Checkpointing です。


=== WAL の探求

まずはシングル WAL を実装することを考えてみましょう。
マルチスレッド実行の場合、複数のトランザクションが同時にログを書こうとするわけなので、
直列化のための排他が必要になります。
通常は、Thread-safe queue 構造を用意して、それを経由して WAL に書き込むことで直列化を実現します。

WAL がボトルネックになると思ったら、パラレル WAL にも挑戦してみてください。
並列に複数の WAL ファイルに書くとしたら、どうやって
トランザクションの依存関係 (Recoverability など) を担保すれば良いでしょうか……これも奥深いテーマです。
Redo log についていえば、CC プロトコルが決定したトランザクションの Serialization order と
矛盾しない順序を保持できれば、Strong recoverable といえます。

ハードウェアアーキテクチャの進化によっては、WAL という仕組みそのものが
効率などの面で過去のものになる可能性もあります。
あくまで WAL は Atomicity と Durability を実現するためのいち手段ということを忘れないでください。


=== Crash Recovery の探求

Crash recovery は機能面では Atomicity と Durability を確保することが目的ですが、
性能面では出来るだけ高速に Recovery を終えることが望ましいです。
ふたつのアプローチがあって、ひとつは並列にログ適用すること、
もうひとつは、ログ適用を待たずにトランザクション実行を受け付けるアプローチです。
具体的に出来ることは WAL の方式や永続データベース本体管理の仕組みにもよります。
私はあまり詳しくないです。


=== Checkpointing の探求

チェックポインティングも奥深いテーマのひとつです。
ナイーブには、Dirty で良いので全ての In-memory データをファイルとして永続化してしまい、
後で WAL データを使って Consistent snapshot を作ります。
最近読んだ論文で、ARIES の系譜 (Fuzzy checkpoinitng) のひとつの終着点だなと思えるものがあったので紹介します。

 * Constant time recovery in Azure SQL database
 ** @<href>{https://dl.acm.org/doi/10.14778/3352063.3352131}
 ** PVLDB2019

別の方法として、前に作った Consistent snapshot に適用できる差分を WAL から直接作ったり
(Log gleaner、先に紹介した FOEDUS 論文)、
WAL よりも論理圧縮された形で差分を生成するなど(CALC 論文、CPR 論文)、
色々とあります。

 * Low-Overhead Asynchronous Checkpointing in Main-Memory Database Systems
 ** @<href>{https://dl.acm.org/doi/10.1145/2882903.2915966}
 ** SIGMOD2016
 ** CALC 論文
 * Concurrent Prefix Recovery: Performing CPR on a Database
 ** @<href>{https://dl.acm.org/doi/10.1145/3299869.3300090}
 ** SIGMOD2019
 ** CPR 論文


DBMS は Crash recovery を短時間で終わらせて新しいトランザクションを早く処理したいので、
その視点でチェックポインティングに要求されるのは、
Crash recovery 時に処理する必要のある WAL サイズを大きくしすぎないことです。
また、ストレージ容量を無駄にしないためにも、
いかに早く効率的にいらなくなった古い WAL データを捨てられるかという視点も必要です。



== 異なる前提や環境で動く DBMS の探求


本書は、ひとつの共有メモリコンピュータでせいぜいマルチスレッドで動かす In-Memory DBMS を想定しています。

これと異なる前提は、例えば、In-Memory DBMS ではなく、
従来そうだったようにメインメモリに全てのデータを置かず、ディスクに置くアプローチが考えられます(Disk-based DBMS)。
この仕組みは、メインメモリよりも大きなサイズのデータベースを扱える一方で、
メインメモリには一部のデータをキャッシュとして管理する必要があります。
たとえば、Disk-based DBMS についての最近のキャッシュ管理アプローチとして、次の論文が参考になるでしょう。

 * Rethinking Logging, Checkpoints, and Recovery for High-Performance Storage Engines
 ** @<href>{https://dl.acm.org/doi/abs/10.1145/3318464.3389716}
 ** SIGMOD2020

これと異なる環境は、例えば、分散システムです。共有メモリコンピュータを複数台連携させて
ソフトウェアが動作するので、用いる故障モデルも異なります。
ネットワークもラックスケールなのか、都市間スケールなのか、地球スケールなのか、
それによって遅延が異なります。
このように分散システムといってもひとつではなく、
状況によって必要な要素技術やアプリケーションも異なってくるはずです。
分散トランザクションシステムについて、以下の 2 つの論文を挙げておきます。


 * Spanner: Google's Globally Distributed Database
 ** @<href>{https://dl.acm.org/doi/10.1145/2491245}
 ** ACM Transactions on Computer Systems, Volume 31, Issue 3, August 2013.
 * CockroachDB: The Resilient Geo-Distributed SQL Database
 ** @<href>{https://dl.acm.org/doi/10.1145/3318464.3386134}
 ** SIGMOD2020



== ハードウェアを活かす探求

新しいハードウェアを活かして DBMS を改善しようとすることも可能です。
例えば、PMEM (Persistent Memory) とか NVRAM (Non-volatile RAM) などと言われている
新しいタイプのメモリがあります。Intel Optane DC Persistent Memory として 2020 年から
販売されています。(2022 年、Optane は終わることが決まったようです……悲しいですね)。

GPU や FPGA の活用も考えられますが、新しいとはいいづらいですし、
トランザクション処理というよりは、専ら分析処理の方で活躍していると思います。

CPU の進化で、新しい仕組みや命令が採用されることがあり、それを生かすこともできます。
最近はメインメモリの距離を CPU からはずいぶん遠くなってしまった一方、
Cache メモリは増えているので、Cache メモリや Prefetch 命令を生かす話もちらほら見掛けます。
Meltdown から始まる一連の脆弱性騒動からもう数年経っていますが、
最近の CPU は外部からアクセスできない領域で実行するなど、セキュリティ機能が増えており、
その機能を用いたセキュアな DBMS のあり方も模索されているようです。

変わり種としては、Google TrueTime や Facebook Time Appliance に代表される、
GPS や原子時計など精度の高い時刻を生成するハードウェアです。
これらを前提にした並行実行制御プロトコルの探求も興味深いでしょう。

まだ見ぬ新しいハードウェアも今後ニーズが認識されれば開発されるかも知れません。
皆さんには、こんな機能や性能を持つハードウェアがあったら何ができるか、
今見えている課題の解決をサポートするハードウェアはどんなものか、という視点を
持ってもらいたいです。
@<secref>{memo|sec-hardware-evolution}も関連すると思いますので参考にしてください。



== アプリケーションの視点での探求

DBMS は多くが汎用用途で作られています。
様々なアプリケーションが動くことが想定されるためです。
一方で、アプリケーションを限定すれば、既存の枠組みを越えたり、
必須とされてきた機能を削ったり緩めたりして、目的に特化した
DBMS を目指すアプローチもあります。
例えば、@<chapref>{concurrency-control}で紹介した、
限定されたワークロードのみを想定した Deterministic DBMS や、
RAMP (Read-atomic Multi-partition) トランザクションと呼ばれる、
Isolation を緩めたアプローチもあります。

本書はトランザクション処理に注目していますが、
データを扱う処理として、データ分析というテーマが別にあります。
SQL はどちらかというと、データ分析のための言語と言っても良いでしょう。
大量のデータを複雑な条件で選択したり、変換したり、ソート、集約、結合、などなど
様々な処理を行えます。
トランザクションをサポートしている DBMS (MySQL や PostgreSQL など) でも出来ますが、
大規模なデータに対して動かしたい場合は、例えば Hadoop や Spark などの処理系が担ってくれます。
それとは別に、特殊なデータベースとして、全文検索エンジンがあります。
特殊なインデクスを用いてキーワードなどによる文書検索を担う DBMS と考えることが出来るでしょう。
トランザクションシステムとこれらのデータ分析等のシステムの統合や連携については、
様々な試みがなされています。HTAP というキーワードがあります。
個人的な感想ですが、データ分析やその基盤を探求している方は、
トランザクション処理を探求している方々よりも人口が多い気がします。


== おわりに

駆け足で説明していまいましたが、これらひとつひとつが大きなテーマだと思います。
トランザクション処理だけでこれだけのことを考えなければならないのです。
OS ほどではないかも知れませんが、かなり入り組んだシステムであることが分かっていただけましたでしょうか。

新しい用途が生まれたり、ハードウェアアーキテクチャの進化に合わせて
DBMS の良いアーキテクチャも変わっていきます。
探究心を忘れずに、今後も DBMS 開発を楽しみましょう。

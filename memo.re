= 雑多なメモ


本章は、本文に書くにはちょっとな、という雑多な記事が押し込まれており、
その多くは他の章から参照されています。順番は特に重要ではありません。


=={sec-block-device} ブロックデバイスとしての永続ストレージ

ブロックデバイスは固定サイズのデータであるブロックを要素とする巨大な配列として抽象化されたデータ保持のためのデバイスです。
配列のインデクスをブロックアドレスといい、アクセスする先頭ブロックのアドレスとブロック単位のサイズ、
すなわち、ひとつのアドレス範囲を指定することで読み書きします。

ブロックデバイスという抽象が必要になったのは HDD の性能特性によるところが大きいと思います。
昔の HDD はブロックサイズが 512bytes でしたが、今の多くの HDD は 4KiB です。
HDD の中にはプラッタと呼ばれる磁性体が塗られた円盤が複数枚入っていて、プラッタの上を滑るように
動くヘッドと呼ばれる細長い三角形の形をした部品が入っています。
ヘッドの先がプラッタ上の任意の位置に移動して磁力の向きを検出したり変更したりできるようになっています。
実際には、ヘッドはプラッタの中心から外周に向かう線上をステップモータの力で移動できるようになっていて、
それに加えてプラッタが回転することによってプラッタ上の任意の極小領域にヘッドを位置合わせして
その位置に記録されているデータを読み書きすることができます。
様々な技術によって微細化の努力は今でも続けられていますが、ヘッドの位置合わせ操作の高速化は
とっくの昔に限界を迎えています。

プラッタもヘッダも物理的に動いて位置合わせするので、特定のブロックアドレスにアクセスするためには
ミリ秒単位の時間がかかります。市販されている 3.5inch の HDD で高々 10ms 程度です。
一度位置を合わせてしまえば、プラッタの回転に合わせてアクセスできるデータが変化するので、
連続領域の読み書き、すなわちシーケンシャルアクセスは比較的高速です。
最近の市販されている 3.5inch HDD だと 200MB/s 程度です。
この連続領域をブロックの配列として捉えることで、アドレスが連続するブロックの読み書きは
高々 1 回の位置合わせで実現できるように作られています。

もし HDD をバイト単位でアクセスできるようにしてしまうと、
1byte アクセスする度に位置合わせが必要となってしまう状況が想定され、
あまりにひどい性能となってしまうので、
ソフトウェア側にブロック単位でのアクセスを強要することによって、性能が下がりすぎるのを防いでいるというわけです。

B+tree は、固定サイズの連続メモリ領域を Node として扱い、
Node をブロックとしてそのままメモリとディスク間でやりとりするときに変換をほとんど必要としないので、
ブロックデバイス上で管理するインデクス構造として広く使われています。
Ext4 や xfs などのファイルシステムの多くは、ブロックデバイス上で
より柔軟なバイト単位のファイルアクセスやファイルおよびディレクトリの管理を行うためのソフトウェアです。
ディレクトリツリーがツリーと呼ばれるように、ファイルシステムは木構造をブロックデバイス上で
管理するという点で B+tree と共通点があります。
現代ではファイル抽象を大前提とするソフトウェアがほとんどであるため、OS にファイルシステムは内蔵されています。
ファイル管理のためのメタデータもやはりブロック単位で管理されます。
DBMS はストレージデバイスをファイルシステムを介さずに使うことのある数少ないソフトウェアのひとつです。

Flash メモリ (NAND 型)は、HDD とは別の物質、仕組みを利用した不揮発性ストレージです。
具体的にはメモリセルと呼ばれる極小領域に必要な量の電子を閉じ込めることで値を書き、
その電子の量を間接的に測定することによって値を読みます。
半導体なので HDD と異なり機械的な位置合わせは不要なのですが、
素子の寿命管理の制約などから、ブロックデバイスとして抽象化するのが典型的な使い方です。
また、複数の単位を並べて並列にアクセスするストライピングによって
シーケンシャルアクセスが相対的に高速となる傾向があります。

バイト単位でアクセスできる不揮発性メモリ(NVRAM)は一部の領域で昔から使われていましたが、
コスト、容量、性能の面から、
SRAM、DRAM、Flash メモリ、HDD、(光学メディア、テープ) で構成される
典型的なコンピュータストレージ階層で採用されることはこれまでありませんでした。
(実際は DRAM ですら 32bytes や 64bytes などの Cache line 単位でアクセスされるのですが、
512bytes や 4KiB に比べれば十分小さい単位といえるでしょう……)
2018 年に 3D Xpoint メモリを使った SSD 製品が、2019 年に NVDIMM として Optane DC Persistent Memory が投入され、
ストレージ階層に食い込もうとチャレンジしています。
(なんということでしょう、2022 年現在、Optane memory のビジネスが終わってしまうことが確実なようです……)
低コスト、大容量、Flash メモリよりも高性能であるようなバイト単位のアクセスが可能な NVRAM が台頭すれば、
ブロックデバイスに最適化されたソフトウェアは淘汰される運命ですので、ハードウェアの研究開発には
DBMS 屋さんとしては目を光らせておく必要が常にあります。



=={sec-hardware-evolution} ハードウェアの進化とそれに追随するソフトウェア


ハードウェアの進化によって、効率の良いソフトウェアのアーキテクチャはときどきガラリと変わります。
昨今だと、以下の点が大きな変革だと思います:

 * @<b>{CPU の メニーコア化。} とにかく並列に動かさないと性能が出ない時代が到来しています。
 * @<b>{メインメモリサイズの増大。} データベースがすっぽり収められる場合もあるくらいメインメモリが増えてきました。
   In-memory DBMS が真面目に探求されるようになったのがそれを表しています。
 * @<b>{HDD から Flash memory へ。} シーケンシャルアクセスとランダムアクセスの性能差が縮まり、明示的な永続化命令が必要ないケースも出てきました。
 * @<b>{高速ネットワークの低価格化。} Ethernet 10G、どんどん安くなっています。
 * @<b>{GPGPU や FPGA の進化。} トランザクション処理にとっては今のところ影響は少ないと思いますが、分析系の処理には大きな影響があります。Deep Learning の発展と共に、機械学習を用いた最適化も多く模索されるようになりました。
 * @<b>{NVRAM。} 2019 年現在、3D Xpoint が発売にこぎつけましたが、それがメモリ階層の中で不可欠な役割を担うようになるかまだ五分五分と私は見ています。2021 年現在、雲行きが怪しいです。。2022 年現在、Optane のビジネスが終わることがほぼ確実となりました。
 * @<b>{CXL (Compute Express Link)。} キャッシュコヒーレンシを保ったメモリアクセスのエコシステム化の時代が来そうです。
   2022 年現在、まだプロトタイプの評価くらいしか目にしないのですが、
   メモリデバイスを PCI-express スロットにポン付けしてメインメモリを増やせたり、
   様々なアクセラレータが登場しそうな未来を感じています。


トランザクション処理という視点で見ると、今の DBMS 研究の世界で見えているものと、
現実の DBMS 実装には大きな乖離があります。
今の主流の実装は、1990年代のハードウェアにおいて最適なソフトウェアから継ぎ足し継ぎ足しで進化してきているものが多いと思います。
商用の SQL をサポートするようなコードベースが大きい DBMS (Oracle とか MS SQL とか IBM DB2 とか) は、この変化についていくのが大変です。
それまで儲けたお金で莫大な投資をしなければ、ついていけません。
MySQL、PostgreSQL などは、どこまで付いていけるか、見物ですね。
現代のハードウェアに最適な構成を伴って新たに出てくる OSS や商用の DBMS に取って代わられる可能性も十分あります。
最近ですと、NewSQL と呼ばれるバックエンドが分散ストレージになっているプロダクト群、具体的には TiDB や CochroachDB などの勢いが凄そうです(2021 年、個人の感想です)。
それでも、DBMS はユーザから最も「枯れている」ことを要求されるソフトウェアのひとつなので、
一旦市民権を確立した DBMS 実装の寿命は相対的に長いと思います。今後の展開が楽しみですね。


=={sec-readable-code} 読みやすいコードを書きましょう

プログラムコードは、機械が解釈し実行する側面(機械解釈)の他に、
他人もしくは明日の自分が読んで内容を理解したり修正する側面(人間解釈)があります。
だから、機械にとって明快(典型的にはコンパイル可能で、Undefined behavior がない)である
ことだけでなく、人間であるプログラマにとっても読みやすい必要があります。
趣味で書く使い捨てのコードなら可読性は必要ありません。
しかし、多くのコードは人間に読まれ、長く使うコードはメンテナンスが必須であることから、
可読性は生産性に直結する重要な要素です。アドホックに生成される例外的なものを除いて、
レビューなしで本番運用されるコードはまずありませんから、
仕事で書くコードは当然可読性に配慮する必要があります。

コーディングするときに可読性に配慮するとして、何に気をつけるべきでしょうか。
それは、文章執筆において読んでもらうための配慮と同じで、
対象読者が誤解するリスクを減らすことです。
まずは目的です。設計意図と言っても良いです。
自然言語による文章は構造を持ち、幹と枝葉の他に根という概念があります。
根は書いた人がそれを書いた目的です。
文章であれば根も自然言語で書いてしまえばよいのですが、コードではどうでしょうか。
少なくとも現代の常識では機械に目的や意図を共有する必要はないので、
根の部分はコードとして表現されていません。
コメントで目的を書きましょうとの教えは、これに対応するものです。
目的が伝われば、やっていることについて誤解されるリスクが減ります。

変数や関数、構造体などの名前は重要だと説かれます。
これも機械解釈には関係なく、コードを読んだ人間が、
コードが扱っているデータやその操作についてのモデル(抽象的な構造と操作)を理解しやすくするための工夫です。
名前の情報を使って読者の脳内で再現されるモデルがより正確であれば、
やっていることについて誤解されるリスクが減ります。

誤解されるリスクを減らすという原則を知っていれば、
文章執筆と同様コーディングにおいても、可読性について正解はないが気をつけるべき点があることが納得できるでしょう。
個々のテクニックについて説明されている有名な本として「リーダブルコード」(オライリー・ジャパン)があります。
私も持っていますし、オススメもしています。
機械に誤解なく解釈してもらうことがコーディングの第一義であるのはいうまでもありませんが、
人間にも誤解なく解釈してもらうための可読性の重要性についても分かってもらえたら幸いです。
文章と同じで、他人に読んでもらいフィードバックを受けること、
自分もまた他人のコードを読みフィードバックすること、
これらの実践の他に、本などで知識を入れていくことで、
読みやすいコードを書けるようになっていくと思います。


=={sec-requirements-and-specification} 要件と仕様

要件(Requirements) と仕様(Specification) という言葉はよく使われますが、
その言葉を使う人や状況によって意味がブレている気がするので、
私なりの理解を簡単に説明しておきます。

要件は、主にそのソフトウェアを使う人達が求める機能などについての
重要な側面を自然言語や図表などを用いて説明したものです。
機能についての側面は一番重要なので、機能要件という言葉があります。
それ以外は非機能要件という言葉でまとめられてしまうこともありますが、
性能やセキュリティ、可用性、保守性、拡張性など色々とあります。
要件といったとき、全ての側面が網羅的に、具体的な実装が考えられるほど
詳細に語られているものであることはまずないと思って良いでしょう。
ユースケース(そのソフトウェアの一部機能を使うことを含んだ業務や作業を表すキリの良い単位)
が語られることが多いと思います。
ソフトウェアを使う人達と作る人達は別々であることが多く、
主に使う人達に欲しいものは何かを聞いて整理し、欲しいものはこういうものですねと確認する段取りを
要件定義と呼ぶことが多いと思います。

要件を満たすように、対象ソフトウェアのあるべき姿について網羅的に詳細を詰めていったものが仕様です。
ソフトウェア設計という仕事の成果物だと考えても良いです。
プログラムコードそのものを仕様という人はほぼいない(プログラムコードのことは実装=Implementationと呼ぶ)ので、
自然言語および図表などを使ったドキュメントで
仕様(の一部)を表現することが多いと思います。これを仕様書と呼びます。
仕様書と設計書とを分けて考える流儀もあるようですが、
それを読んだり確認する人達のために、成果物を分ける場合もあると捉えるのが良さそうです。

仕様書はどんな情報を含んでいるべきか、具体的に説明しましょう。
まず第一に、ソフトウェアが様々な側面でどう分割されているのか、ということが書かれているべきです。
側面の例としては、モデルの分割(ユースケースを実現するためのデータや操作の抽象としてのモデル)、
ソースコードの分割(関数、クラス、ライブラリ)、
動作単位の分割(スレッド、プロセス、サービス)、
データの分割(種類、属性などによって区別)、
環境の分割(ネットワークセグメント、サーバ、拠点、など)、などです。
どう分割するかは自明ではなく、むしろ設計の重要な決め事なのです@<fn>{footnote_ddd}。
分割されたそれぞれを、ここでは部品と呼ぶことにしましょう。
部品には通常名前がついています。どのような名前をつけるかも設計の範疇です。

//footnote[footnote_ddd][DDD という手法は、ソフトウェアをどう分割すべきかについて延々と議論しています。]


仕様書に書かれているべきことの第二は、
部品についての性質や振舞についての説明、部品同士の依存関係や相互作用の有無やそのあり方、
部品同士(や外部のソフトウェアやハードウェアと)のやりとり(通信、呼び出し)があるならその形式や手続、
部品とソフトウェアを使う人間とのやりとり(表示画面の構成や入力方法、それらの制限など)、
あたりです。それぞれについて何であって何でないかを書く、という指針があるようです。

仕様書について書かれているべきことの第三は、
部品やその相互作用を通じて要件がどのように満たされるかについてです。
ソフトウェアのあるべき姿は要件を満たすことが目的ですから、
それについて十分な説明があり、読む人が納得できることが必要でしょう。

仕様変更という言葉の存在が仕様書に書いてあるべきことの性質を良く表していると思います。
仕様が変更されるとき、変更されたのとは別の部分にも変更が必要となることがあります。
仕様変更の影響範囲がどこまでなのか、システム全体が矛盾なく機能するように、何をどう直さないといけないのか、
これをハッキリさせる必要があります。
また、仕様変更の影響範囲が要件にまで及ぶこともあるでしょう。
それらを検討し、ソフトウェアのあるべき姿を一貫性を欠くことなく見直すための道具としての側面が
仕様書にはあると思います。

仕様書は、仕様について網羅的に書いてあるべきではあるけれども、
仕様の全てが記述されているものでもないと思います。
成果物としての仕様書を作成する場合でも、どこまで詳細に決めるか、書くかは程度問題だということです。
ひとつの極端な場合として、その仕様書を元に実装すれば、誰が実装しても出来るものはそんなに変わらないだろうと
概ね期待できるもの、という基準があると思いますが、本当にそこまでコストをかけて仕様書を作っている
プロジェクトがどれだけあるのか、私は疑問を持っています。
(具体例を知らないのでそこまでコストをかけて作っている人がもしいたらゴメンナサイ)。

実装(ソフトウェアコード)が仕様情報を全て含んでいるかと言われるとそういうわけでもありません。
実装は、主にコンピュータへの命令(アルゴリズム)としての表現を使って、仕様を満たすように作られます。
しかし、仕様を満たすことは、必ずしも仕様について表現されていることを意味しません。
さすがにまったく仕様情報がないとコードレビューにすら支障をきたす
(実装の正しさとは仕様を満たすことで、コードレビューはそれを確認する行為です)ので、
関係ある仕様についての情報は、ソースコードコメント内で、自然言語による説明や仕様書の該当部分への参照として表現されます。
極端な場合には、仕様書などなく、ソースコードコメントにしか仕様情報が書かれていないこともありますし、
設計および実装をした人の頭の中にしか仕様情報がないこともあります(そして、忘れてしまっていることも)。
実装の視点から仕様を見れば、実装するために仕様が定まっている必要があるだけで、
仕様はソフトウェアの実体ではなく、あくまで抽象だといえるでしょう。

ソフトウェアを作る一連の流れは、要件の検討という概要から出発して仕様と呼ばれるものに
向かって徐々に詳細が固まっていき、プログラムコードとしての実装に至る、
という捉え方ができると思います。
すると、実装時に矛盾や問題が発覚し、仕様や要件まで戻って考え直す必要が出てくる
こと(いわゆる要件見直し、仕様変更と呼ばれる出戻り)が不思議でもなんでもない
当たり前の出来事であることが分かるでしょう。
対象となるソフトウェアのあるべき姿とその実装方法を完璧に見通せていたならば
出戻りはないのかも知れませんが、それが分かってない状況から始まるのが典型的な設計という行為でありますので、
出戻りがないソフトウェア開発というのは理想ではあっても現実ではないでしょう。




=={sec-about-test} テストについて

コードを書いたり修正したときにとりあえず(手動でも良いから)動作確認をする行為をスモークテストというそうです。
スモークテストはそれはそれでとても大事なテストです。
一方、私がここで話題にしたいものは、リグレッションテストと呼ばれています。
リグレッションテストとは、それまで期待通りに動いていたはずの機能がコード修正によって意図せず壊れたとき、
それに出来るだけ早く気付くためのテストです。
ですから、

 1. コードを変更したときにすぐに実行してパスするかどうかを確認できること
 2. CI (Continuous Integration) ツールなどで自動化できること(テストの準備や実行をするスクリプト等の用意、テスト結果もコマンドの返り値などで判別できるようになっていること)

が必要になります。
今回、CI で動かす必要が必ずしもあるかは分かりませんが、
たとえば @<tt>{make utest} などと実行したらすぐにテストが走るようにしておくことが重要です。

私は、C++ でプログラムを書く場合、同僚の作ってくれた簡易なヘルパ関数を使ってテストを作り、
それを @<tt>{make utest} で動かせるようにしています。

 * @<href>{https://github.com/herumi/cybozulib/blob/master/include/cybozu/test.hpp}

C++ 向けのテストフレームワークの有名所としては Google Test が挙げられるでしょう。

 * @<href>{https://github.com/google/googletest}

比較的新しい言語 (Go とか Rust とか) には公式のテストフレームワークが用意されていることが多いと思います。
余程の事情がなければそれに従っておけば良いと思います。

原則としては、終了コードでパスしたかどうかを判別できるようなテストプログラムを
テストしたい項目毎に書けば良いと思います。
もう少し複雑なテストをしたい場合は、環境や入力データの作成、テスト実行、
結果のチェックなどをする専用のスクリプトを書いたりします。
これもコマンド一発で動かせて、成功失敗を返り値などでチェックできるようにしておけば CI で動かせます。

実運用で使う、バグを踏むとものすごく困ったことになる、
メンテナンスを少なくとも数年続ける必要があるようなプログラムと違って、
今回のような学習用、プロトタイピング的なプログラムを作る場合に、このようなテストにたくさん時間をかけたくないでしょう。
ただ、何回も実行するのが面倒くさいと人間はサボるようになりますから、
何回も実行する必要があるテストは自動化しておきましょう。
本書に出てくる機能を例に挙げると、Crash recovery は事ある度にテストして欲しいです。
また、データ構造とアルゴリズムのコードは、比較的テストしやすい一方で
複雑であるためバグを入れてしまいやすいです。
よって、積極的にテストコードを書きましょう。
ファイル操作やネットワーク通信など外部とのやりとりが絡むコードをテストするのは
相対的に面倒くさいです。部品毎のテストが無理そうなら、多少大きな単位になってしまっても、
テストしやすい粒度でテストしてください。

目安として、自分が複雑だと感じるコードや機能については、まず間違いなくテストが必要です。
色々な状態や入力に対して動くかなあ、と不安になったらテストの書き時です。
経験則として、そのようなコードは大抵の場合バグっています。
もしテストを書かなくても大丈夫だと安心していたコードがバグっていたことが判明したら、
あなたの安心を感じるセンサーを修正する必要があります。

バグが入りにくい設計、実装を心がけることが重要なのはいうまでもありません。
しかし、バグがないプログラムなどない、と言われるくらいにバグは身近なものですから、
品質の良いソフトウェアはテストについての実践と切っても切りはなせません。




=={sec-recoverability} Recoverability や Strong recoverability について


DBMS は最低でも Recoverability (RC) を満たす必要がありますが、
RC のみを満たそうとすると、Cascading aborts (Abort 処理の連鎖) 機構を
DBMS に持たせる必要があるので、設計実装の複雑さやそれに伴うオーバーヘッドを考慮すると
オススメできません。昔はあったようですが、現代の DBMS プロダクトで Cascading aborts 機構を供えている
ものは少ないです(例外として Hekaton という DBMS があります)。

Cascading aborts を防ぐには、トランザクション A が書いた Record を
トランザクション B が読もうとするとき、A が Commit するまで待てば良いです。
この制約を Avoiding cascading aborts (ACA) といいます。ACA を満たせば RC も満たします。

ACA だけだと Crash recovery 時に処理が複雑になります。
具体的には、トランザクション A がある Record を書いた後、トランザクション B が同一 Record を
書き、A のみ Abort 扱いで B が Commit 扱いになった場合です。
この場合、A の Undo 処理の後に B の Redo 処理を行う必要があります。
B の Redo よりも A の Undo を後で行うと、
B の書いたデータが A の Undo 処理によって消えてしまう可能性があるからです。
まとめて Redo した後、まとめて Undo するというより単純な Crash recovery を実現するには、
A が Commit するまで B が書くのを待てば良いです。ACA にこれを加えた制約を
Strictness (ST) といいます。

Crash recovery が目的であれば ST で十分です。
しかし、もっと強い制約が必要なケースがあります。
それは Log を他のホストにレプリケーションして、適用し、レプリカ側で Read-only トランザクションを実行する
ような構成です。
問題が起きるのはトランザクション A がある Record を読んだ後、トランザクション B が同一 Record を上書きし、
しかし Commit 順は B < A となってしまった場合です。
このときレプリケーション先で B までの Log を適用し A の Log がまだ適用されていない
データベースは一貫性のある状態とはいえません。
レプリケーション元では A < B つまり A の後に B が実行されたことになっており、
B < A に基づいた順序が具現化することは Serializability の文脈では許容されないのです。
この問題を防ぐためには、やはり A の Commit を待ってから B は上書きする必要があります。
ST にこれを加えた制約を Rigorousness (RG) といいます。
詳しくは、Transactional Information Systems 本の 11.4 節 Sufficient Syntactic Conditions
を参照ください。RC/ACA/ST/RG についての詳細が知りたい方には以下の論文が参考になると思います。

 * A theory of reliability in database systems
 ** @<href>{https://dl.acm.org/doi/10.1145/42267.42272}
 ** Journal of the ACM, Volume 35, Issue 1, Jan. 1988.
 ** RC/ACA/ST について議論している論文。
 * On rigorous Transaction Scheduling
 ** @<href>{https://dl.acm.org/doi/abs/10.1109/32.92915}
 ** IEEE Transactions on Software Engineering, Volume 17, Issue 9, September 1991.
 ** RG について議論している論文。



上記の議論は、Single-version model を前提にした議論で、Concurrency control の詳細に
踏み入った議論が含まれています(特に、ACA や ST、RG の話)。
本来この話はもう少し単純です。最近私が整理している理論によれば、
Commit の依存関係が含まれる順序での実行結果を再現できれば、Recoverable といえます。
また、Serialization order に基づいた順序で実行結果を再現できれば、Strong recoverable といえます。
(RG は Strong recoverability を満たすと考えて良いです)。
もちろん前提として、Committed なものは実行済みとして結果に含まれる必要はあります。
Crash recovery の要件であれば Recoverable を、レプリカ上の Read-only アクセスも含めて Serializable に
動かしたい場合は Strong recoverable を採用すれば良いです。

最近のインメモリ DBMS では Commit 操作が Pre-commit (Concurrency control による処理が完了)と
Log の永続化に分離されている設計が多く、
Commit を待って読み書きする、という言葉が言葉通りに解釈できないことがありますのでご注意ください。
Pre-commit 操作と Log 永続化操作を分離する設計では、
Serialization order もしくはそれに相当する情報を何らかの方法で WAL データに記録し、
後で必要な制約(Recoverability や Strong recoverability など)を満たす
Commit order を再現して Crash recovery を実行する仕組みが使われます。




#@#== プログラムのセキュリティ原則について

#@#未執筆。

#@#入力を受けいれるお作法について。外部入力はチェックが基本(assert ではなく)。
#@#何を信頼して何を信頼しないのか、の判断基準は設計に反映される必要があるという話。

#@#かもしれない設計、かもしれない実装の心掛けについて。


#@#== 論文を読む

#@#未執筆。
#@#主に国際会議を列挙する。読み方についてもついでに書く。

#@#== トランザクショナルメモリについて

#@#未執筆。
#@#External consistency (Linearizaiblity 相当) の話。
#@#Opacity の話。


#@#== Read-set と Write-set の管理について

#@#未執筆。

#@#== ベンチマークソフトウェアについて

#@#未執筆。YCSB やら TPC-C やら TPC-E やら、他にも。

#@#== In-memory DBMS と Disk-based DBMS の違い

#@#未執筆。

#@#== 分散 DBMS について

#@#未執筆。
#@#合意の話について書く。(Replica) Consistency についての話も。

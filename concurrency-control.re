= 並行実行制御

並行実行制御は英語では Concurrency control と呼びます．
並行に動作する複数トランザクションを
求められる isolation の性質を守りながら実行するための仕組みです．
Isolation は独立性とか分離性とか訳されますが，
要はトランザクション同士の実行が混ざらない性質を言います．
混ざらないとは何かについて厳密に考え始めると，serializability の話になります．

Serializability という用語があって，直列 (serial) にトランザクションを実行したのと等価
であることを意味します．
等価とは何か，を議論しはじめると込み入ってきますが，
ここでは何らかの等価と見なせるある条件を満たす，と理解しておいてください．
一番分かりやすいのは結果的にデータベース本体の状態が等価であること
(Final State Serializability) ですが，
これだと crash が起きたときに困ったり，途中で変な値を読んでしまったり，
そもそも条件を満たす現実的なプロトコルが作れないという話がありますので，
通常はより強い制約を入れた等価性の定義を使います．)

Anomaly という概念があります．理想は serializable に動くことなのですが，
並行実行制御のプロトコルによってはそれが守られないケースがあり，
それらのケース毎に分類して名前をつけたものです．
詳細が気になる人は，このあたりを見てみてください．
いろんなAnomaly https://qiita.com/kumagi/items/5ef5e404546736ebac49

プロトコルの詳細についてはあまり触れませんが，
ごくごく基本的な serializable を実現するプロトコルである
S2PL (Strict two-phase locking) を紹介しましょう．
S2PL は Conflict Serializability という等価性を満たします(Final State Serializability より制約が強いです)．
S2PL プロトコルのルールは，比較的簡単です．
トランザクションの途中で，アクセスする record は必ず read または write ロックを取って，
他のトランザクションが触れないようにします(read ロック同士はもちろん共存できます)．
一度取ったロックをトランザクションの途中で開放してはいけません．
commit 操作後にロックを開放します．
(厳密には，read ロックについてはもう少し条件が緩いですが，ここでは省略します．)
2PL という名前は，ロックの成長(growing)フェーズと縮退(shring)フェーズに
分かれることからそう呼ばれています．

残念ながら，性能が出ないなどの理由で，
isolation の性質を完全には満たさない (serializable とはいえず，anomaly がある)
プロトコルが多く使われています．
例えば，read committed とか snapshot isolation などです．
用途によっては緩くても問題がないこともありますが，
アプリケーション設計者が注意深く選択しなければならないことは言うまでもありません．

トランザクションを並行に(並列に)実行しようとすると，
途端に難しくなるのがなんとなく分かってもらえましたでしょうか．

システムをシングルスレッドで動かして，かつ，同時にひとつしかトランザクションを
実行しないのであれば，serial 実行ですから，
このシステムは serializable であることは言うまでもありません．



== その他の話題(ここにはキーワードのみを書いておきます．．)

 * Early lock release
 * Multi-version Concurrency Control
 * Optimistic Concurrency Control
 * Deadlock prevension
 * Starvation
 * Recoverability (WAL にも絡む話)
 * Strictness (WAL にも絡む話)

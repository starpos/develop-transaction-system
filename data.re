= データ


「データベースシステム」って何でしょうか。「システム」は、複数の要素が
集まっていてそれらが相互作用しながら何らかの共通した目的のために動いて
いるものです。ここでの「システム」は、「データベース」を保持管理処理す
るためのソフトウェアやハードウェア全体のことを指すと考えるのが良いでしょ
う。それでは、「データベース」って何でしょうか。「ベース」という言葉は
基地という意味なので、「データ」がたくさん整理された状態で管理されてい
るものだと考えるのが良いでしょう。それでは、「データ」って何でしょうか。
この章では「データ」とは何かについて考えます。



== 基本データ型とその等価性および順序

データのそれ以上分解できない基本的な単位を考えましょう。
それは、例えば、整数であったり、文字列であったり、日付だったり、バイナリデータだったりというものです。
それらを基本データ型(Primitive data type)と呼びます。
基本データ型である整数型の具体的な値は例えば 1 や 2 です。これらの値は整数型のインスタンスともいいます。
皆さんはデータベースシステムよりプログラミング言語の知識をたくさん持っていると思いますから、
プログラミング言語を語るときに用いられる用語を適宜使って説明します。

同じ型の値をどうやって区別しましょうか。
それには等価性(Equality)を使います。
型 @<tt>{A} のデータ(変数) @<tt>{a1, a2} の値が等しいとき @<tt>{a1 == a2} とかき、
等しくないとき @<tt>{a1 != a2} と書くことにしましょう。多くのプログラミング言語でこう書きますからね。
基本データ型であれば値が同じ(値は何らかのバイト列もしくはビット列で表現されるとして、
それがまったく同じという意味です)であれば等しい、違えば等しくない、
という定義でまず実用上差し支えないでしょう。
たとえば、整数の 1 と 2 があったとき、@<tt>{1 == 1} で @<tt>{1 != 2} ですね。
文字列だと、@<tt>{"aaa" == "aaa"、"aaa" != "aab"、"aaa" != "aaaa"} などです。

等価性に加えて、同じ型の値同士でよく用いられる関係が、順序(Order) です。
特に全順序がよく使われます。
整数は全順序集合ですね。@<tt>{1 < 2} ですし、@<tt>{2 < 3} です。
日付も過去より現在、現在より未来が新しいという順序を持っています。
文字列は辞書順で大小関係を扱うことが多いです。@<tt>{"aa" < "aab" < "ab" < "abc" < "ba" < "bb"} など。
自分で作った構造体に任意の順序をつけることもありますね。
等価性は前提とすることが多いので @<tt>{==} が定義されており、さらに @<tt>{<} という演算子が
適切に定まれば全順序を定義できます。@<fn>{cpp-order}
//footnote[cpp-order][C++ などでは、@<tt>{!(a < b) && !(b < a)} ならば @<tt>{a == b} とする考え方もあります。これは Total order を前提としているのはいうまでもありません。]


データベースを扱うときにデータの等価性はまず間違いなく必須ですが、
順序については必ずしも必要ありません。
あるデータ型の値の部分集合を考えるとき、
順序を持つデータ型は範囲で表現することができ、
これを利用した範囲検索という効率的なアクセスメソッドを提供できるのが
特徴的です。
一方で、順序を持たない(定義しない)データ型については、
等価性を用いた検索と、全件検索(いわゆるフルスキャン)のみが可能です。



== Record、Table、Key


データとは基本データ型の値の集合といえます。
Relational database systems (関係データベースシステム、以後 RDBMS とかきます)では、
基本データ型を複数まとめて、Record 型というものを定義して使います。
Record 型の値は Record (または Record 値) と呼ばれます。Tuple 型、Tuple (Tuple 値) と呼ばれることもあります。
Tuple という言い方はプログラミング言語でもそのまま使いますね。
Record 型の中の要素を区別するために、それらを Column とか Field と呼び、
区別しやすいように名前をつけます。それらの名前は Column 名、Field 名と呼びます。
Record 型は、プログラミング言語でいうところの構造体 (struct) に相当します。

RDBMS では Record 集合を管理するために、Table という概念が使われます。
ひとつの Table には同じ Record 型のデータが複数格納されます。
RDBMS では Record 型には名前をつけ(られ)ず、Table に名前をつけることで、事実上
Record 型に名前をつけたのとほとんど同じ使い方ができます。
ただし、同じ Record 型を複数の Table で管理できる点で少し異なります
(構造が同じだけれど、別物として扱うということ)。

RDBMS では Record 型を入れ子 (Nested) にして定義することは想定されない場合が多いようです。
これはデータ重複を防ぐ「正規形」の考え方があるためだと思われます。
「正規形」「正規化」はデータベーススキーマ
(Record 型、Table、Key やその他の制約等の定義をまとめたもの)が持っているべき性質
やスキーマの正規形への変換方法を意味します。
トランザクション処理とは直接関係ないので、ここではこれ以上説明しません。
トランザクション処理においては、ひとつの Record (場合によってはその中の Column)を
それ以上分割されないデータアクセスの最小単位と考えます。

以下にスキーマとその Record の例を示しました:

//list[schema_example][]{
Schema example:

Table Human:
  (id: integer, last_name: string, first_name: string, birthday: date)


Records of Human table:
  (1, 'tanaka', 'ichiro', 2000-01-01)
  (2, 'yamada', 'hanako', 2001-08-08)
  (3, 'suzuki', 'jiro',   2000-01-01)
//}

@<tt>{Human} table がひとつ定義されています。@<tt>{Human} の Record 型は
@<tt>{id, last_name, first_name, birthday} という 4 つの Column から構成されます。
それぞれの Column は基本データ型の @<tt>{integer, string, string, date} という型です。
3 つの Record が具体例として挙げられています。
Record 型に含まれる複数の Column 型にはそれぞれ名前がついているので、
順序がないと考えることも可能ですが、
ここでは、プログラミング言語における struct に倣って順序を含めることにしましょう。

同じ Record 型のふたつの Record が等しいとは、素朴には全 Column の値が等しいことを意味しますが、
常にその条件で Record 同士を区別したいわけではありません。
同一性には関係ない補足情報を含めたいときもあるでしょう。
一般に Record の区別をするときには Key という概念が使われます。
Key は Record を入力とする関数もしくはその関数値と考えることができます。
より狭義には、Record 型に含まれる一部の Column 型からなる列を考え、
対応する Record (値) から該当する Column (値) 列を作る関数(一般に Projection と呼ばれます)を
考えます。この場合は関数そのものよりも、元になる Column 型の列で表現することが多いので、
これを、便宜上 Key 型と呼ぶことにします。Key 型を使って、Record から Projection によって作られた値を、
Key もしくは Key 値と呼ぶことにします。
型と値は区別されますが、文脈でどちらのことを指しているのか分かる場合は、単に Key と呼びます。
例えば Table A の Key と言ったときは、Table A の Record 型から作られる何らかの Key 型を指すものとし、
ある Record の Key と言ったときは、その Record における何らかの Key 値を指す、などです。

ひとつの Table について Column の選び方やその順序によって Key 型は複数存在します。
一般に、Record と Key の対応は N:1 の関係があります(ここでの N は一般に複数という意味)。
アプリケーションの視点で、ある Key が Record を区別するのに十分な情報を持っていると見做せるとき、
すなわち、Record と Key の対応に 1:1 の関係があると見做せるとき、
その Key (型) は Unique key (型) といいます。

以下に Key の例を示しました:

//list[key_example][]{

Name key of Human table:
  (last_name, first_name)

Birthday key of Human table:
  (birthday)

NameAndBirthday key of Human table:
  (last_name, first_name, birthday)

Id unique key of Human table:
  (id)


Name key of records:
  ('tanaka', 'ichiro')   id = 1
  ('yamada', 'hanako')   id = 2
  ('suzuki', 'jiro')     id = 3

Birthday key of records:
  (2000-01-01)  id = 1,3
  (2001-08-08)  id = 2

NameAndBirthday key of records:
  ('tanaka', 'ichiro', 2000-01-01)  id = 1
  ('yamada', 'hanako', 2001-08-08)  id = 2
  ('suzuki', 'jiro',   2000-01-01)  id = 3

ID key of records:
  (1)
  (2)
  (3)
//}


@<tt>{Human} table に 4 つの Key (型)を定義しています。
@<tt>{Name, Birthday, NameAndBirthday}, そして @<tt>{Id} です。
RDBMS では Key の定義は対応するインデクスの作成指示を意味しますが、
ここでは単にこのような Key 型を考えてみるという意味で捉えてください。
@<tt>{Name} key は @<tt>{(last_name, first_name)} と書いてありますが、
これは、@<tt>{Name} key は、@<tt>{Human} 型の Record を入力とし、
その Column のうち @<tt>{last_name} および @<tt>{first_name} のみ取り出して Tuple を生成し、
それを出力とする Projection 関数と考えます。

ここで @<tt>{Name} は Unique key でしょうか？ 例の 3 つの Record を見る限りでは
重複しているものはなさそうなので、いまのところ Unique になっているようですが、
今後 Record が追加された場合は Unique 性が担保されなくなってしまうかも知れません。
実は Unique key というのは、現状そうなっているという性質のことではなくスキーマに与える制約のことなのです。
@<tt>{Human} table (型) を定義した人、ここでは管理者とします、が想定する潜在的な Record 集合が
どのようなものかによって決まります。
つまり、管理者がその Key の Unique 性を担保したい場合、システムに制約の指示を与えます。
Unique key 制約が与えられた場合、たとえば、@<tt>{Name} key が Unique だと管理者が指定した場合、
システムは @<tt>{(last_name, first_name)} の組が Unique でなくなるような操作を許しません。
たとえば、既に @<tt>{('tanaka', 'ichiro')} という Name key を持つ Record が存在するのに、同じ
@<tt>{('tanaka', 'ichiro')} を持つ別の Record は、例え他の Column が異なっていても追加できなくなります。
(逆に、典型的な RDBMS は Unique 制約がなければ何もかも同じ Record を複数登録できます。)
もちろん一般には @<tt>{Name} は Unique ではないので、Name を Unique key にすると不便です。
ならば @<tt>{NameAndBirthday} はどうでしょうか。
実際に同性同名で誕生日も同じの人がいる可能性はかなり低いでしょうがゼロというわけでもないでしょう。

Table ひとつにつき、その Unique key の中で主要なものひとつを Primary key と呼びます。
明示的な Primary key がない場合は、隠し Column が用意され、
Table 内で Unique な整数が割り当てられ、Primary key として扱われることが多いです。
(例えば MySQL InnoDB はそのような実装となっています。)
これをサロゲートキー、もしくは代理キーと呼びます。
この例では @<tt>{Id} key が代理キーです。

//note{
今の時代なら皆さんにはマイナンバーが割り当てられていますね。
原則として個人にひとつ Unique な番号を割り当てられることを仕組みとして担保しているので、
国や自治体が国民/住民のデータベースを管理する場合は、
システム毎に代理キー @<tt>{Id} を割り振るよりも、
Parimary key としてマイナンバーを使った方が良いと考える向きもあるかも知れません。
しかし、データベースの扱いにおいて個人情報の保護など別の社会的法律的な制約が発生したり、
未来永劫同じ番号が使い回されることがないのかなど、
Primary key として採用することに疑問がないわけではありません。
アプリケーション・データベース設計において、Primary key をどうするかひとつとっても、
要件に立ち返って考える必要があったりしますので注意したいところです。
//}


ある Table について、任意の Key 値を指定すれば、複数の Record がマッチし得ます。
もちろん存在しない Key 値を指定すればマッチするのは 0 個です。
例えば、Human table から Birthday key の値として @<tt>{2000-01-01} を指定すると、
@<tt>{id 1} と @<tt>{3} の 2 つの Record がマッチします。
Unique key の場合は高々 1 つの Record がマッチします。
データベースにおいてデータを指定する最も基本的な操作が、ある Table において
Key 値を与えて Table を構成する Record 集合の部分集合を指定する操作です。
等価性を用いる場合は、指定したい等価な Key 値の集合を与えます。
順序を用いる場合は、指定したい Key 値の範囲を与えます。
演算子や関数を使ってより複雑な条件を指定することも出来ますが、
最終的には、複数 Table の複数 Record に何らかの順番でアクセスすることになります。

複数の Column 値から構成される Key 値の順序を考える場合、典型的には辞書順を用います。
2 つの整数型からなる Key があって、@<tt>{(a, b)} と表すとき、
その Key の順序は例えば @<tt>{(1, 1) < (1, 2) < (2, 1)} となります。
もちろん、任意の Key について任意の順序を定義し得るわけですが、
自動的に決まる順序として辞書順が採用されるシステムが多いです。
逆に、ある順序で扱いたいから辞書順でそうなるように Key を定義することもあるでしょう。
RDBMS の実装によっては、基本データ型が持つ自然な順序を Ascending (昇順)、
その逆順を Descending (降順) としてそれぞれ @<tt>{ASC, DESC} の演算子で扱えるものがあります。


====[column] 順序と全順序

集合が順序や全順序を持つためにはある性質を満たす必要があります。
具体的には集合の元についての二項関係を表す演算子 @<m>{\le} が反射律、反対称律、推移律を満たせば(半)順序であり、
加えて、集合の任意の 2 つの元が @<m>{\le} で比較可能である場合に全順序といいます。

単純なルールとして、その型の任意の値をユニークな整数や実数に割り当てる関数(単射写像)を用意すれば、全順序の性質を満たします。
整数や実数の Tuple に割り当てても良いです。全順序の型で作る Tuple 型は辞書順を考えれば全順序となります。
Record がそうだったように Key は基本データ型の Tuple でした。
Key 型を構成する全ての Column の型が全順序の性質を持っていれば、
その Key 値にも(辞書順を用いた)自然な全順序が定義されます。

====[/column]



== データの関係とポインタ


データは構造(関係)を持っています。
プログラミング言語では、基本データ型に加えて、
構造体とポインタ(参照)型があれば、
任意のデータ構造を表現することが出来るでしょう@<fn>{footnote_array_type}。

//footnote[footnote_array_type][配列型もありますよ、というツッコミはナシでお願いします……]

RDBMS はポインタ型を直接的には扱わない特徴があります。
では RDBMS でデータ同士の関係を表すにはどうすれば良いのでしょうか。
それは、共通の部分データを持つことで表現します。
R1, R2 という Record 型があり、それぞれが C1 という共通の Column 型を持つものとします。
R1.C1 (R1 型における C1 column という意味) と R2.C1 が等しいレコード同士、
すなわち、@<tt>{R1.C1 == R2.C1} である R1 型の record と R2 型の Record は
関係があるという見做せます。
同一 Record 型 (もしくは 同一 Table 内)の Record 同士に関連を持たせたい場合は、
R1.C1 と R1.C2 という二つの Column を定義しておいて、@<tt>{R1.C1 == R1.C2} という形で関係を持たせることができます。
これらの関係は、一般に、1:1、1:N、M:N という 3 種類のパターンに分類して考えます。

無理矢理ポインタでどのような表現になるかを考えてみましょう。
1:1 の関係は、Record と Record をお互いがお互いを指している状態を表します。
1:N は、N 側の各 Record が 1 側の Record を指している状態、1 側は配列などを持っていて、そこに N 側を指すポインタが
複数格納されている状態を表します。
M:N は M 側の各 Record が配列などで N 側の Record を指すポインタを複数保持している状態と、
N 側の各 Record が配列などで M 側の Record を指すポインタを複数保持している状態と考えることができます。
こんな複雑な関係をポインタで管理したくないですね:)

ポインタの参照外し(Dereference)に相当する操作は、内部結合 (Inner join) です。
アプリケーションが自分でやっても良いですが、RDBMS に任せた方が原理的には高速です。
(ただし、SQL は宣言的言語なので、Query optimizer が必ずしも良い実行計画を
選んでくれるわけではないという辛さがあります。
そこでごにょごにょと Join の順番や使う Index を指定するなどのチューニングをすることで対応します。)

何故ポインタを使わないか、という問いには歴史的経緯があるようなので、
興味のある人は調べてみてください@<fn>{footnote_no_pointer}。
ポインタを扱わないことで、多少窮屈ですが Dangling pointer がない世界に住むことができます。

//footnote[footnote_no_pointer][「ぜひ押さえておきたいデータベースの教科書」 (Leo's Chronicle, @<href>{http://leoclock.blogspot.com/2009/01/blog-post_07.html}) という記事によると、Readings in Database Systems (Red Book, @<href>{http://www.redbook.io/}) の中の解説記事に書いてあるようです。2022 年現在 Red Book 5th Edition がオンラインで気軽に読めますが、目的の記事はおそらく過去の Edition にあった The Roots という章だと思いますが、私は確認できていません。おそらく Relational model と対比すべく Hierarchical(階層型)/Network(ネットワーク型) data model についての話が書いてあると推察します。]






== 最も単純なデータベースのスキーマ

最も単純なデータベースについて考えてみましょう。
まず Table がひとつしかありません。
その Table が採用する Record 型は Key を表す型と Key 以外のデータ Value を表す型の 2 つの Column で定義されます。
Key は Primary key すなわち Unique key です。
これは key-value store と呼ばれるものですね。
Key は文字列型で、Value はバイト列であることが多いです。
Key は文字列型として自然に定義される等価性と順序(辞書順)をサポートしています。

アプリケーションが数値型を必要とするなら、数値型を文字列型に変換して使います。
10進数を用いて数字を文字として足りない分を 0 埋めして桁数を固定した文字列に変換すれば、
辞書順と数値の昇順は一致させることができます。
もちろん文字集合における順序が @<tt>{'0' < '1' < ... < '9'} を満たすことが前提となります。
例えば 10 桁固定にするとして 1 は '0000000001' に変換され、10 は '0000000010' に変換されます。
'0000000001' < '0000000010' ですね。
負の数を扱いたいならコンピュータがそうしているように、符号を表す桁を一番最初に追加して、
補数を用いることで順序を保存したまま変換できます。

スキーマを自在に定義できるデータベースシステムはもちろん有用かつ必要ですが、
学習用として最初に作るべきは、このような単純な key-value を管理するシステムが望ましいです。
なぜなら、これらの複雑さは本質的な仕組みにはあまり関係ないからです @<fn>{secondary-index}。

//footnote[secondary-index][Secondary index と呼ばれるものは、本質的な仕組みに関係あるかも知れません。]



====[column] NULL について

RDBMS の Column はデフォルトで NULL 値が許容されているものがほとんどです。
Haskell でいうところの Maybe 型、Rust でいうところの Option 型です。
Unique 制約と同様に、Column に NOT NULL 制約を指定することはできます。
外部結合(Outer join)をするためには NULL 値が必要なのですが、
現代のプログラミングの常識から考えると、デフォルトは NOT NULL にして欲しいものですね。
NULL を考慮し忘れると演算が想定外の結果になってしまうことがあります。
二項演算子やユーティリティ関数の引数にひとつでも NULL を渡すと結果の多くは NULL になり、
これが罠となります。NULL を含めた演算ルールは三値論理という立派な名前がついているのですが、
多くの場合に我々が期待する演算は、NULL (Nothing, None) 値を何らかのデフォルト値に変換して演算することなのですね。
和のときは 0、積のときは 1、文字列連結のときは空文字列など。

====[/column]


====[column] コンピュータサイエンスで使う数学

あなたがデータ構造やアルゴリズムについて考察したり、生み出したりしたとき、
定式化をする必要が出てくることがあります。
コンピュータサイエンスで用いる数学について学ぶのに良い資料として、

 * Mathematics for Computer Science.
 ** Eric Lehman, F Thomson Leighton, and Albert R Meyer.
 ** @<href>{https://courses.csail.mit.edu/6.042/spring18/mcs.pdf}

を挙げておきます。
特に、集合の基礎と述語論理のところをきちんと学んで使えるように練習することが、
論文を読むときの定理や証明を理解するための第一歩であり、自分で定式化するときの道具にもなると思います。
私はごく最近(2018年現在)この資料の存在を知って勉強したので偉そうなことは言えません。
この資料のライセンスは CC BY-SA 3.0 だそうです。太っ腹ですね。

====[/column]

= ファイルの読み書き


永続ストレージ上のデータ操作は、メインメモリ上でのデータ操作とは異なります。
この章では、典型的な永続データを管理するための方法として、ファイルの読み書きについて説明します。
ファイルシステムは、ほとんどの OS が用意している永続データ管理の仕組みです。
もちろん、ブロックデバイスを直接扱ったり、他のデバイス専用インターフェースを利用して
永続データを操作することも有り得ますが、それは
デバイスの性能特性をより生かしたいとか、特殊なインターフェースを使わざるを得ない、など
特別な理由があるときに限られます。

ここでは Linux OS のシステムコールを使って説明しますが、
各言語で使えるファイル操作ライブラリなどは全部これらのラッパーと思ってもらって良いです。
引数をどのように指定するか、返り値をどう解釈するかなど、具体的なシステムコールの使い方
については説明していませんので、必要に応じて man を見てください。
@<tt>{man 2 open} と実行すると、システムコール @<tt>{open()} の manual を読めます。
数字の意味は @<tt>{man man} コマンドを参照のこと。

ファイル操作の方法は主に二種類存在します。
@<tt>{read/write} を使う方法と @<tt>{mmap} を使う方法です。


== 基本的なシステムコール

=== 共通: @<tt>{open()}, @<tt>{close()}

ファイルを Open したり Close したりします。Open してないファイルは読み書きや Mmap できません。
Open されているファイルはファイルディスクリプタ (通称 @<tt>{fd}) で管理します。
@<tt>{open()} に渡す @<tt>{flags} に指定できる値は一通り眺めておきましょう。
@<tt>{O_APPEND}、@<tt>{O_TRUNC}、@<tt>{O_SYNC}、@<tt>{O_DSYNC}、@<tt>{O_DIRECT} あたり。



=== 読み書き: @<tt>{read()}, @<tt>{write()}, @<tt>{lseek()}

その名の通り、ファイル上でデータを読み書きします。ブロックデバイスとは異なり、
ファイルシステムがよろしくやってくれますので、Byte 単位で任意の Size のデータを読み書きできます。
ただし、ファイルシステムの下にブロックデバイスがいる場合は、最終的に
ブロック単位のアクセスになることはお忘れなく。
ファイル上の位置を指定するシステムコールが @<tt>{lseek()} として分離されているので注意が必要です。
@<tt>{lseek()} と @<tt>{read()}/@<tt>{write()} が合体したものと見做せる @<tt>{pread()}/@<tt>{pwrite()} もあります。
その他、関連するシステムコールとして @<tt>{readv()}, @<tt>{writev()}, @<tt>{preadv()}, @<tt>{pwritev()} などがあります。


典型的な使い方では Read/write はファイルシステムが管理する Page cache を通して
下位の永続ストレージデバイスにアクセスします。
Page cache とは、ページ (4KiB) 単位のメインメモリを用いたブロックデバイス
データのキャッシュ管理機構です。
ユーザーランドから Page cache の振舞いを細かく制御することは難しいですが、
カーネルにヒントを与えることはできます。@<tt>{posix_fadvise()} を参照ください。
DBMS は自分でキャッシュ管理することも多いです。
Page cache を使いたくない場合は @<tt>{O_DIRECT} で Open して使います。
その場合、後述する libaio や iouring を使うことも考えられるでしょう。



=== 永続化: @<tt>{fsync()}, @<tt>{fdatasync()}

データの永続化を行うシステムコールです。
@<tt>{write()} 等の書き込み処理を終えた後に、@<tt>{fsync()}や @<tt>{fdatasync()}を呼んで、正常終了したことを確認しない限り、書き込んだデータが永続化されている保証はありません。
逆に、@<tt>{fsync()}/@<tt>{fdatasync()} を呼ばなくても、永続化されている可能性はもちろんありますので、注意してください。
@<tt>{write()} を呼んだ時点でいつ永続化されても文句はいえませんということです。



=== mmap

@<tt>{mmap()} システムコールは、Open されているファイルの一部(または全部)の連続領域を
プロセスメモリ空間にマップします。@<tt>{munmap()} はマッピングを開放します。
マッピングされたメモリを読み書きすると、ファイルの読み書きができます。
内部的に Page cache をうまく使ってくれますが、ファイルシステムが用意しているキャッシュ管理アルゴリズムに
依存するところが多いので、細かい制御は難しいです。
@<tt>{msync()} システムコールは、変更の永続化を強制します。
@<tt>{msync()} の完了は永続化済みであることを保証しますが、
@<tt>{fsync()} のときと同様にマッピングされたメモリに書き込んだ時点でいつそのデータが永続化されても文句は言えません。


Mmap はファイルのデータ空間をメモリにマップする機能以外に、
メモリを確保する用途 (@<tt>{malloc()} と同様、ただし、内部的な動作は多少違います) でも使えます。
特に Huge page を確保するために使われます。


====[column] Huge page

随分前から CPU は、プロセス毎にメモリ空間を分ける OS の仕組みを高速化する機能を提供してきました。
それが、MMU (Memory Management Unit) と TLB (Translate Lookaside Buffer) cache です。
MMU は OS が用意した Page table を辿って、通常プログラムが使うメモリアドレスである Virtual address から、
実際にメモリ上で扱われるアドレスである Physical address への変換を行なうハードウェアです。
変換といっても Page table を辿るのは複数回のメモリ参照が必要になるため、
それを効率化するハードウェアキャッシュが用意されています。それが TLB cache です。
最近使った変換結果は TLB cache に格納されていて、
そこにエントリが存在すれば、アドレス変換が高速に実行できます。

x86_64 の場合、長らく 4KiB ページが使われてきたのですが、
他にも 2MiB や 1GiB 単位の大きなページをサポートしています。これらを Linux では Huge page と呼びます。
MMU や TLB の処理はページ単位なので、大きなメモリを扱うプログラムやシステムにとっては、
ページサイズが大きい方が TLB で扱えるメモリが事実上大きくなり、性能が向上します。
DBMS は恩恵を受ける典型的なソフトウェアであることは言うまでもありません。

====[/column]


=== Read/write と Mmap どちらをどう使うか

単に新しくファイルを Open して、先頭から順に書いて、Close するのであれば、
Mmap ではなく @<tt>{write()} を使うと良いでしょう。
Read-only (それ以上変更しないことに決めた) ファイル上で検索などの複雑な読み込み操作をするときは、
Mmap 経由でアクセスすればコードが単純になると思います。
それでも参照データをポインタに変換するなどの手間は必要です。
Mmap を使う場合、変更されたデータの永続化を @<tt>{msync()} を呼ぶことで保証することができますが、
逆に永続化されていないことを保証する仕組みが備わっていないことに注意してください。
データがすぐに永続化されては困る場合は、データを別バッファなどに一時的に記録しておいて、
マッピングされたメモリに書くのを遅らせるなどの工夫が必要になるでしょう。
@<tt>{write()} を使う場合は、バッファの書き換えと @<tt>{write()} の呼び出し操作が分離されていますから、
ファイルへの反映タイミングをある程度制御できます。


== その他の話題


=== 非同期 IO

Linux では aio という非同期 IO が使えます。
同期 IO は対応する Read/write システムコールが完了したら IO も完了しています
(Write については、必ずしも永続化されているわけではないことに注意)が、
非同期 IO では IO の Submit と Completion 待ちが別の API として分かれています。
@<tt>{open()} した後、aio 専用のシステムコール(そのラッパー)を呼ぶことに変わりはありませんが、
@<tt>{open()} の @<tt>{flags} 引数に @<tt>{O_DIRECT} をつけること、
使うバッファが Block size に Alignment されていることなどが要求されます。
興味がある人は libaio というライブラリをインストールして使ってみてください。
Linux においては posix aio (@<tt>{struct aiocb} を使う) も使えますが、
libaio (@<tt>{io_context_t} を使う) の方が良いです。

最近の Linux kernel (version 5.1) には @<tt>{io_uring} という新しいインターフェースも加わりました。
@<tt>{io_uring} を使うと IO 毎のシステムコールが不要になり、オーバーヘッドが小さくなる利点があります。
DPDK や SPDK (ユーザーランドとデバイス間のやりとりにおいてカーネルを介さない)ほど
思い切ったアプローチではありませんが、
ユーザーランドとカーネルのやりとりを効率化するという点では近いものを感じます。
いずれ非同期 IO のデファクト・スタンダードは @<tt>{io_uring} になると思われます。


非同期 IO を使う際に注意すべき点があります。それは永続化の対象についてです。
Write 相当の IO の永続化を保証するためには、当該 Write IO が「完了」した後に、
対応する永続化命令 (@<tt>{fsync/fdatasync} もしくはそれらに相当する命令) を「発行」
するという順序を守る必要があります。Write 命令と永続化命令を続けて発行し、
両方が完了したとしても、当該 Write 命令の内容が永続化されている保証はありません。
これは Linux だけじゃなくて、おそらくブロックストレージに関わる全てに共通する仕様だと思いますので、
気をつけてください。私は昔この仕様を勘違いしていて酷い目にあったことがあります(実話)。



=== Hole

Linux ファイルシステムの比較的新しい機能として Hole があります。
ファイルシステムがサポートしている場合は使えます。
文字通り、ファイルに穴 (Hole) を空けます。
Hole は Zero データが入っているように見えますが、ファイルのデータブロックとしての実体は確保されていません。
Hole には使用領域を節約できる、アクセスを高速化できる、という効果があります。
@<tt>{fallocate()} を使って穴を空けます(Punch hole)。
最小 Size や Alignment の制約が強いので注意してください。
必要な場合、@<tt>{lseek()} の @<tt>{whence} 引数に @<tt>{SEEK_DATA} か @<tt>{SEEK_HOLE} を指定して Hole を検知できます。


=== その他 API

@<tt>{ftruncate()} は、ファイルサイズを変更します。
@<tt>{flock()} はファイルを排他します。排他用途で使えるものとして
@<tt>{lockf()} や @<tt>{fcntl()} もありますが、出来ることや挙動が微妙に違うので注意しましょう。


=== メタデータ/ディレクトリの操作

ファイルのメタデータやディレクトリの操作は、別のシステムコール/関数群
があります。

 * @<tt>{fstat()}, @<tt>{stat()}, @<tt>{lstat()}
 * @<tt>{chmod()}, @<tt>{chown()}
 * @<tt>{link()}, @<tt>{symlink()}, @<tt>{unlink()}, @<tt>{rename()}
 * @<tt>{mkdir()}, @<tt>{rmdir()}
 * @<tt>{opendir()}, @<tt>{closedir()}, @<tt>{readdir()}, @<tt>{scandir()} (これらはシステムコールというより glibc 関数)


=== ブロックデバイスについて

多くの OS における多くのファイルシステムは、
Linux ではブロックデバイスと呼ばれる永続ストレージを抽象するデバイスの上で動きます。
詳しくは@<secref>{memo|sec-block-device}に書きましたので参考にしてください。

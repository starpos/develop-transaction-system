= おわりに

本書はよくある入門書と異なり、写経用のソースコードが出てきません。
理由はみっつあります。
ひとつ目は、当初リファレンス実装を書こう書こうと思っていたけれどついぞ書かなかったことです。
ふたつ目は、具体的なリファレンス実装があると
自分で考えながら設計の選択肢を選んでいく楽しさを邪魔してしまうかも知れないと思うようになったことです。
みっつ目は、使うプログラミング言語すら自由に選んでもらいたいなと思うようになったからです。
@<chap>{basic-design}には設計案の概要しか書いてありません。
細かいところはどうすればいいんだ？という読者の皆さんの疑問もごもっともですが、
それを自分で考えるという体験こそが私が皆さんにしてもらいたいことなのです。

一方で、この本は自分で考えてもらった設計やその実装について私のレビューや議論があることを
想定して作られているものであることも確かなので、このままでは片手落ちだという気持ちもあります。
とはいえ、この本を読んでくださって設計実装した全ての方にレビューを提供するというのは現実的な選択肢ではないです。
そこで、セキュリティ・キャンプ全国大会などでこの本の草稿を使って勉強して
くださった先輩方の公開してくれているコードがありますので、
必要な方はそれらを眺めてこの本で足りないものを補ってもらえると助かります。
以下、GitHub レポジトリの URL を列挙します(2022 年 8 月現在、有効なリンクであることを確認しています):

 * @<href>{https://github.com/tiger19816/camp_learning}
 * @<href>{https://github.com/hideh1231/database}
 * @<href>{https://github.com/m1kit/mikrodb}
 * @<href>{https://github.com/momohatt/seccampDB}
 * @<href>{https://github.com/2lu3/SecurityCamp2019}
 * @<href>{https://github.com/kawasin73/txngo}
 * @<href>{https://github.com/KodaiD/seccamp_db_golang}
 * @<href>{https://github.com/Mojashi/trivialDB}
 * @<href>{https://github.com/yujixr/database}
 * @<href>{https://github.com/kappybar/mydb}
 * @<href>{https://github.com/kanade9/trivialdb}
 * @<href>{https://github.com/kgtkr/tkvs}
 * @<href>{https://github.com/yuki2501/yuki-rust-transaction}


== 謝辞

セキュリティ・キャンプ全国大会の2018年〜2022年に
私が講師を勤めたデータベースゼミにおいて、
受講生の皆さんにはこの本の草稿を読んでもらい、
実際に@<chap>{basic-design}および@<chap>{next-step}に沿って
プログラムを作りながら学んでもらいました。
その過程で本書の内容についての様々なフィードバックを頂きました。
一人一人のお名前を挙げるのは控えますが、
この場を借りて御礼申し上げます。


== 著作権表示

@<b>{(C) 2018 Takashi HOSHINO}


== ライセンス

本 PDF および本書のソースコードは、

 * @<href>{https://github.com/starpos/develop-transaction-system.git}

において @<b>{CC BY-NC-SA 4.0} ライセンス@<fn>{footnote_cc_by_nc_sa}にて公開されます。

//footnote[footnote_cc_by_nc_sa][@<href>{https://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja}]

== 更新履歴

 * 2022-08-15 v1.0 公開。
 * 2022-06-22 v0.4 セキュリティ・キャンプ全国大会2022向け。
 * 2021-08-09 v0.3 セキュリティ・キャンプ全国大会2021向け。
 * 2019-08-14 v0.2 セキュリティ・キャンプ全国大会2019と2020向け。
 * 2018-08-28 v0.1 セキュリティ・キャンプ全国大会2018向け。

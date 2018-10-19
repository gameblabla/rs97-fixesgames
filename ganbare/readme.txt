/*==========================================================================
 *
 *  Copyright (C) 2008 あるふぁ～秘密基地(Alpha Secret Base). All Rights Reserved.
 *
 *  ○ゲームタイトル：がんなつポータブル
 *  ○ジャンル      ：アンカーアクション
 *  ○プレイ人数    ：一人
 *  ○バージョン    ：1.04
 *  ○公開日付      ：2008/01/31
 *  ○更新日付      ：2008/04/23
 *
 ==========================================================================*/

・はじめに
　Windows用ソフト、がんばれ！菜月さん！をGP2Xに移植しました。
　操作説明はmanual.htmを見てください。
　GP2X版はリプレイファイルを作成するためディスク容量が必要です。
　SDカードからの起動を推奨します。

  本プログラムはSDL(Simple Directmedia Layer)を使って作成しています
  SDLはLGPLに従って配布されています
  http://www.libsdl.org/license.php

  SDL(Simple Directmedia Layer)
  http://www.libsdl.org/

  各プラットホームでの実行ファイルも収録しています。
  gnp.exe :Windows用の実行ファイル
  gnp     :DebianLinux用の実行ファイル
  gnp.gpe :GP2X用の実行ファイル

  Windows版をコンパイルするにあたって
    makefileはMakefileです。

  gp2x版をコンパイルするにあたって
    makefileはMakefile.gp2xです。
    gp2x.hの#define GP2Xを有効にしてください。

  MacOSX版をコンパイルするにあたって
    makefileはMakefile.macosxです。
    init.cの#define MacOSを有効にしてください。

  Linux版をコンパイルするにあたって
    makefileはMakefile.linuxです。

＜最後に＞

・謝辞
　GP2X版の開発にあたって一色さんにお世話になりました。
　ありがとうございます。
　一色さんのHP：The 59TH STREET ROOM
　URL：http://homepage2.nifty.com/isshiki/


＜連絡＞

・御意見、ご感想などはこちらまで。
    dk@red.interq.or.jp
    http://maglog.jp/alpha-secret-base/


＜更新履歴＞

  2008/04/23    Ver 1.04
                FPS待ちの不具合を修正。
                ステージの開始、終了時に演出を追加しました。
                GP2X版の高速化を行いました。

  2008/01/31    Ver 1.03
                プレイタイムを実装しました。
                色合いを修正しました。
                ミスした場合にキャラクターが消える現象を修正しました。
  2008/01/17    Ver 1.02
                トータルアタックのスコアが保存されていなかった現象の修正。
                裏面のスコアが保存されていなかった現象の修正。
                ブロックの上に水が描画されている現象の修正。
                マップエディターを付属しました。
  2008/01/15    Ver 1.01
                文字表示周りを修正。
                画像の色がおかしい部分の修正。
                ファイル保存時にシステムと同期をとる様に修正。
  2008/01/14    Ver 1.00
                公開


＜ライセンス＞

がんなつポータブル は やわらか スタイルライセンスのもと配布されます。

<ENGLISH>
License
-------

Copyright 2008  Alpha Secret Base. All rights reserved.

Disclaimer / Copyright / Redistribution 

　This game is under the "Yawaraka(flexible)" license. 

　　Use at your own risk. 
　　You can freely modify and redistribute it. Conversions are really Fun! 
　　You should write the changes and the person who did them in the 
　　readme file or in a place that was obvious to find. 

　　If you make a cool modification, please tell me via mail or by any 
　　other means, even though that's not a condition of the license. 
　　Let me enjoy your changes too. 

　　If you have not done any modifications, you can freely distribute 
　　without problems.



<JAPANESE>
ライセンス
-------

Copyright 2008 あるふぁ～秘密基地. All rights reserved. 

 免責・著作・配布 
　柔かいコト 
　　本ゲームは「みんなで楽しく」やわらかいです。 

　　本ゲームを改造したりイジったバージョンを配布しても、何らOKです。改造たのしーい！ 
　　改造したところや改造した人の名前を、分かりやすいところに書いておくと 
　　いいかもしれません。 

　　ナイスな改造が出来たら　メールなどで教えて。俺にも遊ばせてください。 

　　改造とか一切してないものは好きにコピーして配布しちゃって結構結構。 


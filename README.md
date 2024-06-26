## I3実験
### 概要
違うネットワーク越しにもちゃんと繋げることができる”真の”インターネット通信を実現させる。
手順書に則って作ることができるのは、同じプライベートIPアドレスのサブネットに属する二つのエンドポイントでの通信を可能にするものであった。
違うプライベートIPアドレスのサブネットに属する二つのマシンで通信をするための手法の提案をする。
考えられる手法としては以下のものがある。
- 同じVPN(仮想プライベートネットワーク)に二つのマシンが接続する
- public IP addressにポートフォワーディングをする
- トンネリングサービスを使う
- IPv6を使う
  
VPNは東大VPNがあるが、client to client の通信は許可されていないようでダメだった。public IP addressも持ってません。
IPv6が一番簡単かもしれませんが、機器が接続しているルーターが対応しているか、という問題があります。

トンネリングサービスであるlocaltonetを利用する。
トンネリングとはインターネット上の二つのエンドポイントを、仮想の回線（トンネル）によりあたかも同一のエンドポイントとして扱う技術である。
例えば、ローカルホスト192.168.1.10のポート50000にlocatonetでトンネリングをする。
その結果、jp1.localto.netの4000が同一のエンドポイントとして提供されるので、ここに対するパケットは全て
ローカルホストのポート50000に転送されることになる。

説明スライド
https://docs.google.com/presentation/d/1tWqSmZ0ghf1Xom8pAZousgrSz0tkc6N_2IPybLRH8LE/edit#slide=id.g2e6ba33efdc_1_397

### 準備
1. https://localtonet.com/blog/how-to-use-localtonet に書いてあるように、ダウンロードして、```open localtonet```までする。これでシェルが開く。
2. https://localtonet.com/ にアクセスして、無料プランのアカウントを作り、ダッシュボードに移動して、AuthTokenをコピーする。
3. シェルにコピーしてAuthTokenをペーストする。
4. [このリンク](https://localtonet.com/documents/udp) を参考にダッシュボードでトンネリングをする。入力するIPとポート(50000とか大きい数字にする)はローカルの使いたいもの。AuthTokenはDefaultのFreeのもの。
また、wifiに繋ぎかえるたびにIPは変わるので、その度に```ifconfig```などで確認する。
5. スタートをし、シェルでも```Status OK```となっていることを確認する。
6. soxコマンドが入っていなければ、```brew install sox```

### 実行
クライアント側とサーバー側を決めて、どちらもlocaltonetを利用して、トンネルを作成する。

クライアント側
```bash
mkdir bin
make
./bin/phone <自分のlocaltonetのURL> <自分のlocaltonetのport> <サーバーのlocaltonetのURL> <サーバーのlocaltonetのport> <自分のlocaltonetにトンネルしてるポート> | play -t raw -b 16 -c 1 -e s -r 48000 -
```

サーバー側
```bash
mkdir bin
make
./bin/phone <自分のlocaltonetにトンネルしてるポート> | play -t raw -b 16 -c 1 -e s -r 48000 -
```

クライアント側とサーバー側のどちらからコマンドを実行しても問題ない。

実際、日本・オーストラリア間で電話を実行しても、遅延を意識することなく十分に電話できた。

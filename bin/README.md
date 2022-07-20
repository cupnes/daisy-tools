# 作業ディレクトリのbinに配置されるコマンドについて

daisy-toolsは作業ディレクトリ(`dsy-work`)直下の`bin`ディレクトリに、動作に必要な様々なコマンドを配置します。ここではそれらのコマンドについて説明します。

## 共通事項

- daisy-toolsのメインプロセスとして動作する`dsy-sysenv`含め、周期的に動作し続けるコマンドはどれも、自らデーモン化したりはしませんので、必要に応じてバックグラウンド実行するなどしてください

## cell-fitness-logger

作業ディレクトリに存在する細胞(`cell`ディレクトリ)の状況を周期的にチェックし、結果を作業ディレクトリ直下の`log`ディレクトリへHTMLで保存するシェルスクリプトです。オプションによってワンショット実行することも可能です。

なお、メインプロセスである`dsy-sysenv`へは`cell`ディレクトリの読み取り動作しか行いませんので、`cell-fitness-logger`はいつ起動・停止させても`dsy-sysenv`へは影響しません。

### 起動方法

`run`コマンドでdaisy-toolsを開始すると、`dsy-sysenv`と共に自動的に起動されます。

既に`dsy-sysenv`が動作している場合など、`cell-fitness-logger`だけを起動したい場合は、単にこのコマンドを実行すれば良いです。

```bash
$ bin/cell-fitness-logger
```

オプションについて：

> Usage: bin/cell-fitness-logger [--oneshot|INTERVAL_SEC]

- `--oneshot`
  - このオプションを付けて実行すると、`cell`ディレクトリの状況をチェックし`log`ディレクトリへHTMLを保存した後、周期動作せず停止します(ワンショット実行)
- `INTERVAL_SEC`
  - 周期動作する際の、次の実行までのスリープ時間を指定します[単位:秒]
  - デフォルトは600[秒]です

### 停止方法

周期動作を自ら停止するような操作はありません。Ctrl-Cや`kill`コマンドなどで`cell-fitness-logger`のプロセスを停止させてください。

### `log`ディレクトリの見方

`log`ディレクトリ直下にチェックした日時のディレクトリを`YYYYMMDD_hhmmss`の形式で作成します。(例えば2022年7月18日 17時7分30秒だと`20220718_170730`)そして、そのディレクトリの中にその時点の細胞の状況を適応度別のヒストグラムで示したHTML(`index.html`)と、そこから各細胞の詳細な情報を出力したHTMLへリンクを張っています。また、このディレクトリにはチェックした時点の`cell`ディレクトリそのもののコピーも取っているため、細胞だけではありますが、周期的なバックアップツールにもなっています。

なお、`log`ディレクトリ直下の`current_html`というディレクトリには、最新の`YYYYMMDD_hhmmss`ディレクトリのHTMLファイルがコピーされています。常に最新の状況を見るようにしたい場合、`current_html`のHTMLファイルを見れば良いようにしています。

## cell-log-html

`cell-fitness-logger`から実行され、適応度別細胞一覧のヒストグラムや各細胞の詳細のHTMLを生成しているスクリプトです。HTMLファイルの内容を変更したい場合はこのスクリプトを変更してください。

なお、`strace`コマンドを使用しますので、無ければインストールしてください。Debian系ディストリビューションの場合、`apt`コマンドを使用して以下のようにインストール可能です。

```bash
$ sudo apt install strace
```

## create-treeimg-from-syslog

`save-recent-syslog`を使ってsyslogのログを保存するとカレントディレクトリに`syslog.log.gz`というファイル名ができあがります。この状態でこのコマンドを実行すると、その中からdaisy-toolsのログを抽出し、これまで誕生してきた細胞の親子関係のグラフ(家系図のようなもの)を`tree.dot`と`tree.svg`というファイル名で作成します。

ただ、これは作者の環境である一時期使用していたに過ぎないものなので、syslogのパースの仕方などの問題で環境によってはうまく動かないかもしれません。参考程度にどうぞ。短いシェルスクリプトなので改造は容易だと思います。

## dsy-cell2elf

細胞ファイルをELFファイルへ変換します。

### 使い方

`dot`コマンドを使用しますので、無ければgraphvizをインストールしてください。Debian系ディストリビューションの場合、`apt`コマンドを使用して以下のようにインストール可能です。

```bash
$ sudo apt install graphviz
```

引数で細胞ファイル名と出力ファイル名を指定して実行してください。

> Usage: bin/dsy-cell2elf CELL_FILE_NAME ELF_FILE_NAME

- `CELL_FILE_NAME`
  - 細胞ファイル名
  - `cell`ディレクトリ直下を参照しますので、`cell`ディレクトリ直下に存在するファイル名を指定してください
- `ELF_FILE_NAME`
  - 出力ファイル名
  - カレントディレクトリに出力されます

### 備考

これはシェルスクリプトではなくELFバイナリなのでエディタなどで開かないようにご注意ください。ソースコードはリポジトリ直下の[dsy-cell2elf.c](https://github.com/cupnes/daisy-tools/blob/master/dsy-cell2elf.c)です。

また、出力されるELFファイルのヘッダには最低限実行に差し支えない情報しか書いていません。そのため、単純に`objdump`で逆アセンブルする、とかができません。(少し工夫が必要になります。)逆アセンブルする際は、`dsy-elf2asm`を使用してください。

## dsy-cell2pbm

細胞のタンパク質として機械語バイナリではなく画像データを使用する実験を行っていた時のものです。細胞ファイルをPBM形式の画像ファイルへ変換します。

ELFファイルを生成する現状のdaisy-toolsでは使用できません。ですが、`dsy-eval`での実行・評価と`code`ディレクトリに配置するコード化合物の内容次第で機械語バイナリ以外を生成するような実験も技術的には可能です。そのため、そういった実験を試してみる際に参考にはなると思います。

### 使い方

引数で細胞ファイル名と出力する画像の幅・高さ・出力ファイル名を指定して実行してください。

> Usage: bin/dsy-cell2pbm CELL_FILE_NAME WIDTH HEIGHT PBM_FILE_NAME

- `CELL_FILE_NAME`
  - 細胞ファイル名
  - `cell`ディレクトリ直下を参照しますので、`cell`ディレクトリ直下に存在するファイル名を指定してください
- `WIDTH`・`HEIGHT`
  - 出力する画像の幅・高さ[px]
- `PBM_FILE_NAME`
  - 出力ファイル名
  - カレントディレクトリに出力されます

### 備考

これはシェルスクリプトではなくELFバイナリなのでエディタなどで開かないようにご注意ください。ソースコードはリポジトリ直下の[dsy-cell2pbm.c](https://github.com/cupnes/daisy-tools/blob/master/dsy-cell2pbm.c)です。

## dsy-dump-cell

細胞ファイル内に保存されている細胞の情報をダンプします。

### 使い方

引数に細胞ファイルを指定して実行してください。

> Usage: bin/dsy-dump-cell [-v] [-j] CELL_FILE_NAME

- `-v`
  - 細胞ファイルのバイナリとしての情報をより詳しく出力します
  - 具体的には[`codon`構造体](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L56-L100)のダンプ内容が以下のように変わります(あまり重要なフィールドではないので特に`-v`無しで問題無いかと思います)
    - 予約フィールド[`_mf_rsv`](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L85-L86)・[`_rsv`・`_rsv2`](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L91-L93)をダンプするようになる
    - 「タンパク質(機械語命令)」の共用体の[`int64`](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L97)フィールドをダンプするようになる
    - 「タンパク質(機械語命令)」の共用体の[`byte[8]`](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L98)を、実際のサイズ分ではなく、8バイト分全体をダンプするようになる
- `-j`
  - このオプションを付けると出力がJSON形式になります
  - デフォルトはYAMLっぽいリスト形式です
- `CELL_FILE_NAME`
  - 細胞ファイル名
  - `cell`ディレクトリ直下を参照しますので、`cell`ディレクトリ直下に存在するファイル名を指定してください

### 備考

これはシェルスクリプトではなくELFバイナリなのでエディタなどで開かないようにご注意ください。ソースコードはリポジトリ直下の[dsy-dump-cell.c](https://github.com/cupnes/daisy-tools/blob/master/dsy-dump-cell.c)です。

## dsy-edit-cell

指定された細胞ファイルを編集します。

### 使い方

編集内容と対象の細胞ファイルを引数に指定して実行してください。

編集できるのは今の所、属性情報の[適応度(`fitness`)フィールド](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L30-L31)と[取得済み引数の数(`has_args`)フィールド](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L35-L36)のみです。

> Usage: bin/dsy-edit-cell OPTION CELL_FILE_NAME
>
> OPTION
>         --fitness={0..100}              Set .attr.fitness
>         --has_args={0..num_args}        Set .attr.has_args

- `OPTION`
  - `--fitness={0..100}`
    - 属性情報の[適応度(`fitness`)フィールド](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L30-L31)に設定する値を指定します
  - `--has_args={0..num_args}`
    - 属性情報の[取得済み引数の数(`has_args`)フィールド](https://github.com/cupnes/daisy-tools/blob/518ae77289708051025e29324e64efc0a9a5d844/cell.h#L35-L36)に設定する値を指定します
- `CELL_FILE_NAME`
  - 細胞ファイル名
  - `cell`ディレクトリ直下を参照しますので、`cell`ディレクトリ直下に存在するファイル名を指定してください

実行すると、編集後の細胞の状態を標準出力へダンプし、`Is this ok? [y/n]`というメッセージが出力されて止まります。確認し、問題なければ'y'を入力してEnterを入力してください。問題がある場合は'n'を入力しEnterを入力すれば、何もせずに終了します。

### 備考

これはシェルスクリプトではなくELFバイナリなのでエディタなどで開かないようにご注意ください。ソースコードはリポジトリ直下の[dsy-edit-cell.c](https://github.com/cupnes/daisy-tools/blob/master/dsy-edit-cell.c)です。

## dsy-elf2asm

`dsy-cell2elf`で変換したELFファイルを逆アセンブルしてアセンブラのコードを出力するシェルスクリプトです。

### 使い方

まず、`objdump`コマンドを使用しますので、無ければbinutilsをインストールしてください。Debian系ディストリビューションの場合は`apt`コマンドを使用して以下のようにインストール可能です。

```bash
$ sudo apt install binutils
```

そして、引数にELFファイルを指定して実行してください。

> Usage: bin/dsy-elf2asm ELF_FILENAME [ASM_FILENAME]

- `ELF_FILENAME`
  - 機械語命令列の部分を逆アセンブルするELFファイルを指定します
- `[ASM_FILENAME]`
  - 第2引数の指定がある場合は、指定されたファイル名で、逆アセンブル結果をファイルへ保存します

## dsy-exec-cell

細胞の代謝/運動(実行と評価)に`dsy-eval`を使用する現在の設計では使用しなくなったコマンドです。

以前は「代謝」の際、`data`ディレクトリの「データ化合物」を関数の引数に使ったり、戻り値をデータ化合物として`data`ディレクトリへ放出したり、という事を行っていました。ELFファイルを扱うようになった際に、この代謝の仕組みを使わなくなりました。

詳しくは[この辺り](https://github.com/cupnes/daisy-tools/tree/78c6bf7c0b88d60a7c8a1c9f2e0f618af0e11e27/docs/exercise#%E6%BC%94%E7%BF%928%E5%88%9D%E6%9C%9F%E8%A8%AD%E8%A8%88%E3%81%AE%E4%BB%A3%E8%AC%9D%E3%82%92%E8%A9%A6%E3%81%99)に書いていますのでそちらをご覧ください。

## dsy-sysenv

daisy-toolsのメインプロセスです。起動するとdaisy-toolsの周期動作が開始します。

今の所、daisy-tools本体はこのプロセス1つのシングルスレッドで動作します。(`cell-fitness-logger`など外部のプロセスが並列で動作したりはしますが、それらは本体の動作には影響しません。)

### 起動方法

`dsy-sysenv`を実行してください。

> Usage: dsy-sysenv [-h|CYCLE_WAIT_USEC]

- `-h`
  - Usageを表示します
- `CYCLE_WAIT_USEC`
  - 周期動作のインターバルをマイクロ秒単位で指定します
  - デフォルトは1000000[マイクロ秒](1秒)です

なお、`run`コマンドでも起動させることができます。

また、開始するとフォアグラウンドで実行し続けプロンプトは返ってこないので、screenやtmuxなどのターミナルマルチプレクサ上で実行してデタッチしておくとか、何らかの方法で放置しておけるようにするのがおすすめです。

### 停止方法

作業ディレクトリに作成された`running`ファイルを消してください。すると、現周期分の作業を終えたら終了します。

### 再開方法

`running`ファイルを消す方法で停止させていれば、再度起動させると前回実行したところから再開します。

標準出力に`========== XX 周期目 ==========`と出力される周期カウンタの値だけは1から始まりますが、細胞や化合物の状態など、周期カウンタ以外はすべて停止した状態から引き継がれています。

### 備考

これはシェルスクリプトではなくELFバイナリなのでエディタなどで開かないようにご注意ください。ソースコードはリポジトリ直下の[main.c](https://github.com/cupnes/daisy-tools/blob/master/main.c)に`main()`があります。(そこから各ソースファイルが参照されています。)

## run

daisy-toolsを`cell-fitness-logger`と共に起動するシェルスクリプトです。

### 使い方

単に実行するだけです。

> Usage: bin/run [-h]

- `-h`
  - Usageを表示します

### 備考

`cell-fitness-logger`や`dsy-sysenv`の周期動作のインターバルをデフォルトから変更したい場合は、このシェルスクリプトを書き換えて、引数で指定するようにしてください。

## save-recent-syslog

直近でdaisy-toolsを実行開始した分のsyslogのログを`syslog.log.gz`というファイル名でカレントディレクトリへ保存するシェルスクリプトです。

`create-treeimg-from-syslog`がこのファイルを使用します。

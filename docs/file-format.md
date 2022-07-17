# daisy-tools独自のファイルフォーマットについて

daisy-toolsが扱う独自のファイルフォーマットを説明します。

## 共通事項

- バイトオーダーはリトルエンディアンです
- daisy-toolsが動作するディレクトリ直下のどのディレクトリに配置されているかでファイルフォーマットを判別しています
  - そのため、それぞれのファイルフォーマット内に「ファイルタイプ」といったフィールドはありません
  - ファイルフォーマットと異なるディレクトリへファイルを配置しないようにご注意ください(誤動作の原因になります)

## 細胞ファイル

`cell`ディレクトリに配置されたファイルは「細胞ファイル」として扱われます。

### ファイルフォーマット

| オフセット[バイト] | 内容 | サイズ[バイト] |
| --- | --- | --- |
| 0 | 属性情報([`cell_attributes`構造体の`num_codns`まで](https://github.com/cupnes/daisy-tools/blob/b22ce56350680ea8536c1a3c04739c9cd98ea792/cell.h#L19-L50)の内容) | 56 |
| 56 | DNA(`num_codns`個分の[`codon`構造体](https://github.com/cupnes/daisy-tools/blob/b22ce56350680ea8536c1a3c04739c9cd98ea792/cell.h#L56-L100)の内容) | 16 * `num_codns` |
| 56 + (16 * `num_codns`) | タンパク質の列(機械語命令のバイナリ列) | `func_size` |

### 備考

- ファイルフォーマットをソースコードで確認する際は、細胞ファイルのロードを行う[cell_load_from_file()](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.c#L630-L671)の実装が参考になります

## コード/データ化合物ファイル

`code`ディレクトリに配置されたファイルは「コード化合物ファイル」、`data`ディレクトリに配置されたファイルは「データ化合物ファイル」として扱われます。(`dsy-eval`ファイルを使用して細胞の実行・評価を行う新設計ではデータ化合物ファイルは使用しません。)

### ファイルフォーマット

コード化合物ファイル・データ化合物ファイル共に同じファイルフォーマットです。

| オフセット[バイト] | 内容 | サイズ[バイト] |
| --- | --- | --- |
| 0 | 化合物([`compound`構造体](https://github.com/cupnes/daisy-tools/blob/de9685efcd2ec219e7f15b4c33e5b8fd90e60c2e/compound.h#L22-L35)の内容) | 16 |

### 備考

- ファイルフォーマットをソースコードで確認する際は、化合物ファイルのロードを行う[comp_load_from_file()](https://github.com/cupnes/daisy-tools/blob/131b0ea2a0f8e07dae17472c8f80ed8b6c082cac/compound.c#L25-L43)の実装が参考になります

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

細胞ファイルは「属性情報」・「DNA」・「タンパク質」で構成されます。

#### 属性情報

[`cell_attributes`構造体の`filename`以外のフィールド](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.h#L19-L31)がここに配置されています。

| オフセット[バイト] | フィールド名 | 型/長さ |
| --- | --- | --- |
| 0 | 寿命(`life_duration`)[サイクル数][^life_duration_left] | 符号なし32ビット整数 |
| 4 | 余命(`life_left`)[サイクル数][^life_duration_left] | 符号なし32ビット整数 |
| 8 | 適応度(`fitness`)[%] | 符号なし8ビット整数 |
| 9 | 引数の数(`num_args`)[^not_used_with_dsy-eval] | 符号なし8ビット整数 |
| 10 | 取得済み引数の数(`has_args`)[^not_used_with_dsy-eval] | 符号なし8ビット整数 |
| 11 | 戻り値の有無(`has_retval`)[^not_used_with_dsy-eval] | ブール(符号なし8ビット整数) |
| 12 | 関数サイズ(`func_size`)[バイト] | 符号なし32ビット整数 |
| 16 | 引数バッファ(`args_buf[CELL_MAX_ARGS]`)[^not_used_with_dsy-eval] | データ化合物型(64ビット) * 4 |
| 48 | DNAの長さ(コドンの数)(`num_codns`) | 符号なし64ビット整数 |

[^life_duration_left]: 寿命と余命のフィールドをそれぞれ設けている理由は、細胞分裂時に[突然変異しない場合は親の寿命と同じ寿命を子に設定する](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.c#L536-L537)ためです。そして、1周期毎に余命フィールドをデクリメントしていきます。寿命フィールドは変更されることは無く、細胞分裂時に使われます。ちなみに、突然変異する場合、子の寿命は[突然変異後のDNAの長さ(コドンの数)に比例して決められます](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.c#L514-L516)。
[^not_used_with_dsy-eval]: 細胞の実行と評価に`dsy-eval`ファイルを使用する新設計では使用しないフィールドです。

#### DNA

| オフセット[バイト] | フィールド名 | 型/長さ |
| --- | --- | --- |
| 56 | コドンの配列 | [コドン構造体](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.h#L36-L55)(16バイト) * `num_codns` |

#### タンパク質の列

タンパク質の列、すなわち機械語命令のバイナリ列が配置されています。

| オフセット[バイト] | フィールド名 | 型/長さ |
| --- | --- | --- |
| 56 + (16 * `num_codns`) | タンパク質の列 | `func_size`バイトのバイナリ |

### 備考

- 細胞ファイルのファイルフォーマットをソースコードで確認する際は、細胞ファイルのロードを行う[cell_load_from_file()](https://github.com/cupnes/daisy-tools/blob/89613d9fcccea2a4fea078ba394b314cac674d17/cell.c#L630-L671)の実装が参考になります

## コード化合物ファイル

TBD

## データ化合物ファイル

TBD

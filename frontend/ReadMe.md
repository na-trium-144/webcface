# ブランチ

## 命名規則

* `master`
  * 細かな修正以外はこのブランチにはcommit/pushしない
  * 最低限動くことが保証された状態をmasterにpushする
* `dev/適切な名前`
  * 新しい機能を追加したり、リファクタリングをしたりするときに使うブランチ
  * やっていることの名前を`dev/`の後ろにつける
  * ある程度安定したと思ったらmasterへマージする
  * 必要なくなったらリモートレポジトリから削除して良い
* `review/名前`
  * コードレビューするときに使うブランチ

# frontend

## ビルド方法

ルートディレクトリのreadmeを参照してください。

## 設計思想

将来の拡張性を見据えて、機能ごとにディレクトリに分け、ディレクトリ間はできるだけ独立して使えるようにしている

## ディレクトリ構成

ソースコード関連は基本srcに置いている

### components

- `element`
  - 最も小さい要素
- `part`
  - elementを組み合わせた部品
  - API呼び出しとかしていい
- `layout`
  - レイアウトを入れる
  - 画面の1領域を専有するものを入れるイメージ
    - ヘッダー、フッター、商品一覧などjjkjjj
- `function`
  - 見た目を伴わないものを入れる
- `hook`
  - hookを入れる


### features

[Reactベストプラクティスの宝庫！「bulletproof-react」が勉強になりすぎる件](https://zenn.dev/meijin/articles/bulletproof-react-is-best-architecture#features%E3%83%87%E3%82%A3%E3%83%AC%E3%82%AF%E3%83%88%E3%83%AA)の説明の通り

大雑把に、その機能でしか使わないようなものを入れる。

componentは複数の場所から使われうるが、featureはそこでしか使われない。

例えば、認証画面とか？

でもあまりよくわかっていない。

### pages

ページを入れる。

ページ固有のコンポーネントがある場合は、`src/components/`以下と同様にフォルダを作る。

## 命名規則

| 種類                              | 命名規則                  |
| --------------------------------- | ------------------------- |
| ファイル名                        | `userProfileEditForm.tsx` |
| コンポーネント名                  | `<UserProfileEditForm/>`  |
| フック名                          | `useUserLoginEffect`      |
| props、引数、変数名、プロパティ名 | `userName`                |
| 定数名                            | `SNAKE_CASE`              |
| メソッド名                        | `addNumber`               |

参考記事

- [Reactを採用したTypeScriptプロジェクトにおける命名規則 - takoba](https://scrapbox.io/takoba/Reactを採用したTypeScriptプロジェクトにおける命名規則)
- [JavaScriptにおける命名規則の個人的まとめ｜もっこりJavaScript｜ANALOGIC（アナロジック）](https://analogic.jp/naming-convention/)
- [JavaScriptの命名規則早見表（と記法） - Qiita](https://qiita.com/RyosukeSomeya/items/90f8e780b37c53758276)

## npmでインストールしているものの軽い説明

### craco

ビルド設定を管理するツールの1つ。

別にいらないかもしれないと最近思い始めている。

### [emotion](https://ramble.impl.co.jp/1414/)

CSSをjavascriptで書ける。cssファイルはグローバル変数と同じように把握するのが大変だが、emotionを使うとコンポーネント単位でcssを当てられるので便利。

例えば、下のようにできる

```javascript
import {css} from '@emotion/react'

type Props = {
  team: string 
  type: string 
}

/**
 * Robot.
 *
 * @param {Props} props team must be blue or red, type must be er or rr
 */
export default function Robot(props: Props) {
  function getRobotSize(type: string) {
    if (type == "er") {
      return "90px";
    }
    else if (type == "rr") {
      return "30px";
    }
    throw new Error("getRobotSize() props.type " + props.type + " is unknown");
  }


  const styles = css({
    width: getRobotSize(props.type),
    height: getRobotSize(props.type),
    borderWidth: "5px",
    backgroundColor: "white",
    borderStyle: "solid",
    borderColor: props.team,
  })

  return <div css={styles} />;
}
```

導入した理由は、[CSS in JSとは](https://zenn.dev/takuyakikuchi/articles/b1b20f65d4f9cf)にだいたい書いてあるのでそちらを参照してください。

### [mui](https://mui.com/)

デザイン系では知らない人がいないほど有名なライブラリー。

material UIに準拠したボタンやアイコンなどの画面を構成する部品を配布している。

言い換えると、muiにある画面の部品を使えば、簡単に操作しやすいウェブサービスを作れる。

### [apexcharts](https://apexcharts.com/)

某fpsゲーム...ではなく、グラフの描画を簡単に行えるライブラリの1つ。

reactとvueの両方に対応している。

### `react`

言わずとしれたreactを使っています。



### SWR

[はじめに – SWR](https://swr.vercel.app/ja/docs/getting-started)を読むとわかるが、簡単にまとめると

- fetchをいい感じに処理してくれる
- 定期的に自動実行してくれる
- キャッシュとかもいい感じに保存してくれる

サーバーとの通信にこれを使うとかなり便利そうに思える

参考記事

- [SWRの機能を簡単にまとめる - Qiita](https://qiita.com/irico/items/d8f0ed7887f3c490ffa1)
- [はじめに – SWR](https://swr.vercel.app/ja/docs/getting-started)
- [posts](https://zenn.dev/mast1ff/articles/40b3ea4e221c36)
- [refreshInterval](https://swr.vercel.app/docs/options)
- [そうです。わたしがReactをシンプルにするSWRです。](https://zenn.dev/uttk/articles/b3bcbedbc1fd00)

### Typescript

typescriptを導入しています

### `webpack`

`@`がルートディレクトリ(frontend/src/)を示すように設定しています。

もっといろいろな機能を使えるらしいけど、あまり良く知らないので導入していない。

[javascript - @importするときの@の意味 - スタック・オーバーフロー](https://ja.stackoverflow.com/questions/49318/import%E3%81%99%E3%82%8B%E3%81%A8%E3%81%8D%E3%81%AE%E3%81%AE%E6%84%8F%E5%91%B3)

詳細は↑を読んでください

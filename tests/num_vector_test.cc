#include <gtest/gtest.h>
#include <webcface/common/num_vector.h>
#include <vector>
#include <numeric>   // std::accumulate
#include <stdexcept> // std::out_of_range

// Geminiに書かせた

// ## 1. コンストラクタのテスト ##
// ------------------------------------

using namespace webcface;

TEST(NumVectorTest, DefaultConstructor) {
    MutableNumVector v;
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 0.0);
}

TEST(NumVectorTest, ConstructorWithSingleValue) {
    MutableNumVector v(1.23);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 1.23);
}

TEST(NumVectorTest, ConstructorWithVector) {
    std::vector<double> data = {1.1, 2.2, 3.3};
    MutableNumVector v(data);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_DOUBLE_EQ(v[0], 1.1);
    EXPECT_DOUBLE_EQ(v[1], 2.2);
    EXPECT_DOUBLE_EQ(v[2], 3.3);
}

TEST(NumVectorTest, ConstructorWithEmptyVector) {
    // 仕様: 要素数0にはならず、その場合要素数1で値が0となる
    MutableNumVector v(std::vector<double>{});
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 0.0);
}

TEST(NumVectorTest, CopyIsShallow) {
    MutableNumVector v1({1.0, 2.0, 3.0});
    MutableNumVector v2 = v1; // コピーコンストラクタ

    // v2の値を変更
    v2[1] = 99.0;

    // v1も変更されていることを確認 (シャローコピー)
    EXPECT_DOUBLE_EQ(v1[1], 99.0);
    EXPECT_EQ(v1, v2);

    // v1をリサイズ
    v1.push_back(4.0);
    EXPECT_EQ(v2.size(), 4u); // v2のサイズも変わる
    EXPECT_EQ(v1, v2);
}

// ## 2. 代入演算子とassignメソッドのテスト ##
// ------------------------------------

TEST(NumVectorTest, AssignSingleValue) {
    MutableNumVector v({1.0, 2.0});
    v.assign(4.56);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 4.56);
}

TEST(NumVectorTest, AssignmentOperatorSingleValue) {
    MutableNumVector v({1.0, 2.0});
    v = 4.56;
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 4.56);
}

TEST(NumVectorTest, AssignVector) {
    MutableNumVector v(9.9);
    std::vector<double> data = {4.4, 5.5};
    v.assign(data);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_DOUBLE_EQ(v[0], 4.4);
    EXPECT_DOUBLE_EQ(v[1], 5.5);
}

TEST(NumVectorTest, AssignEmptyVector) {
    MutableNumVector v({1.0, 2.0});
    v.assign(std::vector<double>{});
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 0.0);
}

// ## 3. 要素アクセスのテスト ##
// ------------------------------------

TEST(NumVectorTest, ElementAccessAt) {
    MutableNumVector v({1.1, 2.2});
    EXPECT_DOUBLE_EQ(v.at(0), 1.1);
    EXPECT_DOUBLE_EQ(v.at(1), 2.2);

    // 書き換え
    v.at(0) = 5.5;
    EXPECT_DOUBLE_EQ(v.at(0), 5.5);

    // 範囲外アクセスで例外がスローされることを確認
    EXPECT_THROW(v.at(2), std::out_of_range);
}

TEST(NumVectorTest, ElementAccessOperator) {
    MutableNumVector v({1.1, 2.2});
    EXPECT_DOUBLE_EQ(v[0], 1.1);
    EXPECT_DOUBLE_EQ(v[1], 2.2);

    // 書き換え
    v[1] = 6.6;
    EXPECT_DOUBLE_EQ(v[1], 6.6);
}

TEST(NumVectorTest, DataPointer) {
    MutableNumVector v({1.1, 2.2, 3.3});
    double* p = v.data();
    EXPECT_DOUBLE_EQ(p[0], 1.1);
    EXPECT_DOUBLE_EQ(p[1], 2.2);

    // ポインタ経由で書き換え
    p[0] = 7.7;
    EXPECT_DOUBLE_EQ(v[0], 7.7);
}

// ## 4. リサイズ関連のテスト ##
// ------------------------------------

TEST(NumVectorTest, Resize) {
    MutableNumVector v(1.0);

    // サイズを大きくする
    v.resize(3);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_DOUBLE_EQ(v[0], 1.0);
    EXPECT_DOUBLE_EQ(v[1], 0.0); // 新しい要素は0で初期化される想定
    EXPECT_DOUBLE_EQ(v[2], 0.0);

    // サイズを小さくする
    v.resize(2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_DOUBLE_EQ(v[0], 1.0);
    EXPECT_DOUBLE_EQ(v[1], 0.0);

    // サイズを0にする（仕様に基づき、サイズ1で値0になる）
    v.resize(0);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_DOUBLE_EQ(v[0], 0.0);
}

TEST(NumVectorTest, PushBack) {
    // 要素1の状態からpush_back
    MutableNumVector v(1.1);
    v.push_back(2.2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_DOUBLE_EQ(v[0], 1.1);
    EXPECT_DOUBLE_EQ(v[1], 2.2);

    // さらにもう一つ追加
    v.push_back(3.3);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_DOUBLE_EQ(v[2], 3.3);
}

// ## 5. 比較演算子のテスト ##
// ------------------------------------

TEST(NumVectorTest, Comparison) {
    MutableNumVector v1({1.0, 2.0});
    MutableNumVector v2({1.0, 2.0});
    MutableNumVector v3({1.0, 9.0});
    MutableNumVector v4({1.0, 2.0, 3.0});
    MutableNumVector v5(1.0);

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 == v4);
    EXPECT_FALSE(v1 == v5);

    EXPECT_FALSE(v1 != v2);
    EXPECT_TRUE(v1 != v3);
    EXPECT_TRUE(v1 != v4);
    EXPECT_TRUE(v1 != v5);
}

// ## 6. イテレータのテスト ##
// ------------------------------------

TEST(NumVectorTest, Iterators) {
    MutableNumVector v({1.5, 2.5, 3.5});

    // レンジベースforループで合計値を計算
    double sum = 0.0;
    for (double val : v) {
        sum += val;
    }
    EXPECT_DOUBLE_EQ(sum, 7.5);

    // イテレータ経由での書き換え
    double* it = v.begin();
    *it = 9.9;
    EXPECT_DOUBLE_EQ(v[0], 9.9);

    // 標準アルゴリズムとの連携
    EXPECT_DOUBLE_EQ(std::accumulate(v.begin(), v.end(), 0.0), 9.9 + 2.5 + 3.5);
}

// ## 7. NumVectorとの互換性テスト ##
// ------------------------------------

TEST(NumVectorTest, ShallowCopyWithBaseClass) {
    MutableNumVector mut_v({10.0, 20.0});
    NumVector base_v = mut_v; // 基底クラスへコピー

    // コピー先が正しいことを確認 (constアクセス)
    EXPECT_EQ(base_v.size(), 2u);
    EXPECT_DOUBLE_EQ(base_v[0], 10.0);
    EXPECT_DOUBLE_EQ(base_v[1], 20.0);

    // `MutableNumVector`側で値を変更
    mut_v[0] = 11.1;

    // `NumVector`側にも変更が反映されていることを確認
    EXPECT_DOUBLE_EQ(base_v[0], 11.1);
    EXPECT_TRUE(mut_v == base_v);
}

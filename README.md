# transition_recipe_test

## 概要
複数の ROS 2 LifecycleNode をまとめて状態管理し、システム全体の状態を
`SemanticState` として扱う検証用パッケージです。  
状態遷移は `TransitionRecipe`（複数ノードに対する操作手順）として定義し、
`Graph` で得られる state_id と遷移戦略に基づいて実行します。

## 提供する機能
- 複数 LifecycleNode の状態取得と `SemanticState` 化
- `SemanticState` から state_id を引く状態グラフ (`Graph`)
- state_id 間の遷移を `TransitionRecipe` として定義
- 遷移判定ロジック（時間/位置など）を戦略クラスとして分離
- 動作確認用のサンプル LifecycleNode（A/B/C）と簡易シミュレータ

## 仕組み（処理フロー）
1. `multiple_node_manager` が `node_ids` の各ノードに `GetState` を送信
2. 取得した状態を `SemanticState` に集約し `Graph` で state_id を決定
3. `SwitchingStrategy` が遷移条件を判定して target state を決定
4. `recipe_generator` が `TransitionRecipe` を生成
5. `ChangeState` を順番に呼び出してレシピを実行

## モジュール構成
- `include/transition_recipe_test/common_types.hpp`  
  `SemanticState` / `TransitionRecipe` / `ActionStep` などの共通型
- `include/transition_recipe_test/graph.hpp` / `graph_generator.hpp`  
  状態グラフの辞書と YAML からの初期化
- `include/transition_recipe_test/recipe_generator.hpp` / `src/recipe_generator.cpp`  
  state_id 間の遷移レシピ（手書きで定義）
- `include/transition_recipe_test/switching_strategy.hpp` / `src/switching_strategy.cpp`  
  遷移発火判定（時間・位置など）
- `src/multiple_node_manager.cpp`  
  状態取得、Graph マッチ、遷移判定、レシピ実行を統合するメインノード
- `src/sample/a_node.cpp` / `b_node.cpp` / `c_node.cpp`  
  LifecycleNode のサンプル。active 時に `cmd_vel` を出力
- `src/sample/simulator_node.cpp`  
  `cmd_vel` を簡易積分し、`odom` / `current_pose` / TF / Marker を配信

## 使い方

### ビルド
```bash
colcon build --packages-select transition_recipe_test
source install/setup.bash
```

### 起動
```bash
ros2 launch transition_recipe_test test_multiple_target.launch.py
```

### 設定ファイル
- `config/sample/multiple_nodes.yaml`  
  管理対象の `node_ids` を列挙
- `config/sample/state_graph.yaml`  
  state_id と各ノードの状態定義
- `config/sample/sim_params.yaml`  
  シミュレータの初期位置・更新レート

### Launch 引数
- `graph_yaml_path`  
  状態グラフ YAML を上書きするための引数  
  例:
```bash
ros2 launch transition_recipe_test test_multiple_target.launch.py \
  graph_yaml_path:=/path/to/state_graph.yaml
```

## 拡張ポイント
- `switching_strategy.cpp` の判定ロジックを差し替えて遷移条件を変更
- `recipe_generator.cpp` に遷移レシピを追加して組み合わせを拡充
- `state_graph.yaml` を増やしてシステム状態を追加

## 注意点
- `recipe_generator.cpp` に未定義の遷移は空レシピ（description のみ）になります。
- `graph_yaml_path` が空の場合、`multiple_node_manager` は例外を投げます。
- `multiple_node_manager` は `current_pose` を利用します（シミュレータや実機側で publish が必要）。

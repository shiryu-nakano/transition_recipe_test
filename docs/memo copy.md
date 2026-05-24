
### 現状整理（2026-05-09）
- 2つのリポジトリがある
  - それぞれ違う目的、違う機能があるが、最終的に一つにまとめる予定
- [X] ~~*このリポジトリで提供できる機能を確認する*~~ [2026-05-10]
  - graphを用いた状態管理
    - graphの実態はdict[string:state_id：semanticState]
    - lifecycle clientによってtimerでそれぞれのnodeの状態を取得して、semantic stateを作る
    - 
- [X] ~~*もう一つのリポジトリで提供する機能を確認する*~~ [2026-05-10]
  - → 状態遷移の判定をおこなう機能を実装しようとしていた．CORとstrategyをつかっていた．
  - 実装がもう一つのものとコンフリクトするので，
- [X] 最終目的を確認する
  - graphで状態管理。それぞれのノードが状態で、エッジが状態遷移
    - lifecycleと通信して管理対象ノードの状態を監視する部分 
      - 現在は
    - 外界情報をサブスクライブして状態遷移の判定を行う部分

##　現状整理（2026-05-23）
- 判定ロジックを整理する
- [X] ~~*forkしたリポジトリで，開発の準備をする*~~ [2026-05-23 13:42]
  - [X] ~~*forkしたリポジトリを実行して問題ないことを確認する*~~ [2026-05-23 13:42]


- 状態判定の実装について
  - 外部パッケージが行うことになったので今は考えなくてOK！
- COR+Strategyの実装を行う
  - 状態遷移判定自体


- demo用にまとめる/モジュールごとにまとめる
  - [X] ~~*launch/demo_launch.pyに移動する*~~ [2026-05-10]
  - [X] ~~*graph/に関係ファイルを移動させてビルド，実行テストする*~~ [2026-05-10]
  ~~- [ ] multiple_node_managerもdemo用に~~
~~    - [ ] これもインターフェースを実装したものにして，本番用もこれを実装できるようにしたい．~~
* [ ] 条件判定は条件判定だけのノードとして実装する
* [X] ~~*現在のリポジトリは不要なので、transition_recipe_testに統合する*~~ [2026-05-23 21:20]
  * [X] ~~*現在のリポジトリは消す*~~ [2026-05-23 21:20]
  * [X] ~~*judgement nodeは何？消してOK？？*~~ [2026-05-23 21:20]

**pure-pursuit/dwaの単一のlaunchをセットアップする**
- 作るもの
* [ ] これは他のrepoでやる？？→myros2devでやればいいか！！
  - dwa/p.p.を呼び出して起動する
    - [ ] dwaを動かすlaunchを作る
    - [ ] ppを動かすlaunchを作る
    - [ ] 
  - [X] ~~*ppではpath を出さなければならないので注意すること*~~ [2026-05-23 20:11]
    - path smootherでobstacle_simulationのpathを指定すれば良い


**このパッケージをopenにするための準備**
* [ ] manager_nodeも規約に沿ってhpp, cppとして書く
* [ ] manager_nodeは、状態の監視、状態遷移の実行だけを行うnodeとする
  * input: lifecycle client get state の結果
  * output: semanticState current_state (publish?)
  * operation: lifecycle client change state
    * 外部から状態遷移するように入力されたときにのみ


**pp, dwaをlifecycleに書き換える必要がある**

* [ ] 








---



このファイルが目指している方向（私の理解）
現状の動作
timer_callback (L165-243) で 500ms 毎に：

GetState を全 lifecycle node に投げて current_semantic_state_ を更新
state_graph_ で semantic → state_id に変換
その場で switcher_.decide_next_state(state_id, temp_count_, since_last, x_, y_, ...) を呼び、判定 + recipe 生成
recipe が返れば execute_transition_recipe で実行
つまり「状態取得・判定・recipe 生成・実行」を全部このノードが抱え込んでいる。

目指す姿（L131-134 と L168-172 のコメントから）
このノードの責務を縮める方向の移行作業。

残す責務：

lifecycle node 群の状態収集（GetState clients、semantic_state 構築）
state_graph_ で state_id 算出
現在の state_id と「その state になってからの timespan」を外部へ出力（L168-172 のコメント "このノードの出力" が示唆）
recipe の実行（execute_transition_recipe / execute_next_step / transition_map_）
外に出す責務：

「次にどの状態に遷移すべきか」の判定は別ノードへ移す
判定ノードが「(from, to) で遷移して欲しい」と topic を投げてくる
このノードはそれを subscribe して callback で発火させる
SwitchingStrategy の役割変更：

現在：判定 + recipe 生成（decide_next_state に座標や時間も渡している）
変更後：(from, to) → recipe を返すだけの lookup。recipe_generator をそのまま被せる形になる。
L134 の TODO：将来は外部パッケージに切り出す予定だが、移行中なのですぐには消さない。
不要になっていくもの（既にコメントアウト気味）：

pose_callback / odom_callback（L246-267 はコメントアウト済）
pose_sub_ / odom_sub_ のメンバ宣言（L123-124 もコメントアウト）
x_, y_, temp_count_ の保持と判定への引き渡し
ただし L72-78 では subscriber をまだ作っているので過渡期の混在状態
determine_next_state (L411-426) はすでにブロックコメント化済み
必要になる新規要素（推測）
判定ノードからの遷移指示用トピックの Subscription（メッセージ型は from, to を持つ何か、または単に target_state 文字列）
そのコールバックで switcher_.get_recipe(from, to) のような新インターフェースを呼ぶ
current_state_id と since_last を publish する Publisher（判定ノード側がこれを subscribe して判断する想定）
確認したい点
作業に入る前に、いくつか曖昧な部分があります：

- 判定ノードとの通信プロトコルは決まっていますか？（メッセージ型、トピック名、from/to の表現方法）
  - 判定ノードは、状態遷移が必要なときにのみtopicをpubbishする
  - topicは、tuple(string,string)で、(from_state_id, target_state_id)
  - topic名：/transition_request

- current_state_id と timespan の publish は今回の作業に含めますか？ それとも遷移指示の subscribe 側だけ？
  - 今回の作業に含めます
  - これはtimerCallbackで逐次publishします

- SwitchingStrategy の新インターフェース（(from, to) → recipe）の関数名・シグネチャの希望はありますか？
  - call_transition_recipeとかでどうでしょうか？
- 既存の decide_next_state 経路や pose/odom 関連コードは、今回どこまで削るか（完全削除 vs コメントアウトで保留）？
  - コメントアウトで保留しましょう。まだ実験なので
















**gnss-emcl用の拡張→branch:develop/area_state_switch**
- [ ] gnss, emclノードを使ったlaunchファイルの作成
  - [ ] P1:実機で実験する？
  - [ ] P2:シミュレーションベースでも，経過時間だけでもいいので一旦動かしてみてもいいかもしれない
  - [ ] P3:切り替えポイントをyamlファイルから読み込めるようにする→優先度低い．環境は変化しないので暫定でハードコーディングでも差し支えない
    - gnss→emcl→gnss→emcl→gnssと遷移することが予め決まっている
    - 実際は．．．
      - STATE_ALL_UNCONFIGURED→STATE_ALL_OFF→ (ここから開始)
      - GNSS_ONLY→EMCL_ONLY→GNSS_ONLY→EMCL_ONLY→GNSS_ONLY
        - となる
- [ ] P4:Areaの情報をpublishする→経路計画用管理のノードがこれをみて判断に使うと思われる
  - [ ] topic echoでこのトピック監視して正しいことを確認する．
- [ ] ここまででとりあえずタスクPは完了


**preliminary**
* [ ] gnss, emclがそれぞれlifecycleで普通に動くかどうかが怪しい．
* [ ]  gnssに関しては，rtk_judgeをlifecycleにしていれば良い
* [X] ~~*それぞれビルドする*~~ [2025-12-06]
  * [X] ~~*emcl*~~ [2025-12-06]
  * [X] ~~*gnss*~~ [2025-12-06]
* [ ] 単体で立ち上げて，ひとまずchange stateできるかをトピックで試す
  * [ ] gnss
  * [ ] emcl

**advanced**
* [ ] 別パッケージで実装したノードとの連携ができるかを確認する
  * [X] 特にarcanain_simulatorとの連携確認は重要事項
  * [X] simulatorを通してロボットを適当に動かす


**Advanced +**
* [ ] pure_pursuit, dwa, stop, inplace_turnと連携する→READMEに記載する
  * [ ] 4つのnode間での状態遷移をテストする
    * [ ] これは実機に乗せるロジックなので，全探索してテストすること

---

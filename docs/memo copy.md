
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
  - 　
- COR+Strategyの実装を行う
  - 状態遷移判定自体


- demo用にまとめる/モジュールごとにまとめる
  - [X] ~~*launch/demo_launch.pyに移動する*~~ [2026-05-10]
  - [X] ~~*graph/に関係ファイルを移動させてビルド，実行テストする*~~ [2026-05-10]
  ~~- [ ] multiple_node_managerもdemo用に~~
~~    - [ ] これもインターフェースを実装したものにして，本番用もこれを実装できるようにしたい．~~
* [ ] 条件判定は条件判定だけのノードとして実装する
* [ ] 現在のリポジトリは不要なので、transition_recipe_testに統合する
  * [ ] 現在のリポジトリは消す
  * [ ] judgement nodeは何？消してOK？？

**pure-pursuit/dwaの単一のlaunchをセットアップする**
- 作るもの
  - dwa/p.p.を呼び出して起動する
    - [ ] dwaを動かすlaunchを作る
    - [ ] ppを動かすlaunchを作る
    - [ ] 
  - [X] ~~*ppではpath を出さなければならないので注意すること*~~ [2026-05-23 20:11]
    - path smootherでobstacle_simulationのpathを指定すれば良い


**このパッケージをopenにするための準備**
* [ ] manager_nodeも規約に沿ってhpp, cppとして書く
* [ ] 


**pp, dwaをlifecycleに書き換える必要がある**

* [ ] 




















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

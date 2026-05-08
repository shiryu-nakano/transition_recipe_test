
# 作業めも



## 複数のLifecycleNodeがある時の実装テスト 
branch: https://github.com/shiryu-nakano/transition_recipe_test/tree/develop/applicate_multiple_nodes

### チェックリスト

- やりたいこと
  - ２つ以上のlifecycle nodeがあった時に，それらを同時に制御することでシステム全体のモードを切り替える．例えばAとBがある時，Aがon, Bがoffの状態から，Aがoff, Bがonの状態に切り替える．これを管理しやすくするために，``struct Transition Recipe``　を導入した．
  - 複数のnodeの状態の組み合わせによってシステムの状態を一意に定めたいので``struct SemanticState``を導入した．
  
⭐︎一つずつ進める．1つやったら1コミット

**状態の管理**
* [X] ~~*複数のノードがある時にそれらの状態を一度に取得する*~~ [2025-11-24]
  * [X] ~~*node_idをlistに集約しておいてその中身の状態を取得してLOGに出力する*~~ [2025-11-24]
    * [X] ~~*node_idを引数に，そのnodeの状態を返すメソッドの実装→これをforで回して上記を実現する*~~ [2025-11-24]
  * [X] ~~*node_idのlistを外部yamlから作成できるようにする*~~ [2025-11-24]
  * [X] ~~*取得した状態を，``struct SemanticState``として組み立てる*~~ [2025-11-24]
    * [X] ~~*それをLOGに出力する→直接出力した時と結果が変わらないことを確認する．*~~ [2025-11-24]
  * [X] ~~*dict:stateGraph key:SemanticState value:string node_id を**手書きで**実装する*~~ [2025-11-24]
    * [X] ~~*class graphを実装する*~~ [2025-11-24]
    * [X] ~~*上でリアルタイム取得・生成したSemanticStateによって検索できることを確認する*~~ [2025-11-24]
    * [X] ~~*graphをyamlから生成できるようにすること（実態はただの辞書）*~~ [2025-11-25]

**状態の遷移**
* [X] ~~*TransitionRecipeを**手書きで**作って実行する*~~ [2025-11-25]
  * [X] ~~***状態の管理**で実装したLOG出力から**複数のノードが正しく状態遷移している**ことを確認する*~~ [2025-11-25]
  * [ ] 全ての状態遷移に対応するRecipeを**手書きで**書いて実行すること
    * だるすぎる．．．
*  yamlから対応するTransitionRecipeを自動生成すること
    * これはpending

**状態遷移グラフに基づいた管理**
* [X] ~~*state_id (つまりSemanticState->state.id)を引数に，指定した状態に遷移する機能*~~ [2025-11-25]
  * [X] ~~*ノードが3つあれば${2^3}$通り*~~ [2025-11-25]
  * これは本番環境で使わない可能性が高いがテストのために実装してcommitしておく
  * 

**シミュレータの利用**
* [X] ~~*実際にロボットを制御するnodeに拡張する*~~ [2025-11-25]
  * [X] ~~*cmd_velをon_activeで吐き出すノードを実装する*~~ [2025-11-25]
* [X] ~~*simulatorようのノードを実装する*~~ [2025-11-25]
  * [X] ~~*それぞれのノードでsimulatorを使って意図通りに動くか確認する*~~ [2025-11-25]
* [X] ~~*状態遷移の仕組みによって，シミュレーション上でシステム状態の管理ができることを動画で確認する*~~ [2025-11-25]

**整理**
* [X] ~~*ここまできたらpackage内の依存関係と配置を整理する．今回のテストに依存している部分をクリーンにする*~~ [2025-11-26]
  * 次のステップに遷移条件判定が入ってくるので綺麗にしておく必要がある
  * 遷移条件判定を完全に独立させて実装して，今回の状態管理ノードにくっつけられるor一緒に使えるようにする
  * もしかしてareaの判定は別ノードでやるべきか？
  * さらに言えば状態管理と，状態遷移は分けるべきか共存すべきか？
  * いずれにせよ，今作っているものを土台にして考えること

* [X] arcanain_simulator simulatornodeで可視化できるようにする
  * [X] launchでsimulator nodeを読んであれこれ設定すれば動いてくれるはず

---

**状態遷移判定の実装**
これまで秒数カウントなどで状態遷移を実験してきた
* [X] ~~*シミュレーションができるようになったら，場所によってareaの切り替えを行うロジックを実装する→（A）*~~ [2025-12-06]
  * [X] ~~*上記ロジックのテストを行う*~~ [2025-12-06]
    * [X] ~~*subscriberでposeを持って来れるか確認*~~ [2025-12-06]
    * [X] ~~*B1それをもとになんらか条件判定で切り替える*~~ [2025-12-06] 
  * [X] ~~*B2上記の条件判定を行うためのダミークラス，ダミー関数を作成して呼び出す形式に変更*~~ [2025-12-06]
    * [X] ~~*transition strategy?*~~ [2025-12-06]
    * [X] ~~*B3それでも同様の挙動になるのを確認する*~~ [2025-12-06]
    * [X] ~~*B4 外部クラスの関数で判定できるようになったら，その関数から直接所望の``Transition recipe``をもらうように変更する*~~ [2025-12-06]

---
**gnss-emcl用の拡張→branch:develop/area_state_switch**
- [ ] gnss, emclノードを使ったlaunchファイルの作成
  - [ ] P1:実機で実験する？
  - [ ] P2:シミュレーションベースでも，経過時間だけでもいいので一旦動かしてみてもいいかもしれない
  - [ ] P3:切り替えポイントをyamファイルから読み込めるようにする→優先度低い．環境は変化しないので暫定でハードコーディングでも差し支えない
    - gnss→emcl→gnss→emcl→gnssと遷移することが予め決まっている
    - 実際は．．．
      - STATE_ALL_UNCONFIGURED→STATE_ALL_OFF→ (ここから開始)
      GNSS_ONLY→EMCL_ONLY→GNSS_ONLY→EMCL_ONLY→GNSS_ONLY
      となる
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

# LOG


<details><summary>bash</summary>

```bash
ubuntu@dff5a6f80385:~/ros2_ws$ ros2 launch transition_recipe_test test_multiple_target.launch.py
[INFO] [launch]: All log files can be found below /home/ubuntu/.ros/log/2025-11-24-11-48-51-022423-dff5a6f80385-3158
[INFO] [launch]: Default logging verbosity is set to INFO
[INFO] [a_node-1]: process started with pid [3159]
[INFO] [b_node-2]: process started with pid [3161]
[INFO] [c_node-3]: process started with pid [3163]
[INFO] [multiple_node_manager-4]: process started with pid [3165]
[a_node-1] [INFO] [1763984931.171355697] [A_node]: A_node constructed
[c_node-3] [INFO] [1763984931.217189349] [C_node]: C_node constructed
[b_node-2] [INFO] [1763984931.219456658] [B_node]: B_node constructed
[multiple_node_manager-4] [INFO] [1763984931.224357842] [recipe_test_node]: MultipleNodeManager started
[multiple_node_manager-4] [INFO] [1763984931.968986368] [recipe_test_node]: Hello, elapsed 0.50 sec
[multiple_node_manager-4] [INFO] [1763984931.972011224] [recipe_test_node]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984931.972072763] [recipe_test_node]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984931.972111928] [recipe_test_node]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.226660937] [recipe_test_node]: Hello, elapsed 1.00 sec
[multiple_node_manager-4] [INFO] [1763984932.227242909] [recipe_test_node]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.227400443] [recipe_test_node]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.227418317] [recipe_test_node]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.725662391] [recipe_test_node]: Hello, elapsed 1.50 sec
[multiple_node_manager-4] [INFO] [1763984932.726934706] [recipe_test_node]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.726974079] [recipe_test_node]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984932.726994870] [recipe_test_node]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.227736237] [recipe_test_node]: Hello, elapsed 2.00 sec
[multiple_node_manager-4] [INFO] [1763984933.228611196] [recipe_test_node]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.228640653] [recipe_test_node]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.228681692] [recipe_test_node]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.726043966] [recipe_test_node]: Hello, elapsed 2.50 sec
[multiple_node_manager-4] [INFO] [1763984933.726967922] [recipe_test_node]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.727037502] [recipe_test_node]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763984933.727071584] [recipe_test_node]: [C_node] current lifecycle state: id=1, label=unconfigured

```

</details>

これに対して他のターミナルで以下を実行することでnode A~Cがunconfigureからinactivateに変わる．
```bash
ros2 service call /A_node/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
ros2 service call /B_node/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
ros2 service call /C_node/change_state lifecycle_msgs/srv/ChangeState "{transition: {id: 1}}"
```
結果
```bash
[multiple_node_manager-4] [INFO] [1763985306.654051538] [recipe_test_node]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985306.654089038] [recipe_test_node]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985306.654166661] [recipe_test_node]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985307.155331634] [recipe_test_node]: Hello, elapsed 306.01 sec
[multiple_node_manager-4] [INFO] [1763985307.155640045] [recipe_test_node]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985307.155845958] [recipe_test_node]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985307.156272867] [recipe_test_node]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763985307.654512020] [recipe_test_node]: Hello, elapsed 306.51 sec

```
inactiveからactiveも同様である．

ここまでで，複数のノードの状態を取得する部分については実装した

* [X] ~~*commitすること*~~ [2025-11-24]

---

- SemanticStateを構築する
``request_get_all_semantic_state()``
``request_get_semantic_state(nodeid, getstate cliend)``
を実装した．
出力結果は

<details><summary>bash</summary>

```bash

[multiple_node_manager-4] [INFO] [1763988918.458830930] [multiple_node_manager]: Hello, elapsed 12.00 sec
[multiple_node_manager-4] [INFO] [1763988918.459622377] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988918.459682671] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988918.459700255] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988918.459708547] [multiple_node_manager]: [SemanticState] snapshot:
[multiple_node_manager-4] [INFO] [1763988918.459739549] [multiple_node_manager]:   [C_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988918.459748049] [multiple_node_manager]:   [B_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988918.459753174] [multiple_node_manager]:   [A_node] semantic_state=UNCONFIGURED
[a_node-1] [INFO] [1763988918.602272009] [A_node]: [A_node] CONFIGURED
[a_node-1] [WARN] [1763988918.702624609] [A_node.rclcpp]: failed to send response to /A_node/change_state (timeout): client will not receive response, at ./src/rmw_response.cpp:154, at ./src/rcl/service.c:314
[multiple_node_manager-4] [INFO] [1763988918.959318210] [multiple_node_manager]: Hello, elapsed 12.50 sec
[multiple_node_manager-4] [INFO] [1763988918.960796185] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763988918.960907773] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988918.961127406] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988918.961143824] [multiple_node_manager]: [SemanticState] snapshot:
[multiple_node_manager-4] [INFO] [1763988918.961152532] [multiple_node_manager]:   [B_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988918.961160199] [multiple_node_manager]:   [C_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988918.961166700] [multiple_node_manager]:   [A_node] semantic_state=INACTIVE
[multiple_node_manager-4] [INFO] [1763988919.458938001] [multiple_node_manager]: Hello, elapsed 13.00 sec
[multiple_node_manager-4] [INFO] [1763988919.459774492] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763988919.459927289] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988919.459964416] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763988919.459973416] [multiple_node_manager]: [SemanticState] snapshot:
[multiple_node_manager-4] [INFO] [1763988919.459981250] [multiple_node_manager]:   [C_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988919.459987583] [multiple_node_manager]:   [B_node] semantic_state=UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763988919.459992958] [multiple_node_manager]:   [A_node] semantic_state=INACTIVE


```

</details>

---


```c++: Graph.cpp
#include "local_planning_manager/core/graph.hpp"

namespace local_planning_manager
{
    Graph::Graph() = default;

    // constractor
    Graph::Graph(std::unordered_map<std::string, SemanticState> state_dictionary)
        : state_dictionary_(std::move(state_dictionary))
    {
    }

    std::optional<std::string> Graph::getCurrentSemanticState(SemanticState semantic) const noexcept
    {
        for (const auto& [state_id, state_semantic] : state_dictionary_)
        {
            if (state_semantic == semantic)
            {
                return state_id;
            }
        }
        return std::nullopt;
    }

} // namespace local_planning_manager
```

---

- 状態遷移の最小限の実験結果

<details><summary>bash</summary>
ubuntu@dff5a6f80385:~/ros2_ws$ colcon build --packages-select transition_recipe_test
Starting >>> transition_recipe_test
Finished <<< transition_recipe_test [3.32s]                     

Summary: 1 package finished [3.51s]
ubuntu@dff5a6f80385:~/ros2_ws$ source install/setup.bash
ubuntu@dff5a6f80385:~/ros2_ws$ ros2 launch transition_recipe_test test_multiple_target.launch.py
[INFO] [launch]: All log files can be found below /home/ubuntu/.ros/log/2025-11-24-15-46-16-321936-dff5a6f80385-7514
[INFO] [launch]: Default logging verbosity is set to INFO
[INFO] [a_node-1]: process started with pid [7515]
[INFO] [b_node-2]: process started with pid [7517]
[INFO] [c_node-3]: process started with pid [7519]
[INFO] [multiple_node_manager-4]: process started with pid [7529]
[a_node-1] [INFO] [1763999176.426554573] [A_node]: A_node constructed
[c_node-3] [INFO] [1763999176.449955627] [C_node]: C_node constructed
[b_node-2] [INFO] [1763999176.450712419] [B_node]: B_node constructed
[multiple_node_manager-4] [INFO] [1763999176.455728963] [multiple_node_manager]: According to YAML file, This node is managing 3 nodes:
[multiple_node_manager-4] [INFO] [1763999176.456793255] [multiple_node_manager]:   - A_node
[multiple_node_manager-4] [INFO] [1763999176.456942922] [multiple_node_manager]:   - B_node
[multiple_node_manager-4] [INFO] [1763999176.456946880] [multiple_node_manager]:   - C_node
[multiple_node_manager-4] [INFO] [1763999176.458803506] [multiple_node_manager]: Created ChangeState and GetState clients for node 'A_node'
[multiple_node_manager-4] [INFO] [1763999176.460435757] [multiple_node_manager]: Created ChangeState and GetState clients for node 'B_node'
[multiple_node_manager-4] [INFO] [1763999176.460742466] [multiple_node_manager]: Created ChangeState and GetState clients for node 'C_node'
[multiple_node_manager-4] [INFO] [1763999176.460748007] [multiple_node_manager]: Loading state graph from YAML: /home/ubuntu/ros2_ws/install/transition_recipe_test/share/transition_recipe_test/config/state_graph.yaml
[multiple_node_manager-4] [INFO] [1763999176.462590050] [multiple_node_manager]: State graph initialized with 9 states.
[multiple_node_manager-4] [INFO] [1763999176.462876009] [multiple_node_manager]: MultipleNodeManager started
[multiple_node_manager-4] [WARN] [1763999176.964736812] [multiple_node_manager]: [StateGraph] no match for current SemanticState
[multiple_node_manager-4] [INFO] [1763999177.413853799] [multiple_node_manager]: Hello, elapsed 0.50 sec
[multiple_node_manager-4] [INFO] [1763999177.414002174] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.414031966] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.414086007] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.463184158] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999177.463779325] [multiple_node_manager]: Hello, elapsed 1.00 sec
[multiple_node_manager-4] [INFO] [1763999177.464367034] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.464413617] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.464804826] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.965135173] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999177.965742632] [multiple_node_manager]: Hello, elapsed 1.50 sec
[multiple_node_manager-4] [INFO] [1763999177.967779383] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.967888966] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999177.967917341] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.466919190] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999178.468007858] [multiple_node_manager]: Hello, elapsed 2.00 sec
[multiple_node_manager-4] [INFO] [1763999178.468981650] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.469031442] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.469050233] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.963208580] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999178.963882372] [multiple_node_manager]: Hello, elapsed 2.50 sec
[multiple_node_manager-4] [INFO] [1763999178.965313331] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.965414623] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999178.966115998] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.463492891] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999179.464010267] [multiple_node_manager]: Hello, elapsed 3.00 sec
[multiple_node_manager-4] [INFO] [1763999179.464918684] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.464988350] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.465516726] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.965954912] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999179.966326662] [multiple_node_manager]: Hello, elapsed 3.50 sec
[multiple_node_manager-4] [INFO] [1763999179.968090205] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.968184872] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999179.968208122] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.463539058] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999180.463733183] [multiple_node_manager]: Hello, elapsed 4.00 sec
[multiple_node_manager-4] [INFO] [1763999180.464308433] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.464350892] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.464535975] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.966119957] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999180.967783791] [multiple_node_manager]: Hello, elapsed 4.50 sec
[multiple_node_manager-4] [INFO] [1763999180.967916916] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.969173708] [multiple_node_manager]: [A_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999180.969360875] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999181.463511814] [multiple_node_manager]: [StateGraph] matched system_state = ALL_UNCONFIGURED
[multiple_node_manager-4] [INFO] [1763999181.463996314] [multiple_node_manager]: Hello, elapsed 5.00 sec
[multiple_node_manager-4] [INFO] [1763999181.464014522] [multiple_node_manager]: Starting TransitionRecipe: Configure all managed nodes (UNCONFIGURED -> INACTIVE)
[multiple_node_manager-4] [INFO] [1763999181.464024814] [multiple_node_manager]: Step 1 / 3: configure -> A_node (timeout=3.0s)
[a_node-1] [INFO] [1763999181.464933189] [A_node]: [A_node] CONFIGURED
[multiple_node_manager-4] [INFO] [1763999181.465815148] [multiple_node_manager]: Transition 'configure' for A_node succeeded
[multiple_node_manager-4] [INFO] [1763999181.465883690] [multiple_node_manager]: Step 2 / 3: configure -> B_node (timeout=3.0s)
[multiple_node_manager-4] [INFO] [1763999181.465970232] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999181.465991815] [multiple_node_manager]: [B_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999181.466008023] [multiple_node_manager]: [C_node] current lifecycle state: id=1, label=unconfigured
[multiple_node_manager-4] [INFO] [1763999181.466500774] [multiple_node_manager]: Transition 'configure' for B_node succeeded
[multiple_node_manager-4] [INFO] [1763999181.466541440] [multiple_node_manager]: Step 3 / 3: configure -> C_node (timeout=3.0s)
[multiple_node_manager-4] [INFO] [1763999181.467385066] [multiple_node_manager]: Transition 'configure' for C_node succeeded
[multiple_node_manager-4] [INFO] [1763999181.467445482] [multiple_node_manager]: TransitionRecipe finished.
[b_node-2] [INFO] [1763999181.466250065] [B_node]: [B_node] CONFIGURED
[c_node-3] [INFO] [1763999181.466962732] [C_node]: [C_node] CONFIGURED
[multiple_node_manager-4] [WARN] [1763999181.964037881] [multiple_node_manager]: [StateGraph] no match for current SemanticState
[multiple_node_manager-4] [INFO] [1763999181.964627631] [multiple_node_manager]: Hello, elapsed 5.50 sec
[multiple_node_manager-4] [INFO] [1763999181.965468382] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999181.965978882] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999181.967681716] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.464466659] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999182.465107867] [multiple_node_manager]: Hello, elapsed 6.00 sec
[multiple_node_manager-4] [INFO] [1763999182.466046701] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.466118576] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.466263326] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.965088187] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999182.965863688] [multiple_node_manager]: Hello, elapsed 6.50 sec
[multiple_node_manager-4] [INFO] [1763999182.966470646] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.967542355] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999182.967780855] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.466497177] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999183.466861469] [multiple_node_manager]: Hello, elapsed 7.00 sec
[multiple_node_manager-4] [INFO] [1763999183.467968886] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.468044011] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.468069136] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.964564665] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999183.964971040] [multiple_node_manager]: Hello, elapsed 7.50 sec
[multiple_node_manager-4] [INFO] [1763999183.965934541] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.965981833] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999183.966016416] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.467209408] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999184.467814950] [multiple_node_manager]: Hello, elapsed 8.00 sec
[multiple_node_manager-4] [INFO] [1763999184.470260202] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.470366327] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.470392744] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.963933899] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999184.964460274] [multiple_node_manager]: Hello, elapsed 8.50 sec
[multiple_node_manager-4] [INFO] [1763999184.964764274] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.965441775] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999184.965520025] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.463689643] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999185.464057727] [multiple_node_manager]: Hello, elapsed 9.00 sec
[multiple_node_manager-4] [INFO] [1763999185.465257394] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.465286352] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.465296811] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.964322305] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999185.964744139] [multiple_node_manager]: Hello, elapsed 9.50 sec
[multiple_node_manager-4] [INFO] [1763999185.965546181] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.965827264] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999185.966775057] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.463211010] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999186.463821427] [multiple_node_manager]: Hello, elapsed 10.00 sec
[multiple_node_manager-4] [INFO] [1763999186.464897720] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.464970886] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.465004553] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.963950050] [multiple_node_manager]: [StateGraph] matched system_state = STATE_ALL_OFF
[multiple_node_manager-4] [INFO] [1763999186.964518300] [multiple_node_manager]: Hello, elapsed 10.50 sec
[multiple_node_manager-4] [INFO] [1763999186.966250426] [multiple_node_manager]: [A_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.966341135] [multiple_node_manager]: [B_node] current lifecycle state: id=2, label=inactive
[multiple_node_manager-4] [INFO] [1763999186.966362010] [multiple_node_manager]: [C_node] current lifecycle state: id=2, label=inactive
^C[WARNING] [launch]: user interrupted with ctrl-c (SIGINT)
[b_node-2] [INFO] [1763999187.092461500] [rclcpp]: signal_handler(signum=2)
[multiple_node_manager-4] [INFO] [1763999187.092461625] [rclcpp]: signal_handler(signum=2)
[c_node-3] [INFO] [1763999187.092594083] [rclcpp]: signal_handler(signum=2)
[a_node-1] [INFO] [1763999187.092732083] [rclcpp]: signal_handler(signum=2)
[INFO] [b_node-2]: process has finished cleanly [pid 7517]
[INFO] [multiple_node_manager-4]: process has finished cleanly [pid 7529]
[INFO] [a_node-1]: process has finished cleanly [pid 7515]
[INFO] [c_node-3]: process has finished cleanly [pid 7519]
ubuntu@dff5a6f80385:~/ros2_ws$ 

</details>
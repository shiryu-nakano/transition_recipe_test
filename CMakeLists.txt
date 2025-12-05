cmake_minimum_required(VERSION 3.8)
project(transition_recipe_test)

# コンパイルオプション
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 必要パッケージ（実行ノードが使う）
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)



# ========================================
# ヘッダを公開（include/transition_recipe_test）
# ========================================

# Graph や SemanticState などを使う全ターゲットで共通
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ========================================
# graph_lib: ROS 非依存の純粋
# ========================================
add_library(graph_lib
  src/graph.cpp
  src/graph_generator.cpp
)
target_include_directories(graph_lib PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(graph_lib
  yaml-cpp
)
# ============================================
# recipe_library（ROS 非依存）
# ============================================
add_library(recipe_lib
  src/recipe_generator.cpp
)

target_include_directories(recipe_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#　Node群

# ========================================
# simulator_node
# ========================================
add_executable(simulator_node
  src/sample/simulator_node.cpp
)
ament_target_dependencies(simulator_node
  rclcpp
  geometry_msgs
  nav_msgs
  visualization_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
)
target_include_directories(simulator_node PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)


# ========================================
# multiple_node_manager
# ========================================
add_executable(multiple_node_manager
  src/multiple_node_manager.cpp
)
ament_target_dependencies(multiple_node_manager
  rclcpp
  lifecycle_msgs
  geometry_msgs
)
target_include_directories(multiple_node_manager PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# ★ Graph を使うのでここだけ graph_lib をリンク
target_link_libraries(multiple_node_manager
  graph_lib
  recipe_lib
)

# ========================================
# A_node / B_node / C_node（各 LifecycleNode）
# ========================================
add_executable(a_node
  src/sample/a_node.cpp
)
ament_target_dependencies(a_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  geometry_msgs
)
target_include_directories(a_node PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)

add_executable(b_node
  src/sample/b_node.cpp
)
ament_target_dependencies(b_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  geometry_msgs
)
target_include_directories(b_node PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)

add_executable(c_node
  src/sample/c_node.cpp
)
ament_target_dependencies(c_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  geometry_msgs
)
target_include_directories(c_node PUBLIC
  $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# ========================================
# インストール設定
# ========================================

# 実行ファイルとライブラリをインストール
install(TARGETS
  multiple_node_manager
  a_node
  b_node
  c_node
  graph_lib
  recipe_lib
  simulator_node     
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib           # 静的ライブラリ
  LIBRARY DESTINATION lib           # 共有ライブラリ
  RUNTIME DESTINATION lib/${PROJECT_NAME}  # 実行ファイル
)

# launch ファイル
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# config（YAML）
install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

# rviz（RViz設定ファイル）
install(DIRECTORY rviz
  DESTINATION share/${PROJECT_NAME}
)

# ヘッダ（common_types.hpp, graph.hpp など）
install(DIRECTORY include/
  DESTINATION include
)

ament_export_targets(export_${PROJECT_NAME})
ament_package()

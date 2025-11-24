cmake_minimum_required(VERSION 3.8)
project(transition_recipe_test)

# コンパイルオプション
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 必要パッケージ
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)

# ========================
# sample_lifecycle_node
# ========================
add_executable(sample_lifecycle_node
  src/sample_lifecycle_node.cpp
)
ament_target_dependencies(sample_lifecycle_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
)

# ========================
# test_recipe_node
# ========================
add_executable(test_recipe_node
  src/test_recipe_node.cpp
)
ament_target_dependencies(test_recipe_node
  rclcpp
  lifecycle_msgs
)

# ========================
# multiple_node_manager
# ========================
add_executable(multiple_node_manager
  src/multiple_node_manager.cpp
)
ament_target_dependencies(multiple_node_manager
  rclcpp
  lifecycle_msgs
) 

# ========================
# A_node
# ========================
add_executable(a_node
  src/a_node.cpp
)
ament_target_dependencies(a_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
)
target_include_directories(a_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# ========================
# B_node
# ========================
add_executable(b_node
  src/b_node.cpp
)
ament_target_dependencies(b_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
)
target_include_directories(b_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# ========================
# C_node
# ========================
add_executable(c_node
  src/c_node.cpp
)
ament_target_dependencies(c_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
)
target_include_directories(c_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)



# このパッケージのヘッダを公開
target_include_directories(sample_lifecycle_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(test_recipe_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(multiple_node_manager PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# === install に追加 ===

# 実行ファイルのインストール / コンパイル済みライブラリのインストール
install(TARGETS
  sample_lifecycle_node
  test_recipe_node
  multiple_node_manager
  a_node
  b_node
  c_node
  DESTINATION lib/${PROJECT_NAME}
)


# launch ファイルのインストール
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# includeのインストール（common_types.hpp 含む）
install(DIRECTORY include/
  DESTINATION include
)

# config ファイルのインストール
install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)
ament_package()

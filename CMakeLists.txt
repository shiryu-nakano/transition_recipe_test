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

# このパッケージのヘッダを公開
target_include_directories(sample_lifecycle_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(test_recipe_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 実行ファイルのインストール
install(TARGETS
  sample_lifecycle_node
  test_recipe_node
  DESTINATION lib/${PROJECT_NAME}
)

# launch ファイルのインストール
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# ヘッダのインストール（common_types.hpp 含む）
install(DIRECTORY include/
  DESTINATION include
)

ament_package()

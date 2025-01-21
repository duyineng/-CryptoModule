## 1 windows版本

## 2 linux版本
使用git clone https://github.com/google/googletest.git命令，将得到googletest仓库
cd googletest进入到这个仓库
使用git checkout tags/v1.15.2将检出到这个特定的提交
之后将整个googletest仓库拷贝到third-part文件夹下，就得到了整个gtest源代码

在server目录下的CMakeLists.txt顶层cmake文件中，添加add_subdirectory(third-part/googletest)，这样CMakeLists.txt顶层文件就能够使用third-part/googletest下的CMakeLists.txt文件


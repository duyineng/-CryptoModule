## 1 windows版本

到github上下载protobuf-cpp-3.19.5.tar.gz压缩包，因为再往后更新的版本，安装的过程非常曲折。其中的v25版本其实就是3.25版本。

解压缩之后用cmake的gui界面进行configure，在where is the source code中选择protobuf-3.19.5/cmake这个目录，对于where to build the binaries则自己指定，之后点击configure来生成

对CMAKE_CONFIGURATION_TYPES中勾选protobuf_BUILD_PROTOC_BINARIES和prtobuf_BUILD_SHARED_LIBS，其他都取消勾选，再点击configure，generate

打开生成的.sln文件，对着解决方案选择生成解决方案，就开始构建了，在生成的Debug目录下找到protoc.exe，libprotocd.dll，libprotobufd.dll，然后放到自己创建的protobuf文件夹中，归类放入bin和lib目录

但是还需要头文件，头文件就从linux版本的头文件中拷贝过来。到linux的/usr/local/include中将整个google文件夹用tar命令打个包，再用ftp工具传到windows中，解压后放入inc中，这样就完成了windows中protobuf的安装

将protc.exe的目录添加到环境变量，就可以在任意目录下使用它

在vs2022中使用protobuf需要添加PROTOBUF_USE_DLLS宏

## 2 linux版本

tar zxvf protobuf-cpp-3.21.12.tar.gz	 解压缩

cd protobuf-3.21.12	进入目录

./configure	执行命令

make	执行命令

sudo make install	执行命令

这样就安装好了，接下来测试

输入protoc --version命令，输出protoc: error while loading shared libraries: libprotoc.so.32: cannot open shared object file: No such file or directory，表明执行protoc的时候找不到libprotoc.so.32动态库，输入sudo find / -name libprotoc.so.32命令，可搜索该动态库的目录，接着打开vim /etc/ld.so.conf，将libprotoc.so.32的路径/usr/local/lib添加到该文件的末尾中，保存退出，执行sudo ldconfig

再执行protoc --version就有显示版本号了

## 3 使用

protobuf安装：
sudo apt install protobuf-compiler

protobuf使用：
protoc Message.proto --cpp_out=. 

protobuf卸载：
sudo apt remove protobuf-compiler

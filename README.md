## 项目架构

client文件夹用来存放客户端程序的代码，将生成client可执行程序，运行于windows平台，作为中间件服务于业务程序，与业务程序之间通过共享内存来通信，因此client和业务程序部署于同一台windows下。
业务客户端程序在启动后，通过ftok()函数获得共享内存的key，并设置"SHM_KEY"环境变量的值为to_string(key)，之后用户通过输入账号密码登录业务客户端程序，然后往共享内存中存入业务客户端ID，业务服务器ID，server的IP，server的端口等数据，之后

server文件夹用来存放服务器程序的代码，将生成server可执行程序，运行于linux平台，作为中间件服务于业务程序，与业务程序之间通过共享内存来通信，因此server和业务程序部署于同一台linux下。



external_encryption_interface为外联接口类的源代码

share为客户端程序和服务器程序两者共享的代码

## 依赖库

protobuffer [需安装]

openssl



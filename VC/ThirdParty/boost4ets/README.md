[TOC]

# boost4ets

## 介绍

`boost::process`是个比较实用的进程管理工具，但因为VC2010对`C++11`标准支持不完整，所以编译会报错，无法正常使用。

`boost4ets`最初开发的初衷，就是把`boost::process`移植到VC2010中使用。

其实，更好的选择，应该是直接将VC升级到2019版本，这样什么都不用改，就能完美支持boost。

不过本人一直觉得，只有重复造车轮，才能真正学习到别人代码的设计原理，这样得到的东西，才是自己的。

在移植过程中，用到的一些技术要点及有意思的技巧，可参考[process移植笔记](doc/process移植笔记.md)。

# 相关链接

* 作者邮箱: xinghun87@163.com
* 作者博客：[https://blog.csdn.net/xinghun61](https://blog.csdn.net/xinghun61)
* process移植笔记：[process移植笔记](doc/process移植笔记.md)

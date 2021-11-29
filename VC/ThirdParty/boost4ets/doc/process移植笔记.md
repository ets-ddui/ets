[TOC]

# process移植笔记

## 元函数

`元函数`是在`boost::mpl`的文档中看到的，感觉挺有意思，而且也被process大量使用，因此，有必要先做个铺垫，这样后面的内容才好展开。

在理解元函数之前，先看看C++中的一个常规函数长啥样，两者结合，可能会更容易理解元函数的概念。

### 常规函数的基本要素

```C++
int MyFunc(int fld1)
{
    //根据实际需要，干点有意义的事情

    return 111;
}
```

上面定义了一个函数，名字为MyFunc，其包含3个重要部分：

* 入参
* 函数体
* 返回结果

MyFunc的作用是：将输入的入参fld1，经过一定的处理后，得到我们想要的结果111。

### 元函数使用场景举例

为了对元函数有个初步印象，我们先来看一个实际的使用场景。

试想有个通讯模块，负责将本机的数据发给远程主机，这时，我们要考虑以下这些问题：

* 对于整数类型，不同机器的字节序不一样，有的是小端，有的是大端，传输时，统一用大端
* 字符串，我们希望增加一个长度指示符，用于接收方计算字符串的实际长度
* 浮点数，不同机器的表示格式不尽相同，我们将其转换为整数后，再发送

按面向对象的设计思想，我们自然会想到，将不同的类型，封装成不同的类。例如，CInt负责整数的大小端转换，CStr负责字符串的处理，CFloat负责将浮点数转换为64位的整数值。

接下来，我们还需要定义一个Send函数，负责数据的发送：

```C++
template<typename T>
int Send(const T &t)
{
    MyFunc<T> s(t); //注意，这里的代码是不合法的，C++不支持这种语法，编译会报错

    //将s转换后的内容，发送出去
}
```

上面代码想实现的目标是：

1. 入参T对应本机的实际类型int、char *、float等。
2. Send中，先将`t`转换为对应的CInt、CStr、CFloat的实例，进行数据转换，然后，将转换后的数据，发送出去。

这里的关键点在于对s的类型声明，如果有一个类似MyFunc的“函数”，可以将输入类型T，转换为我们想要的CInt、CStr等类型的话，上面的代码就能通过编译了。

### 元函数定义

C++中并没有元函数的声明语法，在mpl中，是借用模板来实现的。

```C++
template<typename fld1> //元函数入参
struct MyFunc
{
    typedef ...... var1; //可使用typedef定义一些变量
    //std和mpl中，提供了一些标准模板，可实现类似条件判断、循环等操作

    typedef ...... type; //这个type就是元函数的返回结果，算是mpl中约定俗成的做法
};
```

在MyFunc这个模板类的“{}”内部，相当于是元函数的函数体，模板参数就是元函数的入参，type为出参。

而我们前面写的Send函数，就变成下面这个样子：

```C++
template<typename T>
int Send(const T &t)
{
    MyFunc<T>::type s(t); //区别就在这里

    //将s转换后的内容，发送出去
}
```

在真实世界中，元函数的实现往往不止一个模板定义，而是使用了模板部分特化技术，分多个模板实现。代码往往会分散在多个文件中，元函数更多只算是一种概念上的抽象。
如下样例所示：

```C++
//通用声明
template<typename fld1>
struct MyFunc
{
};
//针对每种类型的特化版本
template<>
struct MyFunc<int>
{
    typedef CInt type;
};
template<>
struct MyFunc<char *>
{
    typedef CStr type;
};
template<>
struct MyFunc<float>
{
    typedef CFloat type;
};
```

## child类

child算是本次移植最核心的一个类，也是最有意思的一个类。
在C++中定义一个函数，形参个数是固定的，而且入参顺序也必须和定义完全一致。
但child却非常“另类”，并没遵循这个基本原则，构造时，参数可以随便输。
这种用法，我只在脚本语言中看到过。

为了实现以上用法，process通过下面这个特殊的构造函数，达到此目的：

```C++
// <boost/process/detail/child_decl.hpp>
class child
{
public:
    template<typename ...Args>
    explicit child(Args&&...args);
}

// <boost/process/child.hpp>
template<typename ...Args>
child::child(Args&&...args)
    : child(::boost::process::detail::execute_impl(std::forward<Args>(args)...)) {}
```

核心逻辑在execute_impl这个函数中，其位于“<boost/process/detail/execute_impl.hpp>”，此函数只是做了下字符串的转换，只要入参中包含一个unicode字符串，就将所有ansi字符串转为unicode字符串。然后，将转换后的参数转发给basic_execute_impl处理，而child构造的核心逻辑，就在此函数中。

### child的构造过程

在Windows中，创建进程的API是[CreateProcess](https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa)。
这个函数可以说是相当复杂，光入参就有10个，其中lpStartupInfo这个结构体，又有18个成员变量，这样算下来，就有27个要素需要关注。

process的做法是将这27个要素按功能相近程度进行分类，每个类别定义一个类来处理(所有类均从handler_base继承)。
例如，处理镜像文件路径、命令行入参，使用类exe_cmd_init，工作目录设置用类start_dir_init，异步管道用async_pipe_in、async_pipe_out等等。

basic_execute_impl先将入参划分为两类：

1. 字符串等原始类型
2. async_pipe_in这类从handler_base继承的类型

之后的所有工作，实际上就是把第一类入参，转换为handler_base的子类，然后，再和第二类入参重新组合成新的数组，再交给executor类创建进程。

### make_builders_from_view

make_builders_from_view是个元函数，其作用是将第一类入参，转换为builder类的set集合。
转换过程分为两个步骤，先使用initializer_tag元函数找tag标识类，
然后，再用这个标识类，通过initializer_builder元函数，找对应的builder类。

#### append_set

多个入参有可能对应同一个builder类，make_builders_from_view在处理时，需要提供相同类合并的能力。
由于在boost::fusion中没找到类似的功能，因此，就仿造as_vector，写了这个append_set元函数。
这个元函数有两个入参：set类型的类型集合、新类型。
如果新类型不在set中，append_set就会将新类型添加到set中，得到一个新的set。

其中用到的关键技巧就在于对boost::fusion::result_of::size和迭代器的使用。
使用size获取原set中类型的实际个数，再通过迭代器将类型展开，之后和新类型一起，用as_set元函数构造新set。
类型展开的过程用到了模板特化，写起来有点繁琐。
如果编译器支持可变模板参数的话，实现上可以简化很多。

通过append_set得到经过整合的builder类集合后，接下来就是将入参值传给这些builder类。

#### get_initializers仿函数

有了builder类的集合，再借助boost::fusion中的for_each算法(可参考STL中的同名函数来理解)，就可以通过builder类创建我们最终的结果：handler_base子类。
算法中用到的仿函数就是get_initializers，其中除了对“operator ()”运算符做重载外，还需要定义“operator ()”运算符的返回值类型`result`。

对于result元函数，最初我是按下面这样写的：

```C++
struct get_initializers
{
    template<typename Element> //这里的Element，我最初以为是前面得到的builder类型，但实际上理解错了
    struct result
    {
        typedef typename get_initializers_result<Element>::type type;
    };

    template<typename Element>
    typename result<Element>::type
    operator ()(Element &e) const
    {
        return e.get_initializer();
    }
};
```

上面的代码无法通过编译，这样一来，我们就没法用单步调试的方式来定位问题了。
要排查boost::fusion的问题，一定要学会阅读编译器的错误输出，如下样例：

```txt
D:\boost4ets\boost_1_64_0\boost/utility/detail/result_of_iterate.hpp(160): 参见对正在编译的类 模板 实例化“boost::tr1_result_of<F>”的引用
with
[
    F=ets::process::detail::get_initializers (ets::process::detail::error_builder)
]
D:\boost4ets\boost_1_64_0\boost/fusion/view/transform_view/detail/apply_transform_result.hpp(31): 参见对正在编译的类 模板 实例化“boost::result_of<F>”的引用
with
[
    F=ets::process::detail::get_initializers (ets::process::detail::error_builder)
]
```

这是VC2010的实际输出，越靠上面的错误，在调用栈中，越靠近下面(按堆栈从高到低的方向压入参数的角度来看的)。
错误分为3个部分：

1. `D:\boost4ets\boost_1_64_0\boost/fusion/view/transform_view/detail/apply_transform_result.hpp(31)`：错误文件名及行号
2. `boost::result_of<F>`：出现问题的类型定义
3. `F=ets::process::detail::get_initializers (ets::process::detail::error_builder)`：与第2部分的模板入参对应

实际上，上面的错误信息，已经把错误原因告诉我们了，问题就出在apply_transform_result上，我们先看看其定义：

```C++
// <boost/fusion/view/transform_view/detail/apply_transform_result.hpp>
template <typename F>
struct apply_transform_result
{
    template <typename T0>
    struct apply<T0, void_>
        : boost::result_of<F(T0)> //关键点在这里
    {};
};
```

关键点在result_of的模板参数上，这里实际上是个“函数类型”，其入参类型为T0，返回值类型为F。
最终这个函数类型，会传递给get_initializers::result，作为其Element入参。
我们在result的实现中，实际想要的是函数类型中的入参类型T0，而不是函数类型本身。
因此，对result的实现要做特化，将函数类型展开：

```C++
struct get_initializers
{
    template<typename F>
    struct result;

    template <typename Element>
    struct result<get_initializers(Element)> //通过模板特化，将函数类型展开，这时，Element才是我们真正想要的builder类型
    {
        typedef typename get_initializers_result<Element>::type type;
    };
};
```

> 虽然编译器的错误输出，可为问题排查提供不少信息，但在实际开发中，还是有发现过信息输出不完整(甚至是前后信息对不上)的情况，如果大家有更好的排查方法，欢迎指教。
> 另外，boost提供了一个模板函数`type_id`，可在运行时获取类型信息，但前提条件是程序能正常编译，感觉在mpl框架下开发时，作用有限。

### executor类

进程创建的最后一步，实际上就是对前面得到的handler_base子类数组，一个个的调用其接口函数，对CreateProcess函数的27个要素进行赋值。过程如下：

1. 调用on_setup_t仿函数，将参数从handler_base子类，移到executor内部
2. 调用CreateProcess创建进程
3. 将进程创建结果，通知给每个handler_base子类(参见on_success_t、on_error_t的调用源码)

#### boost::fusion::transform_view引起的坑

transform_view从逻辑上，可以当成一个新的序列容器来看待，但容器中的值，只有在真正使用时，才会创建，而且，在使用完后，会马上释放(相当于使用了临时变量)。
executor中将cmd_line、work_dir等变量定义为指针类型，这导致在遍历transform_view中的元素时，这些变量指向了一个临时变量的地址，当遍历结束后，这些地址就失效了，导致后面进程创建出现问题。
本次代码移植，通过在transform_view之外，加了层as_vector调用，对转换结果进行缓存，来规避此问题。

## 样例代码

```C++
#define BOOST_ASIO_HAS_MOVE 1 //boost::asio对右值引用的版本识别似乎有问题，VC2010好像支持，这里手动开启

#include <boost/asio.hpp>
#include <process/async_pipe.hpp>
#include <process/child.hpp>
#include <process/search_path.hpp>
#include <process/io.hpp>

int main(int argc, char* argv[])
{
    boost::asio::io_service ios;
    std::vector<char> buf(100);

    ets::process::async_pipe ap(ios);

    ets::process::child c(ets::process::search_path("cmd"), "/?", ets::process::std_out > ap);

    boost::asio::async_read(ap, boost::asio::buffer(buf),
        [&buf](const boost::system::error_code &ec, std::size_t size){
            std::cout << std::string(buf.begin(), buf.begin() + size);
        });

    ios.run();
    c.wait();
    int result = c.exit_code();

    return result;
}
```

样例代码选择的是process官方教程中的异步IO样例，为了适配VC2010，进行了一点调整：

1. 在引用任何boost头文件之前，需定义`BOOST_ASIO_HAS_MOVE`宏，原因是boost对VC2010右值引用支持的判断不准，需手工进行修正
2. boost4ets将迁移的代码都放在了`ets`这个命名空间中，但异步IO相关的功能依然在`boost`命名空间中，使用时需注意区分
3. 在windows下，正常是没有`g++`的，因此，我将启动程序改为了`cmd`
4. 官方样例未设置缓存大小，也未输出执行结果，看不出效果，因此，本样例加了结果输出的逻辑(只输出了前100字节的内容)

### 编译

由于process中使用了filesystem这类需要编译的库，因此，需要先编译boost，相关过程请参考官方文档，本笔记不再赘述。
boost的版本建议使用1.64，我是在这个版本上完成的迁移，其他版本不保证兼容。
[官方下载地址](https://boostorg.jfrog.io/native/main/release/1.64.0/source/boost_1_64_0.7z)

编译时的目录结构：

```txt
boost4ets
|__boost_1_64_0
|  |__boost # boost源码
|  |__libs
|  |__tools
|__fusion
|__out
|  |__lib # boost编译后生成的lib库文件路径
|__process # boost4ets定制版本的process源码
|__test
   |__asyn_io.cpp # 本测试样例源码
```

* boost_1_64_0是下载的boost_1_64_0.7z的解压目录
* out是按boost官方文档，编译出来的库文件路径
* fusion、process、test对应本代码库

打开VC2010的命令行编译环境，将当前路径切换到`boost4ets\test`，输入以下命令：

```bat
cl /I"..\boost_1_64_0" /I".." /D "WIN32" /D "NDEBUG" /MD /EHsc asyn_io.cpp /link /LIBPATH:"..\out\lib" "shell32.lib"
```

如果编译顺利，会在当前目录下生成文件`asyn_io.exe`，执行后有如下输出：

```bat
D:\boost4ets\test>asyn_io.exe
启动 Windows 命令解释器的一个新实例

CMD [/A | /U] [/Q] [/D] [/E:ON | /E:OFF] [/F:ON | /F:OFF] [/V
```

## 个人的一点疑问，如有错误，欢迎大家指正

### boost::fusion的算法

对basic_execute_impl中直接使用filter_if这类算法(官方boost::process的做法)，我表示没有看懂。
fusion中的很多算法，本质上只是对view的一层包装，但却会增加一个副作用，就是给容器加上常量限定。
如下是filter_if的代码片段：

```C++
// <boost/fusion/algorithm/transformation/filter_if.hpp>
template <typename Pred, typename Sequence>
BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
inline typename result_of::filter_if<Sequence const, Pred>::type
filter_if(Sequence const& seq) //这里加了常量限定
{
    return filter_view<Sequence const, Pred>(seq);
}
```

之后在executor类中使用时，由于很多handler_base子类的on_setup声明并未添加const，所以，应该会导致编译报错才对。

### 管道的奇怪设计

async_pipe在构造时，会创建两个管道_source、_sink，前者用于读，后者用于写，所以，我最初想当然的像下面这样写代码：

```C++
boost::asio::io_service ios;
ets::process::async_pipe ap(ios);

bp::child c("cmd.exe", ets::process::std_in < ap, ets::process::std_out > ap); //将标准输入和标准输出都重定向到ap
```

我的本意是希望在进程启动后，可以一边读取进程的输出，还想让进程在输入流上等待我的操作，但实际上，进程启动后，没有任何输出结果，进程闪退。

通过排查代码，发现在async_pipe_in(async_pipe对应的handler_base子类，async_pipe_out有类似逻辑)中有个奇怪的处理，其on_success事件会把输出流的句柄关掉，个人理解，其原因有可能是async_pipe_in只处理输入，因此，输出流没意义，删除之。

但既然只用一个管道，为何在async_pipe中同时创建两个句柄呢？

### char_converter_t实现方式探讨

这个类是用来做字符类型转换的，在execute_impl中用到，最初在代码移植时，我将这个类删掉了，只要不混用字符集的话，也没什么问题。
但在写这篇文章时，因为样例代码中同时用了search_path和单字节字符串，编译报错，所以，才又将这个功能加回来了。

代码使用“transform_view + 仿函数call_char_converter”的方式实现，但在实现过程中遇到一个问题，就是result该如何定义？
由于char_converter(char_converter_t本质上只是char_converter的别名定义)只有一个函数`conv`定义，最初的想法是能否用`decltype`通过函数返回值进行推导，代码如下：

```C++
template<typename Char>
struct call_char_converter
{
    template<typename F>
    struct result;

    template <typename Element>
    struct result<call_char_converter(Element)>
    {
        typedef typename std::remove_cv<typename std::remove_reference<Element>::type>::type res_type;
        typedef decltype(ets::process::detail::char_converter<Char, res_type>::conv(res_type())) type;
    };

    template<typename Element>
    typename result<call_char_converter(Element &)>::type
        operator ()(Element &e) const
    {
        return ets::process::detail::char_converter<Char, Element>::conv(e);
    }
};
```

可惜编译报错了，问题出在`...::conv(res_type())`这行，报错原因有很多，例如，没有默认构造函数、数组类型无法构造、引用类型无法构造等等。

感觉用decltype的路行不通，所以，我采用了一种笨办法，针对每个`char_converter`，都定义一个类型转换的元函数(实际上是借鉴了transform_view定义仿函数的做法)。

新代码如下：

```C++
    template <typename Element>
    struct result<call_char_converter(Element &)>
    {
        typedef typename ets::process::detail::char_converter<Char, Element> res_char_converter;
        typedef typename boost::detail::tr1_result_of_impl<
            res_char_converter,
            Element,
            boost::detail::has_result_type<res_char_converter>::value
        >::type type; //这行的意思是，如果res_char_converter中有result_type定义，就将其作为返回类型，否则，使用res_char_converter中的result元函数处理
    };
```

下面是char_converter的实现：

```C++
template<>
struct char_converter<wchar_t, const char*>
{
    typedef std::wstring result_type; //加了这行

    static std::wstring conv(const char* in)
    {
        std::size_t size = 0;
        while (in[size] != '\0') size++;
        return ::ets::process::detail::convert(in, in + size);
    }
};
```

#### 深入char_converter

##### 对char_converter通用实现版本的思考

```C++
template<typename Char, typename T>
struct char_converter
{
    static T&  conv(T & in)
    {
        return in;
    }
    static T&& conv(T&& in)
    {
        return std::move(in);
    }
    static const T&  conv(const T & in)
    {
        return in;
    }
};
```

这个实现，个人认为有两个问题：

1. 对于引用入参的函数的模板参数，const是可以传递到T中的，因此，版本1和版本3的实现，有点重复了
2. 针对版本2，如果传入的实参是左值，则右值引用`T&&`会退化为`T&`，因此，导致和版本1重复定义

因此，迁移后的代码，我直接删除了版本2和版本3的实现，只保留了版本1，目前暂未发现问题。

> 关于函数模板参数类型的推导，我参考了网上的一篇文章，地址为《[C++之类型推导](https://blog.csdn.net/weixin_43374723/article/details/94767729)》

##### 为何没有basic_native_environment的特化版本

针对环境变量的处理，提供了两个类：basic_native_environment、basic_environment。

char_converter有针对basic_environment的特化版本，但没有提供basic_native_environment的。
最初个人对这两个类的关系理解错了，所以，对process只提供一个版本的特化不是太理解。
看了实现代码后才知道：

* basic_native_environment是用来操作当前进程的环境变量的，相当于是对`GetEnvironmentStrings`、`SetEnvironmentVariable`这些系统API的包装，一旦通过其修改某个环境变量的值，实际上会影响当前进程的执行
* basic_environment可以理解为是键值对的数组，对其修改，不会影响当前进程，当创建子进程时，应该通过这个类来为子进程指定新的环境变量

在使用时，可以先用basic_native_environment获取当前进程环境变量的值后，导入到basic_environment中，再根据需要对部分值进行修改，然后，用修改后的basic_environment创建新进程。

### initializer_tag

```C++
char sExec[] = "cmd";
ets::process::child c(sExec);
```

上面这段代码编译会报错，提示`error C2027: 使用了未定义类型“ets::process::detail::initializer_tag<T>”`。
错误原因是`sExec`的类型是`char (&)[4]`，而`initializer_tag`只针对常量版本的字符数组提供了定义：

```C++
template<std::size_t Size> struct initializer_tag<const char    [Size]> { typedef cmd_or_exe_tag<char>     type;};
```

从这点来看，感觉1.64版本的process库，也有可以完善的地方。

# 相关链接

* 作者邮箱: xinghun87@163.com
* 作者博客：[https://blog.csdn.net/xinghun61](https://blog.csdn.net/xinghun61)

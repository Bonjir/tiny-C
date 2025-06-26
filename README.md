# tiny-C
<<<<<<< HEAD
大三上编译原理大作业-迷你C语言编译器
=======
这是一个大三上编译原理大作业-迷你C语言编译器

可以实现**词法分析（Lexical analysis）、语法分析（Syntax analysis）、中间语言生成（Intermediate Representation）**

输入tiny-C代码如下：

<img src="https://github.com/Bonjir/tiny-C/blob/main/.images/Readme/input_c_code.jpg" width="75%" height="75%" />

词法分析、语法分析得到结果如下：

（先判断每个token的类型，然后构建语法树）

<img src="https://github.com/Bonjir/tiny-C/blob/main/.images/Readme/lexic&syntax_analysis.jpg" width="75%" height="75%" />

然后模拟运行，得到结果（绿色数字）

其中有语法分析错误的地方会输出出来（红色报错）

最后中间语言生成（青色的代码，类汇编的三变量语言）

<img src="https://github.com/Bonjir/tiny-C/blob/main/.images/Readme/cal&raise_error&assemble.jpg" width="75%" height="75%" />

tiny-C语法可以支持while循环、if-else判断、变量定义、输出显示。

还是很有意思的一个实验，遂了小时候就一直想做编译器的梦
>>>>>>> 1e5694b (commit complete project)

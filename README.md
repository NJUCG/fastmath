# FastMath & VecMat

目前 FastMath 与 VecMat 之间互不依赖。
最低标准版本要求: c++17

## FastMath
数学库 FastMath 是一个 one-header only 的库。  

想要使用，只需 `include "FastMath.h"` 即可，所有函数均被封装在 `namespace fm` 中，例如 `fm::sin(x)` 。

暂时仅支持double和float，其他类型（如int）请调用时自行按需类型转换。

提供了用精度换取速度的服务，目前支持如下档位：
```cpp
namespace fm{
    enum speed_option{
        ESpeedStd=0, //直接调用std
        ESpeedNormal=1, //保证float最后一位级别精度
        ESpeedFast1=2, //保证所有输入min(绝对误差,相对误差)不超过1e-4，不保证nan,inf的特殊处理
        ESpeedFast2=3, //保证所有输入min(绝对误差,相对误差)不超过2e-3，不保证nan,inf的特殊处理        
        ESpeedFast3=4, //保证所有输入min(绝对误差,相对误差)不超过4e-2，不保证nan,inf的特殊处理
    };
}
```
想要调控精度，可以：

* 在构建阶段通过定义宏 `FM_SPEED_DEFAULT` 配置默认值：如在CMake中 `add_definitions(-DFM_SPEED_DEFAULT=ESpeedNormal)`。注：若未定义宏`FM_SPEED_DEFAULT`，则会默认取值`ESpeedNormal`。
* 在使用时显式指明：即例如 `fm::sin(x,fm::ESpeedFast1)` 未显式指明的均默认取值 `FM_SPEED_DEFAULT`

项目目录文件说明：

`FastMath.h` ：one-header only 数学库

`DiscardedImpl.h` ：保存所有曾找到的“优化实现”，这些实现均由于测试结果发现O3下没有优化效果而被废弃，但由于不排除其他测试环境下可能有用，也为了记录测过哪些实现，将他们保留在此文件。

`TestMain.cpp` ：是用以测试 `FastMath.h` 速度的测试框架

## VecMat
数学库 VecMat 是一个 one-header only 的库。  
想要使用，只需 `include "VecMat.h"` 即可（但是需要保证文件`VecMatMult.inl.h`处在`VecMatMult.h`的同目录下），所有实现均被封装在 `namespace vecmat` 中，例如4维float向量 `vecmat::vec<4,float>` 。

#### API Referance

TODO

#### 项目目录文件说明

`VecMat.h` ：one-header only 向量矩阵库

`VecMatMult.inl.h` ：仅用于VecMat.h中 `#include`，提供N,M,K较小(<=4)时矩阵乘法的特化实现(M,K,N各四种取值，共64种)

`VecMatTestMain.cpp` ：是用以测试 `VecMat.h` 速度的测试程序

`testdep/*`：Eigen库，用于测试
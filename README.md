# RayTracing
使用C++实现了离线的光线追踪渲染器，渲染结果如下：
## 漫反射渲染结果：
![](https://github.com/BestBlade/RayTracing/blob/main/2048.png "SPP2048")  
## 重要性采样渲染结果：
![](https://github.com/BestBlade/RayTracing/blob/main/1280.png "SPP1280")  

### AreaLight     顶部面光源类
定义面光源的位置方向和采样方式
### Bounds3       包围盒类
定义包围盒之间的求交求并和光线与包围盒求交
### BVHTree       BVH树加速求交
定义BVH树的节点与BVH树结构用于分割场景加速光线求交
### Function      常用函数
定义随机数，叉乘，点乘等常用运算
### Intersection  光线与物体相交点类
定义光线与平面的交点的属性，坐标，是否相交，材质等
### Light         光源类
定义了AreaLight基类
### main          主函数
载入模型，构建场景，进行渲染，定义全局变量
### Material      材质类
定义了Lambert漫反射模型材质和Cook Torrance的微表面模型
### Mesh          物体类
定义了.obj模型文件的存储方法
### OBJ_LOADER    载入模型，Use the MIT license.
使用了MIT的载入模型文件
### Object        物体虚基类
### Ray           光线类
定义了光线的O+tD
### Scene         场景类
定义了模型构建的场景
### Sphere        球面类
定义了球面和与光线求交的方式
### Triangle      三角面类
定义了三角形类和与光线求交方式
### Vector        向量类
定义了基本的向量的运算操作

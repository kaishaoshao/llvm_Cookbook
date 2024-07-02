# llvm_Cookbook

LLVM Cookbook一书的书籍以及源码(llvm相关学习书籍)

## 源码目录结构

```.
. 
├── docx   ------> 书籍
└── src    ------> 源码（单元划分）

```

## 第一章 LLVM设计与使用

### 工作原理

通过testfile的例子我们了解到LLVM优化器通过使用不同参数为用户提供了不同的优化Pass,但整体风格一致。

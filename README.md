## introduction
A toy compiler has support for array, struct, function, if, while.
Use Flex & Bison to finish parser and LLVM to generate object file.
CMake build system.

## build

### OSX | Linux

Download LLVM Pre-Build binaries from [LLVM Download](http://releases.llvm.org/download.html#9.0.0) or build from source.

My LLVM binaries version is 9.0.0

Point $LLVM_DIR in CMakeList.txt to your binaries path contains cmake build files.

Then enjoy my bugly code :)

### Windows

DO NOT TESTED. Similar as OSX or Linux.

## compile command
./parser -h for list of para

./parser -f xxx.hc -o xxx.bc
lli xxx.bc // use llvm interpreter to exec
llc -filetype=obj xxx.bc // get object file 
clang xxx.o -o xxx // link exectable file

## **warning**
**缺乏测试样例！！！！有很多 bug！！！！请勿直接使用**
**MISSING TESTCASE!! DO NOT use directly!!**

## Released under MIT License

Copyright (c) 2019 Yuxuan Xiao.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<memory>
#include<queue>
using namespace std;
//栈区buffer
char buffer[256];
size_t bytes_used=0;
//往buffer里存，存进buffer的是数据
template<typename Arg>
void encode(Arg arg){
    *reinterpret_cast<Arg*>(buffer+bytes_used)=arg;
    bytes_used+=sizeof(Arg);
}
void decode(){
    //把对应地址转换成目标类型的指针，再解引用
    int show_int=*reinterpret_cast<int*>(buffer);
    char show_char=*reinterpret_cast<char*>(buffer+sizeof(int));
    cout<<show_int<<endl;
    cout<<show_char<<endl;
    //free(buffer);
}

queue<int*> q;
int* normal;
void queuetest(){
    // std::unique_ptr<int> q1(new int(22222));
    // normal=q1.get();//不是说好了独占指针吗。。这种情况显然有两个指针指向同一片区域啊。。
    // q.push(std::move(q1));//用move是把q1的控制权移动给了另一个独占指针，这之后q1不再拥有对象的控制权（不再指向对象）
    // //尽管退出作用域后q1指针将会立刻被释放，但q1的区域并没有被释放，因为队列中的独占指针接管了q1的区域！！！

    int* a=new int(22222);
    normal=a;
    q.push(std::move(a));//移动操作只对独占指针才有效
    //delete a;//但是这里不能释放内存，因为这里释放了外面就用不到了！
}
int main(){
    const char* level="[INFO]";
    int i=111;
    char c='c';
    //encode<int>(i);
    //encode<char>(c);
    //decode();
    //另外，给独占指针赋一个裸指针，会出现内存释放的问题，裸指针释放的释放的时候不会释放指向的空间，而独占指针是会释放指向空间的
    unique_ptr<int[]> p_a(new int[5]);//如果智能指针的模板是一个C数组类型，那么相当于一个数组指针，对其get会返回一个指针/数组（int*或int[]）
    p_a.get()[0]=111;
    int* a=&p_a.get()[0];//对unique_ptr<int[]>get操作，返回的是一个数组/指针，这一点可能让你感到怪异，因为和其他情况不一样（例如，对unique_ptr<int>get操作返回的是int*）
    int b=p_a.get()[0];//不能对模板参数为数组的独占指针解引用（别的模板参数可以，但模板参数为数组类型是不支持引用）
    //cout<<b<<*a<<endl;
    queuetest();
    //int t=*q.front();
    cout<<*normal<<endl;
    cout<<*q.front()<<endl;
    delete q.front();//如果你用的不是独占指针，那么一定要在这里记得释放掉内存，防止内存泄漏
    q.pop();
    //cout<<t<<endl;
}

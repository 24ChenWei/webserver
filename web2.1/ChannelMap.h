#pragma once

struct ChannelMap
{
    int size; //记录指针指向的数组的元素个数
    // struct Channel* list[];
    struct Channel** list;
};

//初始化
struct ChannelMap* ChannelMapInit(int size);
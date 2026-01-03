#pragma once
#include<cinttypes>
#include<map>
class Magic{
public:
  struct BindingMeta{
    uint32_t SetIndex;
    uint32_t BindingIndex;
    uint32_t StageBits;
    uint32_t Value;
  };



  Magic();

};


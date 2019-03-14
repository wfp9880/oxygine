#pragma once
#include "oxygine-framework.h"

using namespace oxygine;


DECLARE_SMART(Number, spNumber);
DECLARE_SMART(Counter, spCounter);

class Counter: public ClipRectActor
{
public:
    Counter();
    ~Counter();

    void init(const Resources* res, float numberWidth);

    void reset();

    void setValue(int v);
    int  getValue() const;

protected:
    void added(Event*);
    void add();
    enum { NUM = 8 };

    int _dest;
    int _src;
    spNumber _numbers[NUM];
    bool _running;
};
#ifndef _state_h_
#define _state_h_

class State
{
public:
    virtual void enterState() = 0;
    virtual State* run() = 0;
    virtual void exitState() = 0;
};

#endif
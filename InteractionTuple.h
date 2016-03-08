//
//  InteractionTuple.h
//  CyberSpider
//
//  Created by Atibhav Mittal on 3/7/16.
//  Copyright © 2016 ati. All rights reserved.
//

#ifndef INTERACTIONTUPLE_H_
#define INTERACTIONTUPLE_H_

#include <string>

struct InteractionTuple
{
    InteractionTuple()
    {}
    
    InteractionTuple(const std::string& f, const std::string& t, const std::string& c)
    : from(f), to(t), context(c)
    {}
    
    std::string from;
    std::string to;
    std::string context;
};

inline
bool operator==(const InteractionTuple& I1, const InteractionTuple& I2)
{
    if(I1.from == I2.from && I1.to == I2.to && I1.context == I2.context)
    {
        return true;
    }
    return false;
}

#endif // INTERACTIONTUPLE_H_
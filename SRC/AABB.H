#pragma once

#include <stdint.h>

inline uint8_t AABB_test_point(int32_t px, int32_t py,
                               int32_t bx, int32_t by, int32_t bw, int32_t bh)
{
    uint8_t test = 0;
    
    if((px >= bx) && (px < (bx+bw)))
        if((py >= by) && (py < (by+bh)))
            test=1;
    
    return test;
}
#pragma once

#include <math.h>

inline float lnr_to_log_approx_nrm(float value)
{
    if(value < 0.001f) value = 0.001f;
    if(value > 0.999f) value = 0.999f;
    
    value = 1.f - value;
    
    return (log(value)/(-6.907755f));
}
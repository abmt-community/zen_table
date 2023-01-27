#ifndef CNC_STEPPER_H
#define CNC_STEPPER_H CNC_STEPPER_H
#include <cstdint>

namespace zen_table{

//@node: auto
struct stepper{
    int64_t in_pos = 0;
    bool in_reset  = false;
    bool out_dir   = false;
    bool out_step  = true; // start with false
    bool param_invert_dir = false;
    
    
    int64_t pos;
    void init();
    void tick();
};

} // namespace cnc

#endif // CNC_STEPPER_H

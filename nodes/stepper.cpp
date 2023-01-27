#include "stepper.h"

using namespace std;
using namespace zen_table;

void stepper::init(){
    pos = in_pos;
}

void stepper::tick(){
    int64_t set_pos = in_pos;
    if(in_reset){
        pos = set_pos;
    }
    
    if(out_step == false){
        // complete the step
        out_step = true;
        return;
    }
    
    if(set_pos == pos){
        return;
    }
    
    // you only come here when out_step is high
    // from last step and there are steps to do.
    // -> set out_step to low and set dir
    out_step = false;
    if(set_pos > pos){
        ++pos;
        out_dir = true;
    }
    if(set_pos < pos){
        --pos;
        out_dir = false;
    }
}
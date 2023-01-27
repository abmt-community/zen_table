#include "virtual_table.h"

using namespace std;
using namespace zen_table;

void virtual_table::init(){
    out_img.realloc(param_size,param_size);
    last_x = in_x;
    last_y = in_y;
}

void virtual_table::tick(){
    out_img.draw_line(last_x, last_y, in_x, in_y, 255);
    last_x = in_x;
    last_y = in_y;
}
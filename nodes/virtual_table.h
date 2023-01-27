#ifndef ZEN_TABLE_VIRTUAL_TABLE_H
#define ZEN_TABLE_VIRTUAL_TABLE_H ZEN-TABLE_VIRTUAL_TABLE_H
#include <abmt/img.h>

namespace zen_table{

//@node:auto
//@raster: 500ms
struct virtual_table{
    double in_x;
    double in_y;
    
    double last_x;
    double last_y;
    
    int param_size = 600;
    
    abmt::img_gray out_img;
    
    void init();
    void tick();
};

} // namespace zen-table

#endif // ZEN-TABLE_VIRTUAL_TABLE_H

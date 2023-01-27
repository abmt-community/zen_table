#ifndef ZEN_TABLE_PLAYER_H
#define ZEN_TABLE_PLAYER_H ZEN_TABLE_PLAYER_H

#include <abmt/math/vec.h>
#include <abmt/math/ray.h>
#include <vector>

namespace zen_table{

//@compile: gcode_parser.cpp
//@compile: thr_parser.cpp

//@node: auto
//@raster: 10ms;
struct player{
    double out_x;
    double out_y;
    double in_speed = 0;
    
    double param_radius_mm = 300;
    std::string param_track_dir = "/opt/tracks";
    
    std::vector<abmt::vec2> points;
    int idx = 0;
    
    abmt::ray2d ray = abmt::ray2d::from_2p({0,0},{0,1});
    double ray_len = 0;
    double ray_pos = 0;
    
    void init();
    void tick();
    void calc_next_ray();
};


} // namespace zen_table

#endif // ZEN_TABLE_PLAYER_H

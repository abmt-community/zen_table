#ifndef CNC_TABLE_H
#define CNC_TABLE_H CNC_TABLE_H

namespace zen_table{

//@node: auto
struct controller{
    double in_x = 0;
    double in_y = 0;
    double in_mag_rad = 0;
    double in_mag_ang = 0;
    double in_normal_speed = 10;
    
    double out_tick_rad;
    bool   out_reset_rad;
    double out_tick_ang;
    bool   out_reset_ang;
    double out_max_speed = 0;
    
    
    double param_ticks_per_round = 4762.5; // 762/32*200
    double param_ticks_per_mm = 2.5;
    double param_radius_mm = 300;
    
    double param_mag_zero_level_rad = 5000;
    double param_mag_zero_level_ang = 3000;
    
    double param_max_ticks_per_sec = 800;
    
    double current_ang;
    double last_d_ang  = 0;
    
    enum{
        reset,
        start,
        rad_drive_away,
        rad_drive_to_zero,
        ang_drive_away,
        ang_drive_to_zero,
        calibrated,
        run,
    } state;
    
    void init();
    void calc_output();
    void tick();
    
};


} // namespace cnc

#endif // CNC_TABLE_H

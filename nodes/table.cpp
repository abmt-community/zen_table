#include "table.h"

#include <cmath>
#include <iostream>
using namespace std;
using namespace zen_table;


void controller::init(){
    state = reset;
    out_max_speed = 0;
    cout << "Start calibration." << endl;
}

void controller::calc_output(){
    double x = in_x - param_radius_mm;
    double y = in_y - param_radius_mm;
    double r = sqrt(x*x + y*y);
    
    if(r != r){
        return; // nan
    }
    
    if(r > param_radius_mm){
        r = param_radius_mm;
    }
    out_max_speed = 2*r*M_PI * (param_max_ticks_per_sec / param_ticks_per_round) ;
    if(out_max_speed > param_ticks_per_mm * param_max_ticks_per_sec){
        out_max_speed = param_ticks_per_mm * param_max_ticks_per_sec;
    }
    if(out_max_speed > in_normal_speed){
        out_max_speed = in_normal_speed;
    }
    if(out_max_speed < in_normal_speed/10){
       out_max_speed = in_normal_speed/10;
    }
    
    out_tick_rad = r*param_ticks_per_mm;
    
    double delta_ang = 0;
    double ang = 0;
    if( (x > 0.005 || x < -0.005) && (y > 0.005 || y < -0.005) ){
        ang = atan2(y, x);
        if(ang < 0){
            ang = 2*M_PI + ang; // Vorzeichen + korrekt
        }
        delta_ang = ang - current_ang ;
    }
    
    if(delta_ang > M_PI){
        delta_ang = delta_ang - 2*M_PI ;
    }else if(delta_ang < -1*M_PI) {
        delta_ang = 2*M_PI + delta_ang;
    }
    
    double max_d_ang = 2*3.1415/param_ticks_per_round*param_max_ticks_per_sec;
    
    if(delta_ang > max_d_ang){
        delta_ang = max_d_ang;
    }
    if(delta_ang < -max_d_ang){
        delta_ang = -max_d_ang;
    }
    
    double max_accel = max_d_ang / 100 / 10; // go to max speed in 10 sec
    if( delta_ang > 0 && (delta_ang - last_d_ang) > max_accel){
        delta_ang = last_d_ang + max_accel;
    }else if ( delta_ang < 0 && (delta_ang - last_d_ang) < -max_accel){
        delta_ang = last_d_ang - max_accel;
    }
    
    out_tick_ang += delta_ang/M_PI/2.0*param_ticks_per_round;
    current_ang  += delta_ang;
    last_d_ang = delta_ang;
    
    if(current_ang > 2*M_PI){
        current_ang -= 2*M_PI;
    }else if(current_ang < 0){
        current_ang = 2*M_PI + current_ang;
    }
    //cout << "x: " << x << " y:" << y << " r: " << out_tick_rad << " a:" << out_tick_ang << endl;
}

void controller::tick(){
    double calib_speed = param_max_ticks_per_sec / 100 / 2;
    if(state == run){
        calc_output();
        
    }else if( state == reset){
        //cout << "reset" << endl;
        out_tick_rad = 0;
        out_tick_ang = 0;
        out_reset_rad = true;
        out_reset_ang = true;
        state = start;
    }else if( state == start){
        //cout << "start" << endl;
        out_reset_rad = false;
        out_reset_ang = false;
        if(in_mag_rad > 10 && in_mag_ang > 10){
            // magnethometer connected
            state = rad_drive_away;
            //out_tick_rad = 0;
        }
    }else if( state == rad_drive_away){
        //cout << "rad drive away" << endl;
        out_tick_rad += calib_speed;
        if(in_mag_rad < param_mag_zero_level_rad / 2 ){
            state = rad_drive_to_zero;
            //out_tick_rad = 0;
        }
    }else if( state == rad_drive_to_zero){
        //cout << "rad drive to zero" << endl;
        out_tick_rad -= calib_speed;
        if(in_mag_rad >= param_mag_zero_level_rad){
            state = ang_drive_away;
            out_tick_rad = 0;
            out_reset_rad = true;
            //out_tick_ang = 0;
        }
        
    }else if( state == ang_drive_away){
        //cout << "ang drive away" << endl;
        out_tick_ang += calib_speed;
        if(in_mag_ang < param_mag_zero_level_ang/2 ){
            state = ang_drive_to_zero;
            //out_tick_ang = 0;
        }
        
    }else if( state == ang_drive_to_zero){
        //cout << "ang drive to zero" << endl;
        out_tick_ang -= calib_speed;
        if(in_mag_ang >= param_mag_zero_level_ang){
            out_tick_ang = 0;
            out_reset_ang = true;
            current_ang = 0;
            state = calibrated;
        }
    }else if( state == calibrated){
        cout << "Calibration finished." << endl;
        out_reset_rad = false;
        out_reset_ang = false;
        state = run;
    }
}
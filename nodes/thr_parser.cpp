#include "thr_parser.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include "axe.h"

using namespace std;
using namespace thr_parser;

string read_file(std::string filename) {
        ifstream f(filename.c_str());
        stringstream ss;
        ss << f.rdbuf();
        f.close();
        return ss.str();
}


struct polar_point{
    double ang = 0;
    double r = 0;
    polar_point operator-(polar_point p){
        return {ang - p.ang, r - p.r};
    }
    
    polar_point operator+(polar_point p){
        return {ang + p.ang, r + p.r};
    }
    
    polar_point operator*(double f){
        return {ang*f, r*f};
    }
    
    double calc_step_faktor(double r_current, double ds = 0.001){
        double faktor_r   = abs(ds/r);
        double faktor_phi = abs(2*ds/(2*ang*r_current));
        double res = faktor_r;
        if(res > faktor_phi){
            res = faktor_phi;
        }
        if(res > 1){
            res = 1;
        }
        return res;
    }
};

std::vector<polar_point> read_thr_file(std::string filename = "/opt/test.thr", double radius = 300){
    std::vector<polar_point> res; 
    double theta = 0; 
    double roh = 0; 
     
    auto sp = axe::r_lit(" ") | axe::r_lit("\t"); 
    auto nl = axe::r_lit("\n"); 
     
    auto polar_line = (*sp & axe::r_double(theta) && *sp &  axe::r_double(roh) & *sp)   
                    >>  axe::e_ref( [&](...){ 
                            if(roh > 1){
                                roh = 1;
                            }
                            if(roh < 0){
                                roh = 0;
                            }
                            res.push_back({theta,roh*radius}); 
                        });   
    auto any    = axe::r_any() - nl; 
    auto line   = ~polar_line & *any & nl; 
    auto parser = *line; 
     
    auto file = read_file(filename); 
    parser(file.begin(),file.end()); 

    return res;
}

std::vector<abmt::vec2> thr_parser::parse(std::string filename, double radius){
    std::vector<abmt::vec2> res;
    auto points = read_thr_file(filename, radius);
    if(points.size() <= 1){
        return res;
    }
    
    int idx = 1;
    auto current_pos = points[0];
    abmt::vec2 last_p2d;
    
    while(points.size() > idx){
        auto delta_p = points[idx] - current_pos;
        ++idx;
        double delta_sum = 0;
        do{
            auto f = delta_p.calc_step_faktor(current_pos.r,1);
            if(delta_sum + f > 1){
                f = 1 - delta_sum;
            }
            current_pos = current_pos + delta_p*f;
            delta_sum += f;
            abmt::vec2 p2d = {radius + current_pos.r*cos(current_pos.ang), radius + current_pos.r*sin(current_pos.ang)};
            res.push_back(p2d);
            auto delta2d = last_p2d - p2d;
            last_p2d = p2d;
        }while(delta_sum < 1);
    } //while size
    
    return res;
}



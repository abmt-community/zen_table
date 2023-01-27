#include "player.h"

#include <iostream>
#include <filesystem>
#include <stdlib.h>

#include "gcode_parser.h"
#include "thr_parser.h"


namespace fs = std::filesystem;

using namespace std;
using namespace abmt;
using namespace zen_table;



bool is_gcode(fs::path p){
        auto ext = p.extension();
        if(ext == ".ngc" || ext == ".gcode"){
                return true;
        }
        return false;   
}

bool is_thr(fs::path p){
        if(p.extension() == ".thr"){
                return true;
        }
        return false;
}


std::vector<abmt::vec2> parse_gcode_file(std::string filename){
    std::vector<abmt::vec2> res;
    
    gcode::parser p;
    p.g_handlers[1] = [&]{
		res.push_back({p.x.v, p.y.v });
	};
	
	p.g_handlers[0] = p.g_handlers[1];
	p.parse_file(filename);
	
	return res;
}

std::vector<abmt::vec2> load_next_file(string dir, double radius){
    std::vector<fs::path> files;
    for(auto& p: fs::directory_iterator(dir)) {
            if(fs::is_regular_file(p)) {
                    if(is_gcode(p)|| is_thr(p)){
                            files.push_back(p);
                    }
            }
    }
    if(files.size() > 0){
            srand(::time(0));
            int idx = rand() % files.size();
            auto file = files[idx]; 
            cout << "Load track: " << file.string() << endl;
            if(is_thr(file)){
                    return thr_parser::parse(file,radius);
            }else if(is_gcode(file)){
                    return parse_gcode_file(file);
            }
    }

    cout << "No tracks in '" << dir << "'. Loading idle pattern." << endl;
    std::vector<abmt::vec2> res;
    res.push_back({radius,radius});
    res.push_back({0,radius});
    res.push_back({2*radius,radius});
    res.push_back({radius,radius});
    return res;
}

void player::init(){
    points.clear();

    points = load_next_file(param_track_dir, param_radius_mm);
    points[0] = {param_radius_mm, param_radius_mm}; // first point is center
    idx = 0;
    calc_next_ray();
    out_x = ray.g(ray_pos).x;
    out_y = ray.g(ray_pos).y;
}

void player::tick(){
    ray_pos += in_speed / 100; // 10ms
    if(ray_pos > ray_len){
        auto rest = ray_len - ray_pos;
        calc_next_ray();
        ray_pos = rest;
    }
    out_x = ray.g(ray_pos).x;
    out_y = ray.g(ray_pos).y;
}

void player::calc_next_ray(){
    auto start = points[idx];
    while(true){
        idx++;
        if(idx >= points.size()){
            idx = 0;
            points = load_next_file(param_track_dir, param_radius_mm);
        }
        auto end = points[idx];
        
        ray = ray2d::from_2p(start, end);
        ray_pos = 0;
        ray_len = (end-start).len();
        if(ray_len < 1){
            //cout << "Step to next point to small. Skip this point: " << idx << endl;
        }else{
            return; 
        }
    }
}
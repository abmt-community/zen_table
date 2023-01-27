/*
 * parser.cpp
 *
 *  Created on: 29.12.2016
 *      Author: hva
 */

#include "gcode_parser.h"
#include "axe.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace abmt;

namespace gcode{

parser::parser(){
	line_handler = [&]{
		//cout << "parsed line: '" << line << "'"<< endl;
		if(g.updated){
			// cout << "parsed g command: " << g.v << endl;
			if(g_handlers.count(g.v) > 0){
				g_handlers[g.v]();
			}else{
				result.push_back(line);
			}
		}else if(m.updated){
			// cout << "parsed m command: " << m.v << endl;
			if(m_handlers.count(m.v) > 0){
				m_handlers[g.v]();
			}else{
				result.push_back(line);
			}
		}else{
			// cout << "parsed other command" << endl;
			result.push_back(line);
		}
		reset_registers();

	};
}

void parser::reset_registers(){
	for(auto reg:{&g,&m,&x,&y,&z,&f,&s,&i,&j,&p}){
		reg->old_val = reg->v;
		reg->updated = false;
	}
	comment = "";
}

void parser::parse_file(std::string filename){
	ifstream f(filename.c_str());
	stringstream ss;
	ss << f.rdbuf();
	parse_string(ss.str());
}

void parser::parse_string(std::string gcode){

	/**
	 * Extractors
	 */
	auto ex_failed = axe::e_ref([&](string::iterator a, string::iterator b){
		if(a != b){
			string cmd = string(a,b);
			cout << "fail: '" << cmd << "'"<< endl;;
		}
	});

	auto ex_line = axe::e_ref([&](string::iterator a, string::iterator b){
		line = string(a,b);
		line_handler();
	});

	/**
	 * Rules
	 */
	auto sp = axe::r_lit(" ");
	auto nl = ~sp & ~axe::r_lit("\r") & axe::r_lit("\n");

	// set_update is a bit complicated:
	// it creates a anonymous function, witch changes a register
	// this function can be used as an extractor
	auto set_update = [&](reg& r)->std::function<void(std::string::iterator,std::string::iterator)>{
		return [&](std::string::iterator,std::string::iterator){
			if(r.update_on_change){
				if(r.v != r.old_val){
					r.updated = true; r.on_update();
				}
			}else{
				r.updated = true; r.on_update();
			}
		};
	};

	auto r_g		= (axe::r_lit("G") & *sp & axe::r_double(g.v) & *sp ) >> axe::e_ref(set_update(g));
	auto r_m		= (axe::r_lit("M") & *sp & axe::r_double(m.v) & *sp ) >> axe::e_ref(set_update(m));
	auto r_f		= (axe::r_lit("F") & *sp & axe::r_double(f.v) & *sp ) >> axe::e_ref(set_update(f));
	auto r_s		= (axe::r_lit("S") & *sp & axe::r_double(s.v) & *sp ) >> axe::e_ref(set_update(s));
	auto r_x 		= (axe::r_lit("X") & *sp & axe::r_double(x.v) & *sp ) >> axe::e_ref(set_update(x));
	auto r_y		= (axe::r_lit("Y") & *sp & axe::r_double(y.v) & *sp ) >> axe::e_ref(set_update(y));
	auto r_z		= (axe::r_lit("Z") & *sp & axe::r_double(z.v) & *sp ) >> axe::e_ref(set_update(z));
	auto r_i		= (axe::r_lit("I") & *sp & axe::r_double(i.v) & *sp ) >> axe::e_ref(set_update(i));
	auto r_j		= (axe::r_lit("J") & *sp & axe::r_double(j.v) & *sp ) >> axe::e_ref(set_update(j));
	auto r_p		= (axe::r_lit("P") & *sp & axe::r_double(p.v) & *sp ) >> axe::e_ref(set_update(p));

	auto r_comment = (( axe::r_lit("(")
					 	 & *(axe::r_any() - axe::r_lit(")"))
						 & axe::r_lit(")")
					   )|(
					     axe::r_lit(";") &  *(axe::r_any() - axe::r_lit("\n"))
					 ))  >> comment; // axe::e_ref([&](...){cout << "comment"<< endl;});


	auto failed_line = *(axe::r_any() - axe::r_lit("\n")) >> ex_failed;
	auto r_line = +( r_g | r_m | r_f | r_s | r_x | r_y | r_z | r_i | r_j | r_p | r_comment) | failed_line;

	auto parser = *(r_line >> ex_line & nl);
	parser(gcode.begin(), gcode.end());
}

string parser::result_to_str(){
	stringstream out;
	out.setf(ios::fixed,ios::floatfield);
	out.precision(4);
	for(auto s:result){
		out << s << "\n";
	}
	return out.str();
}

string simplify(string g_code){
	parser p;

	p.line_handler = [&]{
		if(p.comment != ""){
			p.result.push_back(p.comment);
		}

		if(p.f.updated){
			stringstream s;
			s << p.f.register_name  << p.f.v;
			p.result.push_back(s.str());
			p.f.updated = false;
		}

		if(p.s.updated){
			stringstream s;
			s << p.s.register_name  << p.s.v;
			p.result.push_back(s.str());
			p.s.updated = false;
		}

		stringstream s;
		bool first = true;
		for(auto reg:{&p.g,&p.m,&p.x,&p.y,&p.z,&p.f,&p.s,&p.i,&p.j,&p.p}){
			if(reg->updated){
				if(first){
					first = false;
				}else{
					s << " ";
				}
				s << reg->register_name << reg->v;
			}
		}
		if(s.str() != ""){
			p.result.push_back(s.str());
		}
		p.reset_registers();
	};

	p.parse_string(g_code);
	return p.result_to_str();
}

string linearisize(string g_code, double divide){
	parser p;

	p.g_handlers[1] = [&]{
		stringstream cmd;
		cmd.setf(ios::fixed,ios::floatfield);
		cmd.precision(4);

		vec3 start({p.x.old_val,p.y.old_val,p.z.old_val});
		vec3 end ({p.x.v,p.y.v,p.z.v});
		start.z = end.z; // ignore changes in z;
		double len = (end - start).len();
		int cnt = len/divide;
		auto l = ray<3,double>::from_2p(start,end);
		cout << "dirl: " << l.dir.len() << endl;
		for(int i = 1; i < cnt;++i){
			vec3 point = l.g(i*divide);
			cmd << "G01 X " << point.x << " Y " << point.y << " Z " << point.z;
			p.result.push_back(cmd.str());
			cmd.str("");
			cmd.clear();
		}
		// last line:
		cmd << "G01 X " << end.x << " Y " << end.y << " Z " << end.z;
		cout << "cmd g1 subdivide len: " << len << endl;
		p.result.push_back(cmd.str());
	};

	p.g_handlers[0] = p.g_handlers[1]; // transform g0 to g1;

	auto calc_arc = [&](bool clockwise){
		stringstream cmd;
		cmd.setf(ios::fixed,ios::floatfield);
		cmd.precision(4);

		vec3 start({p.x.old_val,p.y.old_val,p.z.old_val});
		vec3 end ({p.x.v,p.y.v,p.z.v});
		//end.z = 0;
		//start.z = 0;
		vec3 mid = vec3({p.i.v,p.j.v,0}) + start;
		vec3 s2 = start - mid;
		vec3 e2 = end - mid;
		double r = e2.len();
		double ang = asin( s2.cross(e2).len()/e2.len()/s2.len() );
		double len_b = ang*r;
		int cnt = len_b / divide;
		double base_ang = atan2(s2.y,s2.x);

		for(int i = 1; i < cnt;++i){
			double ang2 = ang/cnt*i;
			if(clockwise){
				ang2 = -ang2;
			}
			vec3 point;
			point.x = mid.x + s2.x*cos(ang2) - s2.y*sin(ang2);
			point.y = mid.y + s2.x*sin(ang2) + s2.y*cos(ang2);
			point.z = end.z;
			cmd << "G01 X " << point.x << " Y " << point.y << " Z " << point.z;
			p.result.push_back(cmd.str());
			cmd.str("");
			cmd.clear();
		}
		cmd << "G01 X " << end.x << " Y " << end.y << " Z " << end.z;
		p.result.push_back(cmd.str());

	};


	p.g_handlers[2] = [&]{
		calc_arc(true);
	};struct limit_res{
		double x_min =  10000;
		double x_max = -10000;

		double y_min =  10000;
		double y_max = -10000;
	};

	p.g_handlers[3] = [&]{
		calc_arc(false);
	};

	p.parse_string(g_code);

	return p.result_to_str();
}

limit_res limit(string g_code, double z_lim){
	limit_res res;
	parser p;

	p.g_handlers[1] = [&]{
		if(p.z.v < z_lim){
			if(p.x.v > res.x_max){
				res.x_max = p.x.v;
			}
			if(p.x.v < res.x_min){
				res.x_min = p.x.v;
			}
			if(p.y.v > res.y_max){
				res.y_max = p.y.v;
			}
			if(p.y.v < res.y_min){
				res.y_min = p.y.v;
			}
		}
	};

	p.parse_string(g_code);
	return res;
}


string move(string g_code, double rot_deg, vec3 trans){
	parser p;

	p.g_handlers[1] = [&]{
		vec3 res ({p.x.v,p.y.v,p.z.v});
		vec3 diff = res + trans;
		res.x += trans.x ;// + diff.x*cos(rot_deg/180*M_PI) - diff.y*sin(rot_deg/180*M_PI);
		res.y += trans.y ;// + diff.x*sin(rot_deg/180*M_PI) + diff.y*cos(rot_deg/180*M_PI);
		res.z += trans.z;
		stringstream cmd;
		cmd.setf(ios::fixed,ios::floatfield);
		cmd.precision(4);
		cmd << "G01 X " << res.x << " Y " << res.y << " Z " << res.z;
		p.result.push_back(cmd.str());
	};

	p.parse_string(g_code);
	return p.result_to_str();
}

string move_to_limit(string g_code,limit_res lim ){
	limit_res res;
	parser p;

	p.g_handlers[1] = [&]{
		vec3 res ({p.x.v,p.y.v,p.z.v});
		if(res.x > lim.x_max){
			res.x = lim.x_max;
		}
		if(res.x < lim.x_min){
			res.x = lim.x_min;
		}
		if(res.y > lim.y_max){
			res.y = lim.y_max;
		}
		if(res.y < lim.y_min){
			res.y = lim.y_min;
		}
		stringstream cmd;
		cmd.setf(ios::fixed,ios::floatfield);
		cmd.precision(4);
		cmd << "G01 X " << res.x << " Y " << res.y << " Z " << res.z;
		p.result.push_back(cmd.str());
	};

	p.parse_string(g_code);
	return p.result_to_str();
}

} // namespace gcode

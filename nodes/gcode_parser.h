/*
 * parser.h
 *
 *  Created on: 29.12.2016
 *      Author: hva
 */

#include <string>
#include <map>
#include <vector>
#include <functional>

#include <abmt/math/vec.h>
#include <abmt/math/ray.h>

namespace gcode{


class reg{
public:
	double v = 0;
	bool updated = false;
	bool update_on_change; // update on change only. Set to false for command-regs
	double old_val=0;
	std::function<void()> on_update;
	std::string register_name;
	reg(std::string r, bool u = true):register_name(r),update_on_change(u){
		on_update = [&](){ };
	}
};

class parser{
public:
	std::map<double,std::function<void()>> g_handlers;
	std::map<double,std::function<void()>> m_handlers;

	std::vector<std::string> result;
	std::function<void()> line_handler; // default: pushes back the line in result;
	std::function<void()> fail_handler;
	std::string line;

	std::string comment;

	reg g = reg("G",false);
	reg m = reg("M",false);

	reg x = reg("X");
	reg y = reg("Y");
	reg z = reg("Z");
	reg f = reg("F"); // feed
	reg s = reg("S"); // Motorparamenter: eg: spindle speed
	reg i = reg("I"); // Distance to circlecenter in x
	reg j = reg("J"); // Distance to circlecenter in y
	reg p = reg("P");

	parser();
	void reset_registers(); // called after every line;

	void parse_file(std::string filename);
	void parse_string(std::string gcode);

	std::string result_to_str();

};

struct limit_res{
	double x_min =  10000;
	double x_max = -10000;

	double y_min =  10000;
	double y_max = -10000;

	double width(){
		return x_max - x_min;
	}

	double height(){
		return y_max - y_min;
	}
};

std::string simplify(std::string g_code);
std::string linearisize(std::string g_code, double divide = 0.1);
limit_res   limit(std::string g_code, double z_lim = 0);
std::string move_to_limit(std::string g_code,limit_res lim );
std::string move(std::string g_code, double rot_deg, abmt::vec3 trans);

} // namespace gcode

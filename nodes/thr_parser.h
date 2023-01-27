#ifndef THR_PARSER_THR_PARSER_H
#define THR_PARSER_THR_PARSER_H THR_PARSER_THR_PARSER_H

#include <vector>
#include <abmt/math/vec.h>

namespace thr_parser{

std::vector<abmt::vec2> parse(std::string filename, double radius = 300);

} // namespace thr_parser

#endif // THR_PARSER_THR_PARSER_H

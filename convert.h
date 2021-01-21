#ifndef CONVERT_H
#define CONVERT_H

#include <fstream>
#include <string>
#include "parse.hpp"

std::string to_html(std::string filename);

std::string to_html(std::ifstream &fin);

#endif
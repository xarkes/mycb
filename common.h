#pragma once

#include <iostream>

#ifndef ASSERT
#define ASSERT(cond) { if (!cond) { std::cerr << __FILE__ << ":" << __LINE__ << "Condition '#cond' failed" << std::endl; exit(1); } }
#endif

#pragma once

#ifdef _WIN32
#include "sf_tcpserver_win.hpp"
#else
#include "sf_tcpserver_linux.hpp"
#endif
#include <iostream>
#include "logger.hpp"

namespace zipper {

static logger _logger(std::cout, log_level::debug);

logger& logger::get_logger() {
    return _logger;
}
}
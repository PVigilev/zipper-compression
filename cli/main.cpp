#include <iostream>
#include <cstdint>
#include "decoder_if.hpp"
#include "logger.hpp"

int main() {
    zipper::decoder_if* d = nullptr;
    // std::cout << "Hello world" << std::endl << reinterpret_cast<uint64_t>(d) << std::endl;
    zipper::logger lgr(std::cout, zipper::log_level::debug);
    lgr.log(zipper::log_level::debug, "Hello world ", reinterpret_cast<uint64_t>(d));
    return 0;
}
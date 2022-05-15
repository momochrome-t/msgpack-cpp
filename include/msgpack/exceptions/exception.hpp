#include <stdexcept>
#include <string>
namespace msgpack::exceptions
{
    struct exception : public std::runtime_error
    {
        exception(const std::string &what_arg = "") : runtime_error(what_arg) {}
    };
    struct type_format_mismatch : public exception
    {
        type_format_mismatch(const std::string &what_arg = "") : exception(what_arg) {}
    };
}
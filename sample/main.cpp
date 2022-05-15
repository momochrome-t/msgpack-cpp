#include <iostream>
#include <vector>
#include <msgpack/msgpack.hpp>

void run()
{
    using namespace msgpack;
    std::vector<uint8_t> v{static_cast<uint8_t>(0b10010011u), bool_kind_tag_value<false>, bool_kind_tag_value<true>, type_kind_tag_value<uint8_t>, 124};
    msgpack_tuple_t<fixarray, msgpack_bool_t, msgpack_bool_t, msgpack_arithmetic_t<uint8_t>> tp{v};

    std::cout << std::boolalpha;
    auto f = tp.get<0>();
    std::cout << f.get() << std::endl;
    auto t = tp.get<1>();
    std::cout << t.get() << std::endl;
    auto u = tp.get<2>();
    std::cout << static_cast<uint32_t>(u.get()) << std::endl;
    const auto sum = total_size_of_first<1, msgpack_bool_t, msgpack_bool_t>();
}
int main()
{
    run();
}
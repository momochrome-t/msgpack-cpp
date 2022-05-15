#include <span>
#include <concepts>
#include <utility>
#include <type_traits>
#include <tuple>
#include <msgpack/exceptions/exception.hpp>

namespace msgpack
{
    template <class T>
    concept msgpack_t = requires
    {
        std::enable_if_t<std::is_same_v<
            std::remove_cvref_t<decltype(T::data_tag_size)>,
            std::size_t>>(0);
        std::enable_if_t<std::is_same_v<
            std::remove_cvref_t<decltype(T::length_representation_size)>,
            std::size_t>>(0);
        std::enable_if_t<std::is_same_v<
            std::remove_cvref_t<decltype(T::offset_size)>,
            std::size_t>>(0);
        std::enable_if_t<T::offset_size == T::data_tag_size + T::length_representation_size>(0);
        std::enable_if_t<std::is_same_v<
            std::remove_cvref_t<decltype(T::whole_size)>,
            std::size_t>>(0);
        std::enable_if_t<T::offset_size <= T::whole_size>(0);
    };

    enum class array_format_tag_value
    {
        fixarray,
        array16,
        array32
    };
    using enum array_format_tag_value;

    template <array_format_tag_value format>
    consteval std::size_t tag_size_of()
    {
        if constexpr (format == fixarray)
            return 0;
        if constexpr (format == array16)
            return 1;
        if constexpr (format == array32)
            return 1;
    }

    template <array_format_tag_value format>
    consteval std::size_t length_representation_size_of()
    {
        if constexpr (format == fixarray)
            return 1;
        if constexpr (format == array16)
            return 2;
        if constexpr (format == array32)
            return 4;
    }

    template <std::size_t Index, typename... Types>
    concept index_in_range = Index < sizeof...(Types);

    template <std::size_t Index, msgpack_t FirstElement, msgpack_t... Elements>
    requires index_in_range<Index, FirstElement, Elements...>
    consteval std::size_t total_size_of_first()
    {
        if constexpr (Index == 0)
            return 0;
        else
            return FirstElement::whole_size + total_size_of_first<Index - 1, Elements...>();
    }

    template <std::size_t N, std::size_t M>
    concept is_same_value = N == M;

    template <std::size_t N>
    requires is_same_value<N, 0>
    consteval std::size_t total_size_of_first()
    {
        return 0;
    }
    template <array_format_tag_value format, msgpack_t... Elements>
    struct msgpack_tuple_t
    {
    private:
        std::span<std::uint8_t> data;

    public:
        constexpr static std::size_t data_tag_size = tag_size_of<format>();
        constexpr static std::size_t length_representation_size = length_representation_size_of<format>();
        constexpr static std::size_t offset_size = data_tag_size + length_representation_size;
        constexpr static std::size_t whole_size = offset_size + (Elements::whole_size + ...);
        constexpr msgpack_tuple_t(decltype(data) data) : data(data) {}
        template <std::size_t N>
        constexpr msgpack_tuple_t(std::span<uint8_t, N> data) : data(data) {}
        template <std::size_t N>
        requires index_in_range<N, Elements...>
        constexpr auto get() const
        {
            using type = std::tuple_element_t<N, std::tuple<Elements...>>;
            return type{data.subspan(offset_size).subspan(total_size_of_first<N, Elements...>(), type::whole_size)};
        }
    };

    template <bool>
    constexpr std::uint8_t bool_kind_tag_value;
    template <>
    constexpr std::uint8_t bool_kind_tag_value<false> = 0xc2;
    template <>
    constexpr std::uint8_t bool_kind_tag_value<true> = 0xc3;
    struct msgpack_bool_t
    {
    private:
        const std::span<std::uint8_t> data;

    public:
        constexpr static std::size_t data_tag_size = 0;
        constexpr static std::size_t length_representation_size = 1;
        constexpr static std::size_t offset_size = data_tag_size + length_representation_size;
        constexpr static std::size_t whole_size = offset_size;
        constexpr msgpack_bool_t(decltype(data) data) : data(data) {}
        bool get() const { return data[0] == bool_kind_tag_value<true>; }
    };

    template <typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    template <arithmetic T>
    constexpr std::uint8_t type_kind_tag_value;
    template <>
    constexpr std::uint8_t type_kind_tag_value<uint8_t> = 0xcc;
    template <>
    constexpr std::uint8_t type_kind_tag_value<uint16_t> = 0xcd;
    template <>
    constexpr std::uint8_t type_kind_tag_value<uint32_t> = 0xce;
    template <>
    constexpr std::uint8_t type_kind_tag_value<uint64_t> = 0xcf;
    template <>
    constexpr std::uint8_t type_kind_tag_value<int8_t> = 0xd0;
    template <>
    constexpr std::uint8_t type_kind_tag_value<int16_t> = 0xd1;
    template <>
    constexpr std::uint8_t type_kind_tag_value<int32_t> = 0xd2;
    template <>
    constexpr std::uint8_t type_kind_tag_value<int64_t> = 0xd3;

    constexpr bool is_positive_fixint(std::uint8_t N) { return N >> 7 == 0; }
    constexpr bool is_negative_fixint(std::uint8_t N) { return N >> 5 == 0b111; }

    template <typename T, bool is_fixint>
    concept true_iff_8bit = arithmetic<T> &&((!is_fixint) || sizeof(T) == sizeof(uint8_t));

    template <arithmetic T>
    constexpr T reinterpret(std::span<uint8_t> data)
    {
        constexpr auto size = sizeof(T) / sizeof(uint8_t);
        uint8_t buf[size];
        for (auto i = 0; i < size; i++)
        {
            buf[i] = data[size - i - 1];
        }
        return *reinterpret_cast<T *>(buf);
    }

    template <arithmetic T, bool is_fixint = false>
    requires true_iff_8bit<T, is_fixint>
    struct msgpack_arithmetic_t
    {
    private:
        const std::span<std::uint8_t> data;

    public:
        constexpr static std::size_t data_tag_size = is_fixint ? 1 : 0;
        constexpr static std::size_t length_representation_size = 0;
        constexpr static std::size_t offset_size = data_tag_size + length_representation_size;
        constexpr static std::size_t whole_size = offset_size + sizeof(T) / sizeof(uint8_t);
        constexpr msgpack_arithmetic_t(decltype(data) data) : data(data) {}
        T get() const
        {
            if constexpr (is_fixint)
            {
                if (const auto v = data[0]; is_positive_fixint(v))
                {
                    return v;
                }
                else if (is_negative_fixint(v))
                {
                    return v;
                }
                else
                    throw exceptions::type_format_mismatch();
            }
            if (data[0] != type_kind_tag_value<T>)
                throw exceptions::type_format_mismatch();
            const auto body = data.subspan(1);
            return reinterpret<T>(body);
        }
    };
}
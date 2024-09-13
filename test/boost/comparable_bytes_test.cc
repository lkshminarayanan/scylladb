/*
 * Copyright (C) 2024-present ScyllaDB
 */

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "test/lib/scylla_test_case.hh"
#include "types/types.hh"
#include "utils/comparable_bytes.hh"

BOOST_AUTO_TEST_CASE(test_comparable_bytes_opt) {
    BOOST_REQUIRE(comparable_bytes::from_bytes_view(*int32_type, bytes_view_opt()) == comparable_bytes_opt());
    BOOST_REQUIRE(comparable_bytes::from_managed_bytes(*int32_type, managed_bytes_opt()) == comparable_bytes_opt());
    BOOST_REQUIRE(comparable_bytes::from_managed_bytes_view(*int32_type, managed_bytes_view_opt()) == comparable_bytes_opt());
    BOOST_REQUIRE(comparable_bytes::from_data_value(data_value::make_null(int32_type)) == comparable_bytes_opt());
}

template<std::integral int_type>
void integer_types_test() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(std::numeric_limits<int_type>::min(), std::numeric_limits<int_type>::max());
    const int NUM_OF_ITEMS = 100;

    struct test_item {
        int_type value;
        comparable_bytes_opt bytes;
    };
    std::vector<test_item> items;
    items.reserve(NUM_OF_ITEMS);

    for (int i = 0; i < NUM_OF_ITEMS; i++) {
        // verify comparable bytes encode/decode for integers
        int_type num = dist(gen);
        data_value value(num);
        auto comparable_bytes = comparable_bytes::from_data_value(value);
        auto decoded_value = comparable_bytes->to_data_value(value.type());
        BOOST_REQUIRE_MESSAGE(value == decoded_value, fmt::format("comparable bytes encode/decode failed; expected : {}; actual : {}", value, decoded_value));

        // collect them in a vector to verify ordering
        items.emplace_back(num, std::move(comparable_bytes));
    }

    // Sort the items based on comparable bytes
    std::ranges::sort(items, [](const test_item& a, const test_item& b) {
        return a.bytes < b.bytes;
    });

    // Verify that ordering them based on comparable bytes, sorts the values.
    BOOST_REQUIRE(std::ranges::is_sorted(items, [](const test_item& a, const test_item& b) {
        return a.value < b.value;
    }));
}

BOOST_AUTO_TEST_CASE(test_comparable_bytes_integer_types) {
    integer_types_test<int8_t>();   // tinyint
    integer_types_test<int16_t>();  // smallint
    integer_types_test<int32_t>();  // int
}

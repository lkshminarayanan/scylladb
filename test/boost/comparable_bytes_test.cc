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

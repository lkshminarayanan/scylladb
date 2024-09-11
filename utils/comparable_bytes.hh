/*
 * Copyright (C) 2024-present ScyllaDB
 */

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include "managed_bytes.hh"

class abstract_type;
class data_value;
class comparable_bytes;
using comparable_bytes_opt = std::optional<comparable_bytes>;

class comparable_bytes : public managed_bytes {
    comparable_bytes(const abstract_type& type, managed_bytes_view view);

public:
    // Methods to convert to comparable bytes from standard formats
    static comparable_bytes_opt from_managed_bytes_view(const abstract_type& type, const managed_bytes_view_opt& mbv) {
        if (!mbv) {
            return std::nullopt;
        }

        return comparable_bytes(type, mbv.value());
    }
    static comparable_bytes_opt from_managed_bytes(const abstract_type& type, const managed_bytes_opt& mb) {
        return from_managed_bytes_view(type, managed_bytes_view_opt(mb));
    }
    static comparable_bytes_opt from_bytes_view(const abstract_type& type, std::optional<bytes_view> bv) {
        return from_managed_bytes_view(type, managed_bytes_view_opt(bv));
    }
    static comparable_bytes_opt from_data_value(const data_value& value);

    // Methods to convert comparable bytes to standard serialized format bytes
    managed_bytes_opt to_managed_bytes(const abstract_type& type);
    data_value to_data_value(const shared_ptr<const abstract_type>& type);

    auto operator<=>(const comparable_bytes& other) const {
        return compare_unsigned(managed_bytes_view(*this), managed_bytes_view(other));
    }

    bool operator==(const comparable_bytes& other) const {
        // optimised == overload that checks the length first
        return this->size() == other.size() && (*this <=> other) == 0;
    }
};

// formatters for debugging and testcases
template <> struct fmt::formatter<comparable_bytes> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const comparable_bytes& b, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", managed_bytes_view(b));
    }
};
inline std::ostream& operator<<(std::ostream& os, const comparable_bytes& b) {
    fmt::print(os, "{}", b);
    return os;
}

template <> struct fmt::formatter<comparable_bytes_opt> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const comparable_bytes_opt& opt, FormatContext& ctx) const {
        if (opt) {
            return fmt::format_to(ctx.out(), "{}", *opt);
        }
        return fmt::format_to(ctx.out(), "null");
    }
};

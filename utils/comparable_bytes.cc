/*
 * Copyright (C) 2024-present ScyllaDB
 */

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "utils/comparable_bytes.hh"

#include "concrete_types.hh"

constexpr uint8_t BYTE_MSB_MASK = (1 << 7);

struct comparable_bytes_size_visitor {
    int serialized_bytes_size;

    template <typename T>
    size_t operator()(const integer_type_impl<T>&) {
        // Only first bit is inverted for integer types, so the length remains the same
        return serialized_bytes_size;
    }

    // TODO: Handle other types

    size_t operator()(const abstract_type&) {
        // Unimplemented
        SCYLLA_ASSERT(false);
        return 0;
    }
};

// Returns size of the comparable bytes for the given managed_bytes_view
static size_t comparable_bytes_size(const abstract_type& type, managed_bytes_view view) {
    return visit(type, comparable_bytes_size_visitor{view.size_bytes()});
}

struct to_comparable_bytes_visitor {
    managed_bytes_view regular_bytes_view;
    comparable_bytes& comparable_bytes;

    // Fixed length signed integers encoding;
    // Invert the first bit so that negative numbers are ordered before the positive ones
    template <typename T>
    void operator()(const integer_type_impl<T>&) {
        managed_bytes_mutable_view comparable_bytes_view(comparable_bytes);
        write_fragmented(comparable_bytes_view, regular_bytes_view);
        // invert msb of the first byte in the comparable bytes
        comparable_bytes[0] ^= BYTE_MSB_MASK;
    }

    // TODO: Handle other types

    void operator()(const abstract_type&) {
        // Unimplemented
        SCYLLA_ASSERT(false);
    }
};

comparable_bytes::comparable_bytes(const abstract_type& type, managed_bytes_view view)
    : managed_bytes(managed_bytes::initialized_later(), comparable_bytes_size(type, view)) {
    if (view.empty()) {
        // nothing to do
        return;
    }

    visit(type, to_comparable_bytes_visitor{view, *this});
}

comparable_bytes_opt comparable_bytes::from_data_value(const data_value& value) {
    return from_managed_bytes_view(*value.type(), managed_bytes_view_opt(value.serialize()));
}

struct decoded_bytes_size_visitor {
    const comparable_bytes& cb;

    template <typename T>
    size_t operator()(const integer_type_impl<T>&) {
        // Only first bit is inverted for integer types, so the length remains the same
        return cb.size();
    }

    // TODO: Handle other types

    size_t operator()(const abstract_type&) {
        // Unimplemented
        SCYLLA_ASSERT(false);
        return 0;
    }
};

// Returns size of the comparable bytes for the given managed_bytes_view
static size_t decoded_bytes_size(const abstract_type& type, const comparable_bytes& cb) {
    return visit(type, decoded_bytes_size_visitor{cb});
}

struct from_comparable_bytes_visitor {
    managed_bytes_view comparable_bytes_view;
    managed_bytes& decoded_bytes;

    // First bit is inverted for the fixed length signed integers.
    template <typename T>
    void operator()(const integer_type_impl<T>&) {
        managed_bytes_mutable_view decoded_bytes_view(decoded_bytes);
        write_fragmented(decoded_bytes_view, comparable_bytes_view);
        // invert msb of the first byte in the decoded bytes
        decoded_bytes[0] ^= BYTE_MSB_MASK;
    }

    // TODO: Handle other types

    void operator()(const abstract_type&) {
        // Unimplemented
        SCYLLA_ASSERT(false);
    }
};

managed_bytes_opt comparable_bytes::to_managed_bytes(const abstract_type& type) {
    if (empty()) {
        return managed_bytes_opt();
    }

    managed_bytes_view comparable_bytes_view(*this);
    managed_bytes decoded_bytes(managed_bytes::initialized_later(), decoded_bytes_size(type, *this));
    visit(type, from_comparable_bytes_visitor{comparable_bytes_view, decoded_bytes});
    return decoded_bytes;
}

data_value comparable_bytes::to_data_value(const data_type& type) {
    auto decoded_bytes = to_managed_bytes(*type);
    if (!decoded_bytes) {
        return data_value::make_null(type);
    }

    return type->deserialize(decoded_bytes.value());
}

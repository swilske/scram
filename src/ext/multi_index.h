/*
 * Copyright (C) 2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file multi_index.h
/// Helper functions to boost multi_index_container.

#include <boost/multi_index_container.hpp>

#ifndef SCRAM_SRC_EXT_MULTI_INDEX_H_
#define SCRAM_SRC_EXT_MULTI_INDEX_H_

namespace ext {

/// Extracts (i.e., take, remove-return) a value from multi_index container.
///
/// @tparam T  The value type in the container.
/// @tparam K  The key type.
/// @tparam Ts  The rest of the multi_index container type parameters.
///
/// @param[in] key  The key to lookup the value in the container.
/// @param[in,out] container  The container with the associated value.
///
/// @returns The extracted value.
///
/// @pre The value for the given key exists.
///
/// @note This function breaks the contract of multi_index_container
///       by modifying the value outside of the container.
template <typename T, typename K, typename... Ts>
T extract(K&& key, boost::multi_index_container<T, Ts...>* container) noexcept {
  auto it = container->find(std::forward<K>(key));
  assert(it != container->end());
  T result = std::move(const_cast<T&>(*it));  // Theft, contract-violation.
  container->erase(it);  // Requires valid iterator but not value.
  return result;
}

}  // namespace ext

#endif  // SCRAM_SRC_EXT_MULTI_INDEX_H_
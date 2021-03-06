#ifndef _ETL_DATA_CRC32_IMPL_H_INCLUDED
#define _ETL_DATA_CRC32_IMPL_H_INCLUDED

#include <array>
#include <cstddef>
#include <cstdint>

#include "etl/bits.h"
#include "etl/integer_sequence.h"

#include "etl/data/range_ptr.h"

namespace etl {
namespace data {

/*
 * CRC32 Lookup Table / Generator
 *
 * Note: most cases can use the facade defined in crc32.h.
 *
 * We use the optimized CRC32 algorithm proposed in RFC1952, parameterized by
 * the lookup table size.
 *
 * The original implementation allocates the lookup table lazily and calculates
 * its contents at runtime.  In our applications we can't afford such things.
 * Instead, we use constexpr to compute the lookup table at compile time and
 * toss it into ROM.
 *
 * table_l2size below controls the log2 size of the lookup table.  The
 * traditional version would use 8, for a 4KiB table and minimal operations per
 * byte.
 */
template <std::size_t N>
struct Crc32Table {
private:
  static constexpr unsigned table_l2size = N;

  static_assert((table_l2size & (table_l2size - 1)) == 0,
                "table_l2size must itself be a power of two");
  static_assert(table_l2size <= 8,
                "table_l2size must not exceed 8");

  static constexpr unsigned table_size = 1u << table_l2size;

  /*
   * Here's a little function template for repeatedly applying a function object
   * to a seed value.  It's written to be tail-recursive, but I don't think any
   * compilers actually tail-call optimize constexpr evaluations.
   *
   * I'm not certain that it's useful enough to get lifted into a common library
   * header, but it *is* handy here.
   */
  template <typename Fn, typename T>
  static constexpr T iterate(unsigned n, Fn const & fn, T seed) {
    return (n <= 1) ? fn(seed) : iterate(n - 1, fn, fn(seed));
  }
  
  /*
   * The inner loop of the table generation process.  Updates the table entry
   * with a single data bit.  This is implemented as a function object because
   * C++11 doesn't consider function references to be constexpr, even constexpr
   * ones.
   */
  struct Step {
    constexpr std::uint32_t operator()(std::uint32_t c) const {
      return (c & 1) ? (0xedb88320 ^ (c >> 1)) : (c >> 1);
    }
  };
  
  /*
   * Helper for generate, below.
   *
   * For some number of indices, generate the corresponding table entries as an
   * array.  This should normally be called only with an index sequence from 0
   * to 255.
   *
   * This exists to deconstruct the IndexSequence produced in generate, and is
   * the sort of thing we must resort to in languages without pattern matching
   * *inside* of functions.
   */
  template <std::size_t ... Is>
  static constexpr auto generate_(etl::IndexSequence<Is...>)
      -> std::array<std::uint32_t, sizeof...(Is)> {
    return {{ iterate(table_l2size, Step{}, std::uint32_t(Is))... }};
  }
  
  /*
   * Generates a CRC32 lookup table of `table_size` entries, one for each
   * possible chunk of input.
   */
  static constexpr std::array<std::uint32_t, table_size> generate() {
    return generate_(etl::MakeIndexSequence<table_size>{});
  }

  std::array<std::uint32_t, table_size> _table;

public:
  /*
   * Creates a Crc32 implementation using 2^N table entries.
   */
  constexpr Crc32Table() : _table(generate()) {}

  /*
   * Computes the (possibly partial) CRC-32 of the given range of data.  The
   * seed, if provided, is the result from a previous CRC-32 pass -- this can
   * be used to chain calls to incrementally compute the CRC of streaming data.
   */
  std::uint32_t process(RangePtr<std::uint8_t const> data,
                        std::uint32_t seed = 0) const {
    std::uint32_t c = seed ^ 0xffffffff;
    static constexpr auto chunk_mask = etl::bit_mask<table_l2size>();
    static constexpr auto chunks_per_byte = 8 / table_l2size;
  
    for (auto byte : data) {
      for (unsigned i = 0; i < chunks_per_byte; ++i) {
        unsigned chunk = (byte >> (i * table_l2size)) & chunk_mask;
        c = _table[(c ^ chunk) & chunk_mask] ^ (c >> table_l2size);
      }
    }
    return c ^ 0xffffffff;
  }
};

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_CRC32_IMPL_H_INCLUDED

#include <cstddef>

#include "utility.h"
namespace lox {

    template <typename T>
    concept range =
        requires(T t) {
            { t.begin() };
            { t.end() };
        };
    template <typename R>
    concept sized_range =
        requires(R r) {
            { range<R> };
            { r.size() };
        };

    namespace views {

        template <sized_range Range>
        class enumerate {
        public:
            class EnumerateIterator {
            public:
                using value_type = Pair<size_t, typename Range::value_type>;
                EnumerateIterator(size_t pos, Range::value_type* v) : data(pos, *v), iter(v) {
                }

                // used for end calculations
                EnumerateIterator(size_t pos) : data(pos, typename Range::value_type{}), iter(nullptr) {
                }

                value_type operator*() const {
                    return data;
                }

                EnumerateIterator& operator++() {
                    data.first++;
                    data.second = *(iter + 1);
                    iter++;
                    return *this;
                }

                bool operator!=(const EnumerateIterator& rhs) const {
                    return data.first != rhs.data.first;
                }

            private:
                value_type data;
                Range::value_type* iter;
            };
            enumerate(Range& r) : iterator(r.size() == 0 ? EnumerateIterator{0} : EnumerateIterator{0, r.begin()}), _end(r.size()) {
            }
            auto begin() {
                return iterator;
            }
            auto end() const {
                return _end;
            }

        private:
            EnumerateIterator iterator;
            EnumerateIterator _end;
        };

    }
    namespace ranges {
        template <range Range>
        void fill(Range& r, typename Range::value_type v) {
            for (auto& e : r) {
                e = v;
            }
        }
    }
}
#ifndef LOXCPP_ALGORITHM_H_
#define LOXCPP_ALGORITHM_H_
#include <cstddef>
#include <functional>

#include "iterator.h"
#include "loxexception.h"
#include "optional.h"
#include "utility.h"
namespace lox {

    template <typename T>
    concept range =
        requires(T t) {
            { t.begin() };
            { t.end() };
        };
    template <typename T>
    concept reversible_range =
        requires(T t) {
            { t.rbegin() };
            { t.rend() };
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

        template <range Range, typename F>
        class transform {
        public:
            using ReturnType = std::invoke_result_t<F, typename Range::value_type>;
            using value_type = ReturnType;
            class TransformIterator {
            public:
                TransformIterator(const Range::value_type* iter, std::function<ReturnType(typename Range::value_type)> f) : iter(iter), f(f) {}

                ReturnType operator*() const {
                    return f(*iter);
                }

                TransformIterator& operator++() {
                    iter++;
                    return *this;
                }

                bool operator!=(const TransformIterator& rhs) const {
                    return iter != rhs.iter;
                }

            private:
                const Range::value_type* iter;
                std::function<ReturnType(typename Range::value_type)> f;
            };
            transform(const Range& r, F f) : r(r), f(f) {}
            auto begin() const {
                return TransformIterator(r.begin(), f);
            }
            auto end() const {
                return TransformIterator(r.end(), f);
            }

        private:
            const Range& r;
            std::function<ReturnType(typename Range::value_type)> f;
        };

        template <reversible_range Range>
        class reversed {
        public:
            using value_type = Range::value_type;
            reversed(reversible_range auto& range) : r(range) {}

            auto begin() const {
                return ReverseIterator{r.rbegin()};
            }
            auto end() const {
                return ReverseIterator{r.rend()};
            }

        private:
            Range& r;
        };

    }
    namespace ranges {

        template <range Range>
        void fill(Range& r, typename Range::value_type v) {
            for (auto& e : r) {
                e = v;
            }
        }

        template <typename Iterator, typename T>
        void fill_n(Iterator it, size_t count, T value) {
            for (size_t i = 0; i <= count; ++i) {
                *it = value;
                it++;
            }
        }

        template <range Range1, typename Iterator>
        void copy(Range1&& r1, Iterator r2) {
            for (const auto& e : r1) {
                *r2 = e;
                ++r2;
            }
        }

        template <range Range1, typename Iterator>
        void uninitialized_copy(Range1&& r1, Iterator r2) {
            for (const auto& e : r1) {
                std::construct_at((typename Range1::value_type*)r2, e);
                ++r2;
            }
        }

        template <range Range>
        constexpr lox::Optional<typename Range::value_type> max(Range& r) {
            if (r.begin() == r.end()) {
                return {};
            }
            auto val = *r.begin();
            for (const auto& e : r) {
                if (e > val) {
                    val = e;
                }
            }
            return val;
        }

        template <range Range1, range Range2>
        constexpr bool is_equal(Range1& r1, Range2& r2) {
            auto it1 = r1.begin();
            auto it2 = r2.begin();
            while (it1 != r1.end() && it2 != r2.end()) {
                if (*it1++ != *it2++) {
                    return false;
                }
            }

            return it1 == r1.end() && it2 == r2.end();
        }

        template <range Range>
        constexpr bool anyOf(const Range& r, std::function<bool(const typename Range::value_type&)> f) {
            for (const auto& e : r) {
                if (f(e)) {
                    return true;
                }
            }
            return false;
        }

    }
}
#endif
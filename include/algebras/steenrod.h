/** \file steenrod.h
 * A Component for monomials and polynomials over $F_2$.
 * All are encapsulated in the `namespace alg`.
 */

#ifndef STEENROD_H
#define STEENROD_H

#include "myexception.h"
#include "utility.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <vector>

namespace steenrod {
using array = std::vector<int>;
using array2d = std::vector<array>;
using array3d = std::vector<array2d>;

inline constexpr size_t XI_MAX = 8;                     /* Support up to \xi_8 */
inline constexpr int DEG_MAX = (1 << (XI_MAX + 1)) - 1; /* Maximum degree supported in A */

inline constexpr size_t MMILNOR_INDEX_NUM = (XI_MAX + 1) * (XI_MAX + 2) / 2 - 1;
inline constexpr uint64_t MMILNOR_ONE = uint64_t(1) << (MMILNOR_INDEX_NUM - 1);
namespace detail {
    inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMilnorGenI()
    {
        std::array<int, MMILNOR_INDEX_NUM> result = {0};
        size_t n = 0;
        for (int j = 1; j <= (int)XI_MAX + 1; ++j)
            for (int i = j - 1; i >= 0; --i)
                if (n < MMILNOR_INDEX_NUM)
                    result[n++] = i;
        return result;
    }
    inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMilnorGenJ()
    {
        std::array<int, MMILNOR_INDEX_NUM> result = {0};
        size_t n = 0;
        for (int j = 1; j <= (int)XI_MAX + 1; ++j)
            for (int i = j - 1; i >= 0; --i)
                if (n < MMILNOR_INDEX_NUM)
                    result[n++] = j;
        return result;
    }
    inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMilnorGenDeg()
    {
        std::array<int, MMILNOR_INDEX_NUM> result = {0};
        size_t n = 0;
        for (int j = 1; j <= (int)XI_MAX + 1; ++j)
            for (int i = j - 1; i >= 0; --i)
                if (n < MMILNOR_INDEX_NUM)
                    result[n++] = (1 << j) - (1 << i);
        return result;
    }
    inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMilnorGenWeight()
    {
        std::array<int, MMILNOR_INDEX_NUM> result = {0};
        size_t n = 0;
        for (int j = 1; j <= (int)XI_MAX + 1; ++j)
            for (int i = j - 1; i >= 0; --i)
                if (n < MMILNOR_INDEX_NUM)
                    result[n++] = 2 * (j - i) - 1;
        return result;
    }
}  // namespace detail
inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMILNOR_GEN_I = detail::MMilnorGenI();
inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMILNOR_GEN_J = detail::MMilnorGenJ();
inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMILNOR_GEN_DEG = detail::MMilnorGenDeg();
inline constexpr std::array<int, MMILNOR_INDEX_NUM> MMILNOR_GEN_WEIGHT = detail::MMilnorGenWeight();
inline constexpr uint64_t MMILNOR_LEFT_BIT = uint64_t(1) << 63;
inline constexpr uint64_t MMILNOR_MASK_M = (uint64_t(1) << MMILNOR_INDEX_NUM) - 1;
inline constexpr uint64_t MMILNOR_MASK_W = ~MMILNOR_MASK_M;
inline constexpr uint64_t MMILNOR_NULL = 0xffffffffffffffff;

/** Milnor basis for A ordered by May filtration $w(xi_j^{2^i})=2j-1$
 *
 * Each element is represented by a 64-bit unsigned integer
 */

class MMilnor
{
private:
    uint64_t data_;

private:
    static MMilnor AddWeight(uint64_t data)
    {
        uint64_t weight = 0;
        int i = 0;
        for (uint64_t m = data << (64 - MMILNOR_INDEX_NUM); m; m <<= 1, ++i)
            if (m & MMILNOR_LEFT_BIT)
                weight += uint64_t(MMILNOR_GEN_WEIGHT[i]);
        return MMilnor(data + (weight << MMILNOR_INDEX_NUM));
    }

public:
    /**
     * The first 10 bits are used to store the weight.
     * The rest are used to store the exterior monomial.
     */
    MMilnor() : data_(0) {}
    explicit MMilnor(uint64_t data) : data_(data) {}
    static MMilnor FromIndex(size_t i)
    {
        return MMilnor((MMILNOR_ONE >> i) + (uint64_t(MMILNOR_GEN_WEIGHT[i]) << MMILNOR_INDEX_NUM));
    }
    static MMilnor P(int i, int j)
    {
        return MMilnor((MMILNOR_ONE >> (j * (j + 1) / 2 - i - 1)) + (uint64_t(2 * (j - i) - 1) << MMILNOR_INDEX_NUM));
    }
    static uint64_t rawP(int i, int j)
    {
        return MMILNOR_ONE >> (j * (j + 1) / 2 - i - 1);
    }
    static MMilnor Xi(const std::array<int, XI_MAX>& xi)
    {
        uint64_t result = 0;
        uint64_t weight = 0;
        for (int d = 1; d <= xi.size(); ++d) {
            for (int n = xi[size_t(d - 1)], i = 0; n; n >>= 1, ++i) {
                if (n & 1) {
                    int j = i + d;
                    result |= MMilnor::rawP(i, j);
                    weight += 2 * uint64_t(d) - 1;
                }
            }
        }
        return MMilnor(result + (weight << MMILNOR_INDEX_NUM));
    }

public:
    std::array<int, XI_MAX> ToXi() const
    {
        std::array<int, XI_MAX> result = {};
        for (int i : *this)
            result[size_t(MMILNOR_GEN_J[i] - MMILNOR_GEN_I[i] - 1)] += 1 << MMILNOR_GEN_I[i];
        return result;
    }

    bool operator<(MMilnor rhs) const
    {
        return data_ < rhs.data_;
    };

    bool operator==(MMilnor rhs) const
    {
        return data_ == rhs.data_;
    };

    explicit operator bool() const
    {
        return data_;
    }
    uint64_t data() const
    {
        return data_;
    }

    MMilnor mulLF(MMilnor rhs) const
    {
#ifndef NDEBUG /* DEBUG */
        if (gcdLF(rhs))
            throw MyException(0x170c454aU, "gcd(m1,m2)!=1");
#endif
        return MMilnor(((data_ | rhs.data_) & MMILNOR_MASK_M) + ((data_ & MMILNOR_MASK_W) + (rhs.data_ & MMILNOR_MASK_W)));
    }

    MMilnor divLF(MMilnor rhs) const
    {
#ifndef NDEBUG /* DEBUG */
        if (!rhs.divisibleLF(*this))
            throw MyException(0x7ed0a8U, "Not divisible: m1 / m2");
#endif
        return MMilnor(((data_ ^ rhs.data_) & MMILNOR_MASK_M) + ((data_ & MMILNOR_MASK_W) - (rhs.data_ & MMILNOR_MASK_W)));
    }

    bool divisibleLF(MMilnor rhs)
    {
        uint64_t m1 = data_ & MMILNOR_MASK_M;
        uint64_t m2 = rhs.data_ & MMILNOR_MASK_M;
        return m2 >= m1 && !(m1 & (m2 - m1));
    }

    MMilnor gcdLF(MMilnor rhs) const
    {
        return AddWeight(data_ & rhs.data_ & MMILNOR_MASK_M);
    }

    MMilnor lcmLF(MMilnor rhs) const
    {
        return AddWeight((data_ | rhs.data_) & MMILNOR_MASK_M);
    }

    int weight() const
    {
        return (int)(data_ >> MMILNOR_INDEX_NUM);
    }

    int deg() const
    {
        int result = 0;
        for (int i : *this)
            result += MMILNOR_GEN_DEG[i];
        return result;
    }

    std::string StrXi() const;

public:
    class iterator
    {
        friend class MMilnor;

    private:
        uint64_t m_;
        int i_;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using pointer = int*;
        using reference = int;

    public:
        iterator() : m_(0), i_(0) {}

        int operator*() const
        {
            return i_;
        }
        const iterator& operator++()
        {
            for (m_ <<= 1, ++i_; m_; m_ <<= 1, ++i_)
                if (m_ & MMILNOR_LEFT_BIT)
                    break;
            return *this;
        }
        iterator operator++(int)
        {
            iterator copy(*this);
            for (; m_; m_ <<= 1, ++i_)
                if (m_ & MMILNOR_LEFT_BIT)
                    break;
            return copy;
        }
        bool operator==(const iterator& rhs) const
        {
            return m_ == rhs.m_;
        }
        bool operator!=(const iterator& rhs) const
        {
            return m_ != rhs.m_;
        }

    protected:
        iterator(uint64_t m) : m_(m), i_(0)
        {
            for (; m_; m_ <<= 1, ++i_)
                if (m_ & MMILNOR_LEFT_BIT)
                    break;
        }
    };

    iterator begin() const
    {
        return iterator(data_ << (64 - MMILNOR_INDEX_NUM));
    }
    iterator end() const
    {
        return iterator(0);
    }
};
using MMilnor1d = std::vector<MMilnor>;
using MMilnor2d = std::vector<MMilnor1d>;

inline MMilnor mulLF(MMilnor m1, MMilnor m2)
{
    return m1.mulLF(m2);
}

inline MMilnor divLF(MMilnor m1, MMilnor m2)
{
    return m1.divLF(m2);
}

inline bool divisibleLF(MMilnor m1, MMilnor m2)
{
    return m1.divisibleLF(m2);
}

inline MMilnor gcdLF(MMilnor m1, MMilnor m2)
{
    return m1.gcdLF(m2);
}

inline MMilnor lcmLF(MMilnor m1, MMilnor m2)
{
    return m1.lcmLF(m2);
}

/* Elements of A as linear combinations of Milnor basis
 */
struct Milnor
{
    MMilnor1d data;
    Milnor() {}
    explicit Milnor(MMilnor m) : data({m}) {}

    static Milnor P(int i, int j)
    {
        return Milnor(MMilnor::P(i, j));
    }

    Milnor operator+(const Milnor& rhs) const
    {
        Milnor result;
        std::set_symmetric_difference(data.begin(), data.end(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(result.data));
        return result;
    }
    Milnor& operator+=(const Milnor& rhs)
    {
        Milnor tmp;
        std::swap(data, tmp.data);
        std::set_symmetric_difference(tmp.data.cbegin(), tmp.data.cend(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(data));
        return *this;
    }
    Milnor operator*(const Milnor& rhs) const;

    std::string StrXi() const;
};

std::ostream& operator<<(std::ostream& sout, const Milnor& x);
inline Milnor operator*(MMilnor m1, MMilnor m2) ////
{
    return Milnor(m1) * Milnor(m2);
}

/********************************************************
 *                    Modules
 ********************************************************/

/* The left 12 bits will be used to store the basis */
inline constexpr unsigned MMOD_BASIS_BITS = 12;
inline constexpr uint64_t MMOD_MASK_M = (uint64_t(1) << (64 - MMOD_BASIS_BITS)) - 1;
inline constexpr uint64_t MMOD_MASK_V = ~MMOD_MASK_M;
/* Modules over A */
class MMod
{
private:
    uint64_t data_;

public:
    MMod() : data_(0) {}
    MMod(uint64_t data) : data_(data) {}
    MMod(MMilnor m, int v) : data_(m.data() + ((~uint64_t(v)) << (64 - MMOD_BASIS_BITS))) {}

    bool operator==(MMod rhs) const
    {
        return data_ == rhs.data_;
    };
    bool cmpLF(MMod rhs) const
    {
        return data_ < rhs.data_;
    }
    explicit operator bool() const
    {
        return data_;
    }
    MMilnor m() const
    {
        return MMilnor(data_ & MMOD_MASK_M);
    }
    int v() const
    {
        return int((~data_) >> (64 - MMOD_BASIS_BITS));
    }
    std::string Str() const;
    std::string StrXi() const;
};
using MMod1d = std::vector<MMod>;
using MMod2d = std::vector<MMod1d>;

inline bool cmpLF(MMod lhs, MMod rhs)
{
    return lhs.cmpLF(rhs);
}

inline MMod mulLF(MMilnor m, MMod x)
{
    return MMod(mulLF(m, x.m()), x.v());
}

struct Mod
{
    MMod1d data;
    Mod() {}
    Mod(MMod mv) : data({mv}) {}
    Mod(const Milnor& a, int v)
    {
        data.reserve(a.data.size());
        for (MMilnor m : a.data)
            data.push_back(MMod(m, v));
    }

    MMod GetLead() const
    {
#ifndef NDEBUG
        if (data.empty())
            throw MyException(0x900cee0fU, "ModCpt empty");
#endif
        return data[0];
    }

    explicit operator bool() const
    {
        return !data.empty();
    }
    template<typename FnCmp>
    Mod add(const Mod& rhs, FnCmp cmp) const
    {
        Mod result;
        std::set_symmetric_difference(data.begin(), data.end(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(result.data), cmp);
        return result;
    }
    template <typename FnCmp>
    Mod& iadd(const Mod& rhs, FnCmp cmp)
    {
        Mod tmp;
        std::swap(data, tmp.data);
        std::set_symmetric_difference(tmp.data.cbegin(), tmp.data.cend(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(data), cmp);
        return *this;                                                                                                          
    }
    Mod addLF(const Mod& rhs) const
    {
        Mod result;
        std::set_symmetric_difference(data.begin(), data.end(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(result.data), cmpLF);
        return result;
    }
    Mod& iaddLF(const Mod& rhs)
    {
        Mod tmp;
        std::swap(data, tmp.data);
        std::set_symmetric_difference(tmp.data.cbegin(), tmp.data.cend(), rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(data), cmpLF);
        return *this;
    }
    bool operator==(const Mod& rhs) const
    {
        return data == rhs.data;
    };

    std::string StrXi() const;
    std::string Str() const;
};
using Mod1d = std::vector<Mod>;
using Mod2d = std::vector<Mod1d>;
using Mod3d = std::vector<Mod2d>;

inline std::ostream& operator<<(std::ostream& sout, const Mod& x)
{
    return sout << x.Str();
}

namespace detail {
    void MulMilnor(MMilnor lhs, MMilnor rhs, Milnor& result);
    void MulMilnor(MMilnor lhs, MMod rhs, Mod& result);

    template <typename FnCmp>
    void SortMod2(MMod1d& data, FnCmp cmp)
    {
        std::sort(data.begin(), data.end(), cmp);
        for (size_t i = 0; i + 1 < data.size(); ++i)
            if (data[i] == data[i + 1]) {
                data[i] = MMod(0xffffffffffffffff);
                data[++i] = MMod(0xffffffffffffffff);
            }
        ut::RemoveIf(data, [](const MMod& m) { return m == MMod(0xffffffffffffffff); });
    }
}

template <typename FnCmp>
Mod mulMod(MMilnor m, const Mod& x, FnCmp cmp)
{
    Mod result;
    for (MMod m2 : x.data)
        detail::MulMilnor(m, m2, result);
    detail::template SortMod2(result.data, cmp);
    return result;
}

/* Compute the product in the associated graded algebra */
Mod mulLF(MMilnor m, const Mod& x);

template <typename FnMap>
Mod TplSubs(const Mod& x, FnMap map)
{
    Mod result;
    for (const MMod& mv : x.data)
        result += Milnor(mv.m()) * map(mv.v());
    return result;
}

///**
// * Replace v_i with `map[i]`.
// */
//inline Mod subs(const Mod& x, const Mod1d& map)
//{
//    Mod result;
//    for (const MMod& mv : x.data)
//        result += Milnor(mv.m()) * map[mv.v()];
//    return result;
//}

}  // namespace steenrod

#endif
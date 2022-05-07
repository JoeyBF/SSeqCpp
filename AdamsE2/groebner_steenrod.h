/** \file groebner_steenrod.h
 * A Component for Groebner bases.
 */

//#define MYDEPLOY
#ifdef MYDEPLOY
//#define MYDEPLOY_TEST_FILE
#endif

#ifndef GROEBNER_STEENROD_H
#define GROEBNER_STEENROD_H

#include "algebras/benchmark.h"
#include "algebras/steenrod.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace steenrod {

template <typename>
struct is_sequence : std::false_type
{
};
template <typename T>
struct is_sequence<std::vector<T>> : std::true_type
{
    using value_type = T;
};
template <typename T>
struct is_sequence<std::unordered_set<T>> : std::true_type
{
    using value_type = T;
};

template <typename>
struct is_map : std::false_type
{
};
template <typename K, typename T>
struct is_map<std::map<K, T>> : std::true_type
{
    using key_type = K;
    using value_type = T;
};

template <typename T>
uint64_t myhash(const T& x)
{
    if constexpr (is_sequence<T>::value) {
        uint64_t seed = 0;
        for (auto it = x.begin(); it != x.end(); ++it)
            ut::hash_combine(seed, myhash<typename is_sequence<T>::value_type>(*it));
        return seed;
    }
    else if constexpr (is_map<T>::value) {
        uint64_t seed = 0;
        for (auto it = x.begin(); it != x.end(); ++it) {
            ut::hash_combine(seed, myhash<typename is_map<T>::key_type>(it->first));
            ut::hash_combine(seed, myhash<typename is_map<T>::value_type>(it->second));
        }
        return seed;
    }
    else
        return x.hash();
}

template <>
inline uint64_t myhash<int>(const int& x)
{
    return uint64_t(x);
}

template <>
inline uint64_t myhash<uint64_t>(const uint64_t& x)
{
    return uint64_t(x);
}

template <>
inline uint64_t myhash<MMod>(const MMod& x)
{
    return uint64_t(x.data());
}

template <>
inline uint64_t myhash<Mod>(const Mod& x)
{
    return myhash(x.data);
}

struct CriMilnor
{
    int i1 = -1, i2 = -1;
    MMilnor m1, m2;

    /* Compute the pair for two leading monomials. */
    CriMilnor() = default;
    static void SetFromLM(CriMilnor& result, MMilnor lead1, MMilnor lead2, int i, int j)
    {
        MMilnor gcd = gcdLF(lead1, lead2);
        result.m1 = divLF(lead2, gcd);
        result.m2 = divLF(lead1, gcd);
        result.i1 = i;
        result.i2 = j;
    }
    static CriMilnor Single(MMilnor m2, int j)
    {
        CriMilnor result;
        result.m2 = m2;
        result.i1 = -1;
        result.i2 = j;
        return result;
    }

    uint64_t hash() const
    {
        uint64_t seed = 0;
        ut::hash_combine(seed, i1);
        ut::hash_combine(seed, i2);
        ut::hash_combine(seed, m1.data());
        ut::hash_combine(seed, m2.data());
        return seed;
    }
};
using CriMilnor1d = std::vector<CriMilnor>;
using CriMilnor2d = std::vector<CriMilnor1d>;
using CriMilnor3d = std::vector<CriMilnor2d>;
using PtCriMilnor1d = std::vector<CriMilnor*>;
using PtCriMilnor2d = std::vector<PtCriMilnor1d>;

/* Groebner basis of critical pairs */
class CriMilnors
{
    friend class GroebnerMRes;

private:
    int t_trunc_;                                                        /* Truncation degree */
    CriMilnor2d gb_;                                                     /* `pairs_[j]` is the set of pairs (i, j) with given j */
    std::map<int, CriMilnor2d> buffer_min_pairs_;                        /* `buffer_min_pairs_[t]` To generate minimal pairs to compute Sij */
    std::map<int, std::unordered_set<uint64_t>> buffer_redundent_pairs_; /* Used to minimize `buffer_min_pairs_` */
    std::map<int, CriMilnor1d> buffer_singles_;                          /* For computing Sj. `buffer_singles_` stores indices of singles_ */

public:
    CriMilnors(int t_trunc) : t_trunc_(t_trunc) {}

    uint64_t hash() const
    {
        uint64_t seed = 0;
        ut::hash_combine(seed, myhash(gb_));
        ut::hash_combine(seed, myhash(buffer_min_pairs_));
        ut::hash_combine(seed, myhash(buffer_redundent_pairs_));
        ut::hash_combine(seed, myhash(buffer_singles_));
        return seed;
    }

    int t_trunc() const
    {
        return t_trunc_;
    }
    bool empty_min_pairs_for_gb(int t) const
    {
        return buffer_min_pairs_.find(t) == buffer_min_pairs_.end();
    }
    /* Return both critical pairs and critical singles */
    CriMilnor1d Criticals(int t);

    /* Minimize `buffer_min_pairs_[t]` and maintain `pairs_` */
    void Minimize(const MMod1d& leads, int t);

    /* Propogate `buffer_redundent_pairs_` and `buffer_min_pairs_`.
    ** `buffer_min_pairs_` will become a Groebner basis at this stage.
    */
    void AddToBuffers(const MMod1d& leads, MMod mon, int t_v);

    void init(const MMod1d& leads, const array& basis_degrees, int t_min_buffer);
};
using CriMilnors1d = std::vector<CriMilnors>;

struct Filtr
{
    uint64_t data;
    Filtr() : data(~0) {}
    Filtr(MMod m) : data(m.v_raw() | m.w_may()) {}
    Filtr(uint64_t data_) : data(data_) {}
    bool operator<(Filtr rhs) const
    {
        return data < rhs.data;
    }
    bool operator==(Filtr rhs) const
    {
        return data == rhs.data;
    }
    Filtr operator+(uint64_t w_may) const
    {
        return Filtr(data + w_may);
    }
};

/********************************************************
 *                    class GroebnerX2m
 ********************************************************/

class GroebnerX2m
{
    using TypeIndices = std::vector<std::unordered_map<uint64_t, array>>;

private:
    int t_trunc_;

    CriMilnors1d criticals_; /* Groebner basis of critical pairs */

    Mod2d gb_;
    MMod2d leads_;        /* Leading monomials */
    TypeIndices indices_; /* Cache for fast divisibility test */

    array2d basis_degrees_; /* `basis_degrees_x2m_[s][i]` is the degree of w_{s,i} */

public:
    GroebnerX2m(int t_trunc, Mod2d data, array2d basis_degrees, int latest_s, int latest_t);

    uint64_t hash() const
    {
        uint64_t seed = 0;
        ut::hash_combine(seed, myhash(criticals_));
        ut::hash_combine(seed, myhash(basis_degrees_));
        return seed;
    }

public:
    void resize(size_t s)
    {
        if (basis_degrees_.size() < s + 1)
            basis_degrees_.resize(s + 1);
        if (gb_.size() < s) {
            gb_.resize(s);
            leads_.resize(s);
            indices_.resize(s);
            while (criticals_.size() < s)
                criticals_.push_back(CriMilnors(t_trunc_));
        }
    }

    Mod new_gen(size_t s, int t)
    {
        basis_degrees_[s].push_back(t);
        return MMod(MMilnor(), basis_degrees_[s].size() - 1);
    }

    void push_back(Mod g, size_t s)
    {
        MMod m = g.GetLead();
        criticals_[s].AddToBuffers(leads_[s], m, basis_degrees_[s][m.v()]);

        leads_[s].push_back(m);
        indices_[s][m.v_raw()].push_back((int)gb_[s].size());
        gb_[s].push_back(std::move(g));
    }

    const array& basis_degrees(size_t s)
    {
        return basis_degrees_[s];
    }

    const array2d& basis_degrees()
    {
        return basis_degrees_;
    }

    const auto& data() const
    {
        return gb_;
    }

    Mod Reduce(Mod x2m, size_t s) const;
    Mod Reduce(const CriMilnor& p, size_t s) const;
    Mod1d AddRels(size_t s, int t);
};

/********************************************************
 *                    class GroebnerMRes
 ********************************************************/

struct DataMRes
{
    Mod x1, x2, x2m;
    Filtr fil;
    DataMRes() {}
    DataMRes(Mod x1_, Mod x2_, Mod x2m_) : x1(std::move(x1_)), x2(std::move(x2_)), x2m(std::move(x2m_))
    {
        if (x1)
            fil = Filtr(x1.GetLead());
    }
    DataMRes& operator+=(const DataMRes& rhs)
    {
#ifndef NDEBUG
        if (!rhs.valid_x2m())
            throw MyException(0x2ae8baa3U, "Add only when rhs.x2m is valid");
#endif
        if (valid_x2m() && fil == rhs.fil)
            x2m += rhs.x2m;
        x1 += rhs.x1;
        x2 += rhs.x2;
        return *this;
    }
    bool valid_x2m() const
    {
        if (x1)
            return fil == Filtr(x1.GetLead());
        if (x2m)
            return false;
        return true;
    }
    uint64_t hash() const
    {
        uint64_t seed = 0;
        ut::hash_combine(seed, myhash(x1));
        ut::hash_combine(seed, myhash(x2));
        ut::hash_combine(seed, myhash(x2m));
        ut::hash_combine(seed, myhash(fil.data));
        return seed;
    }
};

using DataMRes1d = std::vector<DataMRes>;
using DataMRes2d = std::vector<DataMRes1d>;

inline void Reduce(Mod& x, const DataMRes1d& y, Mod& tmp)
{
    for (size_t i = 0; i < y.size(); ++i)
        if (std::binary_search(x.data.begin(), x.data.end(), y[i].x1.GetLead()))
            x.iadd(y[i].x1, tmp);
}

class GroebnerMRes
{
    using TypeIndices = std::vector<std::unordered_map<uint64_t, array>>;

private:
    int t_trunc_;

    CriMilnors1d criticals_; /* Groebner basis of critical pairs */

    DataMRes2d gb_;
    MMod2d leads_;        /* Leading monomials */
    TypeIndices indices_; /* Cache for fast divisibility test */

    array2d basis_degrees_; /* `basis_degrees[s][i]` is the degree of v_{s,i} */

    GroebnerX2m gb_x2m_;

public:
    /* Initialize from `polys` which already forms a Groebner basis. Must not add more relations. */
    GroebnerMRes(int t_trunc, DataMRes2d data, array2d basis_degrees, Mod2d data_x2m, array2d basis_degrees_x2m, int latest_s, int latest_t);
    static GroebnerMRes load(const std::string& filename, int t_trunc);
    static void reset(const std::string& filename);

    uint64_t hash() const
    {
        uint64_t seed = 0;
        //ut::hash_combine(seed, myhash(criticals_));
        //ut::hash_combine(seed, myhash(gb_x2m_));
        ut::hash_combine(seed, myhash(gb_));
        return seed;
    }

public:
    int t_trunc() const
    {
        return t_trunc_;
    }

    int t_begin() const
    {
        int t = 0;
        for (size_t s = 0; s < gb_.size(); ++s) {
            if (!gb_[s].empty()) {
                auto lead = gb_[s].back().x1.GetLead();
                int t1 = lead.deg_m() + basis_degrees_[s][lead.v()];
                if (t1 > t)
                    t = t1;
            }
        }
        return t;
    }

    const array& basis_degrees(size_t s) const
    {
        return basis_degrees_[s];
    }

    int dim_Ext() const {
        size_t dim = 0; 
        for (size_t s = 0; s < basis_degrees_.size(); ++s)
            dim += basis_degrees_[s].size();
        return (int)dim;
    }

    int dim_Gb() const
    {
        size_t dim = 0;
        for (size_t s = 0; s < gb_.size(); ++s)
            dim += gb_[s].size();
        return (int)dim;
    }

    const array& basis_degrees_x2m(size_t s)
    {
        return gb_x2m_.basis_degrees(s);
    }

    const array2d& basis_degrees_x2m()
    {
        return gb_x2m_.basis_degrees();
    }

    void resize_gb(size_t s)
    {
        if (basis_degrees_.size() < s + 1)
            basis_degrees_.resize(s + 1);
        if (gb_.size() < s) {
            gb_.resize(s);
            leads_.resize(s);
            indices_.resize(s);
            while (criticals_.size() < s)
                criticals_.push_back(CriMilnors(t_trunc_));
            if (s >= 0)
                gb_x2m_.resize(s - 1);
        }
    }

    Mod new_gen(size_t s, int t)
    {
        basis_degrees_[s].push_back(t);
        return MMod(MMilnor(), basis_degrees_[s].size() - 1);
    }

    Mod new_gen_x2m(size_t s, int t)
    {
        return gb_x2m_.new_gen(s, t);
    }

    void push_back(DataMRes g, size_t s)
    {
        MMod m = g.x1.GetLead();
        criticals_[s].AddToBuffers(leads_[s], m, basis_degrees_[s][m.v()]);

        leads_[s].push_back(m);
        indices_[s][m.v_raw()].push_back((int)gb_[s].size());
        gb_[s].push_back(std::move(g));
    }

    void push_back_x2m(Mod g, size_t s)
    {
        gb_x2m_.push_back(std::move(g), s);
    }

    const auto& data() const
    {
        return gb_;
    }

    const auto& data_x2m() const
    {
        return gb_x2m_.data();
    }

    CriMilnor1d Criticals(size_t s, int t, Mod1d& rels_x2m);
    DataMRes Reduce(const CriMilnor& cp, size_t s) const;
    Mod Reduce(Mod x, size_t s) const;
    Mod ReduceX2m(const CriMilnor& cp, size_t s) const;
};

/**
 * Comsume relations from 'rels` and `gb.criticals_` in degree `<= deg`.
 *
 * return the dimension of the calculated range for debugging.
 */
void AddRelsMRes(GroebnerMRes& gb, const Mod1d& rels, int deg);

}  // namespace steenrod

#endif
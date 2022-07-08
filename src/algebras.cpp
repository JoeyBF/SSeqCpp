#include "algebras.h"
#include "myexception.h"
#include "myio.h"
#include <iterator>

#ifndef NDEBUG
#include <iostream>
#endif

namespace alg {

std::string GE::Str() const
{
    std::string result = "x_";
    std::string under = std::to_string(g());
    std::string upper = std::to_string(e());
    if (under.size() > 1)
        result += '{' + under + '}';
    else
        result += under;

    if (upper.size() > 1)
        result += "^{" + upper + '}';
    else
        result += '^' + upper;
    return result;
}

MonTrace Mon::Trace() const  // TODO: Try two bits for one exponents
{
    MonTrace result = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        const int bits_exp1 = 56;
        const int bits_exp2 = 64 - bits_exp1;
        result |= (MonTrace(1) << (data[i].g() % bits_exp1));
        if (data[i].e() >= 2)
            result |= (MonTrace(1) << ((data[i].g() % bits_exp2) + bits_exp1));
    }
    return result;
}

std::string Mon::Str() const
{
    return myio::TplStrCont("", "", "", "1", data.begin(), data.end(), [](GE p) { return p.Str(); });
}

void mulP(const Mon& mon1, const Mon& mon2, Mon& result)
{
    result.data.clear();
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            result.push_back(*k++);
        else if (k->g_raw() < l->g_raw())
            result.push_back(*l++);
        else {
            result.push_back(GE(k->data + l->e()));
            ++k;
            ++l;
        }
    }
    if (k != mon1.end())
        result.data.insert(result.end(), k, mon1.end());
    else
        result.data.insert(result.end(), l, mon2.end());
}

void divP(const Mon& mon1, const Mon& mon2, Mon& result)
{
    result.data.clear();
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            result.push_back(*k++);
#ifndef NDEBUG /* DEBUG */
        else if (k->g_raw() < l->g_raw())
            throw MyException(0x1227de8e, "mon1/mon2 not divisible!\n");
#endif
        else if (k->e() > l->e()) {
            result.push_back(GE(k->data - l->e()));
            ++k;
            ++l;
        }
#ifdef NDEBUG
        else {
            ++k;
            ++l;
        }
#else /* DEBUG */
        else if (k->e() == l->e()) {
            ++k;
            ++l;
        }
        else
            throw MyException(0xa9c74ef9, "mon1/mon2 not divisible!\n");
#endif
    }
#ifndef NDEBUG /* DEBUG */
    if (l != mon2.end())
        throw MyException(0x6cdd66bd, "mon1/mon2 not divisible!\n");
    else
#endif
        result.data.insert(result.end(), k, mon1.end());
}

bool divisible(const Mon& mon1, const Mon& mon2)
{
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            return false;
        else if (k->g_raw() < l->g_raw())
            ++l;
        else if (k->e() > l->e())
            return false;
        else {
            ++k;
            ++l;
        }
    }
    if (k != mon1.end())
        return false;
    return true;
}

void powP(const Mon& mon, int e, Mon& result)
{
    result.data.clear();
    if (e == 0)
        return;
    else if (e == 1) {
        result = mon;
        return;
    }
    for (auto p = mon.begin(); p != mon.end(); ++p)
        result.push_back(GE(p->g_raw() | (p->e() * e)));
}

int log(const Mon& mon1, const Mon& mon2)
{
    if (!mon2) {
        /* log with 0 base */
        throw "f50d7f56";
    }
    int q = -1;

    /* Compute q */
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            ++k;
        else if (k->g_raw() < l->g_raw()) {
            q = 0;
            break;
        }
        else if (k->e() < l->e()) {
            q = 0;
            break;
        }
        else {
            int q1 = k->e() / l->e();
            if (q == -1 || q > q1)
                q = q1;
            ++k;
            ++l;
        }
    }
    if (l != mon2.end())
        q = 0;
    return q;
}

void GcdP(const Mon& mon1, const Mon& mon2, Mon& result)
{
    result.data.clear();
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            ++k;
        else if (k->g_raw() < l->g_raw())
            ++l;
        else {
            result.push_back(GE(k->g_raw() | std::min(k->e(), l->e())));
            ++k;
            ++l;
        }
    }
}

void LcmP(const Mon& mon1, const Mon& mon2, Mon& result)
{
    result.data.clear();
    auto k = mon1.begin(), l = mon2.begin();
    while (k != mon1.end() && l != mon2.end()) {
        if (k->g_raw() > l->g_raw())
            result.push_back(*k++);
        else if (k->g_raw() < l->g_raw())
            result.push_back(*l++);
        else {
            result.push_back(GE(k->g_raw() | std::max(k->e(), l->e())));
            ++k;
            ++l;
        }
    }
    if (k != mon1.end())
        result.data.insert(result.end(), k, mon1.end());
    else
        result.data.insert(result.end(), l, mon2.end());
}

/**
 * Sort the sequence and each time remove a pair of identical elements
 */
void SortMod2(Mon1d& data)
{
    static const Mon MON_NULL = GE(0xffff);
    std::sort(data.begin(), data.end());
    for (size_t i = 0; i + 1 < data.size(); ++i)
        if (data[i] == data[i + 1]) {
            data[i] = MON_NULL;
            data[++i] = MON_NULL;
        }
    ut::RemoveIf(data, [](const Mon& m) { return m == MON_NULL; });
}

void mulP(const Poly& p1, const Poly& p2, Poly& result)
{
    result.data.clear();
    for (size_t i = 0; i < p1.data.size(); ++i)
        for (size_t j = 0; j < p2.data.size(); ++j)
            result.data.push_back(mul(p1.data[i], p2.data[j]));
    std::sort(result.data.begin(), result.data.end());
    SortMod2(result.data);
}

void mulP(const Poly& poly, const Mon& mon, Poly& result)
{
    result.data.clear();
    result.data.reserve(poly.data.size());
    for (const Mon& m : poly.data)
        result.data.push_back(mul(m, mon));
}

void powP(const Poly& poly, uint32_t n, Poly& result, Poly& tmp)
{
    result.data.clear();
    result.data.push_back(Mon());
    if (n == 0)
        return;
    Poly power = poly;
    while (n) {
        if (n & 1)
            result.imulP(power, tmp);
        n >>= 1;
        if (n) {
            power.frobP(tmp);
            std::swap(power, tmp);
        }
    }
}

Poly GetDiff(const Mon& mon, const Poly1d& diffs)
{
    Poly result, tmp_prod, tmp;
    for (auto k = mon.begin(); k != mon.end(); ++k) {
        if (k->e() % 2) {
            mulP(diffs[k->g()], div(mon, GE(k->g_raw() | 1)), tmp_prod);
            result.iaddP(tmp_prod, tmp);
        }
    }
    return result;
}

Poly GetDiff(const Poly& poly, const Poly1d& diffs)
{
    Poly result, tmp_prod, tmp;
    for (const Mon& mon : poly.data) {
        for (auto k = mon.begin(); k != mon.end(); ++k) {
            if (k->e() % 2) {
                mulP(diffs[k->g()], div(mon, GE(k->g_raw() | 1)), tmp_prod);
                result.iaddP(tmp_prod, tmp);
            }
        }
    }
    return result;
}

uint1d Poly2Indices(const Mon1d& poly, const Mon1d& basis)
{
    uint1d result;
    for (const Mon& mon : poly) {
        auto p = std::lower_bound(basis.begin(), basis.end(), mon);
#ifndef NDEBUG
        if (p == basis.end() || mon < (*p)) {
            std::cout << "index not found\n";
            throw "178905cf";
        }
#endif
        result.push_back(uint32_t(p - basis.begin()));
    }
    return result;
}

Mon1d Indices2Poly(const uint1d& indices, const Mon1d& basis)
{
    Mon1d result;
    for (int i : indices)
        result.push_back(basis[i]);
    return result;
}

std::string Poly::Str() const
{
    return myio::TplStrCont("", "+", "", "0", data.begin(), data.end(), [](const Mon& m) { return m.Str(); });
}

}  // namespace alg

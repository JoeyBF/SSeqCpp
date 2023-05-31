#include "algebras/benchmark.h"
#include "algebras/database.h"
#include "algebras/linalg.h"
#include "algebras/utility.h"
#include "groebner_res_const.h"
#include "main.h"
#include <cstring>

int1d HomToK(const Mod& x)
{
    int1d result;
    for (MMod m : x.data)
        if (m.deg_m() == 0)
            result.push_back((int)m.v());
    return result;
}

int1d HomToMSq(const Mod& x, int t_cell);

class DbAdamsResProd : public myio::Database
{
    using Statement = myio::Statement;

public:
    explicit DbAdamsResProd(const std::string& filename) : Database(filename)
    {
        if (newFile_)
            SetVersion();
    }

    void SetVersion()
    {
        execute_cmd("CREATE TABLE IF NOT EXISTS version (id INTEGER PRIMARY KEY, name TEXT, value);");
        Statement stmt(*this, "INSERT INTO version (id, name, value) VALUES (?1, ?2, ?3) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
        stmt.bind_and_step(0, std::string("version"), DB_ADAMS_VERSION);
        stmt.bind_and_step(1, std::string("change notes"), std::string(DB_VERSION_NOTES));
        stmt.bind_and_step(817812698, std::string("t_max"), -1);
        Statement stmt2(*this, "INSERT INTO version (id, name, value) VALUES (1954841564, \"timestamp\", unixepoch()) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
        stmt2.step_and_reset();
    }

    int GetVersion()
    {
        if (has_table("version"))
            return get_int("select value from version where id=0");
        return -1;
    }

    bool ConvertVersion(const DbAdamsResLoader& dbRes)
    {
        if (GetVersion() < 1) { /* to version=1 */
            try {
                begin_transaction();
                std::string table_prefix = "S0_Adams_res";
                /*# Rename old SteenrodMRes prefix */
                if (!has_table(table_prefix + "_generators")) {
                    std::string table_prefix_old = "SteenrodMRes";
                    std::array<std::string, 5> tables = {"_generators", "_products", "_products_time"};
                    for (auto& t : tables)
                        rename_table(table_prefix_old + t, table_prefix + t);
                }
                /*# change generator id */
                /*## load change table */
                std::unordered_map<int, int> id_to_newid;
                {
                    Statement stmt(dbRes, "SELECT oldid, id FROM S0_Adams_res_generators;");
                    while (stmt.step() == MYSQLITE_ROW) {
                        int oldid = stmt.column_int(0), id = stmt.column_int(1);
                        id_to_newid[oldid] = id;
                    }
                }
                /*## change id in S0_Adams_res_generators */
                {
                    std::map<int, int> ids;
                    {
                        Statement stmt(*this, "SELECT rowid, id FROM S0_Adams_res_generators;");
                        while (stmt.step() == MYSQLITE_ROW) {
                            int rowid = stmt.column_int(0);
                            ids[rowid] = id_to_newid.at(stmt.column_int(1));
                        }
                    }
                    {
                        Statement stmt(*this, "UPDATE S0_Adams_res_generators SET id=?1 WHERE rowid=?2;");
                        for (auto& [rowid, id] : ids) {
                            stmt.bind_and_step(-id, rowid);
                        }
                    }
                    execute_cmd("UPDATE S0_Adams_res_generators SET id=-id");
                }
                /*## change id, id_ind in S0_Adams_res_products */
                {
                    std::map<int, int> ids;
                    std::map<int, int> ids_ind;
                    std::map<int, int1d> prod_h_s;
                    {
                        Statement stmt(*this, "SELECT rowid, id, id_ind, prod_h FROM S0_Adams_res_products;");
                        while (stmt.step() == MYSQLITE_ROW) {
                            int rowid = stmt.column_int(0);
                            ids[rowid] = id_to_newid.at(stmt.column_int(1));
                            ids_ind[rowid] = id_to_newid.at(stmt.column_int(2));
                            prod_h_s[rowid] = stmt.column_blob_tpl<int>(3);
                        }
                    }
                    const std::string table_prod = "S0_Adams_res_products";
                    drop_column(table_prod, "prod_h");
                    if (has_column(table_prod, "prod_h_glo"))
                        drop_column(table_prod, "prod_h_glo");
                    add_column(table_prod, "prod_h TEXT");
                    {
                        Statement stmt(*this, "UPDATE S0_Adams_res_products SET id=?1, id_ind=?2, prod_h=?3 WHERE rowid=?4;");
                        for (auto& [rowid, _] : ids) {
                            stmt.bind_and_step(-ids.at(rowid), -ids_ind.at(rowid), myio::Serialize(prod_h_s.at(rowid)), rowid);
                        }
                    }
                    execute_cmd("UPDATE S0_Adams_res_products SET id=-id, id_ind=-id_ind");
                }

                SetVersion();
                end_transaction();
                fmt::print("DbResProd Converted to version=1\n{}", myio::COUT_FLUSH());
            }
            catch (MyException&) {
                return false;
            }
        }
        return true;
    }

public:
    void create_tables(const std::string& table_prefix)
    {
        execute_cmd(fmt::format("CREATE TABLE IF NOT EXISTS {}_generators (id INTEGER PRIMARY KEY, indecomposable TINYINT, s SMALLINT, t SMALLINT);", table_prefix));
        execute_cmd(fmt::format("CREATE TABLE IF NOT EXISTS {}_products (id INTEGER, id_ind INTEGER, prod BLOB, prod_h TEXT, PRIMARY KEY (id, id_ind));", table_prefix));
        execute_cmd(fmt::format("CREATE TABLE IF NOT EXISTS {}_products_time (s SMALLINT, t SMALLINT, time REAL, PRIMARY KEY (s, t));", table_prefix));
    }

    void save_time(const std::string& table_prefix, int s, int t, double time)
    {
        Statement stmt(*this, fmt::format("INSERT OR IGNORE INTO {}_products_time (s, t, time) VALUES (?1, ?2, ?3);", table_prefix));
        stmt.bind_and_step(s, t, time);
    }

    int1d load_old_ids(std::string_view table_prefix) const
    {
        int1d result;
        Statement stmt(*this, fmt::format("SELECT DISTINCT id FROM {}_products ORDER BY id", table_prefix));
        while (stmt.step() == MYSQLITE_ROW) {
            int id = stmt.column_int(0);
            result.push_back(id);
        }
        return result;
    }

    /* result[g][index] = image(v_index) in degree s */
    std::map<int, Mod1d> load_products(std::string_view table_prefix, int s, const int1d& gs_exclude) const
    {
        std::map<int, Mod1d> result;
        Statement stmt(*this, fmt::format("SELECT id, id_ind, prod FROM {}_products WHERE (id>>19)={} ORDER BY id;", table_prefix, s));
        while (stmt.step() == MYSQLITE_ROW) {
            int id = stmt.column_int(0);
            int g = stmt.column_int(1);
            if (!ut::has(gs_exclude, g)) {
                Mod prod;
                prod.data = stmt.column_blob_tpl<MMod>(2);
                ut::get(result[g], LocId(id).v) = std::move(prod);
            }
        }
        return result;
    }
};

class DbAdamsResMap : public myio::Database
{
    using Statement = myio::Statement;

public:
    explicit DbAdamsResMap(const std::string& filename) : Database(filename)
    {
        if (newFile_)
            SetVersion();
    }

    void SetVersion()
    {
        execute_cmd("CREATE TABLE IF NOT EXISTS version (id INTEGER PRIMARY KEY, name TEXT, value);");
        Statement stmt(*this, "INSERT INTO version (id, name, value) VALUES (?1, ?2, ?3) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
        stmt.bind_and_step(0, std::string("version"), DB_ADAMS_VERSION);
        stmt.bind_and_step(1, std::string("change notes"), std::string(DB_VERSION_NOTES));
        stmt.bind_and_step(817812698, std::string("t_max"), -1);
    }

    int GetVersion()
    {
        if (has_table("version"))
            return get_int("select value from version where id=0");
        return -1;
    }

    bool ConvertVersion(const DbAdamsResLoader& dbRes)
    {
        return true;
    }

public:
    void create_tables(const std::string& table_prefix)
    {
        execute_cmd("CREATE TABLE IF NOT EXISTS " + table_prefix + " (id INTEGER PRIMARY KEY, map BLOB, map_h TEXT);");
        execute_cmd("CREATE TABLE IF NOT EXISTS " + table_prefix + "_time (s SMALLINT, t SMALLINT, time REAL, PRIMARY KEY (s, t));");
    }

    void save_time(const std::string& table_prefix, int s, int t, double time)
    {
        Statement stmt(*this, "INSERT OR IGNORE INTO " + table_prefix + "_time (s, t, time) VALUES (?1, ?2, ?3);");
        stmt.bind_and_step(s, t, time);
    }

    int1d load_old_ids(std::string_view table_prefix) const
    {
        int1d result;
        Statement stmt(*this, fmt::format("SELECT DISTINCT id FROM {} ORDER BY id", table_prefix));
        while (stmt.step() == MYSQLITE_ROW) {
            int id = stmt.column_int(0);
            result.push_back(id);
        }
        return result;
    }

    /* result[index] = image(v_index) in degree s */
    Mod1d load_map(std::string_view table_prefix, int s) const
    {
        Mod1d result;
        Statement stmt(*this, fmt::format("SELECT id, map FROM {} WHERE (id>>19)={} ORDER BY id;", table_prefix, s));
        while (stmt.step() == MYSQLITE_ROW) {
            int id = stmt.column_int(0);
            Mod map_;
            map_.data = stmt.column_blob_tpl<MMod>(1);
            int v = LocId(id).v;
            ut::get(result, v) = std::move(map_);
        }
        return result;
    }
};

AdamsResConst AdamsResConst::load(const DbAdamsResLoader& db, const std::string& table, int t_trunc)
{
    DataMResConst2d data = db.load_data(table, t_trunc);
    int2d basis_degrees = db.load_basis_degrees(table, t_trunc);
    return AdamsResConst(std::move(data), std::move(basis_degrees));
}

AdamsResConst AdamsResConst::load_gb_by_basis_degrees(const DbAdamsResLoader& db, const std::string& table, int t_trunc)
{
    DataMResConst2d data;
    int2d basis_degrees = db.load_basis_degrees(table, t_trunc);
    return AdamsResConst(std::move(data), std::move(basis_degrees));
}

void AdamsResConst::rotate(const DbAdamsResLoader& db, const std::string& table, int s, int t_trunc, AdamsDegV2& deg1, AdamsDegV2& deg2)  //// TODO: DEBUG
{
    if (deg2.s >= 0) {
        gb_[deg2.s].clear();
        leads_[deg2.s].clear();
        indices_[deg2.s].clear();
    }

    ut::get(leads_, (size_t)s);
    ut::get(indices_, (size_t)s);
    ut::get(gb_, (size_t)s);
    if (s >= 1 && !(deg1.s == s - 1 && deg1.t == t_trunc)) {
        if (deg1.s >= 0) {
            gb_[deg1.s].clear();
            leads_[deg1.s].clear();
            indices_[deg1.s].clear();
        }

        /* Load (s - 1, t_trunc) */
        size_t sm1 = (size_t)(s - 1);
        DataMResConst1d data_sm1 = db.load_data_s(table, s - 1, t_trunc);
        leads_[sm1].clear();
        indices_[sm1].clear();
        ut::get(leads_, sm1).clear();
        ut::get(indices_, sm1).clear();
        for (int j = 0; j < (int)data_sm1.size(); ++j) {
            leads_[sm1].push_back(data_sm1[j].x1.GetLead());
            indices_[sm1][data_sm1[j].x1.GetLead().v_raw()].push_back(j);
        }
        gb_[sm1] = std::move(data_sm1);
    }

    /* Load (s, t_trunc) */
    DataMResConst1d data_s = db.load_data_s(table, s, t_trunc);
    leads_[s].clear();
    indices_[s].clear();
    for (int j = 0; j < (int)data_s.size(); ++j) {
        leads_[s].push_back(data_s[j].x1.GetLead());
        indices_[s][data_s[j].x1.GetLead().v_raw()].push_back(j);
    }
    gb_[s] = std::move(data_s);

    deg1 = AdamsDegV2(s, t_trunc);
    deg2 = AdamsDegV2(s - 1, t_trunc);
}

/*  F_s ----f----> F_{s-g}
 *   |                |
 *   d                d
 *   |                |
 *   V                V
 *  F_{s-1} --f--> F_{s-1-g}
 */
void compute_products(int t_trunc, int stem_trunc, const std::string& ring)  ////TODO: abstract and avoid repeating code
{
    std::string db_res = ring + "_Adams_res.db";
    std::string table_res = ring + "_Adams_res";
    std::string db_out = ring + "_Adams_res_prod.db";
    std::string table_out = ring + "_Adams_res";

    myio::AssertFileExists(db_res);
    DbResVersionConvert(db_res.c_str()); /*Version convertion */
    DbAdamsResLoader dbRes(db_res);
    auto gb = AdamsResConst::load(dbRes, table_res, t_trunc);
    std::vector<std::pair<int, AdamsDegV2>> id_deg; /* pairs (id, deg) where `id` is the first id in deg */
    int2d vid_num;                                  /* vid_num[s][stem] is the number of generators in (<=stem, s) */
    std::map<AdamsDegV2, Mod1d> diffs;              /* diffs[deg] is the list of differentials of v in deg */
    dbRes.load_generators(table_res, id_deg, vid_num, diffs, t_trunc, stem_trunc);
    int1d gs_hopf, t_gs_hopf;
    if (table_res == "S0_Adams_res" || table_res == "tmf_Adams_res") {
        gs_hopf = dbRes.get_column_int(fmt::format("{}_generators", table_res), "id", "WHERE s=1 ORDER BY id");
        t_gs_hopf = dbRes.get_column_int(fmt::format("{}_generators", table_res), "t", "WHERE s=1 ORDER BY id");
    }

    DbAdamsResProd dbProd(db_out);
    if (!dbProd.ConvertVersion(dbRes)) {
        fmt::print("Version conversion failed.\n");
        throw MyException(0xeb8fef62, "Version conversion failed.");
    }
    dbProd.create_tables(table_out);
    myio::Statement stmt_gen(dbProd, fmt::format("INSERT INTO {}_generators (id, indecomposable, s, t) values (?1, ?2, ?3, ?4);", table_out));   /* (id, is_indecomposable, s, t) */
    myio::Statement stmt_set_ind(dbProd, fmt::format("UPDATE {}_generators SET indecomposable=1 WHERE id=?1 and indecomposable=0;", table_out)); /* id */
    myio::Statement stmt_prod(dbProd, fmt::format("INSERT INTO {}_products (id, id_ind, prod, prod_h) VALUES (?1, ?2, ?3, ?4);", table_out));    /* (id, id_ind, prod, prod_h) */
    myio::Statement stmt_t_max(dbProd, "INSERT INTO version (id, name, value) VALUES (817812698, \"t_max\", ?1) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
    myio::Statement stmt_time(dbProd, "INSERT INTO version (id, name, value) VALUES (1954841564, \"timestamp\", unixepoch()) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");

    dbRes.disconnect();

    const Mod one = MMod(MMilnor(), 0);
    const int1d one_h = {0};

    /* Remove computed range */
    int1d ids_old = dbProd.load_old_ids(table_out);
    ut::RemoveIf(id_deg, [&ids_old](const std::pair<int, AdamsDegV2>& p) { return ut::has(ids_old, p.first); });

    bench::Timer timer;
    timer.SuppressPrint();

    int t_old = -1;
    for (const auto& [id, deg] : id_deg) {
        const auto& diffs_d = diffs.at(deg);
        const size_t diffs_d_size = diffs_d.size();

        if (deg.t == 0) {
            for (size_t i = 0; i < diffs_d_size; ++i)
                stmt_gen.bind_and_step(id + (int)i, 0, deg.s, deg.t);
            diffs.erase(deg);
            continue;
        }

        /* f_{s-1}[g] is the map F_{s-1} -> F_{s-1-deg(g)} dual to the multiplication of g */
        std::map<int, Mod1d> f_sm1 = dbProd.load_products(table_out, deg.s - 1, gs_hopf);
        int1d gs = ut::get_keys(f_sm1); /* indecomposables id's */

        /*# compute fd */
        std::map<int, Mod1d> fd;
        size_t vid_num_sm1 = deg.s > 0 ? (size_t)vid_num[size_t(deg.s - 1)][deg.stem()] : 0;
        for (auto& [g, f_sm1_g] : f_sm1) {
            f_sm1_g.resize(vid_num_sm1);
            fd[g].resize(diffs_d_size);
        }
        ut::for_each_par128(diffs_d_size * gs.size(), [&gs, &fd, &diffs_d, &f_sm1, diffs_d_size](size_t i) {
            int g = gs[i / diffs_d_size];
            size_t j = i % diffs_d_size;
            fd.at(g)[j] = subs(diffs_d[j], f_sm1.at(g));
        });

        /*# compute f */
        std::map<int, Mod1d> f;
        int1d s1;
        for (auto& [g, _] : fd) {
            s1.push_back(deg.s - 1 - LocId(g).s);
            f[g].resize(diffs_d_size);
        }
        ut::for_each_par128(gs.size(), [&gs, &gb, &fd, &f, &s1](size_t i) { gb.DiffInvBatch(fd[gs[i]], f[gs[i]], s1[i]); });

        /*# compute fh */
        std::map<int, int2d> fh;
        for (auto& [g, f_g] : f)
            for (size_t i = 0; i < diffs_d_size; ++i)
                fh[g].push_back(HomToK(f_g[i]));
        if (deg.s > 1) {
            for (size_t i_g = 0; i_g < gs_hopf.size(); ++i_g) {
                int g = gs_hopf[i_g];
                int t = t_gs_hopf[i_g];
                for (size_t i = 0; i < diffs_d_size; ++i)
                    fh[g].push_back(HomToMSq(diffs_d[i], t));
            }
        }

        dbProd.begin_transaction();
        if (t_old != deg.t) {
            stmt_t_max.bind_and_step(t_old);
            stmt_time.step_and_reset();
            t_old = deg.t;
        }
        /* save generators to database */
        for (size_t i = 0; i < diffs_d_size; ++i) {
            stmt_gen.bind_and_step(id + (int)i, 0, deg.s, deg.t);
        }

        /*# save products to database */
        for (auto& [g, f_g] : f) {
            for (size_t i = 0; i < diffs_d_size; ++i) {
                if (f_g[i]) {
                    stmt_prod.bind_and_step(id + (int)i, g, f_g[i].data, myio::Serialize(fh.at(g)[i]));
                }
            }
        }
        if (deg.s > 1) {
            for (size_t i_g = 0; i_g < gs_hopf.size(); ++i_g) {
                int g = gs_hopf[i_g];
                int t = t_gs_hopf[i_g];
                for (size_t i = 0; i < diffs_d_size; ++i) {
                    if (!fh.at(g)[i].empty()) {
                        stmt_prod.bind_and_step(id + (int)i, g, myio::SQL_NULL(), myio::Serialize(fh.at(g)[i]));
                    }
                }
            }
        }

        /*# find indecomposables */
        int2d fx;
        for (const auto& [_, fh_g] : fh) {
            size_t offset = fx.size();
            for (size_t i = 0; i < fh_g.size(); ++i) {
                for (int k : fh_g[i]) {
                    if (fx.size() <= offset + (size_t)k)
                        fx.resize(offset + (size_t)k + 1);
                    fx[offset + (size_t)k].push_back((int)i);
                }
            }
        }
        int1d lead_image = lina::GetLeads(lina::GetSpace(fx));
        int1d indices = lina::add(ut::int_range((int)diffs_d_size), lead_image);

        /*# mark indecomposables in database */
        for (int i : indices) {
            stmt_set_ind.bind_and_step(id + i);
        }

        /*# indecomposable comultiply with itself */
        for (int i : indices) {
            stmt_prod.bind_and_step(id + (int)i, id + (int)i, one.data, myio::Serialize(one_h));
        }

        double time = timer.Elapsed();
        timer.Reset();
        fmt::print("t={} s={} time={}\n{}", deg.t, deg.s, time, myio::COUT_FLUSH());
        dbProd.save_time(table_out, deg.s, deg.t, time);

        dbProd.end_transaction();
        diffs.erase(deg);
    }
    stmt_t_max.bind_and_step(t_old);
}

/*  F_s ----f----> F_{s-g}
 *   |                |
 *   d                d
 *   |                |
 *   V                V
 *  F_{s-1} --f--> F_{s-1-g}
 */
void compute_mod_products(int t_trunc, int stem_trunc, const std::string& mod, const std::string& ring)
{
    std::string db_mod = mod + "_Adams_res.db";
    std::string table_mod = mod + "_Adams_res";
    std::string db_ring = ring + "_Adams_res.db";
    std::string table_ring = ring + "_Adams_res";
    std::string db_out = mod + "_Adams_res_prod.db";
    std::string table_out = fmt::format("{}_over_{}_Adams_res", mod, ring);
    myio::AssertFileExists(db_mod);
    myio::AssertFileExists(db_ring);

    DbResVersionConvert(db_ring.c_str()); /*Version convertion */
    DbAdamsResLoader dbResRing(db_ring);
    auto gbRing = AdamsResConst::load(dbResRing, table_ring, t_trunc);

    std::vector<std::pair<int, AdamsDegV2>> id_deg; /* pairs (id, deg) where `id` is the first id in deg */
    int2d vid_num;                                  /* vid_num[s][stem] is the number of generators in (<=stem, s) */
    std::map<AdamsDegV2, Mod1d> diffs;              /* diffs[deg] is the list of differentials of v in deg */
    {
        DbResVersionConvert(db_mod.c_str()); /*Version convertion */
        DbAdamsResLoader dbRes(db_mod);
        dbRes.load_generators(table_mod, id_deg, vid_num, diffs, t_trunc, stem_trunc);
    }

    DbAdamsResProd dbProd(db_out);
    if (!dbProd.ConvertVersion(dbResRing)) {
        fmt::print("Version conversion failed.\n");
        throw MyException(0xeb8fef62, "Version conversion failed.");
    }
    if (dbProd.has_table(table_mod + "_generators") && !dbProd.has_table(table_out + "_generators")) {
        constexpr std::array<std::string_view, 3> postfixes = {"generators", "products", "products_time"};
        for (auto& postfix : postfixes)
            dbProd.rename_table(fmt::format("{}_Adams_res_{}", mod, postfix), fmt::format("{}_over_{}_Adams_res_{}", mod, ring, postfix));
        fmt::print("Renamming success!\n");
    }
    dbProd.create_tables(table_out);
    myio::Statement stmt_gen(dbProd, fmt::format("INSERT INTO {}_generators (id, indecomposable, s, t) values (?1, ?2, ?3, ?4);", table_out));   /* (id, is_indecomposable, s, t) */
    myio::Statement stmt_set_ind(dbProd, fmt::format("UPDATE {}_generators SET indecomposable=1 WHERE id=?1 and indecomposable=0;", table_out)); /* id */
    myio::Statement stmt_prod(dbProd, fmt::format("INSERT INTO {}_products (id, id_ind, prod, prod_h) VALUES (?1, ?2, ?3, ?4);", table_out));    /* (id, id_ind, prod, prod_h) */
    myio::Statement stmt_t_max(dbProd, "INSERT INTO version (id, name, value) VALUES (817812698, \"t_max\", ?1) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
    myio::Statement stmt_time(dbProd, "INSERT INTO version (id, name, value) VALUES (1954841564, \"timestamp\", unixepoch()) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");

    dbResRing.disconnect();

    const Mod one = MMod(MMilnor(), 0);
    const int1d one_h = {0};

    /* Remove computed range */
    int1d ids_old = dbProd.load_old_ids(table_out);
    ut::RemoveIf(id_deg, [&ids_old](const std::pair<int, AdamsDegV2>& p) { return ut::has(ids_old, p.first); });

    bench::Timer timer;
    timer.SuppressPrint();

    int t_old = -1;
    for (const auto& [id, deg] : id_deg) {
        const auto& diffs_d = diffs.at(deg);
        const size_t diffs_d_size = diffs_d.size();

        /* f_{s-1}[g] is the map F_{s-1} -> F_{s-1-deg(g)} dual to the multiplication of g */
        const int1d empty;
        std::map<int, Mod1d> f_sm1 = dbProd.load_products(table_out, deg.s - 1, empty);
        int1d gs = ut::get_keys(f_sm1); /* indecomposables id's */

        /*# compute fd */
        std::map<int, Mod1d> fd;
        size_t vid_num_sm1 = deg.s > 0 ? (size_t)vid_num[size_t(deg.s - 1)][deg.stem()] : 0;
        for (auto& [g, f_sm1_g] : f_sm1) {
            f_sm1_g.resize(vid_num_sm1);
            fd[g].resize(diffs_d_size);
        }
        ut::for_each_par128(diffs_d_size * gs.size(), [&gs, &fd, &diffs_d, &f_sm1, diffs_d_size](size_t i) {
            int g = gs[i / diffs_d_size];
            size_t j = i % diffs_d_size;
            fd.at(g)[j] = subs(diffs_d[j], f_sm1.at(g));
        });

        /*# compute f */
        std::map<int, Mod1d> f;
        int1d s1;
        for (auto& [g, _] : fd) {
            s1.push_back(deg.s - 1 - LocId(g).s);
            f[g].resize(diffs_d_size);
        }
        ut::for_each_par128(gs.size(), [&gs, &gbRing, &fd, &f, &s1](size_t i) { gbRing.DiffInvBatch(fd[gs[i]], f[gs[i]], s1[i]); });

        /*# compute fh */
        std::map<int, int2d> fh;
        for (auto& [g, f_g] : f)
            for (size_t i = 0; i < diffs_d_size; ++i)
                fh[g].push_back(HomToK(f_g[i]));

        dbProd.begin_transaction();
        if (t_old != deg.t) {
            stmt_t_max.bind_and_step(t_old);
            stmt_time.step_and_reset();
            t_old = deg.t;
        }
        /* save generators to database */
        for (size_t i = 0; i < diffs_d_size; ++i) {
            stmt_gen.bind_and_step(id + (int)i, 0, deg.s, deg.t);
        }

        /*# save products to database */
        for (auto& [g, f_g] : f) {
            for (size_t i = 0; i < diffs_d_size; ++i) {
                if (f_g[i]) {
                    stmt_prod.bind_and_step(id + (int)i, g, f_g[i].data, myio::Serialize(fh.at(g)[i]));
                }
            }
        }

        /*# find indecomposables */
        int2d fx;
        for (const auto& [_, fh_g] : fh) {
            size_t offset = fx.size();
            for (size_t i = 0; i < fh_g.size(); ++i) {
                for (int k : fh_g[i]) {
                    if (fx.size() <= offset + (size_t)k)
                        fx.resize(offset + (size_t)k + 1);
                    fx[offset + (size_t)k].push_back((int)i);
                }
            }
        }
        int1d lead_image = lina::GetLeads(lina::GetSpace(fx));
        int1d indices = lina::add(ut::int_range((int)diffs_d_size), lead_image);

        /*# mark indecomposables in database */
        for (int i : indices) {
            stmt_set_ind.bind_and_step(id + i);
        }

        /*# indecomposable comultiply with itself */
        for (int i : indices) {
            stmt_prod.bind_and_step(id + (int)i, id + (int)i, one.data, myio::Serialize(one_h));
        }

        double time = timer.Elapsed();
        timer.Reset();
        fmt::print("t={} s={} time={}\n{}", deg.t, deg.s, time, myio::COUT_FLUSH());
        dbProd.save_time(table_out, deg.s, deg.t, time);

        dbProd.end_transaction();
        diffs.erase(deg);
    }
    stmt_t_max.bind_and_step(t_old);
    stmt_time.step_and_reset();
}

void SetDbCohMap(const std::string& db_map, const std::string& table_map, const std::string& from, const std::string& to, const Mod1d& images, int sus, int fil)
{
    DbAdamsResMap dbMap(db_map);
    dbMap.create_tables(table_map);
    dbMap.begin_transaction();
    {
        myio::Statement stmt_map(dbMap, fmt::format("INSERT INTO {} (id, map, map_h) VALUES (?1, ?2, ?3);", table_map)); /* (id, map, map_h) */
        for (size_t i = 0; i < images.size(); ++i)
            stmt_map.bind_and_step((int)i + (fil << LOC_V_BITS), images[i].data, myio::Serialize(HomToK(images[i])));
    }
    {
        myio::Statement stmt(dbMap, "INSERT INTO version (id, name, value) VALUES (?1, ?2, ?3) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
        stmt.bind_and_step(651971502, std::string("filtration"), fil);  /* db_key: filtration */
        stmt.bind_and_step(1585932889, std::string("suspension"), sus); /* db_key: suspension */
        stmt.bind_and_step(446174262, std::string("from"), from);       /* db_key: from */
        stmt.bind_and_step(1713085477, std::string("to"), to);          /* db_key: to */
    }
    dbMap.end_transaction();
}

/* cw1 --> cw2
 * Ext^fil(cw2) --> H^*(cw1)
 *
 *  F_s -----f-----> F_{s-fil}
 *   |                |
 *   d                d
 *   |                |
 *   V                V
 *  F_{s-1} --f--> F_{s-1-fil}
 */
void compute_map_res(const std::string& cw1, const std::string& cw2, int t_trunc, int stem_trunc)
{
    std::string db_map = fmt::format("map_Adams_res_{}_to_{}.db", cw1, cw2);
    std::string table_map = fmt::format("map_Adams_res_{}_to_{}", cw1, cw2);
    myio::AssertFileExists(db_map);
    DbAdamsResMap dbMap(db_map);
    if (dbMap.GetVersion() < 3) {
        fmt::print("Map version should be >= 3");
        throw MyException(0x7102b177U, "Map version should be >= 3");
    }
    auto from = dbMap.get_str("select value from version where id=446174262");
    auto to = dbMap.get_str("select value from version where id=1713085477");

    std::string db_cw1 = from + "_Adams_res.db";
    std::string table_cw1 = from + "_Adams_res";
    std::string db_cw2 = to + "_Adams_res.db";
    std::string table_cw2 = to + "_Adams_res";
    myio::AssertFileExists(db_cw1);
    myio::AssertFileExists(db_cw2);

    auto fil = dbMap.get_int("select value from version where id=651971502");
    int sus = dbMap.get_int("select value from version where id=1585932889");
    dbMap.create_tables(table_map);
    myio::Statement stmt_map(dbMap, fmt::format("INSERT INTO {} (id, map, map_h) VALUES (?1, ?2, ?3);", table_map)); /* (id, map, map_h) */
    myio::Statement stmt_t_max(dbMap, "INSERT INTO version (id, name, value) VALUES (817812698, \"t_max\", ?1) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");
    myio::Statement stmt_time(dbMap, "INSERT INTO version (id, name, value) VALUES (1954841564, \"timestamp\", unixepoch()) ON CONFLICT(id) DO UPDATE SET value=excluded.value;");

    DbResVersionConvert(db_cw1.c_str()); /*Version convertion */
    DbAdamsResLoader dbResCw1(db_cw1);
    auto gbCw1 = AdamsResConst::load_gb_by_basis_degrees(dbResCw1, table_cw1, t_trunc);

    std::vector<std::pair<int, AdamsDegV2>> id_deg; /* pairs (id, deg) where `id` is the first id in deg */
    int2d vid_num;                                  /* vid_num[s][stem] is the number of generators in (<=stem, s) */
    std::map<AdamsDegV2, Mod1d> diffs;              /* diffs[deg] is the list of differentials of v in deg */
    {
        DbResVersionConvert(db_cw2.c_str()); /*Version convertion */
        DbAdamsResLoader dbResCw2(db_cw2);
        dbResCw2.load_generators(table_cw2, id_deg, vid_num, diffs, t_trunc - sus, stem_trunc - sus);
    }

    const Mod one = MMod(MMilnor(), 0);
    const int1d one_h = {0};

    /* Remove computed range */
    int1d ids_old = dbMap.load_old_ids(table_map);
    ut::RemoveIf(id_deg, [&ids_old](const std::pair<int, AdamsDegV2>& p) { return ut::has(ids_old, p.first); });

    bench::Timer timer;
    timer.SuppressPrint();

    int t_old = -1;
    AdamsDegV2 deg1_old(-1, -1);
    AdamsDegV2 deg2_old(-1, -1);
    for (const auto& [id, deg] : id_deg) {
        if (deg.s < fil)
            continue;
        const auto& diffs_d = diffs.at(deg);
        const size_t diffs_d_size = diffs_d.size();
        gbCw1.rotate(dbResCw1, table_cw1, deg.s - fil, deg.t + sus, deg1_old, deg2_old);

        /* f_{s-1} is the map F_{s-1} -> F_{s-1-fil} dual */
        Mod1d f_sm1 = dbMap.load_map(table_map, deg.s - 1);
        size_t vid_num_sm1 = deg.s > 0 ? (size_t)vid_num[size_t(deg.s - 1)][deg.stem()] : 0;
        f_sm1.resize(vid_num_sm1);

        /*# compute fd */
        Mod1d fd;
        fd.resize(diffs_d_size);
        ut::for_each_par128(diffs_d_size, [&fd, &diffs_d, &f_sm1, diffs_d_size](size_t i) { fd[i] = subs(diffs_d[i], f_sm1); });

        /*# compute f */
        Mod1d f;
        f.resize(diffs_d_size);
        gbCw1.DiffInvBatch(fd, f, size_t(deg.s - 1 - fil));

        /*# compute fh */
        int2d fh;
        for (size_t i = 0; i < diffs_d_size; ++i)
            fh.push_back(HomToK(f[i]));

        dbMap.begin_transaction();
        if (t_old != deg.t) {
            stmt_t_max.bind_and_step(t_old);
            stmt_time.step_and_reset();
            t_old = deg.t;
        }
        /*# save products to database */
        for (size_t i = 0; i < diffs_d_size; ++i)
            if (f[i])
                stmt_map.bind_and_step(id + (int)i, f[i].data, myio::Serialize(fh[i]));

        double time = timer.Elapsed();
        timer.Reset();
        fmt::print("t={} s={} time={}\n{}", deg.t, deg.s, time, myio::COUT_FLUSH());
        dbMap.save_time(table_map, deg.s, deg.t, time);

        dbMap.end_transaction();
        diffs.erase(deg);
    }
    stmt_t_max.bind_and_step(t_old);
    stmt_time.step_and_reset();
}

void compute_products_with_hi(const std::string& db_res_S0, const std::string& db_res_mod, const std::string& table_res_mod, const std::string& db_out)
{
    int1d ids_h;
    {
        DbResVersionConvert(db_res_S0.c_str()); /*Version convertion */
        DbAdamsResLoader dbResS0(db_res_S0);
        ids_h = dbResS0.get_column_int("S0_Adams_res_generators", "id", "WHERE s=1 ORDER BY id");
    }

    std::vector<std::pair<int, AdamsDegV2>> id_deg;
    std::map<AdamsDegV2, Mod1d> diffs;
    {
        DbAdamsResLoader dbResMod(db_res_mod);
        int2d vid_num;
        dbResMod.load_generators(table_res_mod, id_deg, vid_num, diffs, 500, 500);
    }

    DbAdamsResProd dbProd(db_out);
    dbProd.execute_cmd("CREATE TABLE IF NOT EXISTS " + table_res_mod + "_products_hi (id INTEGER, id_ind INTEGER, prod_h_glo TEXT, PRIMARY KEY (id, id_ind));");

    myio::Statement stmt_prod(dbProd, "INSERT INTO " + table_res_mod + "_products_hi (id, id_ind, prod_h_glo) VALUES (?1, ?2, ?3);");

    for (const auto& [id, deg] : id_deg) {
        auto& diffs_d = diffs.at(deg);

        dbProd.begin_transaction();

        /* Save the hj products */
        for (size_t i = 0; i < diffs_d.size(); ++i) {
            for (size_t j = 0; j < ids_h.size(); ++j) {
                int1d prod_hi = HomToMSq(diffs_d[i], 1 << j);
                if (!prod_hi.empty()) {
                    int1d prod_hi_glo;
                    for (int k : prod_hi)
                        prod_hi_glo.push_back(LocId(deg.s - 1, k).id());

                    stmt_prod.bind_and_step(id + (int)i, ids_h[j], myio::Serialize(prod_hi_glo));
                }
            }
        }

        dbProd.end_transaction();
        diffs.erase(deg);
    }
}

/* Compute the product with hi */
int main_prod_hi(int argc, char** argv, int& index, const char* desc)
{
    std::string cw = "S0";
    std::string db_S0 = "S0_Adams_res.db";
    std::string db_mod = "<cw>_Adams_res.db";
    std::string db_out = "<cw>_Adams_res_prod.db";

    myio::CmdArg1d args = {{"cw", &cw}};
    myio::CmdArg1d op_args = {{"db_S0", &db_S0}, {"db_mod", &db_mod}, {"db_out", &db_out}};
    if (int error = myio::LoadCmdArgs(argc, argv, index, PROGRAM, desc, VERSION, args, op_args))
        return error;

    if (db_mod == "<cw>_Adams_res.db")
        db_mod = cw + "_Adams_res.db";
    std::string table_mod = cw + "_Adams_res";
    if (db_out == "<cw>_Adams_res_prod.db")
        db_out = cw + "_Adams_res_prod.db";

    compute_products_with_hi(db_S0, db_mod, table_mod, db_out);

    return 0;
}

int main_prod(int argc, char** argv, int& index, const char* desc)
{
    std::string ring = "S0";
    int t_max = 0, stem_max = DEG_MAX;

    myio::CmdArg1d args = {{"ring", &ring}, {"t_max", &t_max}};
    myio::CmdArg1d op_args = {{"stem_max", &stem_max}};
    if (int error = myio::LoadCmdArgs(argc, argv, index, PROGRAM, desc, VERSION, args, op_args))
        return error;

    compute_products(t_max, stem_max, ring);
    return 0;
}

int main_prod_mod(int argc, char** argv, int& index, const char* desc)
{
    std::string mod;
    std::string ring;
    int t_max = 0, stem_max = DEG_MAX;

    myio::CmdArg1d args = {{"mod", &mod}, {"ring", &ring}, {"t_max", &t_max}};
    myio::CmdArg1d op_args = {{"stem_max", &stem_max}};
    if (int error = myio::LoadCmdArgs(argc, argv, index, PROGRAM, desc, VERSION, args, op_args))
        return error;

    compute_mod_products(t_max, stem_max, mod, ring);
    return 0;
}

int main_map_res(int argc, char** argv, int& index, const char* desc)
{
    std::string cw1, cw2;
    int t_max = 0, stem_max = DEG_MAX;

    myio::CmdArg1d args = {{"cw1", &cw1}, {"cw2", &cw2}, {"t_max", &t_max}};
    myio::CmdArg1d op_args = {{"stem_max", &stem_max}};
    if (int error = myio::LoadCmdArgs(argc, argv, index, PROGRAM, desc, VERSION, args, op_args))
        return error;

    compute_map_res(cw1, cw2, t_max, stem_max);
    return 0;
}

// Copyright 2010-2014 RethinkDB, all rights reserved
#ifndef RDB_PROTOCOL_REAL_TABLE_HPP_
#define RDB_PROTOCOL_REAL_TABLE_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "rdb_protocol/configured_limits.hpp"
#include "rdb_protocol/context.hpp"
#include "rdb_protocol/protocol.hpp"

namespace ql {
class datum_range_t;
namespace changefeed {
class client_t;
}
}

/* `real_table_t` is a concrete subclass of `base_table_t` that routes its queries across
the network via the clustering logic to a B-tree. The administration logic is responsible
for constructing and returning them from `reql_cluster_interface_t::table_find()`. */

/* `namespace_interface_access_t` is like a smart pointer to a `namespace_interface_t`.
This is the format in which `real_table_t` expects to receive its
`namespace_interface_t *`. This allows the thing that is constructing the `real_table_t`
to control the lifetime of the `namespace_interface_t`, but also allows the
`real_table_t` to block it from being destroyed while in use. */
class namespace_interface_access_t {
public:
    class ref_tracker_t {
    public:
        virtual void add_ref() = 0;
        virtual void release() = 0;
    protected:
        virtual ~ref_tracker_t() { }
    };

    namespace_interface_access_t();
    namespace_interface_access_t(namespace_interface_t *, ref_tracker_t *, threadnum_t);
    namespace_interface_access_t(const namespace_interface_access_t &access);
    namespace_interface_access_t &operator=(const namespace_interface_access_t &access);
    ~namespace_interface_access_t();

    namespace_interface_t *get();

private:
    namespace_interface_t *nif;
    ref_tracker_t *ref_tracker;
    threadnum_t thread;
};

class real_table_t final : public base_table_t {
public:
    /* This doesn't automatically wait for readiness. */
    real_table_t(
            namespace_id_t _uuid,
            namespace_interface_access_t _namespace_access,
            const std::string &_pkey,
            ql::changefeed::client_t *_changefeed_client) :
        uuid(_uuid), namespace_access(_namespace_access), pkey(_pkey),
        changefeed_client(_changefeed_client) { }

    ql::datum_t get_id() const;
    const std::string &get_pkey() const;

    ql::datum_t read_row(ql::env_t *env, ql::datum_t pval, read_mode_t read_mode);
    counted_t<ql::datum_stream_t> read_all(
        ql::env_t *env,
        const std::string &sindex,
        ql::backtrace_id_t bt,
        const std::string &table_name, // The table's own name, for display purposes.
        const ql::datumspec_t &datumspec,
        sorting_t sorting,
        read_mode_t read_mode);
    counted_t<ql::datum_stream_t> read_changes(
        ql::env_t *env,
        const ql::changefeed::streamspec_t &ss,
        ql::backtrace_id_t bt);
    counted_t<ql::datum_stream_t> read_intersecting(
        ql::env_t *env,
        const std::string &sindex,
        ql::backtrace_id_t bt,
        const std::string &table_name,
        read_mode_t read_mode,
        const ql::datum_t &query_geometry);
    ql::datum_t read_nearest(
        ql::env_t *env,
        const std::string &sindex,
        const std::string &table_name,
        read_mode_t read_mode,
        lon_lat_point_t center,
        double max_dist,
        uint64_t max_results,
        const ellipsoid_spec_t &geo_system,
        dist_unit_t dist_unit,
        const ql::configured_limits_t &limits);

    ql::datum_t write_batched_replace(
        ql::env_t *env,
        const std::vector<ql::datum_t> &keys,
        const counted_t<const ql::func_t> &func,
        return_changes_t _return_changes, durability_requirement_t durability);
    ql::datum_t write_batched_insert(
        ql::env_t *env,
        std::vector<ql::datum_t> &&inserts,
        std::vector<bool> &&pkey_is_autogenerated,
        conflict_behavior_t conflict_behavior,
        boost::optional<const ql::func_t&> conflict_func,
        return_changes_t return_changes,
        durability_requirement_t durability);
    bool write_sync_depending_on_durability(ql::env_t *env,
        durability_requirement_t durability);

    /* These are not part of the `base_table_t` interface. They wrap the `read()`,
    and `write()` methods of the underlying `namespace_interface_t` to add profiling
    information. Specifically, they:
      * Set the explain field in the read_t/write_t object so that the shards know
        whether or not to do profiling
      * Construct a splitter_t
      * Call the corresponding method on the `namespace_if`
      * splitter_t::give_splits with the event logs from the shards
    These are public because some of the stuff in `datum_stream.hpp` needs to be
    able to access them. */
    void read_with_profile(ql::env_t *env, const read_t &, read_response_t *response);
    void write_with_profile(ql::env_t *env, write_t *, write_response_t *response);

private:
    namespace_id_t uuid;
    namespace_interface_access_t namespace_access;
    std::string pkey;
    ql::changefeed::client_t *changefeed_client;
};

#endif /* RDB_PROTOCOL_REAL_TABLE_HPP_ */


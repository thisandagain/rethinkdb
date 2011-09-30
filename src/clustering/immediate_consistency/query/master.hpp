#ifndef __CLUSTERING_IMMEDIATE_CONSISTENCY_QUERY_MASTER_HPP__
#define __CLUSTERING_IMMEDIATE_CONSISTENCY_QUERY_MASTER_HPP__

#include "clustering/immediate_consistency/branch/broadcaster.hpp"
#include "clustering/immediate_consistency/query/metadata.hpp"

template<class protocol_t>
class master_t {

public:
    master_t(
            mailbox_cluster_t *c,
            boost::shared_ptr<metadata_readwrite_view_t<namespace_master_metadata_t<protocol_t> > > namespace_metadata,
            typename protocol_t::region_t region,
            broadcaster_t<protocol_t> *b)
            THROWS_ONLY(interrupted_exc_t) :
        cluster(c),
        broadcaster(b),
        read_mailbox(cluster, boost::bind(&master_t<protocol_t>::on_read,
            this, _1, _2, _3, auto_drainer_t::lock_t(&drainer))),
        write_mailbox(cluster, boost::bind(&master_t<protocol_t>::on_write,
            this, _1, _2, _3, auto_drainer_t::lock_t(&drainer))),
        advertisement(cluster,
            metadata_new_member(generate_uuid(), metadata_field(&namespace_master_metadata_t<protocol_t>::masters, namespace_metadata)),
            master_metadata_t<protocol_t>(region, read_mailbox.get_address(), write_mailbox.get_address())
            )
        { }

private:
    void on_read(typename protocol_t::read_t read, order_token_t otok,
            typename async_mailbox_t<void(boost::variant<typename protocol_t::read_response_t, std::string>)>::address_t response_address,
            auto_drainer_t::lock_t keepalive)
            THROWS_NOTHING
    {
        keepalive.assert_is_holding(&drainer);
        try {
            typename protocol_t::read_response_t response = broadcaster->read(read, otok);
            send(cluster, response_address, boost::variant<typename protocol_t::read_response_t, std::string>(response));
        } catch (typename broadcaster_t<protocol_t>::mirror_lost_exc_t e) {
            send(cluster, response_address, boost::variant<typename protocol_t::read_response_t, std::string>(std::string(e.what())));
        } catch (typename broadcaster_t<protocol_t>::insufficient_mirrors_exc_t e) {
            send(cluster, response_address, boost::variant<typename protocol_t::read_response_t, std::string>(std::string(e.what())));
        }
    }

    void on_write(typename protocol_t::write_t write, order_token_t otok,
            typename async_mailbox_t<void(boost::variant<typename protocol_t::write_response_t, std::string>)>::address_t response_address,
            auto_drainer_t::lock_t keepalive)
            THROWS_NOTHING
    {
        keepalive.assert_is_holding(&drainer);
        try {
            typename protocol_t::write_response_t response = broadcaster->write(write, otok);
            send(cluster, response_address, boost::variant<typename protocol_t::write_response_t, std::string>(response));
        } catch (typename broadcaster_t<protocol_t>::mirror_lost_exc_t e) {
            send(cluster, response_address, boost::variant<typename protocol_t::write_response_t, std::string>(std::string(e.what())));
        } catch (typename broadcaster_t<protocol_t>::insufficient_mirrors_exc_t e) {
            send(cluster, response_address, boost::variant<typename protocol_t::write_response_t, std::string>(std::string(e.what())));
        }
    }

    mailbox_cluster_t *cluster;

    broadcaster_t<protocol_t> *broadcaster;

    auto_drainer_t drainer;

    typename master_metadata_t<protocol_t>::read_mailbox_t read_mailbox;
    typename master_metadata_t<protocol_t>::write_mailbox_t write_mailbox;
    resource_advertisement_t<master_metadata_t<protocol_t> > advertisement;
};

#endif /* __CLUSTERING_IMMEDIATE_CONSISTENCY_QUERY_MASTER_HPP__ */

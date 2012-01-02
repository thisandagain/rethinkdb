#include "unittest/gtest.hpp"

#include "rpc/connectivity/cluster.hpp"
#include "rpc/directory/manager.hpp"
#include "unittest_utils.hpp"

namespace unittest {

namespace {

/* `let_stuff_happen()` delays for some time to let events occur */
void let_stuff_happen() {
    nap(1000);
}

}   /* anonymous namespace */

/* `OneNode` starts a single directory node, then shuts it down again. */

void run_one_node_test() {
    int port = 10000 + rand() % 20000;
    connectivity_cluster_t c(port);
    directory_readwrite_manager_t<int> directory_manager(&c, 5);
    let_stuff_happen();
}
TEST(RPCDirectoryTest, OneNode) {
    run_in_thread_pool(&run_one_node_test, 3);
}

/* `ThreeNodes` starts three directory nodes, lets them contact each other, and
then shuts them down again. */

void run_three_nodes_test() {
    int port = 10000 + rand() % 20000;
    connectivity_cluster_t c1(port), c2(port+1), c3(port+2);
    directory_readwrite_manager_t<int> m1(&c1, 101), m2(&c2, 202), m3(&c3, 303);
    c2.join(c1.get_peer_address(c1.get_me()));
    c3.join(c1.get_peer_address(c1.get_me()));
    let_stuff_happen();
}
TEST(RPCDirectoryTest, ThreeNodes) {
    run_in_thread_pool(&run_three_nodes_test, 3);
}

/* `Exchange` tests that directory nodes receive values from their peers. */

void run_exchange_test() {
    int port = 10000 + rand() % 20000;
    connectivity_cluster_t c1(port), c2(port+1), c3(port+2);
    directory_readwrite_manager_t<int> m1(&c1, 101), m2(&c2, 202), m3(&c3, 303);
    c2.join(c1.get_peer_address(c1.get_me()));
    c3.join(c1.get_peer_address(c1.get_me()));
    let_stuff_happen();
    EXPECT_EQ(boost::optional<int>(101), m1.get_root_view()->get_value(c1.get_me()));
    EXPECT_EQ(boost::optional<int>(202), m1.get_root_view()->get_value(c2.get_me()));
    EXPECT_EQ(boost::optional<int>(101), m3.get_root_view()->get_value(c1.get_me()));
}
TEST(RPCDirectoryTest, Exchange) {
    run_in_thread_pool(&run_exchange_test, 3);
}

/* `Update` tests that directory nodes see updates from their peers. */

void run_update_test() {
    int port = 10000 + rand() % 20000;
    connectivity_cluster_t c1(port), c2(port+1), c3(port+2);
    directory_readwrite_manager_t<int> m1(&c1, 101), m2(&c2, 202), m3(&c3, 303);
    c2.join(c1.get_peer_address(c1.get_me()));
    c3.join(c1.get_peer_address(c1.get_me()));
    let_stuff_happen();
    {
        directory_write_service_t::our_value_lock_acq_t lock(&m1);
        EXPECT_EQ(101, m1.get_root_view()->get_our_value(&lock));
        m1.get_root_view()->set_our_value(151, &lock);
    }
    let_stuff_happen();
    EXPECT_EQ(boost::optional<int>(151), m1.get_root_view()->get_value(c1.get_me()));
    EXPECT_EQ(boost::optional<int>(151), m2.get_root_view()->get_value(c1.get_me()));
    EXPECT_EQ(boost::optional<int>(151), m3.get_root_view()->get_value(c1.get_me()));
}
TEST(RPCDirectoryTest, Update) {
    run_in_thread_pool(&run_update_test, 3);
}

/* `Notify` tests that directory peer value watchers are notified when a peer's
value changes. */

void run_notify_test() {
    int port = 10000 + rand() % 20000;
    connectivity_cluster_t c(port);
    directory_readwrite_manager_t<int> m(&c, 8765);
    {
        cond_t got_notified;
        directory_read_service_t::peer_value_subscription_t subs(boost::bind(&cond_t::pulse, &got_notified));
        {
            directory_read_service_t::peer_value_freeze_t freeze(&m, c.get_me());
            subs.reset(&m, c.get_me(), &freeze);
        }
        {
            directory_write_service_t::our_value_lock_acq_t lock(&m);
            m.get_root_view()->set_our_value(4321, &lock);
        }
        let_stuff_happen();
        EXPECT_TRUE(got_notified.is_pulsed());
    }
}
TEST(RPCDirectoryTest, Notify) {
    run_in_thread_pool(&run_notify_test, 3);
}

}   /* namespace unittest */
#ifndef __BTREE_GET_DISTRIBUTION_HPP__
#define __BTREE_GET_DISTRIBUTION_HPP__

#include "btree/keys.hpp"
#include "btree/slice.hpp"
#include "buffer_cache/types.hpp"

void get_btree_key_distribution(btree_slice_t *slice, transaction_t *txn, got_superblock_t& superblock, int depth_limit, int *key_count_out, std::vector<store_key_t> *keys_out);

#endif
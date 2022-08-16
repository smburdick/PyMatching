#include "pymatching/fill_match/matcher/alternating_tree.h"

#include <gtest/gtest.h>

#include "pymatching/fill_match/flooder.h"

struct AltTreeTestData {
    std::vector<pm::GraphFillRegion> gfrs;
    std::vector<pm::DetectorNode> nodes;
    explicit AltTreeTestData(size_t num_elements) {
        gfrs.resize(num_elements);
        nodes.resize(num_elements);
    };
    AltTreeTestData(std::vector<pm::GraphFillRegion> gfrs, std::vector<pm::DetectorNode> nodes);
    pm::AltTreeEdge t(
        std::vector<pm::AltTreeEdge> children, size_t inner_region_id, size_t outer_region_id, bool root = false);
};

pm::AltTreeEdge AltTreeTestData::t(
    std::vector<pm::AltTreeEdge> children, size_t inner_region_id, size_t outer_region_id, bool root) {
    if (inner_region_id != SIZE_MAX && inner_region_id >= nodes.size()) {
        throw std::invalid_argument(
            "inner_region_id=" + std::to_string(inner_region_id) + " >= nodes.size()=" + std::to_string(nodes.size()));
    }
    if (outer_region_id != SIZE_MAX && outer_region_id >= nodes.size()) {
        throw std::invalid_argument(
            "outer_region_id=" + std::to_string(outer_region_id) + " >= nodes.size()=" + std::to_string(nodes.size()));
    }
    pm::AltTreeNode* node;
    pm::CompressedEdge parent_ce(nullptr, nullptr, 0);
    if (root) {
        node = new pm::AltTreeNode(&gfrs[outer_region_id]);
    } else {
        node = new pm::AltTreeNode(
            &gfrs[inner_region_id], &gfrs[outer_region_id], {&nodes[inner_region_id], &nodes[outer_region_id], 0});
    }
    auto edge = pm::AltTreeEdge(node, parent_ce);
    for (auto& child : children) {
        child.edge.loc_from = &nodes[outer_region_id];
        child.edge.loc_to = child.alt_tree_node->inner_to_outer_edge.loc_from;
        edge.alt_tree_node->add_child(child);
    }
    return edge;
}

AltTreeTestData::AltTreeTestData(std::vector<pm::GraphFillRegion> gfrs, std::vector<pm::DetectorNode> nodes)
    : gfrs(std::move(gfrs)), nodes(std::move(nodes)) {
}

pm::AltTreeEdge example_tree(AltTreeTestData& d) {
    return d.t({d.t({}, 7, 8), d.t({d.t({}, 3, 4), d.t({}, 5, 6)}, 1, 2)}, -1, 0, true);
}

pm::AltTreeEdge example_tree_four_children(AltTreeTestData& d) {
    return d.t({d.t({}, 1, 2), d.t({}, 3, 4), d.t({}, 5, 6), d.t({}, 7, 8)}, -1, 0, true);
}

void delete_alternating_tree(pm::AltTreeNode* root) {
    for (auto& child : root->children) {
        delete_alternating_tree(child.alt_tree_node);
    }
    delete root;
}

TEST(AlternatingTree, FindRoot) {
    AltTreeTestData d(10);
    auto n = example_tree(d);
    ASSERT_EQ(n.alt_tree_node->children[1].alt_tree_node->children[0].alt_tree_node->find_root(), n.alt_tree_node);
    ASSERT_EQ(n.alt_tree_node->find_root(), n.alt_tree_node);
    delete_alternating_tree(n.alt_tree_node);
}

TEST(AlternatingTree, UnstableEraseInt) {
    std::vector<int> v = {6, 2, 4, 7, 9, 10};
    bool b1 = pm::unstable_erase(v, [](int x) {
        return x == 7;
    });
    ASSERT_EQ(v, std::vector<int>({6, 2, 4, 10, 9}));
    ASSERT_TRUE(b1);
    bool b2 = pm::unstable_erase(v, [](int x) {
        return x == 6;
    });
    ASSERT_EQ(v, std::vector<int>({9, 2, 4, 10}));
    ASSERT_TRUE(b2);
    std::vector<int> w = {8};
    bool b3 = pm::unstable_erase(w, [](int x) {
        return x == 8;
    });
    ASSERT_EQ(w, std::vector<int>({}));
    ASSERT_TRUE(b3);
    bool b4 = pm::unstable_erase(w, [](int x) {
        return x == 0;
    });
    ASSERT_EQ(w, std::vector<int>({}));
    ASSERT_FALSE(b4);
}

TEST(AlternatingTree, UnstableEraseAltTreeEdge) {
    AltTreeTestData d(10);
    pm::AltTreeEdge x = example_tree_four_children(d);
    std::vector<pm::AltTreeEdge> xc = x.alt_tree_node->children;
    auto xc_copy = xc;
    ASSERT_EQ(xc, x.alt_tree_node->children);
    pm::unstable_erase(xc, [&xc_copy](pm::AltTreeEdge y) {
        return y.alt_tree_node == xc_copy[1].alt_tree_node;
    });
    ASSERT_EQ(xc, std::vector<pm::AltTreeEdge>({xc_copy[0], xc_copy[3], xc_copy[2]}));
    pm::unstable_erase(xc, [&xc_copy](pm::AltTreeEdge y) {
        return y.alt_tree_node == xc_copy[0].alt_tree_node;
    });
    ASSERT_EQ(xc, std::vector<pm::AltTreeEdge>({xc_copy[2], xc_copy[3]}));
    delete_alternating_tree(x.alt_tree_node);
}

TEST(AlternatingTree, AllNodesInTree) {
    AltTreeTestData d(10);
    pm::AltTreeEdge x = example_tree(d);
    ASSERT_EQ(
        x.alt_tree_node->all_nodes_in_tree(),
        std::vector<pm::AltTreeNode*>(
            {x.alt_tree_node,
             x.alt_tree_node->children[0].alt_tree_node,
             x.alt_tree_node->children[1].alt_tree_node,
             x.alt_tree_node->children[1].alt_tree_node->children[0].alt_tree_node,
             x.alt_tree_node->children[1].alt_tree_node->children[1].alt_tree_node}));
    delete_alternating_tree(x.alt_tree_node);
}

TEST(AlternatingTree, TreeEqual) {
    AltTreeTestData d(10);
    pm::AltTreeEdge x = example_tree(d);
    pm::AltTreeEdge x2 = example_tree(d);
    pm::AltTreeEdge y = example_tree_four_children(d);
    pm::AltTreeEdge y2 = example_tree_four_children(d);
    ASSERT_TRUE(x.alt_tree_node->tree_equal(*x2.alt_tree_node));
    ASSERT_TRUE(*x.alt_tree_node == *x2.alt_tree_node);
    ASSERT_TRUE(y.alt_tree_node->tree_equal(*y.alt_tree_node));
    ASSERT_TRUE(y.alt_tree_node->tree_equal(*y2.alt_tree_node));
    ASSERT_FALSE(x.alt_tree_node->tree_equal(*y.alt_tree_node));
    ASSERT_FALSE(*x.alt_tree_node == *y.alt_tree_node);
    y2.alt_tree_node->children.pop_back();
    ASSERT_FALSE(y.alt_tree_node->tree_equal(*y2.alt_tree_node));
    x2.alt_tree_node->children[0].alt_tree_node->outer_region = nullptr;
    ASSERT_FALSE(*x.alt_tree_node == *x2.alt_tree_node);
    delete_alternating_tree(x.alt_tree_node);
    delete_alternating_tree(x2.alt_tree_node);
    delete_alternating_tree(y.alt_tree_node);
    delete_alternating_tree(y2.alt_tree_node);
}

TEST(AlternatingTree, BecomeRoot) {
    AltTreeTestData d(30);
    auto y1 = d.t({d.t({}, 1, 2)}, -1, 0, true);
    auto y2 = d.t({d.t({}, 1, 0)}, -1, 2, true);
    auto v = y1.alt_tree_node->children[0].alt_tree_node;
    v->become_root();
    ASSERT_EQ(*v, *y2.alt_tree_node);
    delete_alternating_tree(v);
    delete_alternating_tree(y2.alt_tree_node);

    auto x1 =
        d.t({d.t({d.t({}, 10, 11), d.t({}, 8, 9), d.t({d.t({}, 6, 7)}, 12, 13)}, 4, 5), d.t({}, 2, 3)}, -1, 1, true);

    auto x2 =
        d.t({d.t({}, 6, 7), d.t({d.t({}, 10, 11), d.t({}, 8, 9), d.t({d.t({}, 2, 3)}, 4, 1)}, 12, 5)}, -1, 13, true);
    auto c = x1.alt_tree_node->children[0].alt_tree_node->children[2].alt_tree_node;
    c->become_root();
    ASSERT_EQ(*c, *x2.alt_tree_node);
    delete_alternating_tree(c);
    delete_alternating_tree(x2.alt_tree_node);
}

TEST(AlternatingTree, CommonAncestor) {
    AltTreeTestData d(20);
    auto t =
        d.t({d.t({d.t({d.t({}, 7, 8, false), d.t({}, 9, 10, false)}, 3, 4, false), d.t({}, 5, 6, false)}, 1, 2, false)},
            -1,
            0,
            true);
    auto anc = t.alt_tree_node->children[0]
                   .alt_tree_node->children[0]
                   .alt_tree_node->children[0]
                   .alt_tree_node->most_recent_common_ancestor(
                       *t.alt_tree_node->children[0].alt_tree_node->children[1].alt_tree_node);
    ASSERT_EQ(anc, t.alt_tree_node->children[0].alt_tree_node);
    ASSERT_TRUE(t.alt_tree_node->children[0].alt_tree_node->children[0].alt_tree_node->visited);
    ASSERT_TRUE(t.alt_tree_node->children[0].alt_tree_node->children[1].alt_tree_node->visited);
    ASSERT_FALSE(t.alt_tree_node->children[0].alt_tree_node->visited);
    ASSERT_FALSE(t.alt_tree_node->visited);
    ASSERT_TRUE(
        t.alt_tree_node->children[0].alt_tree_node->children[0].alt_tree_node->children[0].alt_tree_node->visited);
    delete_alternating_tree(t.alt_tree_node);

    auto t2 = d.t({d.t({}, 12, 13, false)}, -1, 11, true);
    auto t3 = d.t({d.t({}, 15, 16, false)}, -1, 14, true);
    auto anc2 = t3.alt_tree_node->children[0].alt_tree_node->most_recent_common_ancestor(
        *t2.alt_tree_node->children[0].alt_tree_node);
    ASSERT_EQ(anc2, nullptr);
    ASSERT_TRUE(t2.alt_tree_node->visited);
    ASSERT_TRUE(t2.alt_tree_node->children[0].alt_tree_node->visited);
    ASSERT_TRUE(t3.alt_tree_node->visited);
    ASSERT_TRUE(t3.alt_tree_node->children[0].alt_tree_node->visited);
    delete_alternating_tree(t2.alt_tree_node);
    delete_alternating_tree(t3.alt_tree_node);
}

TEST(AlternatingTree, PrunedUpwardPathStoppingBefore) {
    AltTreeTestData d(20);
    auto tree =
        d.t({d.t({d.t({}, 10, 11), d.t({}, 8, 9), d.t({d.t({}, 6, 7)}, 12, 13)}, 4, 5), d.t({}, 2, 3)}, -1, 1, true);
    auto c0 = tree.alt_tree_node->children[0].alt_tree_node;
    auto c02 = c0->children[2].alt_tree_node;
    std::vector<pm::AltTreeEdge> orphans_expected = {c02->children[0], c0->children[0], c0->children[1]};
    std::vector<pm::RegionEdge> pruned_region_edges_expected = {
        pm::RegionEdge(c02->outer_region, c02->inner_to_outer_edge.reversed()),
        pm::RegionEdge(c02->inner_region, c02->parent.edge),
        pm::RegionEdge(c0->outer_region, c0->inner_to_outer_edge.reversed()),
        pm::RegionEdge(c0->inner_region, c0->parent.edge)};
    auto res = c02->prune_upward_path_stopping_before(tree.alt_tree_node, false);
    ASSERT_EQ(res.orphan_edges, orphans_expected);
    ASSERT_EQ(res.pruned_path_region_edges, pruned_region_edges_expected);
    delete_alternating_tree(tree.alt_tree_node);
}

TEST(AlternatingTree, PrunedUpwardBackEdgePathStoppingBefore) {
    AltTreeTestData d(20);
    auto tree =
        d.t({d.t({d.t({}, 10, 11), d.t({}, 8, 9), d.t({d.t({}, 6, 7)}, 12, 13)}, 4, 5), d.t({}, 2, 3)}, -1, 1, true);
    auto c0 = tree.alt_tree_node->children[0].alt_tree_node;
    auto c02 = c0->children[2].alt_tree_node;
    std::vector<pm::AltTreeEdge> orphans_expected = {c02->children[0], c0->children[0], c0->children[1]};
    std::vector<pm::RegionEdge> pruned_region_edges_expected = {
        pm::RegionEdge(c02->inner_region, c02->inner_to_outer_edge),
        pm::RegionEdge(c0->outer_region, c02->parent.edge.reversed()),
        pm::RegionEdge(c0->inner_region, c0->inner_to_outer_edge),
        pm::RegionEdge(tree.alt_tree_node->outer_region, c0->parent.edge.reversed())};
    auto res = c02->prune_upward_path_stopping_before(tree.alt_tree_node, true);
    ASSERT_EQ(res.orphan_edges, orphans_expected);
    ASSERT_EQ(res.pruned_path_region_edges, pruned_region_edges_expected);
    delete_alternating_tree(tree.alt_tree_node);
}

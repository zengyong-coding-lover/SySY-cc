#ifndef __GRAPH__
#define __GRAPH__
#include <bitmap.hh>
#include <utils.hh>
template <typename Node_Info, typename Edge_Info>
class Graph_Edge;
template <typename Node_Info, typename Edge_Info>
class Graph_Node {
private:
    Node_Info info;
    std::vector<Graph_Edge<Node_Info, Edge_Info> *> succs;
    std::vector<Graph_Edge<Node_Info, Edge_Info> *> preds;
    size_t id;

    // some algorithms info
    std::vector<Graph_Node<Node_Info, Edge_Info> *> idom_childs; // 直接支配孩子
    Graph_Node<Node_Info, Edge_Info> *idom_parent; //直接支配父亲
    std::vector<Graph_Node<Node_Info, Edge_Info> *> dom_frontier; // 支配边界

public:
    struct {
        void *extra_info;
        bool is_visited;
    };
    Graph_Node() {
        this->info.set_node(this);
    }
    void add_succ(Graph_Edge<Node_Info, Edge_Info> *succ) {
        succs.push_back(succ);
    }
    void add_pred(Graph_Edge<Node_Info, Edge_Info> *pred) {
        preds.push_back(pred);
    }
    void set_id(size_t id) { this->id = id; }
    size_t get_id() { return this->id; }
    Node_Info &get_info() {
        return info;
    }
    std::vector<Graph_Edge<Node_Info, Edge_Info> *> &get_succs() {
        return succs;
    }
    std::vector<Graph_Edge<Node_Info, Edge_Info> *> &get_preds() {
        return preds;
    }
    ~Graph_Node() {
        for (auto succ : succs) {
            Graph_Node<Node_Info, Edge_Info> *to = succ->get_to();
            to->preds.erase(std::find(to->preds.begin(), to->preds.end(), succ));
            delete succ;
        }
        for (auto pred : preds) {
            Graph_Node<Node_Info, Edge_Info> *from = pred->get_from();
            from->succs.erase(std::find(from->succs.begin(), from->succs.end(), pred));
            delete pred;
        }
    }
    void clear_idom_childs() {
        std::vector<Graph_Node<Node_Info, Edge_Info> *>().swap(this->idom_childs);
    }
    void set_idom_parent(Graph_Node<Node_Info, Edge_Info> *parent) {
        this->idom_parent = parent;
    }
    void add_idom_child(Graph_Node<Node_Info, Edge_Info> *child) {
        this->idom_childs.push_back(child);
    }
    Graph_Node<Node_Info, Edge_Info> *get_idom_parent() {
        return this->idom_parent;
    }
    std::vector<Graph_Node<Node_Info, Edge_Info> *> &get_idom_childs() {
        return this->idom_childs;
    }
    void add_dom_frontier(Graph_Node<Node_Info, Edge_Info> *node) {
        this->dom_frontier.push_back(node);
    }
    void clear_dom_frontier() {
        std::vector<Graph_Node<Node_Info, Edge_Info> *>().swap(this->dom_frontier);
    }
    std::vector<Graph_Node<Node_Info, Edge_Info> *> &get_dom_frontier() {
        return dom_frontier;
    }
    void print_idom_dfs(size_t depth) {
        for (size_t i = 0; i < depth; i++)
            std::cout << "  ";
        depth++;
        std::cout << id << std::endl;
        for (auto child : idom_childs) {
            child->print_idom_dfs(depth);
        }
    }
    void print_dom_frontier() {
        std::cout << this->id << ":{";
        size_t i = 0;
        for (auto frontier : this->dom_frontier) {
            if (i != 0) std::cout << ", ";
            std::cout << frontier->id;
            i++;
        }
        std::cout << "}";
    }
};
template <typename Node_Info, typename Edge_Info>
class Graph_Edge {
private:
    Edge_Info info;
    Graph_Node<Node_Info, Edge_Info> *from;
    Graph_Node<Node_Info, Edge_Info> *to;

public:
    Graph_Edge(Graph_Node<Node_Info, Edge_Info> *from, Graph_Node<Node_Info, Edge_Info> *to)
        : from(from)
        , to(to) {
        from->add_succ(this);
        to->add_pred(this);
        this->info.set_edge(this);
    }
    Edge_Info &get_info() {
        return info;
    }
    Graph_Node<Node_Info, Edge_Info> *get_from() {
        return from;
    }
    Graph_Node<Node_Info, Edge_Info> *get_to() {
        return to;
    }
    void reset_from(Graph_Node<Node_Info, Edge_Info> *from) {
        auto it = find(this->from->get_succs().begin(), this->from->get_succs().end(), this);
        this->from->get_succs().erase(it);
        from->add_succ(this);
        this->from = from;
    }
    void reset_to(Graph_Node<Node_Info, Edge_Info> *to) {
        auto it = this->to->get_preds().find(this);
        this->to->get_preds().erase(it);
        to->add_pred(this);
        this->to = to;
    }
    ~Graph_Edge() {
    }
};
template <typename Node_Info, typename Edge_Info>
class Graph {
private:
    std::list<Graph_Node<Node_Info, Edge_Info> *> nodes;

public:
    Graph() { }
    void add_node_tail(Node_Info info) {
        nodes.push_back(new Graph_Node<Node_Info, Edge_Info>(info));
    }
    void add_node_tail(Graph_Node<Node_Info, Edge_Info> *node) {
        nodes.push_back(node);
    }
    void add_node_head(Graph_Node<Node_Info, Edge_Info> *node) {
        nodes.insert(nodes.begin(), node);
    }
    void add_edge(Edge_Info info, Graph_Node<Node_Info, Edge_Info> *from, Graph_Node<Node_Info, Edge_Info> *to) {
        Graph_Edge<Node_Info, Edge_Info> *edge = new Graph_Edge<Node_Info, Edge_Info>(info, from, to);
        from->add_succ(edge);
    }
    std::list<Graph_Node<Node_Info, Edge_Info> *> &get_nodes() {
        return nodes;
    }
    void print_graph() {
        for (auto node : nodes) {
            std::cout << "Node: ";
            node->get_info().print();
            std::cout << std::endl;
            for (auto succ : node->get_succs()) {
                std::cout << "Edge: ";
                succ->get_info().print();
                std::cout << std::endl;
            }
        }
    }
    void print_list() {
        for (auto node : nodes) {
            node->get_info().print();
            std::cout << std::endl;
        }
    }
    void reset_id() {
        size_t cnt = 0;
        for (auto node : nodes) {
            node->set_id(cnt++);
        }
    }
    ~Graph() {
        for (auto node : nodes) {
            delete node;
        }
    }

    // some graph algorithems
    void init_visited() {
        for (auto node : nodes) {
            node->is_visited = false;
        }
    }
    void delete_visited() {
        auto iter = nodes.begin();
        while (iter != nodes.end()) {
            if (!(*iter)->is_visited) {
                delete *iter;
                iter = nodes.erase(iter);
            }
            else
                iter++;
        }
    }
    // 获得从指定起点开始的连通子图（原地转换）
    void get_connected_graph(Graph_Node<Node_Info, Edge_Info> *node) {
        init_visited();
        std::queue<Graph_Node<Node_Info, Edge_Info> *> q;
        q.push(node);
        while (!q.empty()) {
            Graph_Node<Node_Info, Edge_Info> *cur = q.front();
            q.pop();
            if (cur->is_visited) {
                continue;
            }
            cur->is_visited = true;
            for (auto succ : cur->get_succs()) {
                q.push(succ->get_to());
            }
        }
        delete_visited();
        reset_id();
    }
    void dfs(std::stack<Graph_Node<Node_Info, Edge_Info> *> &s, Graph_Node<Node_Info, Edge_Info> *begin) {
        if (begin->is_visited)
            return;
        begin->is_visited = true;
        for (auto to : begin->get_succs()) {
            dfs(s, to->get_to());
        }
        s.push(begin);
    }
    // 获得逆后序
    std::vector<Graph_Node<Node_Info, Edge_Info> *> get_dfn_seqs(Graph_Node<Node_Info, Edge_Info> *begin) {
        init_visited();
        std::stack<Graph_Node<Node_Info, Edge_Info> *> s;
        dfs(s, begin);
        std::vector<Graph_Node<Node_Info, Edge_Info> *> res;
        while (!s.empty()) {
            res.push_back(s.top());
            s.pop();
        }
        return res;
    }
    void compute_dom(Graph_Node<Node_Info, Edge_Info> *begin) {
        // 清空原有信息
        for (auto &node : this->nodes) {
            node->clear_idom_childs();
            node->set_idom_parent(nullptr);
        }

        std::vector<BitMap> maps(this->nodes.size(), BitMap(this->nodes.size()));
        for (auto &map : maps)
            map.set_full();
        std::vector<Graph_Node<Node_Info, Edge_Info> *> dfn_seqs = get_dfn_seqs(begin);
        reset_id();
        maps[begin->get_id()].set_empty();
        maps[begin->get_id()].add(begin->get_id());
        bool flag = true;
        while (flag) {
            flag = false;
            for (size_t i = 1; i < this->nodes.size(); i++) {
                auto node = dfn_seqs[i];
                BitMap tmp(this->nodes.size());
                tmp.set_full();
                for (auto pred : node->get_preds()) {
                    tmp = tmp & maps[pred->get_from()->get_id()];
                }
                tmp.add(node->get_id());
                if (tmp != maps[node->get_id()]) {
                    maps[node->get_id()] = tmp;
                    flag = true;
                }
            }
        }
        // for (auto map : maps) {
        //     map.print();
        // }
        for (auto node : dfn_seqs) {
            for (auto nd : dfn_seqs) {
                if (nd == node) continue;
                if (!maps[node->get_id()].if_in(nd->get_id())) continue; // 不支配该点
                BitMap tmp = (maps[node->get_id()] & maps[nd->get_id()]) ^ maps[node->get_id()];
                // 不受 node 支配但受所有支配 node 的点支配
                if (tmp.if_in(node->get_id()) && tmp.get_num() == 1) {
                    nd->add_idom_child(node);
                    node->set_idom_parent(nd);
                    break;
                }
            }
        }
        begin->set_idom_parent(nullptr);
    }
    void print_idom_tree(Graph_Node<Node_Info, Edge_Info> *begin) {
        init_visited();
        std::cout << "dom tree:" << std::endl;
        begin->print_idom_dfs(0);
    }
    void compute_dom_frontier(Graph_Node<Node_Info, Edge_Info> *begin) {
        for (auto &node : this->nodes)
            node->clear_dom_frontier();
        compute_dom(begin);
        for (auto &node : this->nodes) {
            if (node->get_preds().size() == 1) continue; // 只有一个前驱不会是支配边界
            auto idom = node->get_idom_parent();
            for (auto &pred_edge : node->get_preds()) {
                auto pred = pred_edge->get_from();
                // 支配前驱的所有节点且不支配该节点（直接支配父亲即可表示）的节点的支配边界为该节点
                while (pred != idom) {
                    pred->add_dom_frontier(node);
                    pred = pred->get_idom_parent();
                }
            }
        }
    }
    void print_dom_frontiers() {
        std::cout << "dom frontier:" << std::endl;
        for (auto &node : this->nodes) {
            node->print_dom_frontier();
            std::cout << std::endl;
        }
    }
};
#endif
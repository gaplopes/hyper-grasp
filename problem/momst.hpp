#ifndef MOMST_HPP
#define MOMST_HPP

#include <problem_base.hpp>

typedef std::pair<int64_t, int32_t> pii;

class UnionFind {
 public:
  UnionFind() = default;

  UnionFind(int32_t size) : parent(size), rank(size, 0) {
    for (int32_t i = 0; i < size; ++i) {
      parent[i] = i;
    }
  }

  bool can_unite(int32_t u, int32_t v) const {
    return find(u) != find(v);
  }

  bool unite(int32_t u, int32_t v) {
    u = find(u);
    v = find(v);
    if (u == v) {
      return false;
    }
    if (rank[u] < rank[v]) {
      std::swap(u, v);
    }
    if (rank[u] == rank[v]) {
      rank[u]++;
    }
    parent[v] = u;
    return true;
  }

 private:
  int32_t find(int32_t u) const {
    if (parent[u] != u) {
      parent[u] = find(parent[u]);
    }
    return parent[u];
  }

  mutable std::vector<int32_t> parent;
  mutable std::vector<int32_t> rank;
};

struct Edge {
  Edge() = default;

  Edge(int32_t idx, int32_t src, int32_t dst, std::vector<int64_t> values) : idx(idx), src(src), dst(dst), values(values) {}

 public:
  int32_t idx;
  int32_t src;
  int32_t dst;
  std::vector<int64_t> values;
};

struct Node {
  Node() = default;

  Node(int idx, std::vector<Edge> edges) : idx(idx), edges(edges) {}

 public:
  int idx;
  std::vector<Edge> edges;
};

template <typename Solution, typename Candidate>
class MOMST : public ProblemBase<Solution, Candidate> {
  MOMST(int32_t V, int32_t E, int32_t M,
        std::vector<Node> nodes,
        std::vector<Edge> edges,
        std::vector<Solution> nondominated_set,
        std::vector<Solution> supported_set)
      : V(V),
        E(E),
        M(M),
        nodes(nodes),
        edges(edges),
        nondominated_set(nondominated_set),
        supported_set(supported_set),
        current_solution(M, 0),
        current_n_edges(0),
        current_used_edges(E, false),
        current_union_find(V) {
    // Sort the edges by increasing values (lexicographically)
    std::vector<std::vector<Edge>> sorted_edges(M);
    for (int i = 0; i < M; i++) {
      sorted_edges[i] = edges;
      std::sort(sorted_edges[i].begin(), sorted_edges[i].end(), [i, M](const Edge& a, const Edge& b) {
        for (int j = 0; j < M; j++) {
          int idx = (i + j) % M;  // Rotate the starting objective
          if (a.values[idx] != b.values[idx]) {
            return a.values[idx] < b.values[idx];
          }
        }
        return false;  // All values are equal
      });
    }
    this->sorted_edges = sorted_edges;
    // Compute the reference point
    this->reference_point = compute_reference_point(M);
  }

 public:
  template <typename IStream>
  static auto from_stream(IStream&& is) -> MOMST {
    int32_t V, M = 2, E;
    is >> V;
    // is >> E; // TODO: Uncomment this line if the number of edges is given in the input
    E = (V * (V - 1)) / 2;
    std::vector<Node> nodes(V);
    std::vector<Edge> edges(E);
    for (int i = 0; i < E; i++) {
      int src, dst;
      is >> src >> dst;
      std::vector<int64_t> values(M);
      for (int j = 0; j < M; j++) {
        is >> values[j];
      }
      edges[i] = Edge(i, src, dst, values);
    }

    for (const auto& edge : edges) {
      nodes[edge.src].edges.push_back(edge);
      nodes[edge.dst].edges.push_back(edge);
    }

    std::vector<Solution> supported_set;
    std::vector<Solution> nondominated_set;
    if (!is.eof()) {
      int32_t n_supported_set;
      is >> n_supported_set;
      supported_set = std::vector<Solution>(n_supported_set, Solution(M, 0));
      for (int i = 0; i < n_supported_set; i++) {
        for (int j = 0; j < M; j++) {
          is >> supported_set[i][j];
        }
      }
      int32_t n_nondominated_set;
      is >> n_nondominated_set;
      nondominated_set = std::vector<Solution>(n_nondominated_set, Solution(M, 0));
      for (int i = 0; i < n_nondominated_set; i++) {
        for (int j = 0; j < M; j++) {
          is >> nondominated_set[i][j];
        }
      }
    }
    return MOMST(V, E, M, nodes, edges, nondominated_set, supported_set);
  }

  void reset() override {
    this->current_solution = Solution(this->M, 0);
    this->current_n_edges = 0;
    this->current_used_edges = std::vector<bool>(this->E, false);
    this->current_union_find = UnionFind(this->V);
  }

  [[nodiscard]] std::vector<Solution> getNondominatedSet() const override {
    return this->nondominated_set;
  }

  [[nodiscard]] std::vector<int64_t> getReferencePoint() const override {
    return this->reference_point;
  }

  [[nodiscard]] std::vector<Candidate> generateCandidates(const Solution& current_solution,
                                                          const HypervolumeIndicator<int64_t, Solution>& hvc_space) const override {
    std::vector<Candidate> candidate_edges;
    for (int i = 0; i < E; i++) {
      if (this->current_used_edges[i]) continue;
      if (!this->current_union_find.can_unite(this->edges[i].src, this->edges[i].dst)) continue;
      Solution aux_lower_bound = compute_bound_kruskal(this->current_union_find, this->current_used_edges, this->current_n_edges + 1, i);
      Solution candidate_lower_bound = Solution(M, 0);
      for (int j = 0; j < M; j++) {
        candidate_lower_bound[j] = this->current_solution[j] + this->edges[i].values[j] + aux_lower_bound[j];
      }
      int64_t hv = hvc_space.contribution(candidate_lower_bound);
      if (hv > 0) {
        candidate_edges.push_back({i, hv});
      }
    }
    std::sort(candidate_edges.begin(), candidate_edges.end(), [](Candidate a, Candidate b) {
      return a.second > b.second;
    });
    return candidate_edges;
  }

  [[nodiscard]] Solution chooseCandidate(const std::vector<Candidate>& selected_candidates) override {
    int32_t random_edge = std::rand() % selected_candidates.size();
    int32_t selected_edge = selected_candidates[random_edge].first;
    for (int j = 0; j < M; j++) {
      this->current_solution[j] += this->edges[selected_edge].values[j];
    }
    this->current_used_edges[selected_edge] = true;
    if (!this->current_union_find.unite(this->edges[selected_edge].src, this->edges[selected_edge].dst)) {
      std::cerr << "Error: The selected edge cannot be added to the current solution\n";
      throw std::runtime_error("Invalid union operation");
    }
    this->current_n_edges += 1;
    return this->current_solution;
  }

  [[nodiscard]] Solution emptySolution() const override {
    return Solution(this->M, 0);
  }

  [[nodiscard]] bool isFeasible(const Solution& solution) const override {
    return current_n_edges == this->V - 1;
  }

  [[nodiscard]] std::tuple<Solution, std::vector<Solution>> improveSolution(const Solution& current_solution, const std::vector<Solution>& solution_set) const override {
    return std::make_tuple(Solution(), std::vector<Solution>());
  }

 private:
  const int32_t V;                               // Number of vertices
  const int32_t E;                               // Number of edges
  const int32_t M;                               // Number of objectives
  const std::vector<Node> nodes;                 // Nodes of the tree
  const std::vector<Edge> edges;                 // Edges of the graph
  const std::vector<Solution> nondominated_set;  // Nondominated set
  const std::vector<Solution> supported_set;     // Supported set
  Solution reference_point;
  std::vector<std::vector<Edge>> sorted_edges;  // Sorted edges of the graph
  Solution current_solution;
  int32_t current_n_edges;
  std::vector<bool> current_used_edges;
  UnionFind current_union_find;

  Solution compute_bound_kruskal(const UnionFind& current_union_find,
                                 const std::vector<bool>& current_used_edges,
                                 const int32_t& current_n_edges,
                                 const int32_t& used_edge) const {
    std::vector<int64_t> lower_bound(M, 0);
    for (int i = 0; i < M; ++i) {
      UnionFind uf_aux = current_union_find;
      int32_t count = this->V - 1 - current_n_edges;
      const auto& edges = sorted_edges[i];
      for (const auto& edge : edges) {
        if (current_used_edges[edge.idx] || edge.idx == used_edge) continue;
        if (!uf_aux.can_unite(edge.src, edge.dst)) continue;
        lower_bound[i] += edge.values[i];
        --count;
        uf_aux.unite(edge.src, edge.dst);
        if (count == 0) break;
      }
    }
    return lower_bound;
  }

  Solution compute_bound_kruskal_for_m(const int32_t& objective) const {
    UnionFind uf = UnionFind(V);
    Solution sol = Solution(M, 0);
    int32_t n_edges = 0;
    int32_t count = this->V - 1;
    const auto& edges = sorted_edges[objective];
    for (const auto& edge : edges) {
      if (!uf.can_unite(edge.src, edge.dst)) continue;
      for (int j = 0; j < M; j++) {
        sol[j] += edge.values[j];
      }
      --count;
      uf.unite(edge.src, edge.dst);
      if (count == 0) break;
    }
    return sol;
  }

  Solution compute_reference_point(const int32_t& M) const {
    if (M == 2) {
      Solution nadir = Solution(M, 0);
      Solution ideal = Solution(M, 0);
      for (int i = 0; i < M; i++) {
        Solution m_solution = compute_bound_kruskal_for_m(i);
        int32_t other = abs(i - 1);
        nadir[other] += m_solution[other];
        ideal[i] = m_solution[i];
      }
      return nadir;
    } else {
      // TODO: Implement the logic to compute the reference point for M > 2
      assert(false);
      return Solution(M, 0);
    }
  }
};

#endif  // MOMST_HPP

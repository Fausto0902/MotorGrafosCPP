#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include "Graph.hpp"
#include "Exceptions.hpp"
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <limits>
#include <algorithm>
#include <type_traits>
#include <stdexcept>

namespace graph_engine {

/**
 * @brief Estructura de resultado para algoritmos de caminos mínimos.
 */
template <typename T, typename W>
struct ShortestPathResult {
    T source;
    std::map<T, W> distances;
    std::map<T, T> predecessors;
    bool hasNegativeCycle = false;
    std::vector<T> negativeCycle;

    bool isReachable(const T& target) const {
        return distances.find(target) != distances.end() && 
               distances.at(target) != std::numeric_limits<W>::infinity();
    }

    std::vector<T> getPath(const T& target) const {
        if (!isReachable(target) || hasNegativeCycle) return {};
        std::vector<T> path;
        T current = target;
        while (true) {
            path.push_back(current);
            if (current == source) break;
            if (predecessors.find(current) == predecessors.end()) return {};
            current = predecessors.at(current);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

/**
 * @brief Estructura de resultado para el algoritmo de flujo máximo.
 */
template <typename T, typename W>
struct MaxFlowResult {
    W maxFlow;
    struct FlowEdge {
        T from;
        T to;
        W flow;
        W capacity;
    };
    std::vector<FlowEdge> flowEdges;
    std::vector<T> minCutSourceSide; // S-set del corte mínimo
    std::vector<T> minCutSinkSide;   // T-set del corte mínimo
};

/**
 * @brief Estructura de resultado para componentes fuertemente conexas.
 */
template <typename T>
struct SCCResult {
    std::vector<std::vector<T>> components;
};

/**
 * @brief Estructura de resultado para ordenación topológica.
 */
template <typename T>
struct TopologicalSortResult {
    std::vector<T> order;
    bool hasCycle;
};

/**
 * @brief Clase estática que contiene los algoritmos avanzados de grafos.
 */
template <typename T, typename W = double>
class Algorithms {
public:
    struct PathResult {
        W distance;
        std::vector<T> path;
        bool hasNegativeCycle = false;
        std::vector<T> negativeCycle;
    };

    static PathResult dijkstra(const Graph<T, W>& graph, const T& source, const T& target) {
        auto res = dijkstra(graph, source);
        return {res.distances[target], res.getPath(target), false, {}};
    }

    static PathResult bellmanFord(const Graph<T, W>& graph, const T& source, const T& target) {
        auto res = bellmanFord(graph, source);
        return {res.distances[target], res.getPath(target), res.hasNegativeCycle, res.negativeCycle};
    }

    /**
     * @brief Algoritmo de Dijkstra para caminos mínimos desde un origen.
     * @throws InvalidWeightException si se detecta una arista con peso negativo.
     */
    static ShortestPathResult<T, W> dijkstra(const Graph<T, W>& graph, const T& source) {
        static_assert(std::is_arithmetic_v<W>, "El peso debe ser un tipo aritmético.");
        
        if (!graph.hasVertex(source)) {
            throw VertexNotFoundException("El vértice origen no existe en el grafo.");
        }

        ShortestPathResult<T, W> result;
        result.source = source;

        for (const auto& v : graph.getVertices()) {
            result.distances[v] = std::numeric_limits<W>::infinity();
        }
        result.distances[source] = 0;

        using PQElement = std::pair<W, T>;
        std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;
        pq.push({0, source});

        while (!pq.empty()) {
            W d = pq.top().first;
            T u = pq.top().second;
            pq.pop();

            if (d > result.distances[u]) continue;

            for (const auto& [v, weight] : graph.getNeighbors(u)) {
                if (weight < 0) {
                    throw InvalidWeightException("Dijkstra no soporta pesos negativos. Use Bellman-Ford.");
                }

                W newDist = result.distances[u] + weight;
                if (newDist < result.distances[v]) {
                    result.distances[v] = newDist;
                    result.predecessors[v] = u;
                    pq.push({newDist, v});
                }
            }
        }

        return result;
    }

    /**
     * @brief Algoritmo de Bellman-Ford con detección y reconstrucción de ciclos negativos.
     */
    static ShortestPathResult<T, W> bellmanFord(const Graph<T, W>& graph, const T& source) {
        static_assert(std::is_arithmetic_v<W>, "El peso debe ser un tipo aritmético.");

        if (!graph.hasVertex(source)) {
            throw VertexNotFoundException("El vértice origen no existe en el grafo.");
        }

        ShortestPathResult<T, W> result;
        result.source = source;
        auto vertices = graph.getVertices();
        size_t n = vertices.size();

        for (const auto& v : vertices) {
            result.distances[v] = std::numeric_limits<W>::infinity();
        }
        result.distances[source] = 0;

        T lastVertexChanged{};
        bool changed = false;

        // Relajación N-1 veces
        for (size_t i = 0; i < n; ++i) {
            changed = false;
            for (const auto& u : vertices) {
                if (result.distances[u] == std::numeric_limits<W>::infinity()) continue;

                for (const auto& [v, weight] : graph.getNeighbors(u)) {
                    if (result.distances[u] + weight < result.distances[v]) {
                        result.distances[v] = result.distances[u] + weight;
                        result.predecessors[v] = u;
                        lastVertexChanged = v;
                        changed = true;
                        
                        if (i == n - 1) {
                            result.hasNegativeCycle = true;
                        }
                    }
                }
            }
            if (!changed) break;
        }

        if (result.hasNegativeCycle) {
            T curr = lastVertexChanged;
            for (size_t i = 0; i < n; ++i) {
                curr = result.predecessors[curr];
            }

            std::vector<T> cycle;
            T start = curr;
            cycle.push_back(start);
            curr = result.predecessors[curr];
            while (curr != start) {
                cycle.push_back(curr);
                curr = result.predecessors[curr];
            }
            cycle.push_back(start);
            std::reverse(cycle.begin(), cycle.end());
            result.negativeCycle = cycle;
        }

        return result;
    }

    /**
     * @brief Algoritmo de Edmonds-Karp para flujo máximo con tolerancia epsilon (1e-9).
     * @throws InvalidWeightException si hay capacidades negativas.
     */
    static MaxFlowResult<T, W> edmondsKarp(const Graph<T, W>& graph, const T& source, const T& sink) {
        static_assert(std::is_arithmetic_v<W>, "La capacidad debe ser un tipo aritmético.");
        const double EPS = 1e-9;

        if (!graph.hasVertex(source) || !graph.hasVertex(sink)) {
            throw VertexNotFoundException("Origen o sumidero no existen en el grafo.");
        }
        if (source == sink) {
            throw FlowException("El origen y el sumidero deben ser distintos.");
        }

        // Red residual implementada como mapa disperso de mapas
        std::map<T, std::map<T, W>> residualCapacity;
        auto vertices = graph.getVertices();

        for (const auto& u : vertices) {
            for (const auto& [v, weight] : graph.getNeighbors(u)) {
                if (weight < 0) {
                    throw InvalidWeightException("La capacidad no puede ser negativa en flujo máximo.");
                }
                residualCapacity[u][v] += weight;
                if (residualCapacity[v].find(u) == residualCapacity[v].end()) {
                    residualCapacity[v][u] = 0;
                }
            }
        }

        W maxFlow = 0;
        std::map<T, T> parent;

        auto bfs = [&]() -> bool {
            parent.clear();
            std::queue<T> q;
            q.push(source);
            std::set<T> visited;
            visited.insert(source);

            while (!q.empty()) {
                T u = q.front();
                q.pop();

                if (residualCapacity.count(u)) {
                    for (auto const& [v, cap] : residualCapacity.at(u)) {
                        if (visited.find(v) == visited.end() && cap > EPS) {
                            parent[v] = u;
                            visited.insert(v);
                            if (v == sink) return true;
                            q.push(v);
                        }
                    }
                }
            }
            return false;
        };

        while (bfs()) {
            W pathFlow = std::numeric_limits<W>::max();
            for (T v = sink; v != source; v = parent[v]) {
                T u = parent[v];
                pathFlow = std::min(pathFlow, residualCapacity[u][v]);
            }

            for (T v = sink; v != source; v = parent[v]) {
                T u = parent[v];
                residualCapacity[u][v] -= pathFlow;
                residualCapacity[v][u] += pathFlow;
            }
            maxFlow += pathFlow;
        }

        MaxFlowResult<T, W> result;
        result.maxFlow = maxFlow;

        // Determinar el corte mínimo (alcanzables desde el origen en la red residual)
        std::set<T> reachable;
        std::queue<T> q;
        q.push(source);
        reachable.insert(source);

        while (!q.empty()) {
            T u = q.front();
            q.pop();
            if (residualCapacity.count(u)) {
                for (auto const& [v, cap] : residualCapacity.at(u)) {
                    if (reachable.find(v) == reachable.end() && cap > EPS) {
                        reachable.insert(v);
                        q.push(v);
                    }
                }
            }
        }

        for (const auto& v : vertices) {
            if (reachable.count(v)) {
                result.minCutSourceSide.push_back(v);
            } else {
                result.minCutSinkSide.push_back(v);
            }
        }

        // Reconstrucción de aristas de flujo
        for (const auto& u : vertices) {
            if (graph.hasVertex(u)) {
                for (const auto& [v, originalCap] : graph.getNeighbors(u)) {
                    W residualCap = 0;
                    if (residualCapacity.count(u) && residualCapacity.at(u).count(v)) {
                        residualCap = residualCapacity.at(u).at(v);
                    }
                    W flowVal = originalCap - residualCap;
                    if (flowVal < 0) flowVal = 0;
                    result.flowEdges.push_back({u, v, flowVal, originalCap});
                }
            }
        }

        return result;
    }

    /**
     * @brief Algoritmo de Tarjan para encontrar Componentes Fuertemente Conexas (SCC).
     */
    static SCCResult<T> tarjanSCC(const Graph<T, W>& graph) {
        SCCResult<T> result;
        std::map<T, int> discoveryTime;
        std::map<T, int> lowLink;
        std::map<T, bool> onStack;
        std::stack<T> st;
        int timer = 0;

        auto vertices = graph.getVertices();
        for (const auto& v : vertices) {
            discoveryTime[v] = -1;
            onStack[v] = false;
        }

        auto strongConnect = [&](auto self, const T& u) -> void {
            discoveryTime[u] = lowLink[u] = ++timer;
            st.push(u);
            onStack[u] = true;

            for (const auto& [v, weight] : graph.getNeighbors(u)) {
                if (discoveryTime[v] == -1) {
                    self(self, v);
                    lowLink[u] = std::min(lowLink[u], lowLink[v]);
                } else if (onStack[v]) {
                    lowLink[u] = std::min(lowLink[u], discoveryTime[v]);
                }
            }

            if (lowLink[u] == discoveryTime[u]) {
                std::vector<T> component;
                while (true) {
                    T node = st.top();
                    st.pop();
                    onStack[node] = false;
                    component.push_back(node);
                    if (node == u) break;
                }
                result.components.push_back(component);
            }
        };

        for (const auto& v : vertices) {
            if (discoveryTime[v] == -1) {
                strongConnect(strongConnect, v);
            }
        }

        return result;
    }

    /**
     * @brief Algoritmo de Kahn para ordenación topológica.
     */
    static TopologicalSortResult<T> kahnTopologicalSort(const Graph<T, W>& graph) {
        if (!graph.isDirected()) {
            throw std::logic_error("La ordenación topológica solo se aplica a grafos dirigidos.");
        }

        TopologicalSortResult<T> result;
        std::map<T, int> inDegree;
        auto vertices = graph.getVertices();

        for (const auto& v : vertices) {
            inDegree[v] = 0;
        }

        for (const auto& u : vertices) {
            for (const auto& [v, weight] : graph.getNeighbors(u)) {
                inDegree[v]++;
            }
        }

        std::queue<T> q;
        for (const auto& v : vertices) {
            if (inDegree[v] == 0) {
                q.push(v);
            }
        }

        while (!q.empty()) {
            T u = q.front();
            q.pop();
            result.order.push_back(u);

            for (const auto& [v, weight] : graph.getNeighbors(u)) {
                inDegree[v]--;
                if (inDegree[v] == 0) {
                    q.push(v);
                }
            }
        }

        result.hasCycle = (result.order.size() != vertices.size());
        return result;
    }
};

} // namespace graph_engine

#endif // ALGORITHMS_HPP

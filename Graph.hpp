#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <queue>
#include <stack>
#include "Exceptions.hpp"

namespace graph_engine {

/**
 * @brief Enumeración para definir la naturaleza del grafo.
 */
enum class GraphDirection {
    Directed,
    Undirected
};

/**
 * @brief Clase Graph: Motor de grafos genérico basado en listas de adyacencia.
 * 
 * @tparam T Tipo de dato para identificar los vértices (debe ser hashable).
 * @tparam W Tipo de dato para los pesos de las aristas (debe ser aritmético).
 */
template <typename T, typename W = double>
class Graph {
    static_assert(std::is_arithmetic_v<W>, "El tipo de peso W debe ser un tipo aritmético (int, double, long, etc.).");

public:
    /**
     * @brief Constructor de la clase Graph.
     * @param direction Define si el grafo es dirigido o no dirigido.
     */
    explicit Graph(GraphDirection direction = GraphDirection::Directed)
        : direction(direction) {}

    /**
     * @brief Añade un nuevo vértice al grafo.
     * @param vertex Identificador del vértice.
     * @return true si el vértice fue añadido, false si ya existía.
     */
    bool addVertex(const T& vertex) {
        if (adjacencyList.find(vertex) == adjacencyList.end()) {
            adjacencyList[vertex] = std::unordered_map<T, W>();
            return true;
        }
        return false;
    }

    /**
     * @brief Añade o actualiza una arista entre dos vértices.
     * Si los vértices no existen, se crean automáticamente.
     * Si la arista ya existe, se actualiza su peso.
     * 
     * @param u Vértice de origen.
     * @param v Vértice de destino.
     * @param weight Peso de la arista.
     */
    void addEdge(const T& u, const T& v, W weight) {
        addVertex(u);
        addVertex(v);

        adjacencyList[u][v] = weight;

        if (direction == GraphDirection::Undirected) {
            adjacencyList[v][u] = weight;
        }
    }

    /**
     * @brief Elimina una arista entre dos vértices.
     * @param u Vértice de origen.
     * @param v Vértice de destino.
     * @throws EdgeNotFoundException si la arista no existe.
     */
    void removeEdge(const T& u, const T& v) {
        if (!hasEdge(u, v)) {
            throw EdgeNotFoundException("No se puede eliminar una arista inexistente.");
        }

        adjacencyList[u].erase(v);

        if (direction == GraphDirection::Undirected) {
            adjacencyList[v].erase(u);
        }
    }

    /**
     * @brief Elimina un vértice y todas sus aristas incidentes.
     * @param vertex Vértice a eliminar.
     * @throws VertexNotFoundException si el vértice no existe.
     */
    void removeVertex(const T& vertex) {
        if (adjacencyList.find(vertex) == adjacencyList.end()) {
            throw VertexNotFoundException("No se puede eliminar un vértice inexistente.");
        }

        // Eliminar todas las aristas que apuntan a este vértice
        for (auto& pair : adjacencyList) {
            pair.second.erase(vertex);
        }

        // Eliminar el vértice y sus aristas salientes
        adjacencyList.erase(vertex);
    }

    /**
     * @brief Comprueba si un vértice existe en el grafo.
     */
    bool hasVertex(const T& vertex) const {
        return adjacencyList.find(vertex) != adjacencyList.end();
    }

    /**
     * @brief Comprueba si existe una arista entre u y v.
     */
    bool hasEdge(const T& u, const T& v) const {
        auto it = adjacencyList.find(u);
        if (it != adjacencyList.end()) {
            return it->second.find(v) != it->second.end();
        }
        return false;
    }

    /**
     * @brief Obtiene el peso de una arista.
     * @throws EdgeNotFoundException si la arista no existe.
     */
    W getWeight(const T& u, const T& v) const {
        auto itU = adjacencyList.find(u);
        if (itU != adjacencyList.end()) {
            auto itV = itU->second.find(v);
            if (itV != itU->second.end()) {
                return itV->second;
            }
        }
        throw EdgeNotFoundException("La arista solicitada no existe en el grafo.");
    }

    /**
     * @brief Devuelve una lista con todos los identificadores de vértices.
     */
    std::vector<T> getVertices() const {
        std::vector<T> vertices;
        vertices.reserve(adjacencyList.size());
        for (const auto& pair : adjacencyList) {
            vertices.push_back(pair.first);
        }
        return vertices;
    }

    /**
     * @brief Devuelve los vecinos de un vértice (aristas salientes).
     * @return Un mapa de destino -> peso.
     * @throws VertexNotFoundException si el vértice no existe.
     */
    const std::unordered_map<T, W>& getNeighbors(const T& vertex) const {
        auto it = adjacencyList.find(vertex);
        if (it == adjacencyList.end()) {
            throw VertexNotFoundException("Vértice no encontrado al intentar obtener vecinos.");
        }
        return it->second;
    }

    /**
     * @brief Obtiene el grado de salida de un vértice.
     */
    size_t getOutDegree(const T& vertex) const {
        return getNeighbors(vertex).size();
    }

    /**
     * @brief Obtiene el grado de entrada de un vértice.
     * En grafos no dirigidos, es igual al grado de salida.
     */
    size_t getInDegree(const T& vertex) const {
        if (!hasVertex(vertex)) {
            throw VertexNotFoundException("Vértice no encontrado al calcular grado de entrada.");
        }

        if (direction == GraphDirection::Undirected) {
            return getOutDegree(vertex);
        }

        size_t count = 0;
        for (const auto& pair : adjacencyList) {
            if (pair.second.find(vertex) != pair.second.end()) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief Devuelve el número total de vértices.
     */
    size_t getVertexCount() const {
        return adjacencyList.size();
    }

    /**
     * @brief Devuelve el número total de aristas.
     */
    size_t getEdgeCount() const {
        size_t count = 0;
        for (const auto& pair : adjacencyList) {
            count += pair.second.size();
        }
        return (direction == GraphDirection::Undirected) ? count / 2 : count;
    }

    /**
     * @brief Indica si el grafo es dirigido.
     */
    bool isDirected() const {
        return direction == GraphDirection::Directed;
    }

    /**
     * @brief Limpia el grafo por completo.
     */
    void clear() {
        adjacencyList.clear();
    }

    /**
     * @brief Devuelve la lista de adyacencia completa (solo lectura).
     */
    const std::unordered_map<T, std::unordered_map<T, W>>& getAdjacencyList() const {
        return adjacencyList;
    }

    bool hasCycle() const {
        std::unordered_map<T, int> color;
        for (const auto& [u, _] : adjacencyList) color[u] = 0;
        auto dfs = [&](const T& u, auto& self) -> bool {
            color[u] = 1;
            if (adjacencyList.count(u)) {
                for (const auto& [v, _] : adjacencyList.at(u)) {
                    if (color[v] == 1) return true;
                    if (color[v] == 0 && self(v, self)) return true;
                }
            }
            color[u] = 2;
            return false;
        };
        for (const auto& [u, _] : adjacencyList) if (color[u] == 0 && dfs(u, dfs)) return true;
        return false;
    }

    std::vector<T> topologicalSort() const {
        std::unordered_map<T, size_t> inDegree;
        for (const auto& [u, _] : adjacencyList) inDegree[u] = 0;
        for (const auto& [u, neighbors] : adjacencyList) for (const auto& [v, _] : neighbors) inDegree[v]++;
        std::queue<T> q;
        for (const auto& [u, deg] : inDegree) if (deg == 0) q.push(u);
        std::vector<T> res;
        while (!q.empty()) {
            T u = q.front(); q.pop(); res.push_back(u);
            if (adjacencyList.count(u)) for (const auto& [v, _] : adjacencyList.at(u)) if (--inDegree[v] == 0) q.push(v);
        }
        return res.size() == adjacencyList.size() ? res : std::vector<T>();
    }

    std::vector<std::vector<T>> getSCCs() const {
        size_t timer = 0;
        std::unordered_map<T, size_t> disc, low;
        std::stack<T> st;
        std::unordered_map<T, bool> onStack;
        std::vector<std::vector<T>> res;
        auto dfs = [&](const T& u, auto& self) -> void {
            disc[u] = low[u] = ++timer; st.push(u); onStack[u] = true;
            if (adjacencyList.count(u)) {
                for (const auto& [v, _] : adjacencyList.at(u)) {
                    if (!disc[v]) { self(v, self); low[u] = std::min(low[u], low[v]); }
                    else if (onStack[v]) low[u] = std::min(low[u], disc[v]);
                }
            }
            if (low[u] == disc[u]) {
                std::vector<T> comp;
                while (true) { T v = st.top(); st.pop(); onStack[v] = false; comp.push_back(v); if (u == v) break; }
                res.push_back(comp);
            }
        };
        for (const auto& [u, _] : adjacencyList) if (!disc[u]) dfs(u, dfs);
        return res;
    }

    struct Edge {
        T from;
        T to;
        W weight;
    };

    std::vector<Edge> getAllEdges() const {
        std::vector<Edge> edges;
        for (const auto& [u, neighbors] : adjacencyList) {
            for (const auto& [v, weight] : neighbors) {
                edges.push_back({u, v, weight});
            }
        }
        return edges;
    }

private:
    /**
     * @brief Estructura de datos principal: Mapa de mapas.
     * El primer nivel es el vértice origen, el segundo nivel es el vértice destino y su peso.
     */
    std::unordered_map<T, std::unordered_map<T, W>> adjacencyList;

    /**
     * @brief Dirección del grafo.
     */
    GraphDirection direction;
};

} // namespace graph_engine

#endif // GRAPH_HPP

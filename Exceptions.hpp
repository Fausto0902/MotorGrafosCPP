#ifndef GRAPH_ENGINE_EXCEPTIONS_HPP
#define GRAPH_ENGINE_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

/**
 * @namespace graph_engine
 * @brief Espacio de nombres principal para el motor de análisis de redes y grafos.
 */
namespace graph_engine {

/**
 * @class GraphException
 * @brief Excepción base para todos los errores relacionados con el motor de grafos.
 * 
 * Hereda de std::runtime_error para proporcionar compatibilidad con el manejo
 * de excepciones estándar de C++.
 */
class GraphException : public std::runtime_error {
public:
    explicit GraphException(const std::string& message)
        : std::runtime_error("GraphException: " + message) {}
};

/**
 * @class VertexNotFoundException
 * @brief Se lanza cuando se intenta acceder a un vértice que no existe en el grafo.
 */
class VertexNotFoundException : public GraphException {
public:
    explicit VertexNotFoundException(const std::string& vertex_id)
        : GraphException("Vertex not found: " + vertex_id) {}
};

/**
 * @class EdgeNotFoundException
 * @brief Se lanza cuando se intenta acceder a una arista inexistente entre dos vértices.
 */
class EdgeNotFoundException : public GraphException {
public:
    explicit EdgeNotFoundException(const std::string& message)
        : GraphException("Edge Error: " + message) {}
    explicit EdgeNotFoundException(const std::string& u, const std::string& v)
        : GraphException("Edge not found between " + u + " and " + v) {}
};

/**
 * @class InvalidWeightException
 * @brief Se lanza cuando se encuentra un peso o capacidad no válida.
 * 
 * Por ejemplo, pesos negativos en el algoritmo de Dijkstra o capacidades negativas
 * en el algoritmo de Edmonds-Karp.
 */
class InvalidWeightException : public GraphException {
public:
    explicit InvalidWeightException(const std::string& message)
        : GraphException("Invalid Weight/Capacity: " + message) {}
};

/**
 * @class AlgorithmException
 * @brief Se lanza cuando un algoritmo no puede completarse debido a la estructura del grafo.
 * 
 * Por ejemplo, intentar realizar una ordenación topológica en un grafo con ciclos.
 */
class AlgorithmException : public GraphException {
public:
    explicit AlgorithmException(const std::string& algorithm_name, const std::string& reason)
        : GraphException("Algorithm [" + algorithm_name + "] failed: " + reason) {}
};

/**
 * @class NegativeCycleException
 * @brief Se lanza específicamente cuando se detecta un ciclo negativo en algoritmos de caminos mínimos.
 */
class NegativeCycleException : public AlgorithmException {
public:
    explicit NegativeCycleException(const std::string& message = "Negative cycle detected in the graph.")
        : AlgorithmException("Bellman-Ford/SPFA", message) {}
};

/**
 * @class ParseException
 * @brief Se lanza durante la serialización o deserialización si el formato es incorrecto.
 * 
 * Proporciona información detallada sobre errores sintácticos o semánticos en el JSON.
 */
class JSONParseException : public GraphException {
public:
    explicit JSONParseException(const std::string& message, size_t line = 0, size_t column = 0)
        : GraphException("JSON Parse Error: " + message + 
                         (line > 0 ? " at line " + std::to_string(line) + ", col " + std::to_string(column) : "")) {}
};

using ParseException = JSONParseException;

/**
 * @class FlowException
 * @brief Se lanza ante errores específicos en algoritmos de flujo máximo.
 */
class FlowException : public AlgorithmException {
public:
    explicit FlowException(const std::string& reason)
        : AlgorithmException("Edmonds-Karp", reason) {}
};

} // namespace graph_engine

#endif // GRAPH_ENGINE_EXCEPTIONS_HPP

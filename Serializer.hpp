#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include "Graph.hpp"
#include "Exceptions.hpp"

namespace graph_engine {

    /**
     * @brief Representación interna de tipos JSON para el motor de serialización.
     */
    enum class JSONType {
        Object,
        Array,
        String,
        Number,
        Boolean,
        Null
    };

    /**
     * @brief Estructura de datos para representar un nodo JSON de forma genérica.
     * Se utiliza como paso intermedio entre el string JSON y la estructura Graph.
     */
    struct JSONValue {
        JSONType type;
        std::string s_val;
        double d_val;
        bool b_val;
        std::vector<JSONValue> a_val;
        std::map<std::string, JSONValue> o_val;

        JSONValue();
        JSONValue(JSONType t);
        JSONValue(const std::string& s);
        JSONValue(double d);
        JSONValue(bool b);

        // Métodos de acceso con validación estricta
        const JSONValue& at(const std::string& key) const {
            if (type != JSONType::Object) throw ParseException("El nodo JSON no es un objeto.");
            auto it = o_val.find(key);
            if (it == o_val.end()) throw ParseException("Clave JSON no encontrada: " + key);
            return it->second;
        }

        bool contains(const std::string& key) const {
            return type == JSONType::Object && o_val.find(key) != o_val.end();
        }
    };

    /**
     * @brief Analizador léxico y sintáctico para JSON estándar.
     * Implementado en Serializer.cpp mediante un parser descendente recursivo.
     */
    class JSONParser {
    public:
        JSONParser(const std::string& input);
        JSONValue parse();
    private:
        std::string json_str;
        size_t pos;
        size_t line;
        size_t col;
        void skipWhitespace();
        char peek() const;
        char get();
        JSONValue parseValue();
        JSONValue parseObject();
        JSONValue parseArray();
        std::string parseString();
        double parseNumber();
        JSONValue parseBoolean();
        JSONValue parseNull();
    };

    /**
     * @brief Clase estática para la serialización y deserialización de Grafos.
     * Utiliza plantillas para soportar cualquier tipo de vértice T y peso W.
     */
    class Serializer {
    public:
        /**
         * @brief Convierte un grafo en una cadena JSON estándar.
         */
        template <typename T, typename W>
        static std::string serialize(const Graph<T, W>& graph) {
            std::ostringstream oss;
            oss << "{\n";
            
            // 1. Dirección
            oss << "  \"directed\": " << (graph.isDirected() ? "true" : "false") << ",\n";

            // 2. Vértices
            oss << "  \"vertices\": [\n";
            auto vertices = graph.getVertices();
            for (size_t i = 0; i < vertices.size(); ++i) {
                oss << "    " << escapeString(valueToString(vertices[i]));
                if (i < vertices.size() - 1) oss << ",";
                oss << "\n";
            }
            oss << "  ],\n";

            // 3. Aristas
            oss << "  \"edges\": [\n";
            auto edges = graph.getAllEdges();
            for (size_t i = 0; i < edges.size(); ++i) {
                oss << "    {\n";
                oss << "      \"from\": " << escapeString(valueToString(edges[i].from)) << ",\n";
                oss << "      \"to\": " << escapeString(valueToString(edges[i].to)) << ",\n";
                oss << "      \"weight\": " << std::fixed << std::setprecision(6) << edges[i].weight << "\n";
                oss << "    }";
                if (i < edges.size() - 1) oss << ",";
                oss << "\n";
            }
            oss << "  ]\n";
            oss << "}";
            return oss.str();
        }

        /**
         * @brief Reconstruye un grafo a partir de una cadena JSON.
         * Valida estrictamente la estructura y los tipos de datos.
         */
        template <typename T, typename W>
        static Graph<T, W> deserialize(const std::string& json) {
            JSONParser parser(json);
            JSONValue root = parser.parse();

            if (root.type != JSONType::Object) {
                throw ParseException("El JSON raíz debe ser un objeto.");
            }

            if (!root.contains("directed") || !root.contains("vertices") || !root.contains("edges")) {
                throw ParseException("Faltan campos obligatorios en el JSON (directed, vertices, edges).");
            }

            const JSONValue& directedNode = root.at("directed");
            if (directedNode.type != JSONType::Boolean) {
                throw ParseException("El campo 'directed' debe ser de tipo boolean.");
            }
            bool isDirected = directedNode.b_val;
            Graph<T, W> graph(isDirected ? GraphDirection::Directed : GraphDirection::Undirected);

            const JSONValue& verticesArr = root.at("vertices");
            if (verticesArr.type != JSONType::Array) throw ParseException("El campo 'vertices' debe ser un array.");
            
            for (const auto& vNode : verticesArr.a_val) {
                graph.addVertex(stringToValue<T>(vNode.type == JSONType::String ? vNode.s_val : std::to_string(vNode.d_val)));
            }

            const JSONValue& edgesArr = root.at("edges");
            if (edgesArr.type != JSONType::Array) throw ParseException("El campo 'edges' debe ser un array.");

            for (const auto& eNode : edgesArr.a_val) {
                T from = stringToValue<T>(eNode.at("from").type == JSONType::String ? eNode.at("from").s_val : std::to_string(eNode.at("from").d_val));
                T to = stringToValue<T>(eNode.at("to").type == JSONType::String ? eNode.at("to").s_val : std::to_string(eNode.at("to").d_val));
                W weight = static_cast<W>(eNode.at("weight").d_val);
                graph.addEdge(from, to, weight);
            }
            return graph;
        }

    private:
        /**
         * @brief Escapa caracteres especiales para strings JSON.
         */
        static std::string escapeString(const std::string& s) {
            std::ostringstream oss;
            oss << "\"";
            for (char c : s) {
                switch (c) {
                    case '"':  oss << "\\\""; break;
                    case '\\': oss << "\\\\"; break;
                    case '\b': oss << "\\b";  break;
                    case '\f': oss << "\\f";  break;
                    case '\n': oss << "\\n";  break;
                    case '\r': oss << "\\r";  break;
                    case '\t': oss << "\\t";  break;
                    default:
                        if ('\x00' <= c && c <= '\x1f') {
                            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                        } else {
                            oss << c;
                        }
                }
            }
            oss << "\"";
            return oss.str();
        }

        /**
         * @brief Helper para convertir T a string (usado en serialización).
         */
        template <typename U>
        static std::string valueToString(const U& val) {
            if constexpr (std::is_same_v<U, std::string>) {
                return val;
            } else {
                std::ostringstream oss;
                oss << val;
                return oss.str();
            }
        }

        /**
         * @brief Helper para convertir string a T (usado en deserialización).
         */
        template <typename U>
        static U stringToValue(const std::string& s) {
            if constexpr (std::is_same_v<U, std::string>) {
                return s;
            } else {
                std::istringstream iss(s);
                U val;
                if (!(iss >> val)) {
                    throw ParseException("Error de conversión de tipo al deserializar valor: " + s);
                }
                return val;
            }
        }
    };

} // namespace graph_engine

#endif // SERIALIZER_HPP

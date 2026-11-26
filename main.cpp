#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Graph.hpp"
#include "Algorithms.hpp"
#include "Serializer.hpp"
#include "Exceptions.hpp"

using namespace graph_engine;

// --- Utilidades de Formateo para el Reporte ---

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << " TEST SUITE: " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void printSubHeader(const std::string& subtitle) {
    std::cout << "\n--- " << subtitle << " ---\n";
}

void report(int testNum, const std::string& description, bool passed) {
    std::cout << "[TEST " << std::setw(2) << std::setfill('0') << testNum << "] "
              << std::left << std::setw(45) << std::setfill(' ') << description
              << " -> " << (passed ? "PASSED" : "FAILED") << "\n";
    assert(passed);
}

// --- Batería de Pruebas (Exactamente 24 Tests Únicos) ---

void runGraphStructureTests(int& counter) {
    printSubHeader("Estructura de Grafo y Topología");

    // Test 1: Grafo Vacío
    {
        Graph<std::string, double> g(GraphDirection::Directed);
        report(++counter, "Grafo vacio inicializado", g.getVertices().empty() && g.getEdgeCount() == 0);
    }

    // Test 2: Inserción de Vértices y Aristas
    {
        Graph<int, int> g(GraphDirection::Directed);
        g.addVertex(1);
        g.addVertex(2);
        g.addEdge(1, 2, 10);
        report(++counter, "Insercion de vertices y aristas", g.hasVertex(1) && g.hasEdge(1, 2) && g.getWeight(1, 2) == 10 && g.getEdgeCount() == 1);
    }

    // Test 3: Grafo No Dirigido (Simetría)
    {
        Graph<char, double> g(GraphDirection::Undirected);
        g.addEdge('A', 'B', 5.5);
        report(++counter, "Simetria en grafo no dirigido", g.hasEdge('A', 'B') && g.hasEdge('B', 'A') && g.getWeight('B', 'A') == 5.5);
    }

    // Test 4: Sobrescritura de Pesos (No Multigrafo)
    {
        Graph<int, int> g(GraphDirection::Directed);
        g.addEdge(1, 2, 10);
        g.addEdge(1, 2, 20);
        report(++counter, "Actualizacion de peso (no multigrafo)", g.getWeight(1, 2) == 20 && g.getEdgeCount() == 1);
    }

    // Test 5: Detección de Ciclos (DFS)
    {
        Graph<int, int> g(GraphDirection::Directed);
        g.addEdge(1, 2, 1);
        g.addEdge(2, 3, 1);
        g.addEdge(3, 1, 1);
        report(++counter, "Deteccion de ciclo dirigido", g.hasCycle());
    }

    // Test 6: Ordenación Topológica (Kahn)
    {
        Graph<std::string, int> g(GraphDirection::Directed);
        g.addEdge("Ropa", "Zapatos", 1);
        g.addEdge("Calcetines", "Zapatos", 1);
        auto topo = g.topologicalSort();
        bool valid = !topo.empty() && topo.back() == "Zapatos";
        report(++counter, "Ordenacion Topologica (Kahn)", valid);
    }

    // Test 7: Componentes Fuertemente Conexas (Tarjan)
    {
        Graph<int, int> g(GraphDirection::Directed);
        g.addEdge(1, 2, 1); g.addEdge(2, 1, 1); // SCC 1
        g.addEdge(2, 3, 1);
        g.addEdge(3, 3, 1); // SCC 2 (self-loop)
        auto sccs = g.getSCCs();
        report(++counter, "Componentes Fuertemente Conexas (Tarjan)", sccs.size() == 2);
    }
}

void runPathfindingTests(int& counter) {
    printSubHeader("Algoritmos de Caminos Mínimos");

    // Test 8: Dijkstra Estándar
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, 1.0);
        g.addEdge(2, 3, 2.0);
        g.addEdge(1, 3, 4.0);
        auto res = Algorithms<int, double>::dijkstra(g, 1, 3);
        report(++counter, "Dijkstra: Camino mas corto", std::abs(res.distance - 3.0) < 1e-9 && res.path.size() == 3);
    }

    // Test 9: Dijkstra con Pesos Negativos (Excepción)
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, -1.0);
        bool caught = false;
        try {
            Algorithms<int, double>::dijkstra(g, 1, 2);
        } catch (const InvalidWeightException&) {
            caught = true;
        }
        report(++counter, "Dijkstra: Rechazo de pesos negativos", caught);
    }

    // Test 10: Bellman-Ford con Pesos Negativos (Válido)
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, 5.0);
        g.addEdge(2, 3, -2.0);
        auto res = Algorithms<int, double>::bellmanFord(g, 1, 3);
        report(++counter, "Bellman-Ford: Pesos negativos validos", std::abs(res.distance - 3.0) < 1e-9);
    }

    // Test 11: Bellman-Ford: Detección de Ciclo Negativo
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, 1.0);
        g.addEdge(2, 3, 1.0);
        g.addEdge(3, 1, -5.0);
        auto res = Algorithms<int, double>::bellmanFord(g, 1, 3);
        report(++counter, "Bellman-Ford: Deteccion de ciclo negativo", res.hasNegativeCycle);
    }

    // Test 12: Bellman-Ford: Reconstrucción de Ciclo Negativo
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, 1.0);
        g.addEdge(2, 3, 1.0);
        g.addEdge(3, 1, -5.0);
        auto res = Algorithms<int, double>::bellmanFord(g, 1, 3);
        report(++counter, "Bellman-Ford: Reconstruccion de ciclo", !res.negativeCycle.empty() && res.negativeCycle.front() == res.negativeCycle.back());
    }

    // Test 13: Nodos Inalcanzables
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addVertex(1); g.addVertex(2);
        auto res = Algorithms<int, double>::dijkstra(g, 1, 2);
        report(++counter, "Caminos minimos: Nodo inalcanzable", res.distance == std::numeric_limits<double>::infinity());
    }
}

void runFlowTests(int& counter) {
    printSubHeader("Flujo Máximo y Corte Mínimo");

    // Test 14: Edmonds-Karp Básico
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(0, 1, 10);
        g.addEdge(0, 2, 10);
        g.addEdge(1, 2, 2);
        g.addEdge(1, 3, 4);
        g.addEdge(1, 4, 8);
        g.addEdge(2, 4, 9);
        g.addEdge(3, 5, 10);
        g.addEdge(4, 3, 6);
        g.addEdge(4, 5, 10);
        auto res = Algorithms<int, double>::edmondsKarp(g, 0, 5);
        report(++counter, "Edmonds-Karp: Flujo maximo correcto", std::abs(res.maxFlow - 19.0) < 1e-9);
    }

    // Test 15: Edmonds-Karp: Validación de Corte Mínimo
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(0, 1, 10);
        g.addEdge(1, 2, 5);
        auto res = Algorithms<int, double>::edmondsKarp(g, 0, 2);
        bool inSource = false, inSink = false;
        for(int v : res.minCutSourceSide) if(v == 0) inSource = true;
        for(int v : res.minCutSinkSide) if(v == 2) inSink = true;
        report(++counter, "Edmonds-Karp: Particion de corte minimo", inSource && inSink && res.maxFlow == 5.0);
    }

    // Test 16: Edmonds-Karp: Excepción Source == Sink
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addVertex(1);
        bool caught = false;
        try {
            Algorithms<int, double>::edmondsKarp(g, 1, 1);
        } catch (const GraphException&) {
            caught = true;
        }
        report(++counter, "Edmonds-Karp: Error si Source == Sink", caught);
    }
}

void runSerializationTests(int& counter) {
    printSubHeader("Serialización y Parser JSON Nativo");

    // Test 17: Serialización y Deserialización Exitosa
    {
        Graph<std::string, double> g(GraphDirection::Directed);
        g.addEdge("A", "B", 1.5);
        g.addEdge("B", "C", 2.5);
        std::string json = Serializer::serialize(g);
        auto g2 = Serializer::deserialize<std::string, double>(json);
        report(++counter, "JSON: Serializacion/Deserializacion integra", g2.hasEdge("A", "B") && g2.getWeight("B", "C") == 2.5);
    }

    // Test 18: Error de Sintaxis (Falta Llave)
    {
        std::string malformed = "{\"directed\": true, \"vertices\": [], \"edges\": []";
        bool caught = false;
        try {
            Serializer::deserialize<int, double>(malformed);
        } catch (const JSONParseException&) {
            caught = true;
        }
        report(++counter, "JSON: Error de sintaxis (llave faltante)", caught);
    }

    // Test 19: Error de Campos Obligatorios
    {
        std::string missingFields = "{\"directed\": true, \"vertices\": []}";
        bool caught = false;
        try {
            Serializer::deserialize<int, double>(missingFields);
        } catch (const JSONParseException&) {
            caught = true;
        }
        report(++counter, "JSON: Validacion de campos obligatorios", caught);
    }

    // Test 20: Error de Tipos de Datos
    {
        std::string wrongType = "{\"directed\": \"yes\", \"vertices\": [], \"edges\": []}";
        bool caught = false;
        try {
            Serializer::deserialize<int, double>(wrongType);
        } catch (const JSONParseException&) {
            caught = true;
        }
        report(++counter, "JSON: Validacion de tipos de datos", caught);
    }

    // Test 21: Grafo con Vértices Aislados en JSON
    {
        std::string json = "{\"directed\": false, \"vertices\": [1, 2, 3], \"edges\": [{\"from\": 1, \"to\": 2, \"weight\": 10.0}]}";
        auto g = Serializer::deserialize<int, double>(json);
        report(++counter, "JSON: Soporte para vertices aislados", g.hasVertex(3) && !g.hasEdge(3, 1));
    }
}

void runEdgeCaseTests(int& counter) {
    printSubHeader("Casos Límite y Robustez");

    // Test 22: Auto-bucles (Self-loops)
    {
        Graph<int, int> g(GraphDirection::Directed);
        g.addEdge(1, 1, 50);
        report(++counter, "Soporte para auto-bucles", g.hasEdge(1, 1) && g.getWeight(1, 1) == 50);
    }

    // Test 23: Grafo Denso (K4 completo)
    {
        Graph<int, int> g(GraphDirection::Undirected);
        for(int i = 0; i < 4; ++i) {
            for(int j = i + 1; j < 4; ++j) {
                g.addEdge(i, j, 1);
            }
        }
        report(++counter, "Grafo denso (K4 completo)", g.getVertices().size() == 4 && g.getEdgeCount() == 6);
    }

    // Test 24: Empate de Rutas en Dijkstra
    {
        Graph<int, double> g(GraphDirection::Directed);
        g.addEdge(1, 2, 10);
        g.addEdge(1, 3, 10);
        g.addEdge(2, 4, 5);
        g.addEdge(3, 4, 5);
        auto res = Algorithms<int, double>::dijkstra(g, 1, 4);
        report(++counter, "Dijkstra: Manejo de empate de rutas", std::abs(res.distance - 15.0) < 1e-9);
    }
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    printHeader("MOTOR DE GRAFOS MODULAR C++17 - BATERIA UNICA");
    
    int testCounter = 0;

    try {
        runGraphStructureTests(testCounter);
        runPathfindingTests(testCounter);
        runFlowTests(testCounter);
        runSerializationTests(testCounter);
        runEdgeCaseTests(testCounter);

        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << " RESUMEN FINAL: " << testCounter << "/24 tests ejecutados exitosamente.\n";
        std::cout << std::string(60, '=') << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] Excepcion no capturada durante los tests: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

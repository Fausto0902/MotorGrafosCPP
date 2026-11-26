# MotorGrafosCPP

Motor modular de analisis de redes y grafos ponderados (dirigidos y no dirigidos) en C++17 sin dependencias externas.

## Caracteristicas
- Graph<T, W> generico con listas de adyacencia, ciclos (DFS), ordenacion topologica (Kahn) y Tarjan.
- Caminos minimos con Dijkstra y Bellman-Ford (deteccion y reconstruccion de ciclos negativos).
- Flujo maximo y corte minimo mediante Edmonds-Karp.
- Parser JSON 100% nativo (Serializer.hpp / Serializer.cpp) y 24 tests unitarios con assert.

## Compilacion
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp Serializer.cpp -o MotorGrafos.exe
./MotorGrafos.exe

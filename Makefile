CXX      = g++
CXXFLAGS = -std=c++20 -Wall

GRAFO_SRC         = grafo/grafo.cpp
CAMINO_SRC        = camino/camino.cpp
SOLVER_GREEDY_SRC = solverGreedy/solverGreedy.cpp

HEADERS = nodo/nodo.hpp grafo/grafo.hpp camino/camino.hpp solverGreedy/solverGreedy.hpp

all: testGrafo testCamino testSolverGreedy

testGrafo: $(GRAFO_SRC) grafo/testGrafo.cpp nodo/nodo.hpp grafo/grafo.hpp
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) grafo/testGrafo.cpp

testCamino: $(GRAFO_SRC) $(CAMINO_SRC) camino/testCamino.cpp nodo/nodo.hpp grafo/grafo.hpp camino/camino.hpp
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) camino/testCamino.cpp

testSolverGreedy: $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) solverGreedy/testSolverGreedy.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) solverGreedy/testSolverGreedy.cpp

clean:
	rm -f main testGrafo testCamino testSolverGreedy

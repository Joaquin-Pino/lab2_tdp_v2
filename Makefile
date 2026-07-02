CXX      = g++
CXXFLAGS = -std=c++20 -Wall

GRAFO_SRC         = grafo/grafo.cpp
CAMINO_SRC        = camino/camino.cpp
SOLVER_GREEDY_SRC = solverGreedy/solverGreedy.cpp
ALGORITMO_SRC     = algoritmos/algoritmo.cpp
KOPT_SRC          = kopt/kopt.cpp

HEADERS = nodo/nodo.h grafo/grafo.h camino/camino.h solverGreedy/solverGreedy.h

all: testGrafo testCamino testSolverGreedy testAlgoritmo testKopt

testGrafo: $(GRAFO_SRC) grafo/testGrafo.cpp nodo/nodo.h grafo/grafo.h
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) grafo/testGrafo.cpp

testCamino: $(GRAFO_SRC) $(CAMINO_SRC) camino/testCamino.cpp nodo/nodo.h grafo/grafo.h camino/camino.h
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) camino/testCamino.cpp

testSolverGreedy: $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) solverGreedy/testSolverGreedy.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) solverGreedy/testSolverGreedy.cpp

testAlgoritmo: $(ALGORITMO_SRC) algoritmos/testAlgoritmo.cpp algoritmos/algoritmo.h
	$(CXX) $(CXXFLAGS) -o $@ $(ALGORITMO_SRC) algoritmos/testAlgoritmo.cpp

testKopt: $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) kopt/testKopt.cpp $(HEADERS) algoritmos/algoritmo.h kopt/kopt.h
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) kopt/testKopt.cpp

clean:
	rm -f main testGrafo testCamino testSolverGreedy testAlgoritmo testKopt

CXX      = g++
CXXFLAGS = -std=c++20 -Wall

GRAFO_SRC         = grafo/grafo.cpp
CAMINO_SRC        = camino/camino.cpp
SOLVER_GREEDY_SRC = solverGreedy/solverGreedy.cpp
ALGORITMO_SRC     = algoritmos/algoritmo.cpp
KOPT_SRC          = kopt/kopt.cpp
BREAKOUT_SRC      = breakout/breakout.cpp
SCATTER_SRC       = scatter/scatter.cpp
BNB_SRC           = branchAndBound/branchAndBound.cpp

HEADERS = nodo/nodo.h grafo/grafo.h camino/camino.h solverGreedy/solverGreedy.h

SOLVERS_SRC = $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) $(BREAKOUT_SRC) $(SCATTER_SRC) $(BNB_SRC)
SOLVERS_HDR = $(HEADERS) algoritmos/algoritmo.h kopt/kopt.h breakout/breakout.h scatter/scatter.h branchAndBound/branchAndBound.h

all: main testGrafo testCamino testSolverGreedy testAlgoritmo testKopt testBreakout testScatter testBranchAndBound

main: $(SOLVERS_SRC) main.cpp $(SOLVERS_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOLVERS_SRC) main.cpp

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

testBreakout: $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) $(BREAKOUT_SRC) breakout/testBreakout.cpp $(HEADERS) algoritmos/algoritmo.h kopt/kopt.h breakout/breakout.h
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) $(BREAKOUT_SRC) breakout/testBreakout.cpp

testScatter: $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) $(SCATTER_SRC) scatter/testScatter.cpp $(HEADERS) algoritmos/algoritmo.h kopt/kopt.h scatter/scatter.h
	$(CXX) $(CXXFLAGS) -o $@ $(GRAFO_SRC) $(CAMINO_SRC) $(SOLVER_GREEDY_SRC) $(ALGORITMO_SRC) $(KOPT_SRC) $(SCATTER_SRC) scatter/testScatter.cpp

testBranchAndBound: $(SOLVERS_SRC) branchAndBound/testBranchAndBound.cpp $(SOLVERS_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOLVERS_SRC) branchAndBound/testBranchAndBound.cpp

clean:
	rm -f main testGrafo testCamino testSolverGreedy testAlgoritmo testKopt testBreakout testScatter testBranchAndBound

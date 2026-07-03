import random
import heapq
import argparse


def dijkstra(graph, start, vertices):
    distances = [float('inf')] * vertices
    distances[start] = 0
    queue = [(0, start)]
    while queue:
        dist, u = heapq.heappop(queue)
        if dist > distances[u]:
            continue
        for v, w, _ in graph[u]:
            if distances[u] + w < distances[v]:
                distances[v] = distances[u] + w
                heapq.heappush(queue, (distances[v], v))
    return distances


def generarGrafo(num_vertices, num_aristas, w_max):
    if w_max < num_vertices:
        raise ValueError(
            f"w_max ({w_max}) debe ser al menos igual a num_vertices ({num_vertices}) "
            f"para poder repartir peso positivo entre las {num_vertices - 1} aristas "
            f"de la cadena base."
        )

    graph = {i: [] for i in range(num_vertices)}
    aristas_generadas = 0

    # Cadena base 0 -> 1 -> ... -> N-1, garantiza que siempre exista un camino
    # factible. El peso de cada arista se acota por w_max // num_vertices, de
    # forma que el peso total de la cadena completa queda estrictamente por
    # debajo de w_max sin importar el resultado del sorteo (ver justificación
    # matemática en la explicación adjunta).
    w_max_cadena = w_max // num_vertices
    for i in range(num_vertices - 1):
        w = random.randint(1, w_max_cadena)
        p = random.randint(1, 1000)
        graph[i].append((i + 1, w, p))
        aristas_generadas += 1

    # --- Ajuste 1: escalar el peso de las aristas aleatorias respecto a w_max ---
    # Antes, el peso de las aristas aleatorias se sorteaba en un rango fijo
    # (1 a 1.000.000) sin relación con la escala real de la instancia. Esto
    # hacía que, para instancias con w_max pequeño o num_vertices grande, las
    # aristas aleatorias fueran sistemáticamente mucho más caras que las de la
    # cadena base, dejando a la cadena base como la única solución competitiva
    # en la práctica. Ahora el rango se deriva de w_max.
    w_rand_max = max(1, w_max // 10)

    # --- Ajuste 2: anti-correlación entre peso y beneficio en aristas aleatorias ---
    # Antes, peso y beneficio se sorteaban de forma completamente independiente,
    # así que todas las aristas aleatorias compartían, en promedio, la misma
    # relación beneficio/peso (mala, respecto a la cadena base). Esto no daba
    # a un algoritmo de optimización ningún dilema real: nunca convenía desviarse
    # de la cadena. Ahora se introduce una anti-correlación con ruido: existen
    # aristas aleatorias deliberadamente eficientes (bajo peso, alto beneficio)
    # y otras deliberadamente ineficientes (alto peso, bajo beneficio), de forma
    # que el algoritmo deba discriminar cuáles vale la pena tomar.
    while aristas_generadas < num_aristas:
        u = random.randint(0, num_vertices - 1)
        v = random.randint(0, num_vertices - 1)
        if u == v:
            continue

        base = random.random()               # variable de anti-correlación, 0..1
        ruido = random.uniform(-0.3, 0.3)     # ruido para no ser perfectamente inverso

        factor_peso = min(max(base + ruido, 0.0), 1.0)
        factor_beneficio = min(max((1.0 - base) + ruido, 0.0), 1.0)

        w = max(1, int(1 + factor_peso * (w_rand_max - 1)))
        p = max(1, int(1 + factor_beneficio * 999))

        graph[u].append((v, w, p))
        aristas_generadas += 1

    return graph


def guardarGrafo(graph, num_vertices, num_aristas, w_max, filename):
    with open(filename, 'w') as f:
        f.write(f"{num_vertices} {num_aristas} {w_max}\n")
        for u in graph:
            for v, w, p in graph[u]:
                f.write(f"{u} {v} {w} {p}\n")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Genera un grafo aleatorio con pesos y beneficios por arista."
    )
    parser.add_argument(
        "-v", "--vertices",
        type=int,
        required=True,
        help="Número de vértices del grafo."
    )
    parser.add_argument(
        "-a", "--aristas",
        type=int,
        required=True,
        help="Número de aristas del grafo."
    )
    parser.add_argument(
        "-w", "--wmax",
        type=int,
        default=1000000,
        help="Peso máximo W permitido para un camino factible (por defecto: 1000000)."
    )
    parser.add_argument(
        "-o", "--output",
        type=str,
        default="grafo.txt",
        help="Nombre del archivo de salida (por defecto: grafo.txt)."
    )
    return parser.parse_args()


def main():
    args = parse_args()

    num_vertices = args.vertices
    num_aristas = args.aristas
    w_max = args.wmax
    filename = args.output

    if num_vertices < 2:
        print("Error: el número de vértices debe ser al menos 2.")
        return

    minimo_aristas = num_vertices - 1
    if num_aristas < minimo_aristas:
        print(
            f"Error: con {num_vertices} vértices se necesitan al menos "
            f"{minimo_aristas} aristas para garantizar la cadena base 0→...→N-1."
        )
        return

    try:
        graph = generarGrafo(num_vertices, num_aristas, w_max)
    except ValueError as e:
        print(f"Error: {e}")
        return

    # verificar factibilidad con Dijkstra
    dist = dijkstra(graph, 0, num_vertices)
    if dist[num_vertices - 1] > w_max:
        print("Advertencia: el camino de menor peso excede W. El grafo no tiene solución factible.")
    else:
        print(f"Grafo factible. Peso mínimo hasta destino: {dist[num_vertices - 1]}")

    guardarGrafo(graph, num_vertices, num_aristas, w_max, filename)
    print(f"Grafo guardado en {filename}")


if __name__ == "__main__":
    main()

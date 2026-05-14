#include <stdio.h>
#include <stdlib.h>

// representa uma aresta na lista de adjacências
typedef struct Vizinho { 
    int id; // ID do vértice de destino
    struct Vizinho* prox; // ponteiro para o próximo vizinho
} Vizinho;

typedef struct Vertice {
    int id;
    int grau;
    Vizinho* lista;
    struct Vertice* prox;
} Vertice;

typedef struct Grafo {
    Vertice* vertices;
    int n_vertices;
    int n_arestas;
    int n_lacos;
    int n_multiplas;
} Grafo;

typedef struct ListaTamanhos {
    int tamanho;
    struct ListaTamanhos* prox;
} ListaTamanhos;

Grafo* criar_grafo() {
    Grafo* g = (Grafo*) malloc(sizeof(Grafo));
    g->vertices = NULL;
    g->n_vertices = 0;
    g->n_arestas = 0;
    g->n_lacos = 0;
    g->n_multiplas = 0;
    return g;
}

Vertice* buscar(Grafo* g, int id) {
    Vertice* v = g->vertices;
    while (v != NULL) {
        if (v->id == id) return v;
        v = v->prox;
    }
    return NULL;
}

Vertice* inserir_v(Grafo* g, int id) {
    Vertice* novo = (Vertice*) malloc(sizeof(Vertice));
    novo->id = id;
    novo->grau = 0;
    novo->lista = NULL;
    novo->prox = g->vertices;
    g->vertices = novo;
    g->n_vertices++;
    return novo;
}

int ja_tem_vizinho(Vertice* v, int destino) {
    Vizinho* temp = v->lista;
    while (temp != NULL) {
        if (temp->id == destino) return 1;
        temp = temp->prox;
    }
    return 0;
}

void add_vizinho(Vertice* v, int destino) {
    Vizinho* novo = (Vizinho*) malloc(sizeof(Vizinho));
    novo->id = destino;
    novo->prox = v->lista;
    v->lista = novo;
    v->grau++;
}

void carregar_dados(Grafo* g, char* arquivo) {
    FILE* f = fopen(arquivo, "r");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }
    int v1, v2;
    while (fscanf(f, "%i %i", &v1, &v2) == 2) {
        g->n_arestas++;
        if (v1 == v2) g->n_lacos++;

        Vertice* vert1 = buscar(g, v1);
        if (vert1 == NULL) vert1 = inserir_v(g, v1);

        Vertice* vert2 = buscar(g, v2);
        if (vert2 == NULL) vert2 = inserir_v(g, v2);

        if (v1 != v2 && ja_tem_vizinho(vert1, v2)) g->n_multiplas++;

        add_vizinho(vert1, v2);
        if (v1 != v2) add_vizinho(vert2, v1);
    }
    fclose(f);
}

int busca_profundidade(Vertice* v, Grafo* g, int* visitados, int* ids_mapeados) {
    int idx = -1;
    for (int i = 0; i < g->n_vertices; i++) {
        if (ids_mapeados[i] == v->id) { idx = i; break; }
    }
    if (visitados[idx]) return 0;
    visitados[idx] = 1;
    int contagem = 1;
    Vizinho* viz = v->lista;
    while (viz != NULL) {
        Vertice* v_vizinho = buscar(g, viz->id);
        int idx_viz = -1;
        for (int i = 0; i < g->n_vertices; i++) {
            if (ids_mapeados[i] == viz->id) { idx_viz = i; break; }
        }
        if (!visitados[idx_viz])
            contagem += busca_profundidade(v_vizinho, g, visitados, ids_mapeados);
        viz = viz->prox;
    }
    return contagem;
}

void analisar_componentes(Grafo* g) {
    if (g->n_vertices == 0) return;

    int* ids_mapeados = (int*) malloc(g->n_vertices * sizeof(int));
    int* visitados = (int*) calloc(g->n_vertices, sizeof(int));

    Vertice* atual = g->vertices;
    for (int i = 0; i < g->n_vertices; i++) {
        ids_mapeados[i] = atual->id;
        atual = atual->prox;
    }

    int n_componentes = 0;
    ListaTamanhos* lista_tam = NULL;

    atual = g->vertices;
    while (atual != NULL) {
        int idx_atual = -1;
        for (int i = 0; i < g->n_vertices; i++) {
            if (ids_mapeados[i] == atual->id) { idx_atual = i; break; }
        }
        if (!visitados[idx_atual]) {
            n_componentes++;
            int tam = busca_profundidade(atual, g, visitados, ids_mapeados);
            ListaTamanhos* novo_tam = (ListaTamanhos*) malloc(sizeof(ListaTamanhos));
            novo_tam->tamanho = tam;
            novo_tam->prox = lista_tam;
            lista_tam = novo_tam;
        }
        atual = atual->prox;
    }

    printf("\n--- COMPONENTES CONEXOS ---\n");
    printf("Total de componentes: %i\n", n_componentes);

    FILE* arq = fopen("componentes.txt", "w");

    ListaTamanhos* p = lista_tam;
    int comp_id = 1;
    while (p != NULL) {
        printf("Componente %i: %i vertices\n", comp_id++, p->tamanho);
        fprintf(arq, "%i\n", p->tamanho);
        ListaTamanhos* aux = p;
        p = p->prox;
        free(aux);
    }

    fclose(arq);
    free(ids_mapeados);
    free(visitados);
}

void mostrar_info(Grafo* g) {
    if (g->vertices == NULL) return;

    int min = g->vertices->grau;
    int max = g->vertices->grau;

    Vertice* atual = g->vertices;
    while (atual != NULL) {
        if (atual->grau < min) min = atual->grau;
        if (atual->grau > max) max = atual->grau;
        atual = atual->prox;
    }

    printf("--- RELATORIO DO GRAFO ---\n");
    printf("Grau Minimo: %i\n", min);
    printf("Grau Maximo: %i\n", max);

    if (g->n_lacos == 0 && g->n_multiplas == 0) {
        printf("Tipo: Grafo Simples\n");
    } else {
        printf("Tipo: Multigrafo\n");
        printf("Quantidade de lacos: %i\n", g->n_lacos);
        printf("Arestas multiplas: %i\n", g->n_multiplas);
    }
}

int main() {
    Grafo* g = criar_grafo();
    carregar_dados(g, "teste2.csv");
    mostrar_info(g);
    analisar_componentes(g);
    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>

using namespace std;

// Estrutura para os parâmetros de cada configuração da questão
struct CacheConfig {
    int id;
    int capacity;  // em words
    int blockSize; // em words
    int ways;      // 1 para direta, 2 para associativa em conjunto de 2 vias
};

// Estrutura da linha de cache (adiciona LRU para controle de substituição)
struct CacheLine {
    bool valid;
    unsigned int tag;
    int lru; // 0 para mais recente, 1 para mais antigo
    CacheLine() : valid(false), tag(0), lru(0) {}
};

// Função principal de simulação
void simulateCache(const string& filename, const CacheConfig& config) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Erro ao abrir o arquivo: " << filename << endl;
        return;
    }

    // Cálculo da quantidade de conjuntos e bits dos campos
    int numSets = config.capacity / (config.blockSize * config.ways);
    int offsetBits = log2(config.blockSize);
    int indexBits = log2(numSets);
    // Endereços possuem 30 bits de tamanho para words (offset de byte dispensado)
    int tagBits = 30 - indexBits - offsetBits; 

    // Matriz de cache: linhas x vias
    vector<vector<CacheLine>> cache(numSets, vector<CacheLine>(config.ways));

    int hits = 0;
    int misses = 0;
    unsigned int address;

    // Leitura sequencial de cada linha (endereço decimal)
    while (file >> address) {
        // Encontra o bloco do endereço, seu índice e sua tag
        unsigned int blockAddress = address / config.blockSize;
        unsigned int index = blockAddress % numSets;
        unsigned int tag = blockAddress / numSets;

        bool hit = false;
        int hitWay = -1;

        // Verificação de Acerto (Hit)
        for (int w = 0; w < config.ways; ++w) {
            if (cache[index][w].valid && cache[index][w].tag == tag) {
                hit = true;
                hitWay = w;
                break;
            }
        }

        if (hit) {
            hits++;
            // Atualiza status LRU para a cache de 2 vias
            if (config.ways == 2) {
                if (cache[index][hitWay].lru == 1) { 
                    cache[index][hitWay].lru = 0;       // Torna este o mais recente
                    cache[index][1 - hitWay].lru = 1;   // Envelhece o outro
                }
            }
        } else {
            // Falha (Miss)
            misses++;
            int replaceWay = -1;
            
            // Busca uma via vazia
            for (int w = 0; w < config.ways; ++w) {
                if (!cache[index][w].valid) {
                    replaceWay = w;
                    break;
                }
            }

            // Se cheio, aplica política de substituição (LRU)
            if (replaceWay == -1) { 
                for (int w = 0; w < config.ways; ++w) {
                    if (cache[index][w].lru == 1) {
                        replaceWay = w;
                        break;
                    }
                }
            }

            // Cache diretamente mapeada sempre sobrescreve a via 0
            if (config.ways == 1) replaceWay = 0;

            cache[index][replaceWay].valid = true;
            cache[index][replaceWay].tag = tag;

            // Atualiza estado LRU pós-substituição
            if (config.ways == 2) {
                cache[index][replaceWay].lru = 0;
                cache[index][1 - replaceWay].lru = 1;
            }
        }
    }

    double total = hits + misses;
    double hitRate = (hits / total) * 100.0;
    double missRate = (misses / total) * 100.0;

    cout << "Config " << setw(2) << config.id << " ("
         << (config.ways == 1 ? "Direta" : "2 Vias") << ") | "
         << "Cap: " << setw(3) << config.capacity << " | "
         << "Bloco: " << setw(2) << config.blockSize << " | "
         << "Campos (Tag/Index/Offset): " << setw(2) << tagBits << "/" << setw(2) << indexBits << "/" << setw(2) << offsetBits << " bits | "
         << "Hit: " << fixed << setprecision(2) << setw(5) << hitRate << "% | "
         << "Miss: " << setw(5) << missRate << "%" << endl;

    file.close();
}

int main() {
    // 14 configurações mapeadas conforme descrição do trabalho
    vector<CacheConfig> configs = {
        // Caches Diretamente Mapeadas (1-7)
        {1, 128, 16, 1}, {2, 128, 32, 1}, {3, 256, 16, 1}, {4, 256, 32, 1},
        {5, 512, 16, 1}, {6, 512, 32, 1}, {7, 512, 64, 1},
        
        // Caches Associativas em Conjunto de 2 Vias (8-14)
        {8, 128, 16, 2}, {9, 128, 32, 2}, {10, 256, 16, 2}, {11, 256, 32, 2},
        {12, 512, 16, 2}, {13, 512, 32, 2}, {14, 512, 64, 2}
    };

    vector<string> files = {"trace_address1.dat", "trace_address2.dat"};

    for (const string& file : files) {
        cout << "=========================================================================================\n";
        cout << "Simulando Arquivo: " << file << "\n";
        cout << "=========================================================================================\n";
        for (const auto& config : configs) {
            simulateCache(file, config);
        }
        cout << "\n";
    }

    return 0;
}
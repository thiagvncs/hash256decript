#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <openssl/sha.h>
#include <iomanip>

using namespace std;

// Função para criptografar com SHA-256
string criptografar_sha256(const string& texto) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, texto.c_str(), texto.length());
    SHA256_Final(hash, &sha256);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Carrega palavras de um arquivo
vector<string> carregar_palavras(const string& arquivo) {
    vector<string> palavras;
    ifstream f(arquivo);
    if (!f.is_open()) {
        cout << "Erro: O arquivo '" << arquivo << "' não foi encontrado." << endl;
        return palavras;
    }

    string linha;
    while (getline(f, linha)) {
        linha.erase(remove_if(linha.begin(), linha.end(), ::isspace), linha.end());
        if (!linha.empty()) {
            transform(linha.begin(), linha.end(), linha.begin(), ::tolower);
            palavras.push_back(linha);
        }
    }
    f.close();
    cout << palavras.size() << " palavras carregadas do arquivo '" << arquivo << "'." << endl;
    return palavras;
}

// Verifica se é possível formar uma passphrase com o tamanho total
bool verificar_possibilidade(const vector<string>& palavras, int tamanho_total) {
    set<int> comprimentos;
    for (const string& palavra : palavras) {
        comprimentos.insert(palavra.length());
    }

    vector<int> comp_vec(comprimentos.begin(), comprimentos.end());
    for (int i1 = 0; i1 < comp_vec.size(); i1++)
        for (int i2 = i1; i2 < comp_vec.size(); i2++)
            for (int i3 = i2; i3 < comp_vec.size(); i3++)
                for (int i4 = i3; i4 < comp_vec.size(); i4++)
                    for (int i5 = i4; i5 < comp_vec.size(); i5++) {
                        if (comp_vec[i1] + comp_vec[i2] + comp_vec[i3] + comp_vec[i4] + comp_vec[i5] == tamanho_total) {
                            return true;
                        }
                    }
    return false;
}

// Gera todas as permutações de 5 palavras e testa
void testar_permutacoes(vector<string>& cinco_palavras, const string& hash_alvo, string& resultado, long long& tentativas) {
    sort(cinco_palavras.begin(), cinco_palavras.end()); // Ordena para iniciar permutações
    do {
        tentativas++;
        string passphrase = cinco_palavras[0] + cinco_palavras[1] + cinco_palavras[2] + cinco_palavras[3] + cinco_palavras[4];
        string hash_candidato = criptografar_sha256(passphrase);

        if (hash_candidato == hash_alvo) {
            resultado = passphrase;
            cout << "\nPassphrase encontrada: " << passphrase << endl;
            cout << "Hash correspondente: " << hash_candidato << endl;
            return;
        }

        if (tentativas % 10000000 == 0) {
            cout << "Processando... " << tentativas << " combinações/permutações testadas." << endl;
            cout << "Última passphrase válida: " << passphrase << endl;
            cout << "Hash correspondente: " << hash_candidato << endl << endl;
        }
    } while (next_permutation(cinco_palavras.begin(), cinco_palavras.end()));
}

// Gera combinações de 5 palavras e testa todas as permutações
void gerar_combinacoes(const vector<string>& palavras, int tamanho_total, const string& hash_alvo, string& resultado) {
    long long tentativas = 0;
    map<char, vector<string>> palavras_por_letra;

    // Agrupa palavras por letra inicial
    for (const string& palavra : palavras) {
        palavras_por_letra[palavra[0]].push_back(palavra);
    }

    vector<char> letras;
    for (const auto& par : palavras_por_letra) {
        letras.push_back(par.first);
    }

    // Gera combinações de 5 letras diferentes
    for (int l1 = 0; l1 < letras.size(); l1++)
        for (int l2 = l1 + 1; l2 < letras.size(); l2++)
            for (int l3 = l2 + 1; l3 < letras.size(); l3++)
                for (int l4 = l3 + 1; l4 < letras.size(); l4++)
                    for (int l5 = l4 + 1; l5 < letras.size(); l5++) {
                        const vector<string>& grupo1 = palavras_por_letra[letras[l1]];
                        const vector<string>& grupo2 = palavras_por_letra[letras[l2]];
                        const vector<string>& grupo3 = palavras_por_letra[letras[l3]];
                        const vector<string>& grupo4 = palavras_por_letra[letras[l4]];
                        const vector<string>& grupo5 = palavras_por_letra[letras[l5]];

                        for (const string& p1 : grupo1)
                            for (const string& p2 : grupo2)
                                for (const string& p3 : grupo3)
                                    for (const string& p4 : grupo4)
                                        for (const string& p5 : grupo5) {
                                            tentativas++;
                                            vector<string> cinco_palavras = {p1, p2, p3, p4, p5};
                                            string passphrase_base = p1 + p2 + p3 + p4 + p5;

                                            if (passphrase_base.length() != tamanho_total) {
                                                continue;
                                            }

                                            // Testa todas as permutações das 5 palavras
                                            testar_permutacoes(cinco_palavras, hash_alvo, resultado, tentativas);
                                            if (!resultado.empty()) {
                                                return; // Sai se encontrou a passphrase
                                            }
                                        }
                    }
    cout << "\nNenhuma passphrase correspondente encontrada após " << tentativas << " tentativas." << endl;
}

// Função de força bruta
string forca_bruta_descriptografar(const string& hash_alvo, int tamanho_passphrase = 29) {
    vector<string> palavras = carregar_palavras("palavras.txt");
    if (palavras.empty()) {
        cout << "Nenhuma palavra carregada. Encerrando o programa." << endl;
        return "";
    }

    cout << "Total de palavras carregadas: " << palavras.size() << endl;

    if (!verificar_possibilidade(palavras, tamanho_passphrase)) {
        cout << "Erro: Não é possível formar uma passphrase com o comprimento total de " << tamanho_passphrase << " caracteres." << endl;
        return "";
    }

    cout << "Iniciando a descriptografia com força bruta..." << endl;
    cout << "Gerando combinações de 5 palavras com comprimento total de " << tamanho_passphrase << " caracteres.\n" << endl;

    string resultado;
    gerar_combinacoes(palavras, tamanho_passphrase, hash_alvo, resultado);
    return resultado;
}

int main() {
    cout << "=== CRIPTOGRAFIA/DESCRIPTOGRAFIA SHA-256 ===\n" << endl;
    cout << "Digite a opção: Criptografar/Descriptografar (C/D): ";
    string opcao;
    getline(cin, opcao);
    transform(opcao.begin(), opcao.end(), opcao.begin(), ::toupper);

    if (opcao == "C") {
        cout << "Digite sua passphrase (exatamente 29 caracteres, 5 palavras concatenadas): ";
        string texto;
        getline(cin, texto);
        if (texto.length() != 29) {
            cout << "Erro: A passphrase deve ter exatamente 29 caracteres." << endl;
            return 1;
        }
        string texto_criptografado = criptografar_sha256(texto);
        cout << "\nTexto criptografado (SHA-256): " << texto_criptografado << "\n" << endl;
    }
    else if (opcao == "D") {
        cout << "Digite o hash SHA-256 para descriptografar: ";
        string hash_alvo;
        getline(cin, hash_alvo);
        if (hash_alvo.length() != 64) {
            cout << "Erro: O hash SHA-256 deve ter exatamente 64 caracteres." << endl;
            return 1;
        }
        forca_bruta_descriptografar(hash_alvo);
    }
    else {
        cout << "Opção inválida. Digite 'C' para criptografar ou 'D' para descriptografar." << endl;
    }

    return 0;
}
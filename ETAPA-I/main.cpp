#include <iostream>
#include <fstream>
#include <string>

void simular_ULA(const std::string& instrucao, int pc, std::ofstream& log_file, int A, int B) {
    
    int F0 = instrucao[0] - '0';
    int F1 = instrucao[1] - '0';
    int ENA = instrucao[2] - '0';
    int ENB = instrucao[3] - '0';
    int INVA = instrucao[4] - '0';
    int INC = instrucao[5] - '0';

    int A_en = A & ENA;
    int B_en = B & ENB;
    
    int A_inv = A_en ^ INVA;
    
    int vem_um = INC;

    int S = 0;
    int vai_um = 0;

    if (F0 == 0 && F1 == 0) {
        
        S = A_inv & B_en;
        
    } else if (F0 == 0 && F1 == 1) {
        
        S = A_inv | B_en;
        
    } else if (F0 == 1 && F1 == 0) {
        
        S = (~B_en) & 1;
        
    } else if (F0 == 1 && F1 == 1) {
        
        S = A_inv ^ B_en ^ vem_um;
        
        vai_um = (A_inv & B_en) | (vem_um & (A_inv ^ B_en));
        
    }

    log_file << "IR: " << instrucao << ", PC: " << pc 
             << ", A: " << A << ", B: " << B 
             << ", S: " << S << ", Vai-um: " << vai_um << "\n";
}

int main() {
    std::ifstream arquivo_instrucoes("programa_etapa1.txt");
    std::ofstream arquivo_log("saida_etapa1.txt");

    if (!arquivo_instrucoes.is_open()) {
        std::cerr << "Erro ao abrir o arquivo programa_etapa1.txt" << std::endl;
        return 1;
    }

    // Para não deixar fixos, configuração via terminal para A e B
    int A, B;
    std::cout << "Digite o valor para a entrada A (0 ou 1): ";
    std::cin >> A;
    std::cout << "Digite o valor para a entrada B (0 ou 1): ";
    std::cin >> B;

    if ((A != 0 && A != 1) || (B != 0 && B != 1)) {
        std::cerr << "Erro: A e B devem ser 0 ou 1." << std::endl;
        return 1;
    }

    std::string linha;
    int pc = 1;

    while (std::getline(arquivo_instrucoes, linha)) {
        if (!linha.empty() && linha.back() == '\r') {
            linha.pop_back();
        }

        std::string IR = linha;

        bool valida = (IR.length() == 6);
        for (char c : IR) {
            if (c != '0' && c != '1') {
                valida = false;
                break;
            }
        }

        if (!valida) {
            std::cerr << "Aviso: Linha " << pc 
                    << " ignorada. Instrucao invalida: " << IR << std::endl;
            pc++;
            continue;
        }

        simular_ULA(IR, pc, arquivo_log, A, B);
        pc++;
    }

    arquivo_instrucoes.close();
    arquivo_log.close();

    std::cout << "Processamento concluido. Verifique o arquivo saida_etapa1.txt." << std::endl;

    return 0;
}
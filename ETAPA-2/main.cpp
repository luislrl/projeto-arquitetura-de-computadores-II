#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

struct Registradores {
    int32_t H   = 0;
    int32_t OPC = 0;
    int32_t TOS = 0;
    int32_t CPP = 0;
    int32_t LV  = 0;
    int32_t SP  = 0;
    int32_t PC  = 0;
    int32_t MDR = 0;
    int32_t MAR = 0;
    int8_t  MBR = 0;
} regs;

// desloca esquerda 8 bits
int32_t desloca_sll8(int32_t x) {
    uint32_t ux = static_cast<uint32_t>(x);
    ux <<= 8;
    return static_cast<int32_t>(ux);
}

// desloca direita 1 bit
int32_t desloca_sra1(int32_t x) {
    uint32_t ux  = static_cast<uint32_t>(x);
    uint32_t msb = ux & 0x80000000u;
    ux >>= 1;
    if (msb) ux |= 0x80000000u;
    return static_cast<int32_t>(ux);
}

bool instrucao_valida(const std::string& s) {
    if (s.size() != 21) return false;
    for (char c : s) {
        if (c != '0' && c != '1') return false;
    }
    return true;
}

void imprime_regs(std::ofstream& log, const Registradores& r) {
    log << "  H="   << r.H
        << "  OPC=" << r.OPC
        << "  TOS=" << r.TOS
        << "  CPP=" << r.CPP
        << "  LV="  << r.LV
        << "  SP="  << r.SP
        << "  PC="  << r.PC
        << "  MDR=" << r.MDR
        << "  MAR=" << r.MAR
        << "  MBR=" << static_cast<int>(r.MBR)
        << "\n";
}

void simular_Ciclo_Mic1(const std::string& instrucao, std::ofstream& log_file) {

    const std::string& IR = instrucao;
    Registradores antes = regs;

    // Instrucao: [0..7] ULA | [8..16] barramento C | [17..20] barramento B 
    std::string ula_ctrl = IR.substr(0,  8);
    std::string c_ctrl   = IR.substr(8,  9);
    std::string b_ctrl   = IR.substr(17, 4);

    int SLL8 = ula_ctrl[0] - '0';
    int SRA1 = ula_ctrl[1] - '0';
    int F0   = ula_ctrl[2] - '0';
    int F1   = ula_ctrl[3] - '0';
    int ENA  = ula_ctrl[4] - '0';
    int ENB  = ula_ctrl[5] - '0';
    int INVA = ula_ctrl[6] - '0';
    int INC  = ula_ctrl[7] - '0';

  
    int b_val = std::stoi(b_ctrl, nullptr, 2);
    int32_t B = 0;
    std::string reg_B_nome = "NENHUM";

    switch (b_val) {
        case 0: B = regs.MDR;  reg_B_nome = "MDR";  break;
        case 1: B = regs.PC;   reg_B_nome = "PC";   break;
        case 2:
            // extensão de sinal
            B = static_cast<int32_t>(regs.MBR);
            reg_B_nome = "MBR";
            break;
        case 3:
            // extensão com zeros 
            B = static_cast<int32_t>(static_cast<uint8_t>(regs.MBR));
            reg_B_nome = "MBRU";
            break;
        case 4: B = regs.SP;   reg_B_nome = "SP";   break;
        case 5: B = regs.LV;   reg_B_nome = "LV";   break;
        case 6: B = regs.CPP;  reg_B_nome = "CPP";  break;
        case 7: B = regs.TOS;  reg_B_nome = "TOS";  break;
        case 8: B = regs.OPC;  reg_B_nome = "OPC";  break;
        default:
            throw std::runtime_error(
                "Codigo invalido para o barramento B: " + b_ctrl +
                " (decimal=" + std::to_string(b_val) + ")"
            );
    }

    //  entrada A vem de H
    const int32_t A = regs.H;

    int32_t A_en  = ENA  ? A     : 0;
    int32_t B_en  = ENB  ? B     : 0;
    int32_t A_inv = INVA ? ~A_en : A_en;

    int32_t S = 0;

    if        (F0 == 0 && F1 == 0) {
        S = A_inv & B_en;                               
    } else if (F0 == 0 && F1 == 1) {
        S = A_inv | B_en;                               
    } else if (F0 == 1 && F1 == 0) {
        S = ~B_en;                                     
    } else {
        uint32_t uA   = static_cast<uint32_t>(A_inv);
        uint32_t uB   = static_cast<uint32_t>(B_en);
        uint64_t soma = static_cast<uint64_t>(uA)
                      + static_cast<uint64_t>(uB)
                      + static_cast<uint64_t>(INC);
        S = static_cast<int32_t>(static_cast<uint32_t>(soma));
    }

    if (SLL8 && SRA1) {
        throw std::runtime_error(
            "Instrucao invalida: SLL8 e SRA1 nao podem estar ativos simultaneamente."
        );
    }

    int32_t Sd = S;
    if        (SLL8) {
        Sd = desloca_sll8(S);   
    } else if (SRA1) {
        Sd = desloca_sra1(S);   
    }

    // escreve sd nos registradores habilitados
    std::vector<std::string> regs_C_nomes;

    if (c_ctrl[0] == '1') { regs.H   = Sd; regs_C_nomes.push_back("H");   }
    if (c_ctrl[1] == '1') { regs.OPC = Sd; regs_C_nomes.push_back("OPC"); }
    if (c_ctrl[2] == '1') { regs.TOS = Sd; regs_C_nomes.push_back("TOS"); }
    if (c_ctrl[3] == '1') { regs.CPP = Sd; regs_C_nomes.push_back("CPP"); }
    if (c_ctrl[4] == '1') { regs.LV  = Sd; regs_C_nomes.push_back("LV");  }
    if (c_ctrl[5] == '1') { regs.SP  = Sd; regs_C_nomes.push_back("SP");  }
    if (c_ctrl[6] == '1') { regs.PC  = Sd; regs_C_nomes.push_back("PC");  }
    if (c_ctrl[7] == '1') { regs.MDR = Sd; regs_C_nomes.push_back("MDR"); }
    if (c_ctrl[8] == '1') { regs.MAR = Sd; regs_C_nomes.push_back("MAR"); }

    log_file << "=========================================================\n";

    log_file << "IR: " << IR << "\n\n";

    log_file << "REGISTRADORES NO INICIO:\n";
    imprime_regs(log_file, antes);
    log_file << "\n";

    log_file << "Barramento B: " << reg_B_nome << "\n";

    log_file << "Barramento C: ";
    if (regs_C_nomes.empty()) {
        log_file << "NENHUM";
    } else {
        for (size_t i = 0; i < regs_C_nomes.size(); ++i) {
            log_file << regs_C_nomes[i];
            if (i + 1 < regs_C_nomes.size()) log_file << ", ";
        }
    }
    log_file << "\n\n";

    log_file << "REGISTRADORES NO FIM:\n";
    imprime_regs(log_file, regs);

    log_file << "=========================================================\n\n";
}

int main() {
    const std::string arq_instrucoes  = "programa_tc1.txt";
    const std::string arq_registradores = "registradores_tc1.txt";
    const std::string arq_log         = "saida_etapa2_tarefa2.txt";

    std::ifstream arquivo_instrucoes(arq_instrucoes);
    std::ifstream arquivo_registradores(arq_registradores);
    std::ofstream arquivo_log(arq_log);

    if (!arquivo_instrucoes.is_open()) {
        std::cerr << "Erro: nao foi possivel abrir '" << arq_instrucoes << "'\n";
        return 1;
    }
    if (!arquivo_log.is_open()) {
        std::cerr << "Erro: nao foi possivel criar '" << arq_log << "'\n";
        return 1;
    }

    if (arquivo_registradores.is_open()) {
        std::string nome;
        int32_t val;
        while (arquivo_registradores >> nome >> val) {
            if      (nome == "H")   regs.H   = val;
            else if (nome == "OPC") regs.OPC = val;
            else if (nome == "TOS") regs.TOS = val;
            else if (nome == "CPP") regs.CPP = val;
            else if (nome == "LV")  regs.LV  = val;
            else if (nome == "SP")  regs.SP  = val;
            else if (nome == "PC")  regs.PC  = val;
            else if (nome == "MDR") regs.MDR = val;
            else if (nome == "MAR") regs.MAR = val;
            else if (nome == "MBR") regs.MBR = static_cast<int8_t>(val);
        }
        arquivo_registradores.close();
    } else {
        std::cerr << "Aviso: '" << arq_registradores
                  << "' nao encontrado. Iniciando com zeros.\n";
    }

    arquivo_log << "=== INICIO DA EXECUCAO — ETAPA 2, TAREFA 2 ===\n\n";
    arquivo_log << "ESTADO INICIAL DOS REGISTRADORES:\n";
    imprime_regs(arquivo_log, regs);
    arquivo_log << "\n";

    std::string instrucao;
    int linha_atual = 1;

    while (std::getline(arquivo_instrucoes, instrucao)) {

        if (!instrucao.empty() && instrucao.back() == '\r') {
            instrucao.pop_back();
        }

        if (instrucao.empty()) {
            ++linha_atual;
            continue;
        }

        if (!instrucao_valida(instrucao)) {
            std::cerr << "Aviso: linha " << linha_atual
                      << " ignorada — instrucao invalida: '"
                      << instrucao << "'\n";
            ++linha_atual;
            continue;
        }

        try {
            simular_Ciclo_Mic1(instrucao, arquivo_log);
        } catch (const std::exception& e) {
            std::cerr << "Erro na linha " << linha_atual
                      << ": " << e.what() << "\n";
        }

        ++linha_atual;
    }

    arquivo_instrucoes.close();
    arquivo_log.close();

    std::cout << "Processamento concluido. Log salvo em: " << arq_log << "\n";
    return 0;
}
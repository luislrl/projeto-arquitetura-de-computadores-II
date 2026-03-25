# Emulador da ULA - Mic-1 (Etapa 1)

**Aluno:** Luís (Matrícula: 20230012897)

---

## Estrutura dos Arquivos

* `main.cpp`: Código-fonte principal em C++ contendo a lógica da ULA e a leitura de arquivos.
* `programa_etapa1.txt`: Arquivo de entrada contendo as instruções de 6 bits a serem executadas.
* `saida_etapa1.txt`: Arquivo de log gerado automaticamente pelo programa, contendo o estado do Registrador de Instrução (IR), Contador de Programa (PC), entradas A e B, saída (S) e o bit de Vai-um após cada execução.

---

## Como Compilar e Executar

**Compilação:**
No terminal, navegue até o diretório dos arquivos e compile utilizando GCC:
```bash
g++ main.cpp -o simulador_ula
```
**Execução:**
```bash
./simulador_ula
```
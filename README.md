# RAM Manager — Monitoramento e Alocação Real de Memória

Projeto desenvolvido para a disciplina de **Sistemas Operacionais**, com o objetivo de demonstrar, na prática, conceitos relacionados ao gerenciamento de memória em um sistema operacional Windows.

O sistema foi desenvolvido em **C++**, utilizando chamadas nativas do Windows para consultar informações reais da memória RAM, listar processos ativos e realizar alocações reais de memória dentro do próprio processo.

---

## Funcionalidades

* Exibição da memória RAM física total, disponível e em uso;
* Exibição do percentual de uso da RAM;
* Barra visual de uso da memória no terminal;
* Listagem de processos reais ativos no Windows;
* Exibição de PID, nome do processo, quantidade de threads e uso aproximado de RAM;
* Alocação real de blocos de memória com `VirtualAlloc`;
* Liberação de memória com `VirtualFree`;
* Comparação do uso da RAM antes e depois da alocação;
* Relatório acadêmico simples no terminal;
* Interface em console com cores e organização visual.

---

## Tecnologias Utilizadas

* **C++**
* **Windows API**
* **MinGW / g++**
* **Visual Studio Code**

---

## APIs do Windows Utilizadas

O projeto utiliza chamadas nativas do sistema operacional Windows:

* `GlobalMemoryStatusEx` — consulta informações reais da memória do sistema;
* `VirtualAlloc` — realiza alocação real de memória;
* `VirtualFree` — libera memória alocada;
* `CreateToolhelp32Snapshot` — captura os processos ativos;
* `Process32FirstW` e `Process32NextW` — percorrem a lista de processos;
* `OpenProcess` — abre processos para consulta;
* `GetProcessMemoryInfo` — obtém informações de memória dos processos;
* `SetConsoleTextAttribute` — aplica cores ao terminal.

---

## Como Compilar

No terminal do VS Code, dentro da pasta do projeto, execute:

```
g++ Main.cpp -o RAMManager.exe -lpsapi
```

Para gerar uma versão mais portátil do executável:

```
g++ Main.cpp -o RAMManager.exe -lpsapi -static -static-libgcc -static-libstdc++
```

---

## Como Executar

Após compilar, execute:

```
.\RAMManager.exe
```

---

## Conceitos de Sistemas Operacionais Aplicados
 
* Gerenciamento de memória;
* Memória física;
* Memória virtual;
* Processos;
* Threads;
* Espaço de endereçamento;
* Proteção de memória;
* Alocação e liberação de memória;
* Chamadas ao sistema operacional;
* Monitoramento de recursos.

---
## Pequenas correções

Apos feedback do professor adicionamos as variaveis que especificam o tamanho em MB dos blocos e da quantidade máxima limete de memória.
Sendo elas as variáveis: 

```
LIMITE_MB_POR_BLOCO
```
```
LIMITE_TOTAL_MB
```
---

## Desenvolvedores

Projeto acadêmico desenvolvido para a disciplina de **Sistemas Operacionais**.
ALunos: Caio H. Hort, Cauan V. de Souza, Rodrigo Amaral.

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

using namespace std;

const int MAX_BLOCOS = 50;
const int LIMITE_MB_POR_BLOCO = 512;
const int LIMITE_TOTAL_MB = 2048;

const WORD COR_PADRAO = 7;
const WORD COR_TITULO = 11;
const WORD COR_SUCESSO = 10;
const WORD COR_ALERTA = 14;
const WORD COR_ERRO = 12;
const WORD COR_DESTAQUE = 15;

struct BlocoMemoria {
    int id;
    void* endereco;
    SIZE_T tamanhoBytes;
    bool ocupado;
};

struct DadosMemoria {
    unsigned long long totalFisica;
    unsigned long long livreFisica;
    unsigned long long usadaFisica;
    unsigned long long totalVirtual;
    unsigned long long livreVirtual;
    unsigned long long usadaVirtual;
    DWORD percentualUso;
    bool valido;
};

BlocoMemoria blocos[MAX_BLOCOS];
int proximoId = 1;

int lerInteiro();
void inicializarBlocos();
void menuPrincipal();
void exibirPainelPrincipal();
void exibirMemoriaSistema();
void listarProcessos();
void alocarBlocoMemoria();
void listarBlocosAlocados();
void liberarBlocoMemoria();
void liberarTodosBlocos(bool mostrarMensagem = true);
void gerarRelatorio();
void pausar();
void limparTela();
void definirCor(WORD cor);
void resetarCor();
void exibirCabecalho();
void exibirLinha();
void exibirBarraUso(int percentual, int largura);
void exibirResumoCompactoMemoria();
void exibirComparativoAlocacao(DadosMemoria antes, DadosMemoria depois);
void exibirMensagemSucesso(string mensagem);
void exibirMensagemErro(string mensagem);
void exibirMensagemAlerta(string mensagem);

DadosMemoria obterDadosMemoria();
string converterWideParaString(const wchar_t* texto);

unsigned long long bytesParaMB(unsigned long long bytes);
double bytesParaGB(unsigned long long bytes);
int encontrarPosicaoLivre();
int calcularTotalAlocadoMB();

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    inicializarBlocos();
    menuPrincipal();
    liberarTodosBlocos(false);

    return 0;
}

void menuPrincipal() {
    bool executando = true;

    while (executando) {
        limparTela();
        exibirPainelPrincipal();

        int opcao = lerInteiro();

        switch (opcao) {
            case 1:
                limparTela();
                exibirMemoriaSistema();
                pausar();
                break;

            case 2:
                limparTela();
                listarProcessos();
                pausar();
                break;

            case 3:
                limparTela();
                alocarBlocoMemoria();
                pausar();
                break;

            case 4:
                limparTela();
                listarBlocosAlocados();
                pausar();
                break;

            case 5:
                limparTela();
                liberarBlocoMemoria();
                pausar();
                break;

            case 6:
                limparTela();
                liberarTodosBlocos(true);
                pausar();
                break;

            case 7:
                limparTela();
                gerarRelatorio();
                pausar();
                break;

            case 0:
                executando = false;
                limparTela();
                exibirMensagemAlerta("Finalizando o RAM Manager...");
                break;

            default:
                exibirMensagemErro("Opcao invalida.");
                pausar();
        }
    }
}

void exibirPainelPrincipal() {
    exibirCabecalho();
    exibirResumoCompactoMemoria();

    definirCor(COR_DESTAQUE);
    cout << "\n==================== MENU PRINCIPAL ====================\n";
    resetarCor();

    cout << "[1] Exibir memoria RAM real do sistema\n";
    cout << "[2] Listar processos reais do Windows\n";
    cout << "[3] Alocar bloco real de memoria\n";
    cout << "[4] Listar blocos alocados pelo programa\n";
    cout << "[5] Liberar bloco especifico\n";
    cout << "[6] Liberar todos os blocos\n";
    cout << "[7] Gerar relatorio academico simples\n";
    cout << "[0] Sair\n";

    definirCor(COR_DESTAQUE);
    cout << "========================================================\n";
    resetarCor();

    cout << "Escolha: ";
}

void exibirCabecalho() {
    definirCor(COR_TITULO);
    cout << "+========================================================+\n";
    cout << "|                                                        |\n";
    cout << "|              RAM MANAGER - WINDOWS C++                 |\n";
    cout << "|       Monitoramento e alocacao real de memoria         |\n";
    cout << "|                                                        |\n";
    cout << "+========================================================+\n";
    resetarCor();
}

void exibirResumoCompactoMemoria() {
    DadosMemoria dados = obterDadosMemoria();

    if (!dados.valido) {
        exibirMensagemErro("Nao foi possivel obter dados da memoria.");
        return;
    }

    cout << fixed << setprecision(2);

    definirCor(COR_DESTAQUE);
    cout << "\n------------------- STATUS DA MEMORIA -------------------\n";
    resetarCor();

    cout << left << setw(28) << "RAM fisica total:" 
         << bytesParaGB(dados.totalFisica) << " GB\n";

    cout << left << setw(28) << "RAM fisica em uso:" 
         << bytesParaGB(dados.usadaFisica) << " GB\n";

    cout << left << setw(28) << "RAM fisica disponivel:" 
         << bytesParaGB(dados.livreFisica) << " GB\n";

    cout << left << setw(28) << "Uso atual da RAM:";

    exibirBarraUso((int)dados.percentualUso, 30);

    cout << "\n";

    cout << left << setw(28) << "Alocado pelo programa:" 
         << calcularTotalAlocadoMB() << " MB\n";

    definirCor(COR_DESTAQUE);
    cout << "---------------------------------------------------------\n";
    resetarCor();
}

void inicializarBlocos() {
    for (int i = 0; i < MAX_BLOCOS; i++) {
        blocos[i].id = 0;
        blocos[i].endereco = NULL;
        blocos[i].tamanhoBytes = 0;
        blocos[i].ocupado = false;
    }
}

DadosMemoria obterDadosMemoria() {
    DadosMemoria dados;
    dados.valido = false;

    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status)) {
        return dados;
    }

    dados.totalFisica = status.ullTotalPhys;
    dados.livreFisica = status.ullAvailPhys;
    dados.usadaFisica = dados.totalFisica - dados.livreFisica;

    dados.totalVirtual = status.ullTotalVirtual;
    dados.livreVirtual = status.ullAvailVirtual;
    dados.usadaVirtual = dados.totalVirtual - dados.livreVirtual;

    dados.percentualUso = status.dwMemoryLoad;
    dados.valido = true;

    return dados;
}

void exibirMemoriaSistema() {
    exibirCabecalho();

    DadosMemoria dados = obterDadosMemoria();

    if (!dados.valido) {
        exibirMensagemErro("Erro ao consultar informacoes de memoria.");
        return;
    }

    cout << fixed << setprecision(2);

    definirCor(COR_TITULO);
    cout << "\n================ MEMORIA REAL DO SISTEMA ================\n";
    resetarCor();

    cout << left << setw(30) << "RAM fisica total:"
         << bytesParaGB(dados.totalFisica) << " GB"
         << " (" << bytesParaMB(dados.totalFisica) << " MB)\n";

    cout << left << setw(30) << "RAM fisica disponivel:"
         << bytesParaGB(dados.livreFisica) << " GB"
         << " (" << bytesParaMB(dados.livreFisica) << " MB)\n";

    cout << left << setw(30) << "RAM fisica em uso:"
         << bytesParaGB(dados.usadaFisica) << " GB"
         << " (" << bytesParaMB(dados.usadaFisica) << " MB)\n";

    cout << left << setw(30) << "Percentual de uso:";
    exibirBarraUso((int)dados.percentualUso, 35);
    cout << "\n";

    definirCor(COR_TITULO);
    cout << "\n============= MEMORIA VIRTUAL DO PROCESSO ===============\n";
    resetarCor();

    cout << left << setw(30) << "Virtual total:"
         << bytesParaMB(dados.totalVirtual) << " MB\n";

    cout << left << setw(30) << "Virtual disponivel:"
         << bytesParaMB(dados.livreVirtual) << " MB\n";

    cout << left << setw(30) << "Virtual usada aprox.:"
         << bytesParaMB(dados.usadaVirtual) << " MB\n";

    definirCor(COR_TITULO);
    cout << "\n================ ALOCACAO DO PROGRAMA ===================\n";
    resetarCor();

    cout << left << setw(30) << "Total alocado manualmente:"
         << calcularTotalAlocadoMB() << " MB\n";

    exibirLinha();
}

void listarProcessos() {
    exibirCabecalho();

    cout << "\nQuantos processos deseja listar? ";
    int limite = lerInteiro();

    if (limite <= 0) {
        exibirMensagemErro("Quantidade invalida.");
        return;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        exibirMensagemErro("Erro ao criar snapshot dos processos.");
        return;
    }

    PROCESSENTRY32W processo;
    processo.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(snapshot, &processo)) {
        exibirMensagemErro("Erro ao obter o primeiro processo.");
        CloseHandle(snapshot);
        return;
    }

    definirCor(COR_TITULO);
    cout << "\n====================== PROCESSOS ATIVOS ======================\n";
    resetarCor();

    definirCor(COR_DESTAQUE);
    cout << left << setw(10) << "PID"
         << setw(35) << "Nome"
         << setw(15) << "Threads"
         << setw(20) << "RAM aprox." << "\n";
    cout << "--------------------------------------------------------------------------\n";
    resetarCor();

    int contador = 0;

    do {
        if (contador >= limite) {
            break;
        }

        DWORD pid = processo.th32ProcessID;
        string nome = converterWideParaString(processo.szExeFile);

        if (nome.length() > 32) {
            nome = nome.substr(0, 32) + "...";
        }

        SIZE_T memoriaProcesso = 0;

        HANDLE hProcesso = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            pid
        );

        if (hProcesso != NULL) {
            PROCESS_MEMORY_COUNTERS contadorMemoria;

            if (GetProcessMemoryInfo(
                    hProcesso,
                    &contadorMemoria,
                    sizeof(contadorMemoria)
                )) {
                memoriaProcesso = contadorMemoria.WorkingSetSize;
            }

            CloseHandle(hProcesso);
        }

        cout << left << setw(10) << pid
             << setw(35) << nome
             << setw(15) << processo.cntThreads;

        if (memoriaProcesso > 0) {
            cout << setw(20) << (to_string(bytesParaMB(memoriaProcesso)) + " MB");
        } else {
            cout << setw(20) << "N/D";
        }

        cout << "\n";
        contador++;

    } while (Process32NextW(snapshot, &processo));

    CloseHandle(snapshot);

    definirCor(COR_DESTAQUE);
    cout << "--------------------------------------------------------------------------\n";
    resetarCor();

    cout << "Total listado: " << contador << "\n";
    cout << "Observacao: alguns processos protegidos do Windows podem aparecer como N/D.\n";
}

void alocarBlocoMemoria() {
    exibirCabecalho();

    DadosMemoria antes = obterDadosMemoria();

    definirCor(COR_TITULO);
    cout << "\n================== ALOCACAO REAL DE MEMORIA ==================\n";
    resetarCor();

    cout << "Limite por bloco: " << LIMITE_MB_POR_BLOCO << " MB\n";
    cout << "Limite total do experimento: " << LIMITE_TOTAL_MB << " MB\n";
    cout << "Atualmente alocado: " << calcularTotalAlocadoMB() << " MB\n";

    cout << "\nDigite o tamanho do bloco em MB: ";
    int tamanhoMB = lerInteiro();

    if (tamanhoMB <= 0) {
        exibirMensagemErro("Tamanho invalido.");
        return;
    }

    if (tamanhoMB > LIMITE_MB_POR_BLOCO) {
        exibirMensagemErro("Por seguranca, cada bloco pode ter no maximo " + to_string(LIMITE_MB_POR_BLOCO) + " MB.");
        return;
    }

    int totalAtual = calcularTotalAlocadoMB();

    if (totalAtual + tamanhoMB > LIMITE_TOTAL_MB) {
        exibirMensagemErro("Limite total de alocacao atingido.");
        cout << "Limite configurado: " << LIMITE_TOTAL_MB << " MB\n";
        cout << "Atualmente alocado: " << totalAtual << " MB\n";
        return;
    }

    int posicao = encontrarPosicaoLivre();

    if (posicao == -1) {
        exibirMensagemErro("Limite de blocos atingido. Libere algum bloco antes.");
        return;
    }

    SIZE_T tamanhoBytes = (SIZE_T)tamanhoMB * 1024 * 1024;

    definirCor(COR_ALERTA);
    cout << "\nSolicitando memoria ao Windows...\n";
    resetarCor();

    void* memoria = VirtualAlloc(
        NULL,
        tamanhoBytes,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );

    if (memoria == NULL) {
        exibirMensagemErro("Erro ao alocar memoria real.");
        cout << "Codigo do erro Windows: " << GetLastError() << "\n";
        return;
    }

    char* ponteiro = (char*)memoria;

    for (SIZE_T i = 0; i < tamanhoBytes; i += 4096) {
        ponteiro[i] = 1;
    }

    ponteiro[tamanhoBytes - 1] = 1;

    blocos[posicao].id = proximoId;
    blocos[posicao].endereco = memoria;
    blocos[posicao].tamanhoBytes = tamanhoBytes;
    blocos[posicao].ocupado = true;

    DadosMemoria depois = obterDadosMemoria();

    exibirMensagemSucesso("Bloco alocado com sucesso.");

    cout << "\nID do bloco: " << proximoId << "\n";
    cout << "Tamanho: " << tamanhoMB << " MB\n";
    cout << "Endereco virtual inicial: " << memoria << "\n";

    proximoId++;

    exibirComparativoAlocacao(antes, depois);
}

void exibirComparativoAlocacao(DadosMemoria antes, DadosMemoria depois) {
    if (!antes.valido || !depois.valido) {
        return;
    }

    definirCor(COR_TITULO);
    cout << "\n================ COMPARATIVO DA ALOCACAO ================\n";
    resetarCor();

    cout << fixed << setprecision(2);

    cout << left << setw(25) << "RAM em uso antes:"
         << bytesParaGB(antes.usadaFisica) << " GB"
         << " (" << antes.percentualUso << "%)\n";

    cout << left << setw(25) << "RAM em uso depois:"
         << bytesParaGB(depois.usadaFisica) << " GB"
         << " (" << depois.percentualUso << "%)\n";

    long long diferencaMB = (long long)bytesParaMB(depois.usadaFisica) - (long long)bytesParaMB(antes.usadaFisica);

    cout << left << setw(25) << "Diferenca observada:"
         << diferencaMB << " MB\n";

    cout << left << setw(25) << "Uso atual:";
    exibirBarraUso((int)depois.percentualUso, 35);
    cout << "\n";
}

void listarBlocosAlocados() {
    exibirCabecalho();

    definirCor(COR_TITULO);
    cout << "\n================ BLOCOS ALOCADOS PELO PROGRAMA ================\n";
    resetarCor();

    bool encontrou = false;

    definirCor(COR_DESTAQUE);
    cout << left << setw(10) << "ID"
         << setw(20) << "Tamanho"
         << setw(25) << "Endereco virtual" << "\n";

    cout << "---------------------------------------------------------------\n";
    resetarCor();

    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (blocos[i].ocupado) {
            encontrou = true;

            cout << left << setw(10) << blocos[i].id
                 << setw(20) << (to_string(bytesParaMB(blocos[i].tamanhoBytes)) + " MB")
                 << setw(25) << blocos[i].endereco << "\n";
        }
    }

    if (!encontrou) {
        exibirMensagemAlerta("Nenhum bloco alocado no momento.");
    }

    definirCor(COR_DESTAQUE);
    cout << "---------------------------------------------------------------\n";
    resetarCor();

    cout << "Total alocado: " << calcularTotalAlocadoMB() << " MB\n";
}

void liberarBlocoMemoria() {
    exibirCabecalho();
    listarBlocosAlocados();

    cout << "\nDigite o ID do bloco que deseja liberar: ";
    int id = lerInteiro();

    DadosMemoria antes = obterDadosMemoria();

    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (blocos[i].ocupado && blocos[i].id == id) {
            BOOL liberou = VirtualFree(blocos[i].endereco, 0, MEM_RELEASE);

            if (!liberou) {
                exibirMensagemErro("Erro ao liberar bloco.");
                cout << "Codigo do erro Windows: " << GetLastError() << "\n";
                return;
            }

            int liberadoMB = (int)bytesParaMB(blocos[i].tamanhoBytes);

            blocos[i].id = 0;
            blocos[i].endereco = NULL;
            blocos[i].tamanhoBytes = 0;
            blocos[i].ocupado = false;

            DadosMemoria depois = obterDadosMemoria();

            exibirMensagemSucesso("Bloco liberado com sucesso.");
            cout << "Memoria liberada: " << liberadoMB << " MB\n";

            exibirComparativoAlocacao(antes, depois);

            return;
        }
    }

    exibirMensagemErro("Bloco nao encontrado.");
}

void liberarTodosBlocos(bool mostrarMensagem) {
    int totalLiberado = calcularTotalAlocadoMB();

    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (blocos[i].ocupado) {
            VirtualFree(blocos[i].endereco, 0, MEM_RELEASE);

            blocos[i].id = 0;
            blocos[i].endereco = NULL;
            blocos[i].tamanhoBytes = 0;
            blocos[i].ocupado = false;
        }
    }

    if (mostrarMensagem) {
        if (totalLiberado > 0) {
            exibirMensagemSucesso("Todos os blocos foram liberados.");
            cout << "Total liberado: " << totalLiberado << " MB\n";
        } else {
            exibirMensagemAlerta("Nenhum bloco estava alocado.");
        }
    }
}

void gerarRelatorio() {
    exibirCabecalho();

    DadosMemoria dados = obterDadosMemoria();

    if (!dados.valido) {
        exibirMensagemErro("Erro ao gerar relatorio de memoria.");
        return;
    }

    definirCor(COR_TITULO);
    cout << "\n================ RELATORIO DO RAM MANAGER ================\n";
    resetarCor();

    cout << fixed << setprecision(2);

    cout << "\n1. Memoria fisica do sistema\n";
    cout << "RAM total:      " << bytesParaGB(dados.totalFisica) << " GB\n";
    cout << "RAM em uso:     " << bytesParaGB(dados.usadaFisica) << " GB\n";
    cout << "RAM disponivel: " << bytesParaGB(dados.livreFisica) << " GB\n";
    cout << "Uso atual:      " << dados.percentualUso << "%\n";

    cout << "\nUso visual da RAM:\n";
    exibirBarraUso((int)dados.percentualUso, 45);
    cout << "\n";

    cout << "\n2. Memoria alocada pelo programa\n";
    cout << "Total alocado manualmente: " << calcularTotalAlocadoMB() << " MB\n";

    cout << "\n3. Blocos ativos\n";

    bool encontrou = false;

    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (blocos[i].ocupado) {
            encontrou = true;
            cout << "Bloco ID " << blocos[i].id
                 << " - " << bytesParaMB(blocos[i].tamanhoBytes)
                 << " MB - endereco virtual " << blocos[i].endereco << "\n";
        }
    }

    if (!encontrou) {
        cout << "Nenhum bloco ativo no momento.\n";
    }

    cout << "\n4. Interpretacao academica\n";
    cout << "O programa consulta informacoes reais de memoria fornecidas pelo Windows.\n";
    cout << "Tambem realiza alocacoes reais dentro do proprio espaco de enderecamento.\n";
    cout << "A alocacao fisica final continua sendo controlada pelo kernel do sistema operacional.\n";
    cout << "Por seguranca, o programa nao interfere diretamente na memoria de outros processos.\n";

    definirCor(COR_TITULO);
    cout << "\n===========================================================\n";
    resetarCor();
}

void exibirBarraUso(int percentual, int largura) {
    int preenchido = (percentual * largura) / 100;

    cout << "[";

    if (percentual < 60) {
        definirCor(COR_SUCESSO);
    } else if (percentual < 85) {
        definirCor(COR_ALERTA);
    } else {
        definirCor(COR_ERRO);
    }

    for (int i = 0; i < preenchido; i++) {
        cout << "#";
    }

    resetarCor();

    for (int i = preenchido; i < largura; i++) {
        cout << "-";
    }

    cout << "] " << percentual << "%";
}

void limparTela() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole == INVALID_HANDLE_VALUE) {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}

void definirCor(WORD cor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cor);
}

void resetarCor() {
    definirCor(COR_PADRAO);
}

void exibirLinha() {
    definirCor(COR_DESTAQUE);
    cout << "============================================================\n";
    resetarCor();
}

void exibirMensagemSucesso(string mensagem) {
    definirCor(COR_SUCESSO);
    cout << "\n[OK] " << mensagem << "\n";
    resetarCor();
}

void exibirMensagemErro(string mensagem) {
    definirCor(COR_ERRO);
    cout << "\n[ERRO] " << mensagem << "\n";
    resetarCor();
}

void exibirMensagemAlerta(string mensagem) {
    definirCor(COR_ALERTA);
    cout << "\n[AVISO] " << mensagem << "\n";
    resetarCor();
}

int encontrarPosicaoLivre() {
    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (!blocos[i].ocupado) {
            return i;
        }
    }

    return -1;
}

int calcularTotalAlocadoMB() {
    unsigned long long total = 0;

    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (blocos[i].ocupado) {
            total += blocos[i].tamanhoBytes;
        }
    }

    return (int)bytesParaMB(total);
}

unsigned long long bytesParaMB(unsigned long long bytes) {
    return bytes / 1024 / 1024;
}

double bytesParaGB(unsigned long long bytes) {
    return (double)bytes / 1024.0 / 1024.0 / 1024.0;
}

int lerInteiro() {
    int valor;

    while (!(cin >> valor)) {
        cout << "Digite um numero valido: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return valor;
}

void pausar() {
    cout << "\nPressione ENTER para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

string converterWideParaString(const wchar_t* texto) {
    if (texto == NULL) {
        return "";
    }

    int tamanho = WideCharToMultiByte(
        CP_UTF8,
        0,
        texto,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    if (tamanho <= 1) {
        return "";
    }

    string resultado(tamanho - 1, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        texto,
        -1,
        &resultado[0],
        tamanho,
        NULL,
        NULL
    );

    return resultado;
}
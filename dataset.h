#ifndef DATASET_H
#define DATASET_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

struct Cirurgia
{
    size_t m_tempoPreparo;
    size_t m_tempoOperacao;
    size_t m_tempoLimpeza;
    
    Cirurgia(size_t tempoPreparo, size_t tempoOperacao, size_t tempoLimpeza) :
        m_tempoPreparo(tempoPreparo), m_tempoOperacao(tempoOperacao), m_tempoLimpeza(tempoLimpeza) {}
    
    inline size_t tempoTotal() const { return m_tempoPreparo + m_tempoOperacao + m_tempoLimpeza; }
    
    friend inline bool operator<(const Cirurgia& esq, const Cirurgia& dir)
    { return esq.tempoTotal() < dir.tempoTotal(); }
    
    friend inline bool operator==(const Cirurgia& esq, const Cirurgia& dir)
    { return esq.m_tempoPreparo == dir.m_tempoPreparo &&
             esq.m_tempoOperacao == dir.m_tempoOperacao &&
             esq.m_tempoLimpeza == dir.m_tempoLimpeza; }
};

struct Solucao
{
    std::vector<std::vector<Cirurgia>> m_matriz; ///< Contém a matriz de sala x horário agendada (08-18h)
    
    Solucao(size_t numSalas) { m_matriz.resize(numSalas); }
    
    void imprimir()
    {
        size_t numSala = 1;
        for (const auto& sala : m_matriz)
        {
            size_t makespan = 0;
            std::cout << numSala << ": ";
            for (const auto& operation : sala)
            {
                std::cout << '[' << operation.tempoTotal() << "] ";
                makespan += operation.tempoTotal();
            }
            std::cout << "-> makespan = " << makespan << '\n';
            numSala++;
        }
    }
    
    size_t makespanMaximo() const
    {
        std::vector<size_t> makespanSalas;
        makespanSalas.reserve(m_matriz.size());
        for (const auto& sala : m_matriz)
        {
            size_t makespanSala = 0;
            for (const auto& cirurgia : sala)
                makespanSala += cirurgia.tempoTotal();
            makespanSalas.push_back(makespanSala);
        }
        
        return *std::max_element(makespanSalas.begin(), makespanSalas.end());
    }
    
    // Operações de busca
    
    void doacao()
    {
        auto salaMaisOcupada = std::max_element(m_matriz.begin(), m_matriz.end());
        auto salaAleatoria = m_matriz.begin() + (rand() % m_matriz.size());
        
        salaAleatoria->push_back(salaMaisOcupada->back());
        salaMaisOcupada->pop_back();
    }
    
    void realocacaoAleatoria()
    {
        // Precisamos garantir que a sala de origem tem pelo menos uma cirurgia agendada
        size_t salaA = rand() % m_matriz.size();
        while (m_matriz.at(salaA).empty())
            salaA = rand() % m_matriz.size();
        const size_t salaB = rand() % m_matriz.size();
        const size_t indiceCirurgia = rand() % m_matriz.at(salaA).size();
        
        m_matriz.at(salaB).push_back(m_matriz.at(salaA).at(indiceCirurgia));
        m_matriz.at(salaA).erase(m_matriz.at(salaA).begin() + indiceCirurgia);
    }
    
    void trocaAleatoria()
    {
        // Precisamos garantir que a sala de origem e alvo tenham pelo menos uma cirurgia agendada
        size_t salaA = rand() % m_matriz.size();
        while (m_matriz.at(salaA).empty())
            salaA = rand() % m_matriz.size();
        size_t salaB = rand() % m_matriz.size();
        while (m_matriz.at(salaB).empty())
            salaB = rand() % m_matriz.size();
        const size_t cirurgiaA = rand() % m_matriz.at(salaA).size();
        const size_t cirurgiaB = rand() % m_matriz.at(salaB).size();
        
        std::swap(m_matriz.at(salaA).at(cirurgiaA), m_matriz.at(salaB).at(cirurgiaB));
    }
    
    friend inline bool operator<(const Solucao& esq, const Solucao& dir)
    { return esq.makespanMaximo() < dir.makespanMaximo(); }
    friend inline bool operator==(const Solucao& esq, const Solucao& dir)
    { return esq.makespanMaximo() == dir.makespanMaximo(); }
    friend inline bool operator!=(const Solucao& esq, const Solucao& dir) { return !(esq == dir); }
    
};

struct Dataset
{
    size_t m_numSalas;
    std::vector<Cirurgia> m_cirurgias;
    
    Dataset(const std::string &arquivoCSV)
    {
        std::ifstream streamArquivo(arquivoCSV);
        std::string linha;
        // Obtemos a primeira linha, que contém o número de salas
        std::getline(streamArquivo, linha);
        m_numSalas = std::stoul(linha);
        
        // Leitura das cirurgias
        while (streamArquivo.good())
        {
            std::getline(streamArquivo, linha);
            if (linha.empty())
                continue;
            size_t posPrimeiraVirgula = linha.find(',');
            size_t posSegundaVirgula = linha.find(',', posPrimeiraVirgula + 1);
            size_t tempoPreparo = std::stoul(linha.substr(0, posPrimeiraVirgula));
            
            size_t tempoOperacao = std::stoul(linha.substr(posPrimeiraVirgula + 1,
                                                           posSegundaVirgula - posPrimeiraVirgula - 1));
            size_t tempoLimpeza = std::stoul(linha.substr(posSegundaVirgula + 1));
            
            m_cirurgias.emplace_back(tempoPreparo, tempoOperacao, tempoLimpeza);
        }
        
    }
    
    Solucao GRASP(float alpha, size_t numIteracoes)
    {
        srand(time(nullptr));
        //srand(123);
        
        auto buscaLocal = [](const Solucao &solucaoBase, size_t numIteracoes) -> Solucao
        {
            Solucao atual = solucaoBase;
            
            for (size_t i = 0; i < numIteracoes; i++)
            {
                static constexpr size_t largura = 100;
                Solucao nova = atual;
                auto movimento = rand() % 16;
                if (movimento & 1)
                {
                    auto qnt = 1 + rand() % largura;
                    while (qnt > 0)
                    {
                        nova.trocaAleatoria();
                        qnt--;
                    }
                }
                if (movimento & 2)
                {
                    auto qnt = 1 + rand() % largura;
                    while (qnt > 0)
                    {
                        nova.realocacaoAleatoria();
                        qnt--;
                    }
                }
                if (movimento & 4)
                {
                    auto qnt = 1 + rand() % largura;
                    while (qnt > 0)
                    {
                        nova.doacao();
                        qnt--;
                    }
                }
                if (movimento & 8)
                {
                    auto qnt = 1 + rand() % largura;
                    while (qnt > 0)
                    {
                        nova.trocaAleatoria();
                        nova.realocacaoAleatoria();
                        qnt--;
                    }
                }
                
                std::cerr << "busca: ";
                std::cerr << " nova = " << nova.makespanMaximo();
                std::cerr << " atual = " << atual.makespanMaximo();
                std::cerr << "              \r";

                if (nova < atual)
                    atual = nova;
            }
            
            return atual;
        };
        
        auto construcaoGRASP = [&](float alpha) -> Solucao
        {
            
            Solucao construida(m_numSalas);
            
            auto listaCandidatos = m_cirurgias;
            std::vector<Cirurgia> listaRestrita;
            listaRestrita.reserve(m_cirurgias.size());
            
            while (!listaCandidatos.empty())
            {
                auto minMaxCirurgia = std::minmax_element(listaCandidatos.cbegin(), listaCandidatos.cend());
                const size_t tempoMin = minMaxCirurgia.first->tempoTotal();
                const size_t tempoMax = minMaxCirurgia.second->tempoTotal();
                // Montagem do LRC
                for (const auto& cirurgia : listaCandidatos)
                    if (cirurgia.tempoTotal() >= tempoMin + alpha * (tempoMax - tempoMin))
                        listaRestrita.push_back(cirurgia);
                
                // Se não conseguirmos adicionar nada ao LRC, diminuímos o alpha e tentamos novamente
                if (listaRestrita.empty())
                {
                    alpha -= 0.001;
                    continue;
                }
                
                
                // Inserção de um elemento do LRC
                auto elementoAleatorio = listaRestrita.begin() +
                                         (rand() % listaRestrita.size());
                
                // Remoção do elemento selecionado do LC
                auto posElementoLC = std::find(listaCandidatos.begin(), listaCandidatos.end(),
                                               *elementoAleatorio);
                listaCandidatos.erase(posElementoLC);                
                
                // Adição do elemento selecionado a uma sala aleatória
                construida.m_matriz.at(rand() % m_numSalas).push_back(*elementoAleatorio);
                
                // Limpeza do LRC
                listaRestrita.clear();
            }
            
            return construida;
        };
        
        Solucao melhor = construcaoGRASP(alpha);
        melhor.imprimir();
        for (size_t i = 0; i < numIteracoes; i++)
        {
            Solucao buscada = buscaLocal(melhor, numIteracoes);
            
            if (buscada < melhor)
                melhor = buscada;
            std::cerr << "GRASP: it. " << i;
            std::cerr << " found makespan = " << buscada.makespanMaximo();
            std::cerr << " best makespan = " << melhor.makespanMaximo() << '\n';
        }
        return melhor;
    }
};

#endif // DATASET_H

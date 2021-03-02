#include "dataset.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Erro: Nenhum arquivo de instância passado.\nUso: " << argv[0] << " arquivo.csv\n\n"
                  << "Instâncias devem ser arquivos CSV com o número de salas na primeira linha e, "
                  << "para cada linha seguinte, três valores inteiros:\n* O tempo de preparação para a "
                  << "operação\n* O tempo da operação em si\n* O tempo de limpeza da sala após a operação.\n"
                  << "Os arquivos devem ser separados por vírgulas, não devem conter uma vírgula ao final "
                  << "da linha, e não devem conter aspas.\n";
        return 0;
    }
    
    Dataset data(argv[1]);
    
    data.GRASP(0.5, 1000).imprimir();
    
    return 0;
}

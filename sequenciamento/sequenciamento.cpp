#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    std::cout << "#ARGS = " << argc << "\n";
    std::cout << "PROGRAMA = " << argv[0] << "\n";
    std::cout << "ARG1 = " << argv[1] << ", ARG2 = " << argv[2] << "\n";

    std::ifstream input(argv[1]);
    std::ofstream output(argv[2]);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Erro ao abrir arquivos.\n";
        return 1;
    }

    //
    // ...
    //

    return 0;
}

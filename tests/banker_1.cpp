#include <iostream>
#include <vector>

class BankersAlgorithm {
public:
    int numProcesses;
    int numResourceTypes;

    std::vector<int>              Available;   // [m] - free units of each resource type
    std::vector<std::vector<int>> Max;         // [n][m] - max units each process might ever need
    std::vector<std::vector<int>> Allocation;  // [n][m] - what each process currently holds
    std::vector<std::vector<int>> Need;        // [n][m] - Max - Allocation

    BankersAlgorithm(int n,
                      int m,
                      const std::vector<std::vector<int>>& maxDemand,
                      const std::vector<int>& available)
        : numProcesses(n),
          numResourceTypes(m),
          Available(available),
          Max(maxDemand),
          Allocation(n, std::vector<int>(m, 0)),   // nobody holds anything yet
          Need(n, std::vector<int>(m, 0))
    {
        // Need[i] = Max[i] - Allocation[i]; since Allocation starts at 0,
        // Need starts out equal to Max.
        for (int i = 0; i < numProcesses; ++i) {
            for (int j = 0; j < numResourceTypes; ++j) {
                Need[i][j] = Max[i][j] - Allocation[i][j];
            }
        }
    }

    void print() const {
        std::cout << "Available: ";
        for (int v : Available) std::cout << v << " ";
        std::cout << "\n";

        std::cout << "Max:\n";
        for (int i = 0; i < numProcesses; ++i) {
            std::cout << "  P" << i << ": ";
            for (int v : Max[i]) std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "Allocation:\n";
        for (int i = 0; i < numProcesses; ++i) {
            std::cout << "  P" << i << ": ";
            for (int v : Allocation[i]) std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "Need:\n";
        for (int i = 0; i < numProcesses; ++i) {
            std::cout << "  P" << i << ": ";
            for (int v : Need[i]) std::cout << v << " ";
            std::cout << "\n";
        }
    }
};

int main() {
    // Classic textbook example: 5 processes, 3 resource types (A, B, C)
    int n = 5;
    int m = 3;

    std::vector<std::vector<int>> maxDemand = {
        {7, 5, 3},   // P0
        {3, 2, 2},   // P1
        {9, 0, 2},   // P2
        {2, 2, 2},   // P3
        {4, 3, 3}    // P4
    };

    std::vector<int> available = {3, 3, 2};

    BankersAlgorithm banker(n, m, maxDemand, available);

    std::cout << "=== Initial state ===\n";
    banker.print();

    return 0;
}
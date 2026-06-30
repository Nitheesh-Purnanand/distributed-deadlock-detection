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
          Allocation(n, std::vector<int>(m, 0)),
          Need(n, std::vector<int>(m, 0))
    {
        for (int i = 0; i < numProcesses; ++i) {
            for (int j = 0; j < numResourceTypes; ++j) {
                Need[i][j] = Max[i][j] - Allocation[i][j];
            }
        }
    }

    // ---------------- Part B: Safety Algorithm ----------------
    //
    // Returns true if the CURRENT state (Available/Allocation/Need as they
    // are right now) is safe. If safe, safeSequence is filled with an
    // order processes could finish in.
    bool isSafe(std::vector<int>& safeSequence) {
        std::vector<int> Work = Available;          // working copy, simulated as resources free up
        std::vector<bool> Finish(numProcesses, false);
        safeSequence.clear();

        int finishedCount = 0;

        // Outer loop: keep scanning for a process that can finish, until
        // either everyone has finished or a full pass finds nobody who can.
        while (finishedCount < numProcesses) {

            bool foundOne = false;

            for (int i = 0; i < numProcesses; ++i) {
                if (Finish[i]) continue;   // already finished, skip

                // Can process i finish given what's currently in Work?
                // Need[i] <= Work elementwise.
                bool canFinish = true;
                for (int j = 0; j < numResourceTypes; ++j) {
                    if (Need[i][j] > Work[j]) {
                        canFinish = false;
                        break;
                    }
                }

                if (canFinish) {
                    // Simulate process i finishing: it releases everything
                    // it currently holds back into Work.
                    for (int j = 0; j < numResourceTypes; ++j) {
                        Work[j] += Allocation[i][j];
                    }
                    Finish[i] = true;
                    safeSequence.push_back(i);
                    finishedCount++;
                    foundOne = true;
                }
            }

            // If we scanned every unfinished process and none could finish,
            // we're stuck -> unsafe state. No point looping forever.
            if (!foundOne) {
                return false;
            }
        }

        return true;   // everyone finished -> safe
    }

    // ---------------- Part C: Resource Request Algorithm ----------------
    //
    // Returns true if the request is granted (and commits the change).
    // Returns false if the request is denied (Available/Allocation/Need
    // are left exactly as they were before the call).
    bool requestResources(int processId, std::vector<int> request) {
        // 1. Request must not exceed declared Need
        for (int j = 0; j < numResourceTypes; ++j) {
            if (request[j] > Need[processId][j]) {
                std::cout << "  [DENY] P" << processId
                          << " requested more than its declared Need.\n";
                return false;
            }
        }

        // 2. Request must not exceed what's currently Available
        for (int j = 0; j < numResourceTypes; ++j) {
            if (request[j] > Available[j]) {
                std::cout << "  [WAIT] P" << processId
                          << " requested more than is currently Available.\n";
                return false;
            }
        }

        // 3. Tentatively grant: apply the change
        for (int j = 0; j < numResourceTypes; ++j) {
            Available[j]          -= request[j];
            Allocation[processId][j] += request[j];
            Need[processId][j]       -= request[j];
        }

        // 4. Check safety of this new hypothetical state
        std::vector<int> safeSequence;
        bool safe = isSafe(safeSequence);

        if (safe) {
            // 5a. Confirm: keep the changes, already applied above
            std::cout << "  [GRANT] P" << processId << " request granted. Safe sequence: ";
            for (int p : safeSequence) std::cout << "P" << p << " ";
            std::cout << "\n";
            return true;
        } else {
            // 5b. Roll back: undo the tentative grant exactly
            for (int j = 0; j < numResourceTypes; ++j) {
                Available[j]          += request[j];
                Allocation[processId][j] -= request[j];
                Need[processId][j]       += request[j];
            }
            std::cout << "  [DENY] P" << processId
                      << " request would lead to an UNSAFE state. Rolled back.\n";
            return false;
        }
    }

    void print() const {
        std::cout << "Available: ";
        for (int v : Available) std::cout << v << " ";
        std::cout << "\n";

        std::cout << "Allocation / Need:\n";
        for (int i = 0; i < numProcesses; ++i) {
            std::cout << "  P" << i << "  Alloc: ";
            for (int v : Allocation[i]) std::cout << v << " ";
            std::cout << " Need: ";
            for (int v : Need[i]) std::cout << v << " ";
            std::cout << "\n";
        }
    }
};

int main() {
    // ---------------- Part D: textbook example ----------------
    //
    // 5 processes (P0-P4), 3 resource types (A, B, C)
    // Initial Available: A=3, B=3, C=2
    //
    //         Allocation    Max         Need
    //         A  B  C     A  B  C     A  B  C
    // P0      0  1  0     7  5  3     7  4  3
    // P1      2  0  0     3  2  2     1  2  2
    // P2      3  0  2     9  0  2     6  0  0
    // P3      2  1  1     2  2  2     0  1  1
    // P4      0  0  2     4  3  3     4  3  1

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

    // The constructor sets Allocation to all zeros; we need to seed it
    // with the textbook's actual initial allocation, then recompute Need
    // to match (Need = Max - Allocation).
    std::vector<std::vector<int>> initialAllocation = {
        {0, 1, 0},   // P0
        {2, 0, 0},   // P1
        {3, 0, 2},   // P2
        {2, 1, 1},   // P3
        {0, 0, 2}    // P4
    };

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            banker.Allocation[i][j] = initialAllocation[i][j];
            banker.Need[i][j]       = banker.Max[i][j] - banker.Allocation[i][j];
        }
    }

    std::cout << "=== Initial state ===\n";
    banker.print();

    // --- Test 1: is the initial state safe? ---
    std::cout << "\n=== Test 1: Safety check on initial state ===\n";
    std::vector<int> safeSequence;
    bool safe = banker.isSafe(safeSequence);

    std::cout << "Safe: " << (safe ? "YES" : "NO") << "\n";
    std::cout << "Safe sequence found: ";
    for (int p : safeSequence) std::cout << "P" << p << " ";
    std::cout << "\n";
    std::cout << "(Textbook expects something like P1 P3 P4 P0 P2 -- any valid order is fine)\n";

    // --- Test 2: P1 requests (1, 0, 2) -- should be granted ---
    std::cout << "\n=== Test 2: P1 requests (1,0,2) -- expect GRANT ===\n";
    bool result2 = banker.requestResources(1, {1, 0, 2});
    std::cout << "Result: " << (result2 ? "GRANTED" : "DENIED") << "\n";
    banker.print();

    // --- Test 3: P0 requests (7, 4, 3) -- should be denied ---
    std::cout << "\n=== Test 3: P0 requests (7,4,3) -- expect DENY ===\n";
    bool result3 = banker.requestResources(0, {7, 4, 3});
    std::cout << "Result: " << (result3 ? "GRANTED" : "DENIED") << "\n";
    std::cout << "(Note: this fails the Available check in step 2, since 7 > what's\n"
                 " currently free -- it never even reaches the Safety Algorithm in\n"
                 " step 4. Need[P0] happens to equal Max[P0] here, so this request\n"
                 " asks for everything P0 could ever need in one shot.)\n";
    banker.print();

    // --- Test 4: a request that passes Available but fails Safety ---
    // P0 requests (0, 2, 0): Need[P0] = (7,4,3) so this is within Need.
    // Available after test 2 is (2,3,0), so (0,2,0) is also within Available.
    // But granting it leaves nobody able to finish -> unsafe -> should be denied
    // specifically by the Safety Algorithm in step 4, not steps 1-2.
    std::cout << "\n=== Test 4: P0 requests (0,2,0) -- passes Need/Available checks,\n"
                 "             expect DENY from the Safety Algorithm itself ===\n";
    bool result4 = banker.requestResources(0, {0, 2, 0});
    std::cout << "Result: " << (result4 ? "GRANTED" : "DENIED") << "\n";
    banker.print();

    return 0;
}
#include <iostream>
#include <map>
#include <set>
#include <string>
using namespace std;
struct Process {
    string id;
};

struct Resource {
    string id;
};

class ResourceAllocationGraph {
public:
    // resource id -> process id (who holds this resource)
    map<string, string> allocation;

    // process id -> set of resource ids (what this process is waiting on)
    map<string, set<string>> requests;

    void addAllocationEdge(Resource r, Process p) {
        allocation[r.id] = p.id;
    }

    void removeAllocationEdge(Resource r) {
        allocation.erase(r.id);
    }

    void addRequestEdge(Process p, Resource r) {
        requests[p.id].insert(r.id);
    }

    void removeRequestEdge(Process p, Resource r) {
        requests[p.id].erase(r.id);
    }

    void print() {
        cout << "Allocation edges (Resource -> Process):\n";
        for (auto& [resourceId, processId] : allocation) {
            cout << "  " << resourceId << " -> " << processId << "\n";
        }

        cout << "Request edges (Process -> Resource):\n";
        for (auto& [processId, resourceSet] : requests) {
            for (auto& resourceId : resourceSet) {
                cout << "  " << processId << " -> " << resourceId << "\n";
            }
        }
    }
};

// Part B: derive the Wait-For Graph from a RAG.
//
// Rule: process P -> resource R it wants -> process Q that holds R
//       becomes a WFG edge P -> Q ("P is waiting on Q").
//
// Returns: adjacency list, process id -> set of process ids it waits on.
map<string, set<string>> deriveWaitForGraph(const ResourceAllocationGraph& rag) {
    map<string, set<string>> wfg;

    // 1. Look at every process P that has pending requests
    for (auto& [processId, resourceSet] : rag.requests) {

        // 2. For each resource R that P is waiting on
        for (auto& resourceId : resourceSet) {

            // 3. Safely check if R is currently held by someone
            //    (use .find(), NOT operator[], so we don't insert a bogus entry)
            auto it = rag.allocation.find(resourceId);

            // 4. If R is held by some process Q, and Q isn't P itself,
            //    record the edge P -> Q in the WFG
            if (it != rag.allocation.end()) {
                string heldBy = it->second;
                if (heldBy != processId) {
                    wfg[processId].insert(heldBy);
                }
            }
            // 5. If R isn't in allocation, R is free -> no one to wait on -> no edge
        }
    }

    return wfg;
}

void printWFG(const map<string, set<string>>& wfg) {
    cout << "Wait-For Graph (Process -> Process):\n";
    for (auto& [processId, waitsOnSet] : wfg) {
        for (auto& waitsOnId : waitsOnSet) {
            cout << "  " << processId << " -> " << waitsOnId << "\n";
        }
    }
}

int main() {
    ResourceAllocationGraph rag;

    Process p1{"P1"};
    Process p2{"P2"};
    Process p3{"P3"};

    Resource r1{"R1"};
    Resource r2{"R2"};
    Resource r3{"R3"};
    Resource r4{"R4"};

    // ---- Step 1: initial allocations ----
    rag.addAllocationEdge(r1, p1);   // P1 holds R1
    rag.addAllocationEdge(r2, p2);   // P2 holds R2
    rag.addAllocationEdge(r3, p3);   // P3 holds R3

    cout << "=== After initial allocations ===\n";
    rag.print();

    // ---- Step 2: requests ----
    rag.addRequestEdge(p1, r2);      // P1 wants R2 (held by P2)
    rag.addRequestEdge(p2, r3);      // P2 wants R3 (held by P3)
    rag.addRequestEdge(p3, r1);      // P3 wants R1 (held by P1)  -> cycle P1->P2->P3->P1
    rag.addRequestEdge(p1, r4);      // P1 also wants R4 (free, nobody holds it yet)

    cout << "\n=== After requests added ===\n";
    rag.print();

    // ---- Step 3: P1's request for R4 gets granted ----
    cout << "\nGranting R4 to P1 (remove request, add allocation)...\n";
    rag.removeRequestEdge(p1, r4);
    rag.addAllocationEdge(r4, p1);   // P1 now holds R4

    rag.print();

    // ---- Step 4: P3 releases R3, and its request for R1 is dropped ----
    cout << "\nP3 releases R3, and gives up waiting on R1...\n";
    rag.removeAllocationEdge(r3);
    rag.removeRequestEdge(p3, r1);

    rag.print();

    // ---- Step 5: P2 grabs the now-free R3 directly ----
    cout << "\nP2 grabs the now-free R3...\n";
    rag.addAllocationEdge(r3, p2);

    rag.print();

    // ---- Step 6: P1 releases everything it holds ----
    cout << "\nP1 releases R1 and R4...\n";
    rag.removeAllocationEdge(r1);
    rag.removeAllocationEdge(r4);

    cout << "\n=== Final state ===\n";
    rag.print();

    cout << "\n=== Derived WFG ===\n";
    auto wfg = deriveWaitForGraph(rag);
    printWFG(wfg);

    return 0;
}
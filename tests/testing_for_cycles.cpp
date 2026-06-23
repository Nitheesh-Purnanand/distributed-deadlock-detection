#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

// -------------------- Part A: RAG --------------------

struct Process { std::string id; };
struct Resource { std::string id; };

class ResourceAllocationGraph {
public:
    std::map<std::string, std::string>           allocation;
    std::map<std::string, std::set<std::string>> requests;

    void addAllocationEdge(Resource r, Process p)  { allocation[r.id] = p.id; }
    void removeAllocationEdge(Resource r)           { allocation.erase(r.id); }
    void addRequestEdge(Process p, Resource r)      { requests[p.id].insert(r.id); }
    void removeRequestEdge(Process p, Resource r)   { requests[p.id].erase(r.id); }
};

// -------------------- Part B: derive WFG --------------------

std::map<std::string, std::set<std::string>> deriveWaitForGraph(const ResourceAllocationGraph& rag) {
    std::map<std::string, std::set<std::string>> wfg;
    for (auto& [processId, resourceSet] : rag.requests) {
        for (auto& resourceId : resourceSet) {
            auto it = rag.allocation.find(resourceId);
            if (it != rag.allocation.end()) {
                std::string heldBy = it->second;
                if (heldBy != processId) {
                    wfg[processId].insert(heldBy);
                }
            }
        }
    }
    return wfg;
}

// -------------------- Part C: DFS cycle detection --------------------

enum NodeState { UNVISITED, VISITING, VISITED };

bool dfs(const std::string& node,
         const std::map<std::string, std::set<std::string>>& wfg,
         std::map<std::string, NodeState>& state,
         std::vector<std::string>& path,
         std::set<std::string>& cycleProcesses) {

    state[node] = VISITING;
    path.push_back(node);

    auto it = wfg.find(node);
    if (it != wfg.end()) {
        for (const std::string& neighbor : it->second) {
            if (state[neighbor] == VISITING) {
                auto startIt = std::find(path.begin(), path.end(), neighbor);
                for (auto it2 = startIt; it2 != path.end(); ++it2) {
                    cycleProcesses.insert(*it2);
                }
                return true;
            }
            if (state[neighbor] == UNVISITED) {
                if (dfs(neighbor, wfg, state, path, cycleProcesses)) return true;
            }
        }
    }

    state[node] = VISITED;
    path.pop_back();
    return false;
}

bool hasCycle(const std::map<std::string, std::set<std::string>>& wfg,
              std::set<std::string>& cycleProcesses) {
    std::map<std::string, NodeState> state;
    std::vector<std::string> path;
    for (auto& [p, _] : wfg) state[p] = UNVISITED;
    for (auto& [p, _] : wfg) {
        if (state[p] == UNVISITED) {
            if (dfs(p, wfg, state, path, cycleProcesses)) return true;
        }
    }
    return false;
}

// -------------------- Part D: test helpers --------------------

void runTest(const std::string& name,
             const ResourceAllocationGraph& rag,
             bool expectedCycle) {

    std::cout << "=== " << name << " ===\n";

    auto wfg = deriveWaitForGraph(rag);

    // print WFG edges so you can verify by eye
    std::cout << "WFG edges:\n";
    if (wfg.empty()) {
        std::cout << "  (none)\n";
    }
    for (auto& [p, neighbors] : wfg) {
        for (auto& q : neighbors) {
            std::cout << "  " << p << " -> " << q << "\n";
        }
    }

    std::set<std::string> cycleProcesses;
    bool detected = hasCycle(wfg, cycleProcesses);

    std::cout << "Expected deadlock : " << (expectedCycle ? "YES" : "NO") << "\n";
    std::cout << "Detected deadlock : " << (detected      ? "YES" : "NO") << "\n";

    if (detected) {
        std::cout << "Cycle processes   : ";
        for (auto& p : cycleProcesses) std::cout << p << " ";
        std::cout << "\n";
    }

    bool pass = (detected == expectedCycle);
    std::cout << "Result            : " << (pass ? "PASS" : "FAIL") << "\n";
    std::cout << "\n";
}

// -------------------- Part D: the three scenarios --------------------

void testNoCycle() {
    // Scenario 1: P1 wants R1, but R1 is free (nobody holds it).
    // deriveWaitForGraph skips free resources -> WFG is empty -> no cycle.
    ResourceAllocationGraph rag;

    Process p1{"P1"};
    Resource r1{"R1"};

    // R1 is NOT allocated to anyone — just a pending request into thin air
    rag.addRequestEdge(p1, r1);

    runTest("Scenario 1: no cycle (P1 wants free R1)", rag, false);
}

void testDirectCycle() {
    // Scenario 2: classic 2-process deadlock.
    // P1 holds R1 and wants R2.
    // P2 holds R2 and wants R1.
    // WFG: P1 -> P2, P2 -> P1  (direct cycle)
    ResourceAllocationGraph rag;

    Process p1{"P1"}, p2{"P2"};
    Resource r1{"R1"}, r2{"R2"};

    rag.addAllocationEdge(r1, p1);   // P1 holds R1
    rag.addAllocationEdge(r2, p2);   // P2 holds R2

    rag.addRequestEdge(p1, r2);      // P1 wants R2 (held by P2) -> WFG edge P1->P2
    rag.addRequestEdge(p2, r1);      // P2 wants R1 (held by P1) -> WFG edge P2->P1

    runTest("Scenario 2: direct 2-cycle (P1 <-> P2)", rag, true);
}

void testIndirectCycle() {
    // Scenario 3: 3-process indirect deadlock.
    // P1 holds R1 and wants R2.
    // P2 holds R2 and wants R3.
    // P3 holds R3 and wants R1.
    // WFG: P1->P2->P3->P1  (cycle of length 3)
    ResourceAllocationGraph rag;

    Process p1{"P1"}, p2{"P2"}, p3{"P3"};
    Resource r1{"R1"}, r2{"R2"}, r3{"R3"};

    rag.addAllocationEdge(r1, p1);
    rag.addAllocationEdge(r2, p2);
    rag.addAllocationEdge(r3, p3);

    rag.addRequestEdge(p1, r2);      // P1->P2 in WFG
    rag.addRequestEdge(p2, r3);      // P2->P3 in WFG
    rag.addRequestEdge(p3, r1);      // P3->P1 in WFG (closes the cycle)

    runTest("Scenario 3: indirect 3-cycle (P1->P2->P3->P1)", rag, true);
}

int main() {
    testNoCycle();
    testDirectCycle();
    testIndirectCycle();
    return 0;
}
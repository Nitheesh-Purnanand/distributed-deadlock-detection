#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct Process {
    std::string id;
};

struct Resource {
    std::string id;
};

class ResourceAllocationGraph {
public:
    // resource id -> process id (who holds this resource)
    std::map<std::string, std::string> allocation;

    // process id -> set of resource ids (what this process is waiting on)
    std::map<std::string, std::set<std::string>> requests;

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
        std::cout << "Allocation edges (Resource -> Process):\n";
        for (auto& [resourceId, processId] : allocation) {
            std::cout << "  " << resourceId << " -> " << processId << "\n";
        }

        std::cout << "Request edges (Process -> Resource):\n";
        for (auto& [processId, resourceSet] : requests) {
            for (auto& resourceId : resourceSet) {
                std::cout << "  " << processId << " -> " << resourceId << "\n";
            }
        }
    }
};

// ---------------- Part B: derive the Wait-For Graph ----------------

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

void printWFG(const std::map<std::string, std::set<std::string>>& wfg) {
    std::cout << "Wait-For Graph (Process -> Process):\n";
    for (auto& [processId, waitsOnSet] : wfg) {
        for (auto& waitsOnId : waitsOnSet) {
            std::cout << "  " << processId << " -> " << waitsOnId << "\n";
        }
    }
}

// ---------------- Part C: DFS cycle detection ----------------

enum NodeState { UNVISITED, VISITING, VISITED };

// Recursive DFS helper.
// - node: current process being explored
// - wfg: the wait-for graph (adjacency list)
// - state: tracks UNVISITED / VISITING / VISITED per process
// - path: the current DFS path (stack), used to extract the cycle if found
// - cycleProcesses: filled in with the processes that make up the cycle, if any
//
// Returns true the moment a cycle is found anywhere below `node`.
bool dfs(const std::string& node,
         const std::map<std::string, std::set<std::string>>& wfg,
         std::map<std::string, NodeState>& state,
         std::vector<std::string>& path,
         std::set<std::string>& cycleProcesses) {

    state[node] = VISITING;
    path.push_back(node);

    // look up this node's neighbors (processes it waits on); if it has
    // none, .find() returns end() and we just skip the inner loop
    auto it = wfg.find(node);
    if (it != wfg.end()) {
        for (const std::string& neighbor : it->second) {

            if (state[neighbor] == VISITING) {
                // Found a cycle: neighbor is an ancestor still on the
                // current path. Find where `neighbor` first occurs in
                // `path`, then take everything from there to the end --
                // that's exactly the cycle (neighbor ... node).
                auto startIt = std::find(path.begin(), path.end(), neighbor);
                for (auto it2 = startIt; it2 != path.end(); ++it2) {
                    cycleProcesses.insert(*it2);
                }
                return true;
            }

            if (state[neighbor] == UNVISITED) {
                if (dfs(neighbor, wfg, state, path, cycleProcesses)) {
                    return true;
                }
            }

            // state[neighbor] == VISITED -> already fully explored on some
            // other path, confirmed cycle-free from here, nothing to do
        }
    }

    // done exploring this node with no cycle found below it
    state[node] = VISITED;
    path.pop_back();
    return false;
}

// Runs DFS over every process in the WFG. Returns true if a cycle exists
// anywhere in the graph. If true, cycleProcesses contains the processes
// that form (one of) the cycle(s).
bool hasCycle(const std::map<std::string, std::set<std::string>>& wfg,
              std::set<std::string>& cycleProcesses) {

    std::map<std::string, NodeState> state;
    std::vector<std::string> path;

    // initialize every node that appears as a key to UNVISITED
    // (nodes that only appear as values, e.g. a process that's waited
    // on but never itself waits, get picked up via .find() defaulting
    // safely below)
    for (auto& [processId, _] : wfg) {
        state[processId] = UNVISITED;
    }

    for (auto& [processId, _] : wfg) {
        if (state[processId] == UNVISITED) {
            if (dfs(processId, wfg, state, path, cycleProcesses)) {
                return true;
            }
        }
    }

    return false;
}

// ---------------- Demo ----------------

int main() {
    ResourceAllocationGraph rag;

    Process p1{"P1"};
    Process p2{"P2"};
    Process p3{"P3"};
    Process p4{"P4"};

    Resource r1{"R1"};
    Resource r2{"R2"};
    Resource r3{"R3"};
    Resource r4{"R4"};

    // P1 holds R1, P2 holds R2, P3 holds R3, P4 holds R4
    rag.addAllocationEdge(r1, p1);
    rag.addAllocationEdge(r2, p2);
    rag.addAllocationEdge(r3, p3);
    rag.addAllocationEdge(r4, p4);

    // P1 wants R2 (held by P2)
    rag.addRequestEdge(p1, r2);
    // P2 wants R3 (held by P3)
    rag.addRequestEdge(p2, r3);
    // P3 wants R1 (held by P1)  -> cycle: P1 -> P2 -> P3 -> P1
    rag.addRequestEdge(p3, r1);
    // P4 wants R1 too, but is NOT part of the cycle, just stuck behind it
    rag.addRequestEdge(p4, r1);

    std::cout << "=== RAG ===\n";
    rag.print();

    auto wfg = deriveWaitForGraph(rag);
    std::cout << "\n=== Derived WFG ===\n";
    printWFG(wfg);

    std::set<std::string> cycleProcesses;
    bool deadlocked = hasCycle(wfg, cycleProcesses);

    std::cout << "\n=== Cycle detection result ===\n";
    std::cout << "Deadlock detected: " << (deadlocked ? "YES" : "NO") << "\n";

    if (deadlocked) {
        std::cout << "Processes involved in the cycle: ";
        for (auto& p : cycleProcesses) {
            std::cout << p << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
#ifndef JANUS_WORKLIST
#define JANUS_WORKLIST

#include "cubedepth.hpp"
#include "cubeindex.hpp"

#include <deque>
#include <mutex>
#include <vector>

namespace Janus {
typedef std::vector<uint8_t> Solution;

struct WorkItem {
  CubeIndex cubeIndex;
  CubeDepth cubeDepth;
  Solution work;
  uint8_t depth;
};

class WorkList {
public:
  void clear() { queue.clear(); }

  void push(const WorkItem &workItem) { queue.push_back(workItem); }

  bool pop(WorkItem &workItem) {
    std::lock_guard<std::mutex> lock(workMutex);
    if (!queue.empty()) {
      workItem = queue.front();
      queue.pop_front();
      return true;
    }
    return false;
  }

private:
  std::deque<WorkItem> queue;
  std::mutex workMutex;
};

} // namespace Janus
#endif

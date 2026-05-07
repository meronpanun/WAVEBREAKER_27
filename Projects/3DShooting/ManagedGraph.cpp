#include "ManagedGraph.h"

ManagedGraph::ManagedGraph(const std::string& path)
{
    Load(path);
}

void ManagedGraph::Load(const std::string& path)
{
    m_path = path;
    int newHandle = LoadGraph(path.c_str());
    m_handle.Reset(newHandle);
}

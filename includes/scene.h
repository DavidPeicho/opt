#pragma once

#include <vector>

namespace albedo
{

namespace scene
{

class Instance {

  public:
    using Size = size_t;

  public:

    Instance() noexcept
      : m_ID(++globalId)
    { }

    bool
    operator==(Instance e) const { return e.m_ID == m_ID; }

    bool
    operator!=(Instance e) const { return e.m_ID != m_ID; }

    bool
    operator<(Instance e) const { return e.m_ID < m_ID; }

    // an id that can be used for debugging/printing
    inline Size
    getId() const noexcept { return m_ID; }

  private:
    static std::atomic<Size> globalId{0};
    Size m_ID = 0;
};

class Scene
{
  public:
    Scene();

  public:

    Instance
    createInstance();

    void
    deleteInstance();

  private:
    std::unordered_map<Instance, Instance::Size> m_EntityToIndex;

};

} // nanespace scene

} // nanespace albedo


namespace albedo
{

template <class Instance, class DataType>
Instance
ComponentArray<Instance, DataType>::createComponent(Entity entity, T&& data)
{
  auto instanceIt = m_entityToIndex.find(entity);
  if (instanceIt != m_entityToIndex.end())
  {
    removeComponent(entity);
  }
  auto index = m_instances.size();
  m_instances.push_back(instance);
  m_data.emplace_back(std::move(data));
  m_entityToIndex[instance] = index;
}

template <class Instance, class DataType>
void
ComponentArray<Instance, DataType>::removeComponent(Instance instance)
{
  // TODO: either make this function thread safe, or ask the user to make a
  // thread safe call.

  auto instanceIt = m_entityToIndex.find(instance);
  if (instanceIt == m_entityToIndex.end()) { return; }

  // Index at which the data of this instance is located in every data buffer:
  // transforms, etc...
  auto dataIndex = instanceIt->second;

  // Instance associated to the last data entry. This will be moved at the
  // position of the deleted instance.
  auto lastIndex = m_instances.size() - 1;
  auto lastInstance = m_instances[lastIndex];

  // Move the last element of every array at the position pointed by the
  // instance we are currently removing.
  m_instances[dataIndex] = std::move(m_instances[lastIndex]);
  m_data[dataIndex] = std::move(m_data[lastIndex]);
  m_data.pop_back();
  m_instances.pop_back();

  // Erase the target instance from the index map, and update the index of
  // the previous last element.
  m_entityToIndex.erase(instanceIt);
  if (instance != lastInstance) { m_entityToIndex[lastInstance] = dataIndex; }
}

template <class Instance, class DataType>
bool
ComponentArray<Instance, DataType>::hasComponent(const Entity& entity)
{
  return m_entityToIndex.find(entity) != m_entityToIndex.end();
}

template <class Instance, class DataType>
std::optional<Instance>
ComponentArray<Instance, DataType>::getComponent(const Entity& entity)
{
  auto instanceIt = m_entityToIndex.find(entity);
  if (instanceIt != m_entityToIndex.end())
  {
    return *instanceIt;
  }
  return std::nullopt;
}

template <class Instance, class DataType>
DataType&
ComponentArray<Instance, DataType>::data(Instance instance)
{
  return m_data[instance];
}

template <class Instance, class DataType>
const DataType&
ComponentArray<Instance, DataType>::data(Instance) const
{
  return m_data[instance];
}

}

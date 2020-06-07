namespace albedo
{

template <class DataType>
ComponentId
ComponentArray<DataType>::createComponent(
  const Entity& entity, DataType&& data
)
{
  if (hasComponent(entity))
  {
    // TODO: instance lookep up twice, once in `hasComponent` and once in
    // `removeComponent`.
    removeComponent(entity);
  }
  ComponentId index(m_entities.size());
  m_entities.push_back(entity);
  m_data.emplace_back(std::move(data));
  m_entityToIndex[entity] = index;
  return index;
}

template <class DataType>
void
ComponentArray<DataType>::removeComponent(const Entity& entity)
{
  // TODO: either make this function thread safe, or ask the user to make a
  // thread safe call.

  auto indexIt = m_entityToIndex.find(entity);
  if (indexIt == m_entityToIndex.end())
  {
    return;
  }

  auto index = indexIt->second;

  // Instance associated to the last data entry. This will be moved at the
  // position of the deleted instance.
  auto lastIndex = m_entities.size() - 1;
  Entity lastEntity = m_entities[lastIndex];

  // Move the last element of every array at the position pointed by the
  // instance we are currently removing.
  m_entities[index] = std::move(lastEntity);
  m_data[index] = std::move(m_data[lastIndex]);
  m_data.pop_back();
  m_entities.pop_back();

  // Erase the target instance from the index map, and update the index of
  // the previous last element.
  m_entityToIndex.erase(indexIt);
  if (index != lastIndex) { m_entityToIndex[lastEntity] = index; }
}

template <class DataType>
bool
ComponentArray<DataType>::hasComponent(const Entity& entity)
{
  return m_entityToIndex.find(entity) != m_entityToIndex.end();
}

template <class DataType>
OptionalRef<DataType>
ComponentArray<DataType>::getComponentData(const Entity& entity)
{
  auto instance = getComponent(entity);
  if (instance)
  {
    return m_data[*instance];
  }
  return std::nullopt;
}

template <class DataType>
OptionalRef<const DataType>
ComponentArray<DataType>::getComponentData(const Entity& entity) const
{
  auto instance = getComponent(entity);
  if (instance)
  {
    return m_data[*instance];
  }
  return std::nullopt;
}

template <class DataType>
std::optional<typename DataType::Id>
ComponentArray<DataType>::getComponent(const Entity& entity) const
{
  auto instanceIt = m_entityToIndex.find(entity);
  if (instanceIt != m_entityToIndex.end())
  {
    return instanceIt->second;
  }
  return std::nullopt;
}

}

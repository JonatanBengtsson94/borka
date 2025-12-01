#include "borka_ecs.h"
#include "borka_log.h"

BrEntity create_entity(BrRegistry *registry) {
  BrEntity entity = registry->count++;
  if (entity > MAX_ENTITES) {
    BR_LOG_ERROR("Too many entities in registry");
    return UINT32_MAX;
  }
  registry->masks[entity] = COMPONENT_NONE;
  return entity;
}

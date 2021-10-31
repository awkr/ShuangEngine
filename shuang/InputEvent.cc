#include "InputEvent.h"

#include <map>

const char *toString(const KeyAction action) {
  static std::map<KeyAction, const char *> mapping{
      {KeyAction::NONE, "NONE"},
      {KeyAction::DOWN, "DOWN"},
      {KeyAction::UP, "UP"},
  };
  return mapping.find(action)->second;
}
